#include "WinApiSniffer.h"
#include "DetourSetupApi.h"

decltype(SetupDiEnumDeviceInterfaces)* real_SetupDiEnumDeviceInterfaces = SetupDiEnumDeviceInterfaces;

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
