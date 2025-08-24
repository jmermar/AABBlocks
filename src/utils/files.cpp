#include "files.hpp"
#include "errors.hpp"
#include "logger.hpp"
#include "stb_image/stb_image.h"

#include "types.hpp"
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>
namespace vblck
{
ImageData readImageFromFile(const std::string& path)
{

	int width, height, channels;

	unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 4);

	if(!data)
	{
		LOG_ERR("CANNOT LOAD IMAGE");
		exit(-1);
	}
	ImageData img;
	img.w = width;
	img.h = height;
	img.data.resize(width * height * 4);

	memcpy(img.data.data(), data, width * height * 4);
	stbi_image_free(data);

	return img;
}
bool fileExists(const std::string& path)
{
	if(path.empty())
	{
		return false;
	}

	if(!std::filesystem::exists(path) || !std::filesystem::is_regular_file(path))
	{
		return false;
	}

	return true;
}
void createDirIfNotExists(const std::string& path)
{
	if(!std::filesystem::exists(path))
	{
		std::filesystem::create_directories(path);
	}
}
std::string loadTextFile(const std::string& path)
{
	std::ifstream file(path);

	if(!file.is_open())
	{
		LOG_ERR("CANNOT LOAD file");
		exit(-1);
	}

	std::stringstream buffer;
	buffer << file.rdbuf();

	return buffer.str();
}
std::vector<BlockData> loadBlockData(const std::string& path)
{
	std::ifstream file(path);
	TRY(file.is_open());

	std::string line;
	std::getline(file, line);

	std::vector<BlockData> data;

	while(std::getline(file, line))
	{
		std::stringstream ss(line);
		std::string celda;
		std::vector<std::string> columns;

		while(std::getline(ss, celda, ','))
		{
			columns.push_back(celda);
		}

		TRY(columns.size() >= 8);

		BlockData block;
		block.name = columns[0];
		block.solid = std::stoi(columns[1]);
		block.faces[CHUNK_FACES_FRONT] = std::stoi(columns[2]);
		block.faces[CHUNK_FACES_BACK] = std::stoi(columns[3]);
		block.faces[CHUNK_FACES_LEFT] = std::stoi(columns[4]);
		block.faces[CHUNK_FACES_RIGHT] = std::stoi(columns[5]);
		block.faces[CHUNK_FACES_TOP] = std::stoi(columns[6]);
		block.faces[CHUNK_FACES_BOTTOM] = std::stoi(columns[7]);
		data.push_back(block);
	}
	return data;
}
ImageArrayData readImageArrayFromFile(const std::string& path, uint32_t ncols, uint32_t nrows)
{
	int width, height, channels;

	unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 4);

	TRY(ncols <= (uint32_t)width && !(width % ncols));
	TRY(nrows <= (uint32_t)height && !(height % nrows));

	if(!data)
	{
		LOG_ERR("CANNOT LOAD IMAGE");
		exit(-1);
	}
	ImageArrayData img;
	img.w = width / ncols;
	img.h = height / nrows;
	img.layers = ncols * nrows;
	img.data.resize(ncols * nrows * img.w * img.h * 4);

	for(uint32_t col = 0; col < ncols; col++)
	{
		for(uint32_t row = 0; row < nrows; row++)
		{
			auto layer = col * img.w * img.h * 4 + row * ncols * img.w * img.h * 4;
			for(uint32_t y = 0; y < img.h; y++)
			{
				for(uint32_t x = 0; x < img.w; x++)
				{
					auto srcIdx = (row * img.h + y) * width * 4 + (col * img.w + x) * 4;
					auto dstIdx = layer + img.w * y * 4 + x * 4;

					img.data[dstIdx] = data[srcIdx];
					img.data[dstIdx + 1] = data[srcIdx + 1];
					img.data[dstIdx + 2] = data[srcIdx + 2];
					img.data[dstIdx + 3] = data[srcIdx + 3];
				}
			}
		}
	}

	stbi_image_free(data);

	return img;
}
std::vector<uint8_t> loadBinaryFile(const std::string& path)
{
	std::ifstream file(path, std::ios::binary);

	TRY(file);

	file.seekg(0, std::ios::end);
	std::streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);

	std::vector<uint8_t> buffer(size);
	TRY(file.read(reinterpret_cast<char*>(buffer.data()), size));

	return buffer;
}
} // namespace vblck
