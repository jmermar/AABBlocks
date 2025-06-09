#include "files.hpp"
#include "errors.hpp"
#include "logger.hpp"
#include "stb_image/stb_image.h"

#include <cstdint>
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
