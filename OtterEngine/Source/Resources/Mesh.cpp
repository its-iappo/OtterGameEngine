#include "OtterPCH.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#define RAPIDOBJ_IMPLEMENTATION
#include "rapidobj.hpp"

#include "Resources/Mesh.h"

// Template specialization for hashing glm vectors
namespace std {
	template<>
	struct hash<glm::vec3> {
		size_t operator()(const glm::vec3& vec) const noexcept {
			return (
				(hash<float>()(vec.x) ^
					(hash<float>()(vec.y) << 1)) >> 1) ^
				(hash<float>()(vec.z) << 1);
		}
	};

	template<>
	struct hash<glm::vec2> {
		size_t operator()(const glm::vec2& vec) const noexcept {
			return (
				(hash<float>()(vec.x) ^
					(hash<float>()(vec.y) << 1)) >> 1);
		}
	};
}

namespace OtterEngine {
	struct VertexHash {
		size_t operator()(const Vertex& vertex) const {
			return (
				(std::hash<glm::vec3>()(vertex.mPosition) ^
					(std::hash<glm::vec3>()(vertex.mNormal) << 1)) >> 1) ^
				(std::hash<glm::vec2>()(vertex.mTexCoord) << 1) ^
				(std::hash<glm::vec3>()(vertex.mColor) << 1);
		}
	};

	std::shared_ptr<Mesh> Mesh::LoadFromFile(const std::filesystem::path& path) {
		std::string pathStr = path.string();

		if (path.extension() == ".obj") {
			rapidobj::Result result = rapidobj::ParseFile(
				pathStr,
				rapidobj::MaterialLibrary::Default(rapidobj::Load::Optional)
			);

			if (result.error && result.error.code) {
				OTTER_CORE_WARNING(
					"[MESH] Failed to load mesh '{}': {}",
					pathStr,
					result.error.code.message()
				);
				return nullptr;
			}

			if (!rapidobj::Triangulate(result)) {
				OTTER_CORE_EXCEPT("[MESH] Failed to triangulate mesh {}", pathStr);
			}

			std::vector<Vertex> vertices;
			std::vector<uint32_t> indices;
			std::unordered_map<Vertex, uint32_t, VertexHash> uniqueVertices;

			for (const auto& shape : result.shapes) {
				for (const auto& index : shape.mesh.indices) {
					Vertex vertex{};

					// Position
					vertex.mPosition = {
						result.attributes.positions[3 * index.position_index + 0],
						result.attributes.positions[3 * index.position_index + 1],
						result.attributes.positions[3 * index.position_index + 2]
					};

					// Normal
					if (index.normal_index >= 0) {
						vertex.mNormal = {
							result.attributes.normals[3 * index.normal_index + 0],
							result.attributes.normals[3 * index.normal_index + 1],
							result.attributes.normals[3 * index.normal_index + 2]
						};
					}
					else {
						vertex.mNormal = { 0.0f, 0.0f, 0.0f };
					}

					// Texture coordinates
					if (index.texcoord_index >= 0) {
						vertex.mTexCoord = {
							result.attributes.texcoords[2 * index.texcoord_index + 0],
							1.0f - result.attributes.texcoords[2 * index.texcoord_index + 1]  // Flip V coord (Vulkan requirement)
						};
					}
					else {
						vertex.mTexCoord = { 0.0f, 0.0f };
					}

					vertex.mColor = { 1.0f, 1.0f, 1.0f };

					if (uniqueVertices.count(vertex) == 0) {
						uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
						vertices.push_back(vertex);
					}

					indices.push_back(uniqueVertices[vertex]);
				}
			}

			OTTER_CORE_LOG(
				"[MESH] Loaded: {} vertices, {} indices from {}",
				vertices.size(),
				indices.size(),
				path.string()
			);

			return std::make_shared<Mesh>(std::move(vertices), std::move(indices));
		}
		return nullptr;
	}
}
