#include "WINAPISNIFFER.h"
#include "DetourDeviceIoControl.h"
#include "DetourFileApi.h"
#include "DetourSetupApi.h"


using convert_t = std::codecvt_utf8<wchar_t>;
std::wstring_convert<convert_t, wchar_t> strconverter;
std::once_flag g_init;
std::string g_dllDir;

struct ReadFileExDetourParams
{
	HANDLE hFile;
	LPVOID lpBuffer;
	DWORD  nNumberOfBytesToRead;
	LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine;
};

static decltype(ReadFileEx)* real_ReadFileEx = ReadFileEx;
static decltype(CloseHandle)* real_CloseHandle = CloseHandle;
static decltype(GetOverlappedResult)* real_GetOverlappedResult = GetOverlappedResult;

std::list<std::string> g_driveStrings;
std::map<HANDLE, std::string> g_handleToPath;
static std::map<LPOVERLAPPED, ReadFileExDetourParams> g_overlappedToRoutine;
std::map<DWORD, std::string> g_ioctlMap;
std::map<DWORD, bool> g_newIoctls;




void CALLBACK ReadFileExCallback(
	DWORD dwErrorCode,
	DWORD dwNumberOfBytesTransfered,
	LPOVERLAPPED lpOverlapped
)
{
	const std::shared_ptr<spdlog::logger> _logger = spdlog::get("WinApiSniffer")->clone("ReadFileExCallback");

	const auto completionParams = g_overlappedToRoutine[lpOverlapped];
	const auto hFile = completionParams.hFile;
	const auto buffer = completionParams.lpBuffer;
	const auto bufferSize = completionParams.nNumberOfBytesToRead;
	const auto completionRoutine = completionParams.lpCompletionRoutine;
	g_overlappedToRoutine.erase(lpOverlapped);

	std::string path = "Unknown";
	if (g_handleToPath.count(hFile))
	{
		path = g_handleToPath[hFile];
	}
#ifndef WINAPISNIFFER_LOG_UNKNOWN_HANDLES
	else
	{
		// Ignore unknown handles
		if (completionRoutine)
			completionRoutine(dwErrorCode, dwNumberOfBytesTransfered, lpOverlapped);
		return;
	}
#endif

	const PUCHAR charInBuf = PUCHAR(buffer);
	const auto bufSize = std::min(bufferSize, dwNumberOfBytesTransfered);
	const std::vector<char> outBuffer(charInBuf, charInBuf + bufSize);

	_logger->info("result = 0x{:08X}, path = {} ({:04d}) -> {:Xpn}",
		dwErrorCode,
		path,
		bufSize,
		spdlog::to_hex(outBuffer)
	);

	completionRoutine(dwErrorCode, dwNumberOfBytesTransfered, lpOverlapped);
}

//
// Hooks ReadFileEx() API
// 
BOOL WINAPI DetourReadFileEx(
	HANDLE       hFile,
	LPVOID       lpBuffer,
	DWORD        nNumberOfBytesToRead,
	LPOVERLAPPED lpOverlapped,
	LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
)
{
	const std::shared_ptr<spdlog::logger> _logger = spdlog::get("WinApiSniffer")->clone("ReadFileEx");

	std::string path = "Unknown";
	if (g_handleToPath.count(hFile))
	{
		path = g_handleToPath[hFile];
	}
#ifndef WINAPISNIFFER_LOG_UNKNOWN_HANDLES
	else
	{
		// Ignore unknown handles
		return real_ReadFileEx(hFile, lpBuffer, nNumberOfBytesToRead, lpOverlapped, lpCompletionRoutine);
	}
#endif

	if (g_overlappedToRoutine.count(lpOverlapped))
	{
		_logger->warn("Same OVERLAPPED used multiple times, passing through directly to function");
		return real_ReadFileEx(hFile, lpBuffer, nNumberOfBytesToRead, lpOverlapped, lpCompletionRoutine);
	}
	g_overlappedToRoutine[lpOverlapped] = { hFile, lpBuffer, nNumberOfBytesToRead, lpCompletionRoutine };

	const auto ret = real_ReadFileEx(hFile, lpBuffer, nNumberOfBytesToRead, lpOverlapped, ReadFileExCallback);
	const auto error = GetLastError();

	_logger->info("success = {}, lastError = 0x{:08X}, path = {}, bufferSize = {}",
		ret ? "true" : "false",
		error,
		path,
		nNumberOfBytesToRead
	);

	return ret;
}

