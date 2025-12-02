#pragma once

#pragma warning(disable: 4100 4201 4244 4389 5054)

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <d3d9.h>
#include <d3dx9math.h>

#include <windows.h>

#include <stdint.h>
#include <stdio.h>
#include <cmath>
#include <string>
#include <cassert>

#include "Utils/DebugLog.hpp"

#include "SafeWrite/SafeWrite.hpp"
#include "Utils/AddressPtr.hpp"
#include "Utils/Bitfield.hpp"

// Game unit conversion constants
constexpr double dM2NI = 69.99125671386719;		// 1 Meter to Ni
constexpr double dDM2NI = dM2NI / 10.0;			// 1 Decimeter to Ni
constexpr double dCM2NI = dM2NI / 100.0;		// 1 Centimeter to Ni
constexpr double dMM2NI = dM2NI / 1000.0;		// 1 Millimeter to Ni

constexpr double dNI2M = 1.0 / dM2NI;			// 1 Ni to Meter
constexpr double dNI2DM = 1.0 / dDM2NI;			// 1 Ni to Decimeter
constexpr double dNI2CM = 1.0 / dCM2NI;			// 1 Ni to Centimeter
constexpr double dNI2MM = 1.0 / dMM2NI;			// 1 Ni to Millimeter

#define EXTERN_DLL_EXPORT extern "C" __declspec(dllexport)

#ifdef NDEBUG
#define ASSUME_ASSERT(x) __assume(x)
#else
#define ASSUME_ASSERT(x) assert(x)
#endif

#define ASSERT_SIZE(a, b) static_assert(sizeof(a) == b, "Wrong structure size!");
#define ASSERT_OFFSET(a, b, c) static_assert(offsetof(a, b) == c, "Wrong member offset!");
#define CREATE_OBJECT(CLASS, ADDRESS) static CLASS* CreateObject() { return CdeclCall<CLASS*>(ADDRESS); };

template <typename T_Ret = uint32_t, typename ...Args>
__forceinline T_Ret ThisCall(uint32_t _addr, const void* _this, Args ...args) {
	return ((T_Ret(__thiscall*)(const void*, Args...))_addr)(_this, std::forward<Args>(args)...);
}

template <typename T_Ret = void, typename ...Args>
__forceinline T_Ret StdCall(uint32_t _addr, Args ...args) {
	return ((T_Ret(__stdcall*)(Args...))_addr)(std::forward<Args>(args)...);
}

template <typename T_Ret = void, typename ...Args>
__forceinline T_Ret CdeclCall(uint32_t _addr, Args ...args) {
	return ((T_Ret(__cdecl*)(Args...))_addr)(std::forward<Args>(args)...);
}

template <typename T_Ret = void, typename ...Args>
__forceinline T_Ret FastCall(uint32_t _addr, Args ...args) {
	return ((T_Ret(__fastcall*)(Args...))_addr)(std::forward<Args>(args)...);
}

#define SPEC_RESTRICT	__declspec(restrict)
#define SPEC_NOINLINE	__declspec(noinline)
#define SPEC_NORETURN	__declspec(noreturn)
#define SPEC_NOALIAS	__declspec(noalias)
