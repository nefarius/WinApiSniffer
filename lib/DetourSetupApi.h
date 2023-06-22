#pragma once

extern decltype(SetupDiEnumDeviceInterfaces)* real_SetupDiEnumDeviceInterfaces;
extern decltype(SetupDiCreateDeviceInfoList)* real_SetupDiCreateDeviceInfoList;
extern decltype(SetupDiCallClassInstaller)* real_SetupDiCallClassInstaller;
extern decltype(SetupDiSetDeviceRegistryPropertyW)* real_SetupDiSetDeviceRegistryPropertyW;
extern decltype(SetupDiSetClassInstallParamsW)* real_SetupDiSetClassInstallParamsW;
extern decltype(SetupDiOpenDevRegKey)* real_SetupDiOpenDevRegKey;
extern decltype(SetupDiEnumDriverInfoW)* real_SetupDiEnumDriverInfoW;

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

BOOL
WINAPI
DetourSetupDiSetDeviceRegistryPropertyW(
	_In_ HDEVINFO DeviceInfoSet,
	_Inout_ PSP_DEVINFO_DATA DeviceInfoData,
	_In_ DWORD Property,
	_In_reads_bytes_opt_(PropertyBufferSize) CONST BYTE* PropertyBuffer,
	_In_ DWORD PropertyBufferSize
);

BOOL
WINAPI
DetourSetupDiSetClassInstallParamsW(
	_In_ HDEVINFO DeviceInfoSet,
	_In_opt_ PSP_DEVINFO_DATA DeviceInfoData,
	_In_reads_bytes_opt_(ClassInstallParamsSize) PSP_CLASSINSTALL_HEADER ClassInstallParams,
	_In_ DWORD ClassInstallParamsSize
);

HKEY
WINAPI
DetourSetupDiOpenDevRegKey(
	_In_ HDEVINFO DeviceInfoSet,
	_In_ PSP_DEVINFO_DATA DeviceInfoData,
	_In_ DWORD Scope,
	_In_ DWORD HwProfile,
	_In_ DWORD KeyType,
	_In_ REGSAM samDesired
);

BOOL
WINAPI
DetourSetupDiEnumDriverInfoW(
	_In_ HDEVINFO DeviceInfoSet,
	_In_opt_ PSP_DEVINFO_DATA DeviceInfoData,
	_In_ DWORD DriverType,
	_In_ DWORD MemberIndex,
	_Out_ PSP_DRVINFO_DATA_W DriverInfoData
);
