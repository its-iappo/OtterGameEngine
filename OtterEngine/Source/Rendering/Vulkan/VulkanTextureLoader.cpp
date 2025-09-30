#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "Core/Logger.h"
#include "Rendering/Vulkan/VulkanUtility.h"

#include "Rendering/Vulkan/VulkanTextureLoader.h"

namespace OtterEngine {
	VulkanTextureLoader::VulkanTextureLoader(VkDevice device, VkPhysicalDevice physicalDevice,
		VkCommandPool commandPool, VkQueue graphicsQueue)
	{
		mDevice = device;
		mPhysicalDevice = physicalDevice;
		mCommandPool = commandPool;
		mGraphicsQueue = graphicsQueue;
		mTexture = VK_NULL_HANDLE;
		mTextureImageMemory = VK_NULL_HANDLE;
		mImageView = VK_NULL_HANDLE;
		mTextureSampler = VK_NULL_HANDLE;
	}

	VulkanTextureLoader::~VulkanTextureLoader()
	{
		ClearResources();
	}

	void VulkanTextureLoader::LoadTexture(const std::string& path)
	{
		int w, h, channels;
		stbi_uc* pixels = stbi_load(path.c_str(), &w, &h,
			&channels, STBI_rgb_alpha);

		if (!pixels) {
			OTTER_CORE_EXCEPT("[VULKAN TEXTURE LOADER] Failed to load texture image!");
		}

		VkDeviceSize imageSize = w * h * 4;

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		VulkanUtility::CreateNewBuffer(mDevice, mPhysicalDevice, imageSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(mDevice, stagingBufferMemory, 0, imageSize, 0, &data);
		memcpy(data, pixels, static_cast<size_t>(imageSize));
		vkUnmapMemory(mDevice, stagingBufferMemory);

		stbi_image_free(pixels);

		VulkanUtility::CreateVkImage(mDevice, mPhysicalDevice,
			mTexture, mTextureImageMemory, w, h,
			VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		VulkanUtility::TransitionImageLayout(mDevice, mCommandPool,
			mTexture, mGraphicsQueue,
			VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		VulkanUtility::CopyBufferToImage(mDevice, mCommandPool,
			mGraphicsQueue, stagingBuffer,
			mTexture, w, h);

		VulkanUtility::TransitionImageLayout(mDevice, mCommandPool,
			mTexture, mGraphicsQueue,
			VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		vkDestroyBuffer(mDevice, stagingBuffer, nullptr);
		vkFreeMemory(mDevice, stagingBufferMemory, nullptr);
	}

	void VulkanTextureLoader::CreateTextureImageView()
	{
		mImageView = VulkanUtility::CreateImageView(mDevice ,mTexture, VK_FORMAT_R8G8B8A8_SRGB);
	}

	void VulkanTextureLoader::CreateTextureSampler()
	{
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;

		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

		samplerInfo.anisotropyEnable = VK_TRUE;

		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(mPhysicalDevice, &properties);
		samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;

		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;

		if (vkCreateSampler(mDevice, &samplerInfo, nullptr, &mTextureSampler)!= VK_SUCCESS) {
			OTTER_CORE_EXCEPT("[VULKAN TEXTURE LOADER] Failed to create texture sampler!");
		}


	}

	void VulkanTextureLoader::ClearResources() {
		vkDestroySampler(mDevice, mTextureSampler, nullptr);
		vkDestroyImageView(mDevice, mImageView, nullptr);
		vkDestroyImage(mDevice, mTexture, nullptr);
		vkFreeMemory(mDevice, mTextureImageMemory, nullptr);
	}
}