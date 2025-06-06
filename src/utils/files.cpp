#include "files.hpp"
#include "logger.hpp"
#include "stb_image/stb_image.h"
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
} // namespace vblck
