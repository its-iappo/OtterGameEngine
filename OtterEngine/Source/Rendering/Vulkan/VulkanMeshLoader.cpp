#include <vector>

#include "Core/Logger.h"
#include "Utils/PathFormat.h"
#include "Resources/Mesh.h"
#include "Rendering/Vertex.h"
#include "Resources/Resources.h"
#include "Rendering/Vulkan/VulkanUtility.h"

#include "Rendering/Vulkan/VulkanMeshLoader.h"

namespace OtterEngine {
	VulkanMeshLoader::VulkanMeshLoader(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue) :
		mDevice(device), mPhysicalDevice(physicalDevice), mCommandPool(commandPool), mGraphicsQueue(graphicsQueue) {
	}

	VulkanMeshLoader::~VulkanMeshLoader()
	{
		ClearResources();
	}

	ResourceHandle<Mesh> VulkanMeshLoader::LoadMesh(const std::filesystem::path& path)
	{
		ClearResources();

		mMeshHandle = Resources::Load<Mesh>(path);

		if (!mMeshHandle || !mMeshHandle->IsValid()) {
			OTTER_CORE_ERROR("[VULKAN MESH LOADER] Failed to load mesh: {}", path);
			return ResourceHandle<Mesh>();
		}

		UploadMeshToGPU(*mMeshHandle);

		OTTER_CORE_LOG("[VULKAN MESH LOADER] Mesh loaded and uploaded to GPU: {} ({} vertices, {} indices)",
			mMeshHandle.GetPath(),
			mMeshHandle->GetVertexCount(),
			mMeshHandle->GetIndexCount());

		return mMeshHandle;
	}

	void VulkanMeshLoader::UploadMeshToGPU(const Mesh& mesh)
	{
		CreateVertexBuffer(mesh.GetVertices());
		CreateIndexBuffer(mesh.GetIndices());
	}

	void VulkanMeshLoader::CreateVertexBuffer(const std::vector<Vertex>& vertices)
	{
		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

		OTTER_CORE_LOG("[VULKAN MESH LOADER] [VERT BUF CREATION] Vertices: {} Buffer size: {} Data ptr: {}", vertices.size(), bufferSize, (void*)vertices.data());

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		VulkanUtility::CreateNewBuffer(mDevice, mPhysicalDevice, bufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(mDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, vertices.data(), (size_t)bufferSize);
		vkUnmapMemory(mDevice, stagingBufferMemory);

		VulkanUtility::CreateNewBuffer(mDevice, mPhysicalDevice, bufferSize,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			mVertexBuffer, mVertexBufferMemory);

		VulkanUtility::CopyBuffer(mDevice, mGraphicsQueue, mCommandPool,
			stagingBuffer, mVertexBuffer, bufferSize);

		vkDestroyBuffer(mDevice, stagingBuffer, nullptr);
		vkFreeMemory(mDevice, stagingBufferMemory, nullptr);
	}

	void VulkanMeshLoader::CreateIndexBuffer(const std::vector<uint32_t>& indices)
	{
		VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		VulkanUtility::CreateNewBuffer(mDevice, mPhysicalDevice, bufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(mDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, indices.data(), (size_t)bufferSize);
		vkUnmapMemory(mDevice, stagingBufferMemory);

		VulkanUtility::CreateNewBuffer(mDevice, mPhysicalDevice, bufferSize,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			mIndexBuffer, mIndexBufferMemory);

		VulkanUtility::CopyBuffer(mDevice, mGraphicsQueue, mCommandPool,
			stagingBuffer, mIndexBuffer, bufferSize);

		vkDestroyBuffer(mDevice, stagingBuffer, nullptr);
		vkFreeMemory(mDevice, stagingBufferMemory, nullptr);
	}

	void VulkanMeshLoader::ClearResources()
	{
		if (mVertexBuffer != VK_NULL_HANDLE) {
			vkDestroyBuffer(mDevice, mVertexBuffer, nullptr);
			mVertexBuffer = VK_NULL_HANDLE;
		}
		if (mVertexBufferMemory != VK_NULL_HANDLE) {
			vkFreeMemory(mDevice, mVertexBufferMemory, nullptr);
			mVertexBufferMemory = VK_NULL_HANDLE;
		}
		if (mIndexBuffer != VK_NULL_HANDLE) {
			vkDestroyBuffer(mDevice, mIndexBuffer, nullptr);
			mIndexBuffer = VK_NULL_HANDLE;
		}
		if (mIndexBufferMemory != VK_NULL_HANDLE) {
			vkFreeMemory(mDevice, mIndexBufferMemory, nullptr);
			mIndexBufferMemory = VK_NULL_HANDLE;
		}

		mMeshHandle = ResourceHandle<Mesh>();
	}
}