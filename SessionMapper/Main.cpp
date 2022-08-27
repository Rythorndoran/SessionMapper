#include "Includes.h"
#include "Mapper.h"

NTSTATUS(NTAPI* EnumerateDebuggingDevicesOriginal)(std::uintptr_t, std::uintptr_t*);
std::uintptr_t hookAddress{};

struct MapperStruct { std::size_t rawDataSize{}; std::uintptr_t rawDataAddress{}; };

using CallDriverEntry = NTSTATUS(__fastcall*) (std::uintptr_t, std::size_t);

NTSTATUS NTAPI EnumerateDebuggingDevicesHook(std::uintptr_t data, std::uintptr_t* status)
{
	if (ExGetPreviousMode() != UserMode || !data) {
		return EnumerateDebuggingDevicesOriginal(data, status);
	}

	MapperStruct requestStruct = { 0 };
	if (!Utils::safeCopyMemory(&requestStruct, reinterpret_cast<void*>(data), sizeof(requestStruct)) || !requestStruct.rawDataSize || !requestStruct.rawDataAddress)
	{
		return EnumerateDebuggingDevicesOriginal(data, status);
	}

	std::uintptr_t driverBase = reinterpret_cast<std::uintptr_t>(ExAllocatePool(NonPagedPool, requestStruct.rawDataSize));

	if (!driverBase) return STATUS_UNSUCCESSFUL;

	memcpy(reinterpret_cast<void*>(driverBase), reinterpret_cast<void*>(requestStruct.rawDataAddress), requestStruct.rawDataSize);

	Mapper::resolveImports(driverBase);

	const auto dosHeaders = reinterpret_cast<PIMAGE_DOS_HEADER>(driverBase);
	const auto ntHeaders = reinterpret_cast<PIMAGE_NT_HEADERS>(driverBase + dosHeaders->e_lfanew);

	const PIMAGE_SECTION_HEADER currentImageSection = IMAGE_FIRST_SECTION(ntHeaders);

	const auto driverAllocationBase = reinterpret_cast<std::uintptr_t>(Utils::allocateSessionSpaceMemory(ntHeaders->OptionalHeader.SizeOfImage)) - PAGE_SIZE;
	if (!driverAllocationBase)
	{
		*status = STATUS_UNSUCCESSFUL;
		return 0;
	}

	for (auto i = 0; i < ntHeaders->FileHeader.NumberOfSections; ++i)
	{
		auto sectionAddress = reinterpret_cast<void*>(driverAllocationBase + currentImageSection[i].VirtualAddress);

		memcpy(sectionAddress, reinterpret_cast<void*>(driverBase + currentImageSection[i].PointerToRawData), currentImageSection[i].SizeOfRawData);
	}

	Mapper::resolveRelocations(driverBase, driverAllocationBase, driverAllocationBase - ntHeaders->OptionalHeader.ImageBase);

	ExFreePool(reinterpret_cast<void*>(driverBase));

	CallDriverEntry mappedEntryPoint = reinterpret_cast<CallDriverEntry>(driverAllocationBase + ntHeaders->OptionalHeader.AddressOfEntryPoint);

	*status = mappedEntryPoint(driverAllocationBase, ntHeaders->OptionalHeader.SizeOfImage);

	return 0;
}

NTSTATUS DriverEntry(const PDRIVER_OBJECT driverObject)
{
	driverObject->DriverUnload = [](PDRIVER_OBJECT driverObject) -> void {
		reinterpret_cast<std::uintptr_t>(InterlockedExchangePointer(RelativeAddress(hookAddress, 7), (PVOID)EnumerateDebuggingDevicesOriginal));
		reinterpret_cast<PKLDR_DATA_TABLE_ENTRY>(driverObject->DriverSection)->BaseDllName.Length = 0;
	};

	const auto [ntoskrnlBase, ntoskrnlSize] = Utils::getKernelModuleByName("ntoskrnl.exe");

	if (!ntoskrnlBase)
	{
		return STATUS_FAILED_DRIVER_ENTRY;
	}

	hookAddress = Utils::scanPattern(reinterpret_cast<std::uint8_t*>(ntoskrnlBase), "PAGE", PBYTE("\x48\x8B\x05\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x8B\xC8\x85\xC0\x78\x40"), "xxx????x????xxxxxx", 19);
	if (!hookAddress)
	{
		return STATUS_FAILED_DRIVER_ENTRY;
	}

	*reinterpret_cast<std::uintptr_t*>(&EnumerateDebuggingDevicesOriginal) = reinterpret_cast<std::uintptr_t>(InterlockedExchangePointer(RelativeAddress(hookAddress, 7), (PVOID)EnumerateDebuggingDevicesHook));

	return STATUS_SUCCESS;
}