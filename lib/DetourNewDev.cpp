#include "WinApiSniffer.h"
#include "DetourNewDev.h"

decltype(DiInstallDevice)* real_DiInstallDevice = DiInstallDevice;

BOOL
WINAPI
DetourDiInstallDevice(
	_In_opt_  HWND hwndParent,
	_In_      HDEVINFO DeviceInfoSet,
	_In_      PSP_DEVINFO_DATA DeviceInfoData,
	_In_opt_  PSP_DRVINFO_DATA DriverInfoData,
	_In_      DWORD Flags,
	_Out_opt_ PBOOL NeedReboot
)
{
	const std::shared_ptr<spdlog::logger> logger = spdlog::get("WinApiSniffer")->clone(__FUNCTION__);

	logger->info("DetourDiInstallDevice called");

	const auto retval = real_DiInstallDevice(
		hwndParent,
		DeviceInfoSet,
		DeviceInfoData,
		DriverInfoData,
		Flags,
		NeedReboot
	);

	std::ostringstream deviceInfoDataSs, driverInfoDataSs;

	deviceInfoDataSs << std::hex << std::uppercase << std::setfill('0');
	driverInfoDataSs << std::hex << std::uppercase << std::setfill('0');

	//
	// Not optional but user might not have supplied it, so better check
	// 
	if (DeviceInfoData)
	{
		const auto charBuf = reinterpret_cast<PUCHAR>(DeviceInfoData);
		const std::vector<char> buffer(charBuf, charBuf + DeviceInfoData->cbSize);
		std::for_each(buffer.cbegin(), buffer.cend(), [&](int c) { deviceInfoDataSs << std::setw(2) << c << " "; });
	}

	const std::string deviceInfoDataHexString = deviceInfoDataSs.str();

	//
	// Optional parameter
	// 
	if (DriverInfoData)
	{
		const auto charBuf = reinterpret_cast<PUCHAR>(DriverInfoData);
		const std::vector<char> buffer(charBuf, charBuf + DriverInfoData->cbSize);
		std::for_each(buffer.cbegin(), buffer.cend(), [&](int c) { driverInfoDataSs << std::setw(2) << c << " "; });
	}

	const std::string driverInfoDataHexString = driverInfoDataSs.str();

	BOOL tmpNeedReboot = FALSE;

	if (NeedReboot)
	{
		tmpNeedReboot = *NeedReboot;
	}

	EventWriteCaptureDiInstallDevice(
		hwndParent,
		DeviceInfoSet,
		deviceInfoDataHexString.c_str(),
		driverInfoDataHexString.c_str(),
		Flags,
		tmpNeedReboot
	);

	return retval;
}
