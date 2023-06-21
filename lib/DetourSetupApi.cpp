#include "WinApiSniffer.h"
#include "DetourSetupApi.h"

decltype(SetupDiEnumDeviceInterfaces)* real_SetupDiEnumDeviceInterfaces = SetupDiEnumDeviceInterfaces;
decltype(SetupDiCreateDeviceInfoList)* real_SetupDiCreateDeviceInfoList = SetupDiCreateDeviceInfoList;

//
// Hooks SetupDiEnumDeviceInterfaces() API
// 
BOOL WINAPI DetourSetupDiEnumDeviceInterfaces(
	HDEVINFO DeviceInfoSet,
	PSP_DEVINFO_DATA DeviceInfoData,
	const GUID* InterfaceClassGuid,
	DWORD MemberIndex,
	PSP_DEVICE_INTERFACE_DATA DeviceInterfaceData
)
{
	const std::shared_ptr<spdlog::logger> logger = spdlog::get("WinApiSniffer")->clone(__FUNCTION__);

	logger->info("DetourSetupDiEnumDeviceInterfaces called");

	const auto retval = real_SetupDiEnumDeviceInterfaces(
		DeviceInfoSet,
		DeviceInfoData,
		InterfaceClassGuid,
		MemberIndex,
		DeviceInterfaceData
	);

	EventWriteCaptureSetupDiEnumDeviceInterfaces(retval, GetLastError(), InterfaceClassGuid);

	return retval;
}

HDEVINFO WINAPI DetourSetupDiCreateDeviceInfoList(
	const GUID* ClassGuid,
	HWND       hwndParent
)
{
	const std::shared_ptr<spdlog::logger> logger = spdlog::get("WinApiSniffer")->clone(__FUNCTION__);

	logger->info("DetourSetupDiCreateDeviceInfoList called");

	return real_SetupDiCreateDeviceInfoList(ClassGuid, hwndParent);
}
