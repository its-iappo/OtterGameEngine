#pragma once

#include <vector>

namespace OtterEngine {
	class Texture {
	private:
		int mWidth, mHeight, mChannels;
		std::vector<uint8_t> mPixels;

	public:
		Texture() = default;
		Texture(int width, int height, int channels, const std::vector<uint8_t>& pixels);
		
		int GetWidth() const { return mWidth; }

		int GetHeight() const { return mHeight; }

		int GetChannels() const { return mChannels; }

		size_t GetByteSize() const { return mPixels.size(); }

		const std::vector<uint8_t>& GetPixels() const& { return mPixels; }

		std::vector<uint8_t> GetPixels()&& { return std::move(mPixels); }

		const uint8_t* GetData() const { return mPixels.data(); }

		uint8_t* GetData() { return mPixels.data(); }

		bool IsValid() { return mWidth > 0 && mHeight > 0 && !mPixels.empty(); }
	};
}