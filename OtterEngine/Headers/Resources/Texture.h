#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Resources/Resources.h"

namespace OtterEngine {
	class Texture {
	private:
		int mWidth = 0;
		int mHeight = 0; 
		int mChannels = 0;
		std::vector<uint8_t> mPixels;

	public:
		Texture() = default;

		Texture(int width, int height, int channels, const std::vector<uint8_t>& pixels);
		
		static std::shared_ptr<Texture> LoadFromFile(const std::string& path);

		bool IsValid() const { return mWidth > 0 && mHeight > 0 && !mPixels.empty(); }
		
		int GetWidth() const { return mWidth; }

		int GetHeight() const { return mHeight; }

		int GetChannels() const { return mChannels; }

		size_t GetByteSize() const { return mPixels.size(); }

		const std::vector<uint8_t>& GetPixels() const& { return mPixels; }

		std::vector<uint8_t> GetPixels()&& { return std::move(mPixels); }

		const uint8_t* GetData() const { return mPixels.data(); }

		uint8_t* GetData() { return mPixels.data(); }
	};
}