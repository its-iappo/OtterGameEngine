#include "Core/Logger.h"

#include "Rendering/Vulkan/VulkanUtility.h"

namespace OtterEngine {
	void VulkanUtility::CreateNewBuffer(VkDevice device,VkPhysicalDevice physDevice,  VkDeviceSize size, VkBufferUsageFlags usages, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
	{
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usages;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
			OTTER_CORE_EXCEPT("[VULKAN UTILITY] Failed to create buffer!");
		}

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = FindMemoryType(physDevice, memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
			OTTER_CORE_EXCEPT("[VULKAN UTILITY] Failed to allocate buffer memory!");
		}

		vkBindBufferMemory(device, buffer, bufferMemory, 0);
	}

	void VulkanUtility::CreateVkImage(VkDevice device, VkPhysicalDevice physDevice, VkImage& img, VkDeviceMemory& imgMem, uint32_t w, uint32_t h, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties)
	{
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = w;
		imageInfo.extent.height = h;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;

		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		imageInfo.usage = usage;

		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.flags = 0;

		if (vkCreateImage(device, &imageInfo, nullptr, &img) != VK_SUCCESS) {
			OTTER_CORE_EXCEPT("[VULKAN UTILITY] Failed to create image!");
		}

		VkMemoryRequirements memReq{};

		vkGetImageMemoryRequirements(device, img, &memReq);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memReq.size;
		allocInfo.memoryTypeIndex = FindMemoryType(physDevice, memReq.memoryTypeBits, properties);

		if (vkAllocateMemory(device, &allocInfo, nullptr, &imgMem) != VK_SUCCESS) {
			OTTER_CORE_EXCEPT("[VULKAN UTILITY] Failed to allocate image memory!");
		}

		if (vkBindImageMemory(device, img, imgMem, 0) != VK_SUCCESS) {
			OTTER_CORE_EXCEPT("[VULKAN UTILITY] Failed to bind image memory!");
		}
	}

	uint32_t VulkanUtility::FindMemoryType(VkPhysicalDevice device, uint32_t filter, VkMemoryPropertyFlags properties) {
		OTTER_CORE_LOG("[VULKAN UTILITY] FindMemoryType: filter=0x{:x}, requestedProps=0x{:x} scanning...", filter, properties);

		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(device, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			if (filter & (1 << i) && // Is the current bit set to 1? If so, this memory type might be suitable.
				(memProperties.memoryTypes[i].propertyFlags & properties) == properties) // Does the GPU memory have the required properties?
			{
				OTTER_CORE_LOG("[VULKAN UTILITY] Found suitable memory type with properties: 0x{:x}", memProperties.memoryTypes[i].propertyFlags);
				return i;
			}
		}

		OTTER_CORE_EXCEPT("[VULKAN UTILITY] Failed to find suitable memory type!");
	}

	VkCommandBuffer VulkanUtility::BeginSingleTimeCommandBuffer(VkDevice device, VkCommandPool commandPool)
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = commandPool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		if (vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer) != VK_SUCCESS) {
			OTTER_CORE_EXCEPT("[VULKAN UTILITY] Failed to allocate command buffers while copying buffer!");
		}

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
			OTTER_CORE_EXCEPT("[VULKAN UTILITY] Failed to begin command buffer while copying buffer!");
		}

		return commandBuffer;
	}

	void VulkanUtility::EndSingleTimeCommandBuffer(VkDevice device, VkCommandBuffer buffer, VkCommandPool pool, VkQueue grQueue)
	{
		if (vkEndCommandBuffer(buffer) != VK_SUCCESS) {
			OTTER_CORE_EXCEPT("[VULKAN UTILITY] Failed to end command buffer while copying buffer!");
		}

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &buffer;

		if (vkQueueSubmit(grQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
			OTTER_CORE_EXCEPT("[VULKAN UTILITY] Failed to submit command buffer while copying buffer!");
		}

		if (vkQueueWaitIdle(grQueue) != VK_SUCCESS) {
			OTTER_CORE_EXCEPT("[VULKAN UTILITY] Failed to wait for graphics queue while copying buffer!");
		}

		vkFreeCommandBuffers(device, pool, 1, &buffer);
	}

	void VulkanUtility::CopyBuffer(VkDevice device, VkQueue queue, VkCommandPool cmdPool, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
		VkCommandBuffer commandBuffer = BeginSingleTimeCommandBuffer(device, cmdPool);

		VkBufferCopy copyRegion{};
		copyRegion.size = size;
		
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
		
		EndSingleTimeCommandBuffer(device, commandBuffer, cmdPool, queue);
	}

	void VulkanUtility::TransitionImageLayout(VkDevice device, VkCommandPool cmdPool, VkImage image, VkQueue grQueue, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
	{
		VkCommandBuffer commandBuffer = BeginSingleTimeCommandBuffer(device, cmdPool);

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;

		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

		barrier.image = image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = 0;

		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;

		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else {
			throw std::invalid_argument("unsupported layout transition!");
		}

		vkCmdPipelineBarrier(
			commandBuffer,
			sourceStage, destinationStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		EndSingleTimeCommandBuffer(device, commandBuffer, cmdPool, grQueue);
	}
	
	void VulkanUtility::CopyBufferToImage(VkDevice device, VkCommandPool commandPool, VkQueue grQueue, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
	{
		VkCommandBuffer commandBuffer = BeginSingleTimeCommandBuffer(device, commandPool);

		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;
		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;
		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = { width, height, 1 };

		vkCmdCopyBufferToImage(commandBuffer, buffer,
			image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,&region);

		EndSingleTimeCommandBuffer(device, commandBuffer, commandPool, grQueue);
	}
}
