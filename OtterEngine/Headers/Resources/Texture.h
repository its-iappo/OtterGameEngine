#pragma once

#include <memory>
#include <vector>
#include <cstdint>
#include <filesystem>

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

		Texture(int width, int height, int channels, std::vector<uint8_t> pixels);

		// Resource concept requires static LoadFromFile and IsValid methods

		static std::shared_ptr<Texture> LoadFromFile(const std::filesystem::path& path);
		bool IsValid() const { return mWidth > 0 && mHeight > 0 && !mPixels.empty(); }
		
		constexpr int	 GetWidth()	   const noexcept { return mWidth; }
		constexpr int	 GetHeight()   const noexcept { return mHeight; }
		constexpr int	 GetChannels() const noexcept { return mChannels; }
		constexpr size_t GetByteSize() const noexcept { return mPixels.size(); }

		const std::vector<uint8_t>& GetPixels() const& noexcept { return mPixels; }
		std::vector<uint8_t>		GetPixels()&&	   noexcept { return std::move(mPixels); }

		const uint8_t* GetData() const noexcept { return mPixels.data(); }
		uint8_t*	   GetData()				{ return mPixels.data(); }
	};
}