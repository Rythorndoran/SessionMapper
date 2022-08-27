#pragma once
#include "Includes.h"
namespace Utils
{
	auto allocateSessionSpaceMemory(std::size_t Size) -> void*;

	auto safeCopyMemory(void* dest, void* src, std::size_t size) -> bool;

	auto getKernelModuleByName(const char* moduleName)->std::tuple<std::uintptr_t, std::size_t>;

	auto scanPattern(std::uint8_t* pBase, LPCSTR Section, BYTE* bMask, const char* szMask, ULONG MaskLength)->std::uintptr_t;
}