BOOL WINAPI DetourGetOverlappedResult(
	HANDLE       hFile,
	LPOVERLAPPED lpOverlapped,
	LPDWORD      lpNumberOfBytesTransferred,
	BOOL         bWait
)
{
	const std::shared_ptr<spdlog::logger> _logger = spdlog::get("WinApiSniffer")->clone("GetOverlappedResult");
	DWORD tmpBytesTransferred;

	const auto ret = real_GetOverlappedResult(hFile, lpOverlapped, &tmpBytesTransferred, bWait);
	const auto error = GetLastError();

	if (lpNumberOfBytesTransferred)
		*lpNumberOfBytesTransferred = tmpBytesTransferred;
	
	std::string path = "Unknown";
	if (g_handleToPath.count(hFile))
	{
		path = g_handleToPath[hFile];
	}
#ifndef WINAPISNIFFER_LOG_UNKNOWN_HANDLES
	else
	{
		// Ignore unknown handles
		return ret;
	}
#endif

	_logger->info("success = {}, lastError = 0x{:08X}, bytesTransferred = {}, path = {}",
		ret ? "true" : "false",
		ret ? ERROR_SUCCESS : error,
		tmpBytesTransferred,
		path
	);
	
	return ret;
}




EXTERN_C IMAGE_DOS_HEADER __ImageBase;

