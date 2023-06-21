#pragma once

extern decltype(SetupDiEnumDeviceInterfaces)* real_SetupDiEnumDeviceInterfaces;
extern decltype(SetupDiCreateDeviceInfoList)* real_SetupDiCreateDeviceInfoList;
extern decltype(SetupDiCallClassInstaller)* real_SetupDiCallClassInstaller;

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

BOOL
WINAPI
DetourSetupDiCallClassInstaller(
	_In_ DI_FUNCTION InstallFunction,
	_In_ HDEVINFO DeviceInfoSet,
	_In_opt_ PSP_DEVINFO_DATA DeviceInfoData
);
