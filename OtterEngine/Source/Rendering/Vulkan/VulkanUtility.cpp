#include "OtterPCH.h"

#include "Rendering/Vulkan/VulkanUtility.h"

namespace OtterEngine {
	void VulkanUtility::CreateNewBuffer(VkDevice device, VkPhysicalDevice physDevice, VkDeviceSize size, VkBufferUsageFlags usages, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
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

	VkImageView VulkanUtility::CreateImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) {
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		VkImageView imageView;
		if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
			OTTER_CORE_EXCEPT("[VULKAN UTILITY] Failed to create image view!");
		}

		return imageView;
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

		if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			if (HasStencilComponent(format)) {
				barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
			}
		}
		else {
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		}

		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = 0;

		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;


		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
			newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {

			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
			newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {

			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
			newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {

			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
				VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		}
		else {
			OTTER_CORE_EXCEPT("[VULKAN UTILITY] Unsupported layout transition!");
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
			1, &region);

		EndSingleTimeCommandBuffer(device, commandBuffer, commandPool, grQueue);
	}

	VkFormat VulkanUtility::FindSupportedFormat(VkPhysicalDevice device, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
	{
		for (VkFormat format : candidates) {
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(device, format, &props);

			if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
				return format;
			}
			else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
				return format;
			}
		}

		OTTER_CORE_EXCEPT("[VULKAN RENDERER] Failed to find supported format!");
	}

	VkFormat VulkanUtility::FindDepthFormat(VkPhysicalDevice device) {
		return FindSupportedFormat(device,
			{ VK_FORMAT_D32_SFLOAT,
			VK_FORMAT_D32_SFLOAT_S8_UINT,
			VK_FORMAT_D24_UNORM_S8_UINT },
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
	};

	QueueFamilyIndices VulkanUtility::FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
		
		QueueFamilyIndices indices;

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (const auto& queueFamily : queueFamilies) {
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				indices.mGraphicsFamily = i;
			}

			VkBool32 presentSupport = false;

			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

			if (presentSupport) {
				indices.mPresentFamily = i;
			}

			if (indices.IsComplete()) {
				break;
			}

			i++;
		}

		return indices;
	}

	bool VulkanUtility::IsDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface, std::vector<const char*> deviceExtensions)
	{
		QueueFamilyIndices indices = VulkanUtility::FindQueueFamilies(device, surface);

		bool extensionsSupported = CheckDeviceExtensionSupport(device, deviceExtensions);

		bool swapChainAdequate = false;
		if (extensionsSupported) {
			SwapchainSupportDetails swapChainSupport = QuerySwapChainSupport(device, surface);
			swapChainAdequate = !swapChainSupport.mFormats.empty() && !swapChainSupport.mPresentModes.empty();
		}

		VkPhysicalDeviceFeatures supportedFeatures{};
		vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

		return indices.IsComplete()
			&& extensionsSupported
			&& swapChainAdequate
			&& supportedFeatures.samplerAnisotropy;
	}

	bool VulkanUtility::CheckDeviceExtensionSupport(VkPhysicalDevice device, std::vector<const char*> deviceExtensions) {
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

		for (const auto& extension : availableExtensions) {
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}

	SwapchainSupportDetails VulkanUtility::QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
		SwapchainSupportDetails details;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.mCapabilities);
		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
		if (formatCount != 0) {
			details.mFormats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.mFormats.data());
		}
		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
		if (presentModeCount != 0) {
			details.mPresentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.mPresentModes.data());
		}
		return details;
	}

	VkSurfaceFormatKHR VulkanUtility::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
	{
		for (const auto& availableFormat : availableFormats) {
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				return availableFormat;
			}
		}

		return availableFormats[0];
	}

	VkExtent2D VulkanUtility::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window) {
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
			return capabilities.currentExtent;
		}
		else {
			int width, height;
			glfwGetFramebufferSize(window, &width, &height);

			VkExtent2D actualExtent = {
				static_cast<uint32_t>(width),
				static_cast<uint32_t>(height)
			};

			actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

			return actualExtent;
		}
	}

	VkPresentModeKHR VulkanUtility::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
	{
		for (const auto& availablePresentMode : availablePresentModes) {
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
				return availablePresentMode;
			}
		}

		return VK_PRESENT_MODE_FIFO_KHR;
	}

	bool VulkanUtility::CheckValidationLayerSupport(const std::vector<const char*>& validationLayers) {
		uint32_t layerCount = 0;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
		std::vector<VkLayerProperties> available(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, available.data());

		OTTER_CORE_LOG("[VULKAN UTILITY] ====================");
		OTTER_CORE_LOG("[VULKAN UTILITY] Available GPU layers:");
		for (const auto& prop : available) {
			OTTER_CORE_LOG("[VULKAN UTILITY] {}", prop.layerName);
		}
		OTTER_CORE_LOG("[VULKAN UTILITY] ====================");

		for (const char* layerName : validationLayers) {
			bool found = false;
			for (const auto& prop : available) {
				if (strcmp(prop.layerName, layerName) == 0) { found = true; break; }
			}
			if (!found) return false;
		}
		return true;
	}
}