BOOL WINAPI DllMain(HINSTANCE dll_handle, DWORD reason, LPVOID reserved)
{
	if (DetourIsHelperProcess())
	{
		return TRUE;
	}

	// ReSharper disable once CppDefaultCaseNotHandledInSwitchStatement
	switch (reason)
	{
	case DLL_PROCESS_ATTACH:
		{
			EventRegisterNefarius_Utilities_WinApiSniffer();

			CHAR dllPath[MAX_PATH];

			GetModuleFileNameA((HINSTANCE)&__ImageBase, dllPath, MAX_PATH);
			PathRemoveFileSpecA(dllPath);
			g_dllDir = std::string(dllPath);

			auto logger = spdlog::basic_logger_mt(
				"WinApiSniffer",
				g_dllDir + "\\WinApiSniffer.log"
			);

#if _DEBUG
			spdlog::set_level(spdlog::level::debug);
			logger->flush_on(spdlog::level::debug);
#else
			logger->flush_on(spdlog::level::info);
#endif

			set_default_logger(logger);

			//
			// Load known IOCTL code definitions
			// 
			Json::Value root;
			std::ifstream ifs(g_dllDir + "\\ioctls.json");
			ifs >> root;

			for (auto& i : root)
			{
				g_ioctlMap[std::stoul(i["HexValue"].asString(), nullptr, 16)] = i["Ioctl"].asString();
			}

			// Get available drive names
			std::vector<WCHAR> buffer(GetLogicalDriveStringsW(0, nullptr));
			DWORD length = GetLogicalDriveStringsW((DWORD)buffer.size(), buffer.data());
			if (length >= buffer.size())
			{
				buffer.resize(length);
				GetLogicalDriveStringsW((DWORD)buffer.size(), buffer.data());
			}

			size_t offset = 0;
			while (true)
			{
				WCHAR* data = buffer.data() + offset;
				if (offset >= buffer.size() || *data == 0)
					break;
	
				std::string drive(strconverter.to_bytes(data));
				g_driveStrings.push_back(drive);
				offset += drive.length() + 1;
			}

			// Also exclude named pipes
			g_driveStrings.emplace_back("pipe\\");
		}

		DisableThreadLibraryCalls(dll_handle);
		DetourRestoreAfterWith();

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach((PVOID*)&real_SetupDiEnumDeviceInterfaces, DetourSetupDiEnumDeviceInterfaces);
		DetourAttach((PVOID*)&real_DeviceIoControl, DetourDeviceIoControl);
		DetourAttach((PVOID*)&real_CreateFileA, DetourCreateFileA);
		DetourAttach((PVOID*)&real_CreateFileW, DetourCreateFileW);
		DetourAttach((PVOID*)&real_ReadFile, DetourReadFile);
		DetourAttach((PVOID*)&real_ReadFileEx, DetourReadFileEx);
		DetourAttach((PVOID*)&real_WriteFile, DetourWriteFile);
		DetourAttach((PVOID*)&real_CloseHandle, DetourCloseHandle);
		DetourAttach((PVOID*)&real_GetOverlappedResult, DetourGetOverlappedResult);
		DetourTransactionCommit();

		break;

	case DLL_PROCESS_DETACH:

		EventUnregisterNefarius_Utilities_WinApiSniffer();

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourDetach((PVOID*)&real_SetupDiEnumDeviceInterfaces, DetourSetupDiEnumDeviceInterfaces);
		DetourDetach((PVOID*)&real_DeviceIoControl, DetourDeviceIoControl);
		DetourDetach((PVOID*)&real_CreateFileA, DetourCreateFileA);
		DetourDetach((PVOID*)&real_CreateFileW, DetourCreateFileW);
		DetourDetach((PVOID*)&real_ReadFile, DetourReadFile);
		DetourDetach((PVOID*)&real_ReadFileEx, DetourReadFileEx);
		DetourDetach((PVOID*)&real_WriteFile, DetourWriteFile);
		DetourDetach((PVOID*)&real_CloseHandle, DetourCloseHandle);
		DetourDetach((PVOID*)&real_GetOverlappedResult, DetourGetOverlappedResult);
		DetourTransactionCommit();

		if (!g_newIoctls.empty())
		{
			std::shared_ptr<spdlog::logger> _logger = spdlog::get("WinApiSniffer")->clone("NewIoctls");
			_logger->info("New IOCTLs:");
			for (auto ioctl : g_newIoctls)
			{
				DWORD code = ioctl.first;
				DWORD deviceType = DEVICE_TYPE_FROM_CTL_CODE(code);
				DWORD function = (code & 0x3FFC) >> 2;
				DWORD method = METHOD_FROM_CTL_CODE(code);
				DWORD access = (code & 0xC000) >> 14;

				std::string methodName =
					method == METHOD_BUFFERED ? "METHOD_BUFFERED" :
					method == METHOD_IN_DIRECT ? "METHOD_IN_DIRECT" :
					method == METHOD_OUT_DIRECT ? "METHOD_OUT_DIRECT" :
					method == METHOD_NEITHER ? "METHOD_NEITHER" :
					"INVALID_METHOD";

				std::string accessName =
					access == FILE_ANY_ACCESS ? "FILE_ANY_ACCESS" :
					access == FILE_READ_ACCESS ? "FILE_READ_ACCESS" :
					access == FILE_WRITE_ACCESS ? "FILE_WRITE_ACCESS" :
					access == (FILE_READ_ACCESS | FILE_WRITE_ACCESS) ? "FILE_READ_ACCESS | FILE_WRITE_ACCESS" :
					"INVALID_ACCESS";

				_logger->info("- Code: 0x{:08X}  Macro: CTL_CODE(0x{:04X}, 0x{:03X}, {}, {})",
					code,
					deviceType,
					function,
					methodName,
					accessName
				);
			}
		}
		break;
	}
	return TRUE;
}
