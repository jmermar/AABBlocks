#pragma once

#include "types.hpp"
#include <cstdint>
#include <string>
#include <unordered_map>
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
	uint32_t w{}, h{}, layers{};
	std::vector<uint8_t> data{};
};

struct TextureAtlas
{
	ImageArrayData albedo{};
	ImageArrayData normal{};
	ImageArrayData metallicRoughness{};
	std::unordered_map<std::string, uint32_t>
		maps;
};

ImageData
readImageFromFile(const std::string& path);
ImageArrayData
readImageArrayFromFile(const std::string& path,
					   uint32_t ncols,
					   uint32_t nrows);
ImageArrayData
readCubeMapFromFile(const std::string& path);

bool fileExists(const std::string& path);
void createDirIfNotExists(
	const std::string& path);
std::vector<std::string>
listFilesInFolder(const std::string& path);

std::vector<uint8_t>
loadBinaryFile(const std::string& path);
std::string loadTextFile(const std::string& path);

std::vector<BlockData>
loadBlockData(const std::string& path,
			  TextureAtlas& textureAtlas);
TextureAtlas
loadBlockTextures(const std::string& basePath);
}; // namespace vblck
