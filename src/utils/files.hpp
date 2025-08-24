#pragma once

#include "types.hpp"
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

struct ImageArrayData
{
	uint32_t w, h, layers;
	std::vector<uint8_t> data;
};

ImageData readImageFromFile(const std::string& path);
ImageArrayData readImageArrayFromFile(const std::string& path, uint32_t ncols, uint32_t nrows);

bool fileExists(const std::string& path);
void createDirIfNotExists(const std::string& path);

std::vector<uint8_t> loadBinaryFile(const std::string& path);
std::string loadTextFile(const std::string& path);

std::vector<BlockData> loadBlockData(const std::string& path);
}; // namespace vblck
