#include "Resources/Texture.h"

namespace OtterEngine {
	Texture::Texture(int width, int height, int channels, const std::vector<uint8_t>& pixels)
		: mWidth(width), mHeight(height), mChannels(channels), mPixels(pixels) {}
}