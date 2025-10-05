#pragma once

#include <memory>
#include <vector>
#include <cstdint>
#include <filesystem>

#include "Core/Logger.h"
#include "Rendering/Vertex.h"
#include "Resources/Resources.h"

namespace OtterEngine {
	class Mesh {
	private:
		std::vector<Vertex> mVertices;
		std::vector<uint32_t> mIndices;
	public:
		Mesh() = default;
		Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices)
			: mVertices(std::move(vertices)), mIndices(std::move(indices)) {
		}

		// Resource concept requires static LoadFromFile and IsValid methods
		static std::shared_ptr<Mesh> LoadFromFile(const std::filesystem::path& path);
		bool IsValid() const { return !mVertices.empty() && !mIndices.empty(); }

		const std::vector<Vertex>& GetVertices()  const { return mVertices; }
		const std::vector<uint32_t>& GetIndices() const { return mIndices; }

		size_t GetVertexCount()		 const { return mVertices.size(); }
		size_t GetIndexCount()		 const { return mIndices.size(); }
		size_t GetVertexBufferSize() const { return mVertices.size() * sizeof(Vertex); }
		size_t GetIndexBufferSize()  const { return mIndices.size() * sizeof(uint32_t); }
	};
}