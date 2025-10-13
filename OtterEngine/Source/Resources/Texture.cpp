#include "OtterPCH.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_NO_SIMD // TODO
#include <stb_image.h>

#include "Resources/Texture.h"

namespace OtterEngine {
	Texture::Texture(int width, int height, int channels, std::vector<uint8_t> pixels)
		: mWidth(width), mHeight(height), mChannels(channels), mPixels(std::move(pixels)) { }

	std::shared_ptr<Texture> Texture::LoadFromFile(const std::filesystem::path& path)
	{
		int width, height, channels;
		std::string pathStr = path.string();
		stbi_uc* pixels = stbi_load(pathStr.c_str(), 
			&width,
            &height, 
			&channels,
			STBI_rgb_alpha);

		if (!pixels) {
			const char* reason = stbi_failure_reason();
			OTTER_CORE_ERROR("[TEXTURE] Failed to load '{}': {}", pathStr, reason ? reason : "Unknown error");
			return nullptr;
		}

		const size_t imageSize = static_cast<size_t>(width) * height * 4;
		std::vector<uint8_t> pixelData(imageSize);
        memcpy(pixelData.data(), pixels, imageSize);

        stbi_image_free(pixels);

		OTTER_CORE_LOG("[TEXTURE] Loaded: {}x{} ({} channels -> 4)", width, height, channels);

		return std::make_shared<Texture>(width, height, 4, std::move(pixelData));
	}
}