#include "WinApiSniffer.h"
#include "DetourDeviceIoControl.h"

extern std::map<HANDLE, std::string> g_handleToPath;
extern std::map<DWORD, std::string> g_ioctlMap;
extern std::map<DWORD, bool> g_newIoctls;

decltype(DeviceIoControl)* real_DeviceIoControl = DeviceIoControl;
decltype(GetOverlappedResult)* real_GetOverlappedResult = GetOverlappedResult;

//
// Hooks DeviceIoControl() API
// 
BOOL WINAPI DetourDeviceIoControl(
	HANDLE hDevice,
	DWORD dwIoControlCode,
	LPVOID lpInBuffer,
	DWORD nInBufferSize,
	LPVOID lpOutBuffer,
	DWORD nOutBufferSize,
	LPDWORD lpBytesReturned,
	LPOVERLAPPED lpOverlapped
)
{
	const std::shared_ptr<spdlog::logger> _logger = spdlog::get("WinApiSniffer")->clone("DeviceIoControl");

	const PUCHAR charInBuf = static_cast<PUCHAR>(lpInBuffer);
	const std::vector<char> inBuffer(charInBuf, charInBuf + nInBufferSize);

	DWORD tmpBytesReturned = 0;

	const auto retval = real_DeviceIoControl(
		hDevice,
		dwIoControlCode,
		lpInBuffer,
		nInBufferSize,
		lpOutBuffer,
		nOutBufferSize,
		&tmpBytesReturned, // might be null, use our own variable
		lpOverlapped
	);

	if (lpBytesReturned)
		*lpBytesReturned = tmpBytesReturned;

	std::string path = "Unknown";
	if (g_handleToPath.count(hDevice))
	{
		path = g_handleToPath[hDevice];
	}
#ifndef WINAPISNIFFER_LOG_UNKNOWN_HANDLES
	else
	{
		// Ignore unknown handles
		return retval;
	}
#endif

	if (g_ioctlMap.count(dwIoControlCode))
	{
		_logger->info("[I] [{}] path = {} ({:04d}) -> {:Xpn}",
			g_ioctlMap[dwIoControlCode],
			path,
			nInBufferSize,
			spdlog::to_hex(inBuffer)
		);
	}
	else
	{
		// Add control code to list of unknown codes
		g_newIoctls[dwIoControlCode] = true;
#ifdef WINAPISNIFFER_LOG_UNKNOWN_IOCTLS
		_logger->info("[I] [0x{:08X}] path = {} ({:04d}) -> {:Xpn}",
			dwIoControlCode,
			path,
			nInBufferSize,
			spdlog::to_hex(inBuffer)
		);
#endif
	}

	if (lpOutBuffer && nOutBufferSize > 0)
	{
		const PUCHAR charOutBuf = static_cast<PUCHAR>(lpOutBuffer);
		const auto bufSize = std::min(nOutBufferSize, tmpBytesReturned);
		const std::vector<char> outBuffer(charOutBuf, charOutBuf + bufSize);

		if (g_ioctlMap.count(dwIoControlCode))
		{
			_logger->info("[O] [{}] path = {} ({:04d}) -> {:Xpn}",
				g_ioctlMap[dwIoControlCode],
				path,
				bufSize,
				spdlog::to_hex(outBuffer)
			);
		}
#ifdef WinApiSniffer_LOG_UNKNOWN_IOCTLS
		else
		{
			_logger->info("[O] [0x{:08X}] path = {} ({:04d}) -> {:Xpn}",
				dwIoControlCode,
				path,
				bufSize,
				spdlog::to_hex(outBuffer)
			);
		}
#endif
	}

	return retval;
}

BOOL WINAPI DetourGetOverlappedResult(
	HANDLE       hFile,
	LPOVERLAPPED lpOverlapped,
	LPDWORD      lpNumberOfBytesTransferred,
	BOOL         bWait
)
{
	DWORD tmpBytesTransferred = 0;

	const auto ret = real_GetOverlappedResult(hFile, lpOverlapped, &tmpBytesTransferred, bWait);
	const auto error = GetLastError();

	if (lpNumberOfBytesTransferred)
	{
		*lpNumberOfBytesTransferred = tmpBytesTransferred;
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

	EventWriteCaptureGetOverlappedResult(ret, error, tmpBytesTransferred, hFile, path.c_str());
	
	return ret;
}
