#include "Mapper.h"

namespace Mapper
{
	template <class Return_Type = uintptr_t, class ...Args> inline auto call_fun(void* fun_ptr, Args ... agrs) -> Return_Type {
		return (reinterpret_cast<auto (__fastcall*)(Args ...)->Return_Type> (fun_ptr))(agrs...);
	}

	std::uint32_t mapDriver(MapperStruct* MapperData)
	{
		static void* pFun = 0;
		std::uint32_t status{ 0 };
		if(!pFun)
		{
			pFun = GetProcAddress(LoadLibrary(L"ntdll.dll"), "NtConvertBetweenAuxiliaryCounterAndPerformanceCounter");
		}
		if(pFun) call_fun<void*>(pFun,0, &MapperData,&status,0);
		return status;
	}

	bool readFile(const std::string& file_path, std::vector<uint8_t>* out_buffer)
	{
		std::ifstream file_ifstream(file_path, std::ios::binary);

		if (!file_ifstream)
			return false;

		out_buffer->assign((std::istreambuf_iterator<char>(file_ifstream)), std::istreambuf_iterator<char>());
		file_ifstream.close();

		return true;
	}
}