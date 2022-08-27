#include "Utils.h"

namespace Utils
{

	auto allocateSessionSpaceMemory(std::size_t Size) -> void*
	{
		HANDLE SectionHandle;
		PVOID SectionObject = NULL;
		PVOID MappingAddress = 0;
		SIZE_T MappingSize = 0;
		LARGE_INTEGER MaximumSize = { 0 };
		MaximumSize.QuadPart = Size;

		ZwCreateSection(&SectionHandle, SECTION_ALL_ACCESS, NULL, &MaximumSize, PAGE_EXECUTE_READWRITE, SEC_COMMIT, NULL);

		ObReferenceObjectByHandle(SectionHandle, SECTION_ALL_ACCESS, MmSectionObjectType, KernelMode, &SectionObject, NULL);

		//Ó³Éäµ½session¿Õ¼ä
		MmMapViewInSessionSpace(SectionObject, &MappingAddress, &MappingSize);

		RtlZeroMemory(MappingAddress, Size);

		return MappingAddress;
	}

	auto safeCopyMemory(void* dest, void* src, std::size_t size) -> bool
	{
		SIZE_T returnSize = 0;
		if (NT_SUCCESS(MmCopyVirtualMemory(PsGetCurrentProcess(), src, PsGetCurrentProcess(), dest, size, KernelMode, &returnSize)) && returnSize == size) {
			return TRUE;
		}

		return FALSE;
	}

	auto getKernelModuleByName(const char* moduleName) -> std::tuple<std::uintptr_t, std::size_t>
	{
		std::uintptr_t moduleStart = 0;
		std::size_t moduleSize = 0;

		std::size_t size{};
		ZwQuerySystemInformation(0xB, nullptr, size, reinterpret_cast<PULONG>(&size));

		const auto listHeader = ExAllocatePool(NonPagedPool, size);
		if (!listHeader)
			return { 0,0 };

		if (const auto status = ZwQuerySystemInformation(0xB, listHeader, size, reinterpret_cast<PULONG>(&size)))
			return { 0,0 };

		auto currentModule = reinterpret_cast<PSYSTEM_MODULE_INFORMATION>(listHeader)->Module;
		for (std::size_t i{}; i < reinterpret_cast<PSYSTEM_MODULE_INFORMATION>(listHeader)->Count; ++i, ++currentModule)
		{
			const auto currentModuleName = reinterpret_cast<const char*>(currentModule->FullPathName + currentModule->OffsetToFileName);
			if (!strcmp(moduleName, currentModuleName))
			{
				moduleStart = reinterpret_cast<std::uintptr_t>(currentModule->ImageBase);
				moduleSize = currentModule->ImageSize;
				ExFreePool(listHeader);
				return { moduleStart,moduleSize };
			}
		}

		ExFreePool(listHeader);
		return { 0,0 };
	}

	auto scanPattern(std::uint8_t* pBase, LPCSTR Section, BYTE* bMask, const char* szMask, ULONG MaskLength) -> std::uintptr_t
	{
		auto bDataCompare = [](const BYTE* pData, const BYTE* bMask, const char* szMask) -> BOOL
		{
			for (; *szMask; ++szMask, ++pData, ++bMask)
			{
				if (*szMask == 'x' && *pData != *bMask)
					return FALSE;
			}
			return (*szMask) == NULL;
		};

		PIMAGE_NT_HEADERS64 pHdr = RtlImageNtHeader(pBase);
		if (!pHdr)
			return NULL;
		PIMAGE_SECTION_HEADER pFirstSection = (PIMAGE_SECTION_HEADER)(pHdr + 1);
		for (PIMAGE_SECTION_HEADER pSection = pFirstSection; pSection < pFirstSection + pHdr->FileHeader.NumberOfSections; pSection++)
		{
			ANSI_STRING s1, s2;
			RtlInitAnsiString(&s1, Section);
			RtlInitAnsiString(&s2, (PCCHAR)pSection->Name);
			if (RtlCompareString(&s1, &s2, TRUE) == 0)
			{
				DWORD64 dwBase = DWORD64((PUCHAR)pBase + pSection->VirtualAddress);
				ULONG dwSize = pSection->Misc.VirtualSize;
				for (auto i = 0ul; i < dwSize - MaskLength; i++)
				{
					if (bDataCompare(PBYTE(dwBase + i), bMask, szMask))
						return DWORD64(dwBase + i);
				}
			}
		}
		return NULL;
	}
}