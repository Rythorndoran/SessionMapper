#pragma once
#include "Utils.h"
namespace Mapper
{
	PIMAGE_SECTION_HEADER TranslateRawSection(PIMAGE_NT_HEADERS nt, DWORD rva);

	PVOID TranslateRaw(PBYTE base, PIMAGE_NT_HEADERS nt, DWORD rva);

	void resolveRelocations(std::uintptr_t imageBase, std::uintptr_t newBase, std::uintptr_t delta);

	BOOLEAN resolveImports(std::uintptr_t imageBase);
}