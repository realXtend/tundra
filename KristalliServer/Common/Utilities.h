#pragma once

#include "clb/Core/Types.h"
#include <string>
#include <vector>

std::string BufferToString(const std::vector<s8>& buffer);
std::vector<s8> StringToBuffer(const std::string& str);

