#pragma once

#include <filesystem>

#include "Resources/Mesh.h"
#include "Resources/Resources.h"

namespace OtterEngine {
	class IMeshLoader {
	public:
		virtual ~IMeshLoader() = default;
		virtual ResourceHandle<Mesh> LoadMesh(const std::filesystem::path& path) = 0;
	};
}