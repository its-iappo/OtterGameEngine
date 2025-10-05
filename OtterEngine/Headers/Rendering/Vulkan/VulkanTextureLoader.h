#pragma once

#include <filesystem>
#include <vulkan/vulkan.h>

#include "Resources/Texture.h"
#include "Resources/Resources.h"
#include "Utils/ITextureLoader.h"

namespace OtterEngine {
	class VulkanTextureLoader final : public ITextureLoader {
	private:
		VkDevice mDevice;
		VkPhysicalDevice mPhysicalDevice;
		VkCommandPool mCommandPool;
		VkQueue mGraphicsQueue;

		ResourceHandle<Texture> mTextureHandle;
		VkImage mTexture;
		VkImageView mImageView;
		VkDeviceMemory mTextureImageMemory;
		VkSampler mTextureSampler;

	public:
		VulkanTextureLoader() = default;
		VulkanTextureLoader(VkDevice device, VkPhysicalDevice physicalDevice,
			VkCommandPool commandPool, VkQueue graphicsQueue);
		~VulkanTextureLoader() override;

		ResourceHandle<Texture> LoadTexture(const std::filesystem::path& path) override;
		void CreateTextureImageView();
		void CreateTextureSampler();

		void ClearResources();

		const ResourceHandle<Texture>& GetTextureHandle() const { return mTextureHandle; }

		VkImage GetCurrentImage() const { return mTexture; }
		VkImageView GetCurrentImageView() const { return mImageView; }
		VkSampler GetCurrentSampler() const { return mTextureSampler; }

	private:
		void UploadTextureToGPU(const Texture& tex);
	};
}