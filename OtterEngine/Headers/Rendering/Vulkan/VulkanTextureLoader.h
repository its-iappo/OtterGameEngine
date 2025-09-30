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

		VkImage mTexture;
		VkImageView mImageView;
		VkDeviceMemory mTextureImageMemory;
		VkSampler mTextureSampler;

	public:
		VulkanTextureLoader() = default;
		VulkanTextureLoader(VkDevice device, VkPhysicalDevice physicalDevice,
			VkCommandPool commandPool, VkQueue graphicsQueue);
		~VulkanTextureLoader() override;

		void LoadTexture(const std::string& path) override;
		void CreateTextureImageView();
		void CreateTextureSampler();

		void ClearResources();

		VkImage GetCurrentImage() const { return mTexture; }
		VkImageView GetCurrentImageView() const { return mImageView; }
		VkSampler GetCurrentSampler() const { return mTextureSampler; }
	};
}