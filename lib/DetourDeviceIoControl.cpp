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

	const auto error = GetLastError();

	if (lpBytesReturned)
	{
		*lpBytesReturned = tmpBytesReturned;
	}

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

	std::ostringstream inSs, outSs;

	inSs << std::hex << std::uppercase << std::setfill('0');
	outSs << std::hex << std::uppercase << std::setfill('0');
	std::for_each(inBuffer.cbegin(), inBuffer.cend(), [&](int c) { inSs << std::setw(2) << c << " "; });

	const std::string inBufferHexString = inSs.str();

	if (lpOutBuffer && nOutBufferSize > 0)
	{
		const PUCHAR charOutBuf = static_cast<PUCHAR>(lpOutBuffer);
		const auto bufSize = std::min(nOutBufferSize, tmpBytesReturned);
		const std::vector<char> outBuffer(charOutBuf, charOutBuf + bufSize);

		std::for_each(outBuffer.cbegin(), outBuffer.cend(), [&](int c) { outSs << std::setw(2) << c << " "; });
	}

	const std::string outBufferHexString = outSs.str();

	std::string ioctlName = g_ioctlMap.count(dwIoControlCode) ? g_ioctlMap[dwIoControlCode] : "Unknown";

	EventWriteCaptureDeviceIoControl(
		dwIoControlCode,
		ioctlName.c_str(), 
		retval, 
		error, 
		path.c_str(), 
		nInBufferSize,
		inBufferHexString.c_str(),
		nOutBufferSize, 
		outBufferHexString.c_str()
	);

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
