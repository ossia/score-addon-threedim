#pragma once
#include <Threedim/TinyObj.hpp>

#include <cstdint>

namespace Threedim
{

std::vector<mesh> PlyFromFile(std::string_view filename, float_vec& data);

bool print_ply_header(const char* filename);

}
