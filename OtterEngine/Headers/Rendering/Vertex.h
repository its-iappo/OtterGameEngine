#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"

namespace OtterEngine {
	struct Vertex {
		glm::vec3 mPosition;
		glm::vec3 mNormal;
		glm::vec2 mTexCoord;
		glm::vec3 mColor;

		bool operator==(const Vertex& other) const {
			return mPosition == other.mPosition &&
				mNormal == other.mNormal &&
				mTexCoord == other.mTexCoord &&
				mColor == other.mColor;
		}
	};
}