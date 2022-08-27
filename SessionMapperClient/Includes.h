#pragma once
#include <windows.h>
#include <fmt/core.h>
#include <fmt/color.h>
#include <cstdint>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <vector>
#include "loadup.hpp"
#include"Mapper_Driver.h"

struct MapperStruct { std::size_t rawDataSize{}; std::uintptr_t rawDataAddress{}; };
