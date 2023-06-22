#include "WinApiSniffer.h"
#include "DetourSetupApi.h"

using convert_t = std::codecvt_utf8<wchar_t>;
static std::wstring_convert<convert_t, wchar_t> strconverter;

decltype(SetupDiEnumDeviceInterfaces)* real_SetupDiEnumDeviceInterfaces = SetupDiEnumDeviceInterfaces;
decltype(SetupDiCreateDeviceInfoList)* real_SetupDiCreateDeviceInfoList = SetupDiCreateDeviceInfoList;
decltype(SetupDiCallClassInstaller)* real_SetupDiCallClassInstaller = SetupDiCallClassInstaller;
decltype(SetupDiSetDeviceRegistryPropertyW)* real_SetupDiSetDeviceRegistryPropertyW = SetupDiSetDeviceRegistryPropertyW;
decltype(SetupDiSetClassInstallParamsW)* real_SetupDiSetClassInstallParamsW = SetupDiSetClassInstallParamsW;
decltype(SetupDiOpenDevRegKey)* real_SetupDiOpenDevRegKey = SetupDiOpenDevRegKey;
decltype(SetupDiEnumDriverInfoW)* real_SetupDiEnumDriverInfoW = SetupDiEnumDriverInfoW;
decltype(SetupOpenInfFileW)* real_SetupOpenInfFileW = SetupOpenInfFileW;
decltype(SetupFindFirstLineW)* real_SetupFindFirstLineW = SetupFindFirstLineW;
decltype(SetupDiSetDeviceInstallParamsW)* real_SetupDiSetDeviceInstallParamsW = SetupDiSetDeviceInstallParamsW;

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

	//EventWriteCaptureSetupDiEnumDeviceInterfaces(retval, GetLastError(), InterfaceClassGuid);

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

BOOL
WINAPI
DetourSetupDiEnumDriverInfoW(
	_In_ HDEVINFO DeviceInfoSet,
	_In_opt_ PSP_DEVINFO_DATA DeviceInfoData,
	_In_ DWORD DriverType,
	_In_ DWORD MemberIndex,
	_Out_ PSP_DRVINFO_DATA_W DriverInfoData
)
{
	const std::shared_ptr<spdlog::logger> logger = spdlog::get("WinApiSniffer")->clone(__FUNCTION__);

	logger->info("DetourSetupDiEnumDriverInfoW called");

	// TODO: port me to ETW!

	if (DriverInfoData)
	{
		const std::string description(strconverter.to_bytes(DriverInfoData->Description));
		const std::string mfgName(strconverter.to_bytes(DriverInfoData->MfgName));
		const std::string providerName(strconverter.to_bytes(DriverInfoData->ProviderName));

		logger->info("DriverType = 0x{:08X}, MemberIndex = 0x{:08X}, DriverInfoData->DriverType = 0x{:08X}, DriverInfoData->Description = {}, DriverInfoData->MfgName = {}, DriverInfoData->ProviderName = {}",
			DriverType,
			MemberIndex,
			DriverInfoData->DriverType,
			description,
			mfgName,
			providerName
		);

		/*
		const PUCHAR asBuffer = PUCHAR(DriverInfoData);
		const std::vector<char> buffer(asBuffer, asBuffer + DriverInfoData->cbSize);

		logger->info("DriverInfoData = {:Xpn}",
			spdlog::to_hex(buffer)
		);
		*/
	}

	return real_SetupDiEnumDriverInfoW(
		DeviceInfoSet,
		DeviceInfoData,
		DriverType,
		MemberIndex,
		DriverInfoData
	);
}

HINF
WINAPI
DetourSetupOpenInfFileW(
	_In_ PCWSTR FileName,
	_In_opt_ PCWSTR InfClass,
	_In_ DWORD InfStyle,
	_Out_opt_ PUINT ErrorLine
)
{
	const std::shared_ptr<spdlog::logger> logger = spdlog::get("WinApiSniffer")->clone(__FUNCTION__);

	const std::string fileName(strconverter.to_bytes(FileName));

	logger->info("FileName = {}", fileName);

	return real_SetupOpenInfFileW(
		FileName,
		InfClass,
		InfStyle,
		ErrorLine
	);
}

BOOL
WINAPI
DetourSetupFindFirstLineW(
	_In_ HINF InfHandle,
	_In_ PCWSTR Section,
	_In_opt_ PCWSTR Key,
	_Out_ PINFCONTEXT Context
)
{
	const std::shared_ptr<spdlog::logger> logger = spdlog::get("WinApiSniffer")->clone(__FUNCTION__);

	const std::string section(strconverter.to_bytes(Section));
	const std::string key(strconverter.to_bytes(Key));

	logger->info("Section = {}, Key = {}",
		section, key
	);

	return real_SetupFindFirstLineW(
		InfHandle,
		Section,
		Key,
		Context
	);
}

BOOL
WINAPI
DetourSetupDiSetDeviceInstallParamsW(
	_In_ HDEVINFO DeviceInfoSet,
	_In_opt_ PSP_DEVINFO_DATA DeviceInfoData,
	_In_ PSP_DEVINSTALL_PARAMS_W DeviceInstallParams
)
{
	const std::shared_ptr<spdlog::logger> logger = spdlog::get("WinApiSniffer")->clone(__FUNCTION__);

	if (DeviceInstallParams)
	{
		DWORD flags = DeviceInstallParams->Flags;
		DWORD flagsEx = DeviceInstallParams->FlagsEx;
		const std::string driverPath(strconverter.to_bytes(DeviceInstallParams->DriverPath));

		logger->info("Flags = 0x{:08X}, FlagsEx = 0x{:08X}, DriverPath = {}",
			flags, flagsEx, driverPath
		);
	}

	return real_SetupDiSetDeviceInstallParamsW(
		DeviceInfoSet,
		DeviceInfoData,
		DeviceInstallParams
	);
}
