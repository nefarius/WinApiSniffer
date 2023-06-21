#pragma once

extern decltype(SetupDiEnumDeviceInterfaces)* real_SetupDiEnumDeviceInterfaces;
extern decltype(SetupDiCreateDeviceInfoList)* real_SetupDiCreateDeviceInfoList;

BOOL WINAPI DetourSetupDiEnumDeviceInterfaces(
	HDEVINFO DeviceInfoSet,
	PSP_DEVINFO_DATA DeviceInfoData,
	const GUID* InterfaceClassGuid,
	DWORD MemberIndex,
	PSP_DEVICE_INTERFACE_DATA DeviceInterfaceData
);

HDEVINFO WINAPI DetourSetupDiCreateDeviceInfoList(
	const GUID* ClassGuid,
	HWND       hwndParent
);
