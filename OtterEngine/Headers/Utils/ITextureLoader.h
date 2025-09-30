#pragma once

#include <string>

#include "Resources/Texture.h"

namespace OtterEngine {
	class ITextureLoader {
	public:
		virtual ~ITextureLoader() = default;
		virtual void LoadTexture(const std::string& path) = 0;
	};
}