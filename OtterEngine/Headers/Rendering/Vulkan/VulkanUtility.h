#pragma once

#include <vulkan/vulkan.h>

namespace OtterEngine {
	class VulkanUtility {
	public:
		static uint32_t FindMemoryType(VkPhysicalDevice device, uint32_t filter, VkMemoryPropertyFlags properties);

		static void CreateNewBuffer(VkDevice device, VkPhysicalDevice physDevice, VkDeviceSize size, VkBufferUsageFlags usages, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

		static void CreateVkImage(VkDevice device, VkPhysicalDevice physDevice, VkImage& img, VkDeviceMemory& imgMem, uint32_t w, uint32_t h, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties);

		static VkImageView CreateImageView(VkDevice device, VkImage image, VkFormat format);

		static VkCommandBuffer BeginSingleTimeCommandBuffer(VkDevice device, VkCommandPool commandPool);

		static void EndSingleTimeCommandBuffer(VkDevice device, VkCommandBuffer buffer, VkCommandPool pool, VkQueue grQueue);

		static void CopyBuffer(VkDevice device, VkQueue queue, VkCommandPool cmdPool, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

		static void TransitionImageLayout(VkDevice device, VkCommandPool cmdPool, VkImage image, VkQueue grQueue, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

		static void CopyBufferToImage(VkDevice device, VkCommandPool commandPool, VkQueue grQueue, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	};
}
