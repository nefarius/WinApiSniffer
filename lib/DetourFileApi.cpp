#include "WINAPISNIFFER.h"
#include "DetourFileApi.h"

extern std::list<std::string> g_driveStrings;
extern std::map<HANDLE, std::string> g_handleToPath;

using convert_t = std::codecvt_utf8<wchar_t>;
static std::wstring_convert<convert_t, wchar_t> strconverter;

decltype(CreateFileA)* real_CreateFileA = CreateFileA;
decltype(CreateFileW)* real_CreateFileW = CreateFileW;
decltype(ReadFile)* real_ReadFile = ReadFile;
decltype(WriteFile)* real_WriteFile = WriteFile;
decltype(CloseHandle)* real_CloseHandle = CloseHandle;

//
// Hooks CreateFileA() API
// 
HANDLE WINAPI DetourCreateFileA(
	LPCSTR lpFileName,
	DWORD dwDesiredAccess,
	DWORD dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD dwCreationDisposition,
	DWORD dwFlagsAndAttributes,
	HANDLE hTemplateFile
)
{
	const std::string path(lpFileName);

	const bool isOfInterest = (path.rfind("\\\\", 0) == 0);

	const auto handle = real_CreateFileA(
		lpFileName,
		dwDesiredAccess,
		dwShareMode,
		lpSecurityAttributes,
		dwCreationDisposition,
		dwFlagsAndAttributes,
		hTemplateFile
	);

	const auto error = GetLastError();

	if (isOfInterest)
	{
		// Verify that this is not a regular file path
		for (const auto& drive : g_driveStrings)
		{
			// //?/C:/...
			// 0123456
			if (path.rfind(drive, 4) != std::string::npos)
			{
				// Path is to a file, ignore
				return handle;
			}
		}

		if (handle != INVALID_HANDLE_VALUE)
		{
			g_handleToPath[handle] = path;
		}

		EventWriteCaptureCreateFileA(lpFileName, handle, error);
	}

	return handle;
}

//
// Hooks CreateFileW() API
// 
HANDLE WINAPI DetourCreateFileW(
	LPCWSTR lpFileName,
	DWORD dwDesiredAccess,
	DWORD dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD dwCreationDisposition,
	DWORD dwFlagsAndAttributes,
	HANDLE hTemplateFile
)
{
	const std::string path(strconverter.to_bytes(lpFileName));

	const bool isOfInterest = (path.rfind("\\\\", 0) == 0);

	const auto handle = real_CreateFileW(
		lpFileName,
		dwDesiredAccess,
		dwShareMode,
		lpSecurityAttributes,
		dwCreationDisposition,
		dwFlagsAndAttributes,
		hTemplateFile
	);

	const auto error = GetLastError();

	if (isOfInterest)
	{
		// Verify that this is not a regular file path
		for (const auto& drive : g_driveStrings)
		{
			// //?/C:/...
			// 0123456
			if (path.rfind(drive, 4) != std::string::npos)
			{
				// Path is to a file, ignore
				return handle;
			}
		}

		if (handle != INVALID_HANDLE_VALUE)
		{
			g_handleToPath[handle] = path;
		}

		EventWriteCaptureCreateFileW(lpFileName, handle, error);
	}

	return handle;
}

//
// Hooks ReadFile() API
// 
BOOL WINAPI DetourReadFile(
	HANDLE       hFile,
	LPVOID       lpBuffer,
	DWORD        nNumberOfBytesToRead,
	LPDWORD      lpNumberOfBytesRead,
	LPOVERLAPPED lpOverlapped
)
{
	const PUCHAR charInBuf = PUCHAR(lpBuffer);
	DWORD tmpBytesRead = 0;

	const auto ret = real_ReadFile(hFile, lpBuffer, nNumberOfBytesToRead, &tmpBytesRead, lpOverlapped);
	const auto error = GetLastError();

	if (lpNumberOfBytesRead)
	{
		*lpNumberOfBytesRead = tmpBytesRead;
	}

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

	const auto bufSize = std::min(nNumberOfBytesToRead, tmpBytesRead);
	const std::vector<char> outBuffer(charInBuf, charInBuf + bufSize);

	std::ostringstream ss;

	ss << std::hex << std::uppercase << std::setfill('0');
	std::for_each(outBuffer.cbegin(), outBuffer.cend(), [&](int c) { ss << std::setw(2) << c << " "; });

	const std::string hexString = ss.str();

	EventWriteCaptureReadFile(ret, error, hFile, path.c_str(), nNumberOfBytesToRead, tmpBytesRead, hexString.c_str());

	return ret;
}

//
// Hooks WriteFile() API
// 
BOOL WINAPI DetourWriteFile(
	HANDLE       hFile,
	LPCVOID      lpBuffer,
	DWORD        nNumberOfBytesToWrite,
	LPDWORD      lpNumberOfBytesWritten,
	LPOVERLAPPED lpOverlapped
)
{
	const std::shared_ptr<spdlog::logger> _logger = spdlog::get("WinApiSniffer")->clone("WriteFile");

	const PUCHAR charInBuf = PUCHAR(lpBuffer);
	DWORD tmpBytesWritten = 0;

	const auto ret = real_WriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, &tmpBytesWritten, lpOverlapped);
	const auto error = GetLastError();

	if (lpNumberOfBytesWritten)
	{
		*lpNumberOfBytesWritten = tmpBytesWritten;
	}

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

	const std::vector<char> inBuffer(charInBuf, charInBuf + nNumberOfBytesToWrite);

	std::ostringstream ss;

	ss << std::hex << std::uppercase << std::setfill('0');
	std::for_each(inBuffer.cbegin(), inBuffer.cend(), [&](int c) { ss << std::setw(2) << c << " "; });

	const std::string hexString = ss.str();

	EventWriteCaptureWriteFile(ret, error, hFile, path.c_str(), nNumberOfBytesToWrite, tmpBytesWritten, hexString.c_str());

	return ret;
}

BOOL DetourCloseHandle(
	HANDLE hObject
)
{
	const auto ret = real_CloseHandle(hObject);

	std::string path = "Unknown";
	if (g_handleToPath.count(hObject))
	{
		path = g_handleToPath[hObject];
		g_handleToPath.erase(hObject);
	}
#ifndef WINAPISNIFFER_LOG_UNKNOWN_HANDLES
	else
	{
		// Ignore unknown handles
		return ret;
	}
#endif

	EventWriteCaptureCloseHandle(hObject, path.c_str());

	return ret;
}
