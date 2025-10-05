#pragma once

#include <functional>

#include "Resources/Texture.h"
#include "Resources/Resources.h"

namespace OtterEngine {
	class ITextureLoader {
	public:
		virtual ~ITextureLoader() = default;
		virtual ResourceHandle<Texture> LoadTexture(const std::filesystem::path& path) = 0;
	};
}