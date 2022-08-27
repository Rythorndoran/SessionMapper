#pragma once
#include"Includes.h"

namespace Mapper
{
	std::uint32_t mapDriver(MapperStruct* MapperData);

	bool readFile(const std::string& file_path, std::vector<uint8_t>* out_buffer);
}