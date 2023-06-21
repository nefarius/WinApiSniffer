#include "WinApiSniffer.h"
#include "DetourSetupApi.h"

decltype(SetupDiEnumDeviceInterfaces)* real_SetupDiEnumDeviceInterfaces = SetupDiEnumDeviceInterfaces;
decltype(SetupDiCreateDeviceInfoList)* real_SetupDiCreateDeviceInfoList = SetupDiCreateDeviceInfoList;
decltype(SetupDiCallClassInstaller)* real_SetupDiCallClassInstaller = SetupDiCallClassInstaller;
decltype(SetupDiSetDeviceRegistryPropertyW)* real_SetupDiSetDeviceRegistryPropertyW = SetupDiSetDeviceRegistryPropertyW;
decltype(SetupDiSetClassInstallParamsW)* real_SetupDiSetClassInstallParamsW = SetupDiSetClassInstallParamsW;
decltype(SetupDiOpenDevRegKey)* real_SetupDiOpenDevRegKey = SetupDiOpenDevRegKey;

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

BOOL
WINAPI
DetourSetupDiCallClassInstaller(
	_In_ DI_FUNCTION InstallFunction,
	_In_ HDEVINFO DeviceInfoSet,
	_In_opt_ PSP_DEVINFO_DATA DeviceInfoData
)
{
	const std::shared_ptr<spdlog::logger> logger = spdlog::get("WinApiSniffer")->clone(__FUNCTION__);

	logger->info("DetourSetupDiCallClassInstaller called");

	return real_SetupDiCallClassInstaller(InstallFunction, DeviceInfoSet, DeviceInfoData);
}

BOOL
WINAPI
DetourSetupDiSetDeviceRegistryPropertyW(
	_In_ HDEVINFO DeviceInfoSet,
	_Inout_ PSP_DEVINFO_DATA DeviceInfoData,
	_In_ DWORD Property,
	_In_reads_bytes_opt_(PropertyBufferSize) CONST BYTE* PropertyBuffer,
	_In_ DWORD PropertyBufferSize
)
{
	const std::shared_ptr<spdlog::logger> logger = spdlog::get("WinApiSniffer")->clone(__FUNCTION__);

	logger->info("DetourSetupDiSetDeviceRegistryPropertyW called");

	return real_SetupDiSetDeviceRegistryPropertyW(
		DeviceInfoSet,
		DeviceInfoData,
		Property,
		PropertyBuffer,
		PropertyBufferSize
	);
}

BOOL
WINAPI
DetourSetupDiSetClassInstallParamsW(
	_In_ HDEVINFO DeviceInfoSet,
	_In_opt_ PSP_DEVINFO_DATA DeviceInfoData,
	_In_reads_bytes_opt_(ClassInstallParamsSize) PSP_CLASSINSTALL_HEADER ClassInstallParams,
	_In_ DWORD ClassInstallParamsSize
)
{
	const std::shared_ptr<spdlog::logger> logger = spdlog::get("WinApiSniffer")->clone(__FUNCTION__);

	logger->info("DetourSetupDiSetClassInstallParamsW called");

	return real_SetupDiSetClassInstallParamsW(
		DeviceInfoSet,
		DeviceInfoData,
		ClassInstallParams,
		ClassInstallParamsSize
	);
}

HKEY
WINAPI
DetourSetupDiOpenDevRegKey(
	_In_ HDEVINFO DeviceInfoSet,
	_In_ PSP_DEVINFO_DATA DeviceInfoData,
	_In_ DWORD Scope,
	_In_ DWORD HwProfile,
	_In_ DWORD KeyType,
	_In_ REGSAM samDesired
)
{
	return real_SetupDiOpenDevRegKey(
		DeviceInfoSet,
		DeviceInfoData,
		Scope,
		HwProfile,
		KeyType,
		samDesired
	);
}
