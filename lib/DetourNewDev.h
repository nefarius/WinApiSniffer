#pragma once

extern decltype(DiInstallDevice)* real_DiInstallDevice;

BOOL
WINAPI
DetourDiInstallDevice(
	_In_opt_  HWND hwndParent,
	_In_      HDEVINFO DeviceInfoSet,
	_In_      PSP_DEVINFO_DATA DeviceInfoData,
	_In_opt_  PSP_DRVINFO_DATA DriverInfoData,
	_In_      DWORD Flags,
	_Out_opt_ PBOOL NeedReboot
);
