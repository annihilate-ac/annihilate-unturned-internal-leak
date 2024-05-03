#pragma once
#define _CRT_SECURE_NO_WARNINGS
#define NO_SECURITY
#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS

#include <math.h>
#include <cstdint>
#include <vector>
#include <string>
#include <mutex>
#include <iostream>
#include <unordered_map>
#include <xmmintrin.h>
#include <intrin.h>
#include <Windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <utility>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <codecvt>
#include <array>
#include <shared_mutex>

#include "kiero/kiero.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"
#include "framework/framework.h"

#include "lazy_importer.h"

#ifndef NO_SECURITY
#include <VirtualizerSDK.h>
#endif 

typedef HRESULT(__stdcall* Present) (IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
typedef LRESULT(CALLBACK* WNDPROC)(HWND, UINT, WPARAM, LPARAM);
typedef uintptr_t PTR;

void DEBUG_LOG(const char* szText) {
	std::ofstream out("logs.log", std::ios::out | std::ios::app);

	if (!out)
		return;

	out << szText << std::endl;

	out.close();
}

//#include "C:\Users\gamer\Desktop\code virtualizer\StealthMode\C\StealthCodeArea\StealthCodeArea_Max.h"
//
//STEALTH_AUX_FUNCTION;
//
//void stealth_code_area()
//{
//
//    STEALTH_AREA_START;
//
//    STEALTH_AREA_CHUNK;
//    STEALTH_AREA_CHUNK;
//    STEALTH_AREA_CHUNK;
//    STEALTH_AREA_CHUNK;
//
//    STEALTH_AREA_CHUNK;
//    STEALTH_AREA_CHUNK;
//    STEALTH_AREA_CHUNK;
//    STEALTH_AREA_CHUNK;
//
//    STEALTH_AREA_CHUNK;
//    STEALTH_AREA_CHUNK;
//    STEALTH_AREA_CHUNK;
//    STEALTH_AREA_CHUNK;
//
//    STEALTH_AREA_CHUNK;
//    STEALTH_AREA_CHUNK;
//    STEALTH_AREA_CHUNK;
//    STEALTH_AREA_CHUNK;
//
//    STEALTH_AREA_CHUNK;
//    STEALTH_AREA_CHUNK;
//    STEALTH_AREA_CHUNK;
//    STEALTH_AREA_CHUNK;
//
//    STEALTH_AREA_CHUNK;
//    STEALTH_AREA_CHUNK;
//    STEALTH_AREA_CHUNK;
//    STEALTH_AREA_CHUNK;
//
//    STEALTH_AREA_CHUNK;
//    STEALTH_AREA_CHUNK;
//    STEALTH_AREA_CHUNK;
//    STEALTH_AREA_CHUNK;
//
//    STEALTH_AREA_CHUNK;
//    STEALTH_AREA_CHUNK;
//    STEALTH_AREA_CHUNK;
//    STEALTH_AREA_CHUNK;
//
//    STEALTH_AREA_CHUNK;
//    STEALTH_AREA_CHUNK;
//    STEALTH_AREA_CHUNK;
//    STEALTH_AREA_CHUNK;
//
//    STEALTH_AREA_CHUNK;
//    STEALTH_AREA_CHUNK;
//    STEALTH_AREA_CHUNK;
//    STEALTH_AREA_CHUNK;
//
//    STEALTH_AREA_CHUNK;
//    STEALTH_AREA_CHUNK;
//    STEALTH_AREA_CHUNK;
//    STEALTH_AREA_CHUNK;
//
//    STEALTH_AREA_CHUNK;
//    STEALTH_AREA_CHUNK;
//    STEALTH_AREA_CHUNK;
//    STEALTH_AREA_CHUNK;
//
//    STEALTH_AREA_END;
//
//    return;
//
//}