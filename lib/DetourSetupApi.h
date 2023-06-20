#pragma once

extern decltype(SetupDiEnumDeviceInterfaces) *real_SetupDiEnumDeviceInterfaces;

BOOL WINAPI DetourSetupDiEnumDeviceInterfaces(
	HDEVINFO DeviceInfoSet,
	PSP_DEVINFO_DATA DeviceInfoData,
	const GUID* InterfaceClassGuid,
	DWORD MemberIndex,
	PSP_DEVICE_INTERFACE_DATA DeviceInterfaceData
);
