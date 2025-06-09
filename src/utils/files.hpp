#pragma once

#include <cstdint>
#include <string>
#include <vector>
namespace vblck
{
struct ImageData
{
	uint32_t w, h;
	std::vector<uint8_t> data;
};

ImageData readImageFromFile(const std::string& path);

std::vector<uint8_t> loadBinaryFile(const std::string& path);
}; // namespace vblck
