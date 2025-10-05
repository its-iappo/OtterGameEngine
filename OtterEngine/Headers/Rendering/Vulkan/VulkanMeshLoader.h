#pragma once

#include <vector>
#include <filesystem>
#include <vulkan/vulkan.h>
#include <cstdint>
#include <optional>
#include "Core/Logger.h"

#include "Resources/Mesh.h"
#include "Rendering/Vertex.h"
#include "Resources/Resources.h"

#include "Utils/IMeshLoader.h"

namespace OtterEngine {
	class VulkanMeshLoader final : public IMeshLoader {
	private:
		VkDevice mDevice;
		VkPhysicalDevice mPhysicalDevice;
		VkCommandPool mCommandPool;
		VkQueue mGraphicsQueue;

		ResourceHandle<Mesh> mMeshHandle;

		VkBuffer mVertexBuffer = VK_NULL_HANDLE;
		VkDeviceMemory mVertexBufferMemory = VK_NULL_HANDLE;

		VkBuffer mIndexBuffer = VK_NULL_HANDLE;
		VkDeviceMemory mIndexBufferMemory = VK_NULL_HANDLE;

	public:
		VulkanMeshLoader() = default;

		VulkanMeshLoader(VkDevice device, VkPhysicalDevice physicalDevice,
			VkCommandPool commandPool, VkQueue graphicsQueue);

		~VulkanMeshLoader() override;

		ResourceHandle<Mesh> LoadMesh(const std::filesystem::path& path) override;

		VkBuffer GetVertexBuffer() const { return mVertexBuffer; }
		VkBuffer GetIndexBuffer()  const { return mIndexBuffer; }
		uint32_t GetIndexCount()   const { return mMeshHandle ? mMeshHandle->GetIndexCount() : 0; }

		const ResourceHandle<Mesh>& GetMeshHandle() const { return mMeshHandle; }

		void ClearResources();

	private:
		void UploadMeshToGPU	(const Mesh& mesh);
		void CreateVertexBuffer (const std::vector<Vertex>& vertices);
		void CreateIndexBuffer  (const std::vector<uint32_t>& indices);
	};
}