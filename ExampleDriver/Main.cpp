#include <ntifs.h>

NTSTATUS DriverEntry(ULONG64 mappedImageBase, ULONG64 mappedImageSize)
{
	DbgPrintEx(77,0,"[%s] Driver Mapped in [%p] & Size [0x%x]",__FUNCTION__, mappedImageBase, mappedImageSize);
	return STATUS_SUCCESS;
}