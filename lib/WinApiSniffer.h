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
#include <newdev.h>
#include "XUSB.h"
#include "WinApiSnifferETW.h"

//
// STL
// 
#include <algorithm>
#include <codecvt>
#include <fstream>
#include <iostream>
#include <iterator>
#include <list>
#include <locale>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include <iomanip>

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