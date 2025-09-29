#pragma once

#include <string>
#include <vulkan/vulkan.h>

#include "Utils/ITextureLoader.h"

namespace OtterEngine {
	class VulkanTextureLoader : public ITextureLoader {
	private:
		VkDevice mDevice;
		VkPhysicalDevice mPhysicalDevice;
		VkCommandPool mCommandPool;
		VkQueue mGraphicsQueue;
		VkImage mImage;
		VkDeviceMemory mTextureImageMemory;

	public:
		VulkanTextureLoader() = default;
		VulkanTextureLoader(VkDevice device, VkPhysicalDevice physicalDevice,
			VkCommandPool commandPool, VkQueue graphicsQueue);
		~VulkanTextureLoader() override;

		void LoadTexture(Texture& texture, const std::string& path) override;

		void ClearResources();

		VkImage GetCurrentImage() const { return mImage; }
	};
}