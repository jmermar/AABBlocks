#pragma once

#include <vector>
#include <cstdint>
#include <string>
namespace vblck {
struct ImageData {
	uint32_t w,  h;
	std::vector<uint8_t> data;
};

ImageData readImageFromFile(const std::string& path);
};
