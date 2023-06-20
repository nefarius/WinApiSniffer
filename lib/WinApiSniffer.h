#pragma once

//
// WinAPI
// 
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <SetupAPI.h>
#include <Shlwapi.h>
#include <initguid.h>
#include <winioctl.h>
#include "XUSB.h"
#include "WinApiSnifferETW.h"

//
// STL
// 
#include <string>
#include <codecvt>
#include <locale>
#include <map>
#include <list>
#include <iostream>
#include <fstream>

//
// JSON
// 
#include <json/json.h>

//
// Hooking
// 
#include <detours/detours.h>

//
// Logging
// 
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/fmt/bin_to_hex.h>

//
// Feature flags
// 
#define WINAPISNIFFER_LOG_UNKNOWN_IOCTLS
//#define WINAPISNIFFER_LOG_UNKNOWN_HANDLES