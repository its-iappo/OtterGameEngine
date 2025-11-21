#include "OtterPCH.h"

#include "Utils/PathFormat.h"
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

	ResourceHandle<Texture> VulkanTextureLoader::LoadTexture(const std::filesystem::path& path)
	{
		ClearResources();
		mTextureHandle = Resources::Load<Texture>(path);

		if (!mTextureHandle) {
			OTTER_CORE_ERROR("[VULKAN TEXTURE LOADER] Failed to load texture: {}", path);
			return ResourceHandle<Texture>();
		}

		UploadTextureToGPU(*mTextureHandle);

		CreateTextureImageView();
		CreateTextureSampler();

		OTTER_CORE_LOG("[VULKAN TEXTURE LOADER] Texture loaded and uploaded to GPU: {}",
			mTextureHandle.GetPath());

		return mTextureHandle;
	}

	void VulkanTextureLoader::CreateTextureImageView()
	{
		mImageView = VulkanUtility::CreateImageView(mDevice ,mTexture, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
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
			OTTER_CORE_CRITICAL("[VULKAN TEXTURE LOADER] Failed to create texture sampler!");
		}
	}

	void VulkanTextureLoader::UploadTextureToGPU(const Texture& tex)
	{
		VkDeviceSize imageSize = tex.GetByteSize();
		const uint8_t* pixels = tex.GetData();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		VulkanUtility::CreateNewBuffer(mDevice, mPhysicalDevice, imageSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(mDevice, stagingBufferMemory, 0, imageSize, 0, &data);
		memcpy(data, pixels, static_cast<size_t>(imageSize));
		vkUnmapMemory(mDevice, stagingBufferMemory);

		uint32_t width = static_cast<uint32_t>(tex.GetWidth());
		uint32_t height = static_cast<uint32_t>(tex.GetHeight());

		VulkanUtility::CreateVkImage(mDevice, mPhysicalDevice,
			mTexture, mTextureImageMemory,
			width, height,
			VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		VulkanUtility::TransitionImageLayout(mDevice, mCommandPool,
			mTexture, mGraphicsQueue,
			VK_FORMAT_R8G8B8A8_SRGB, 
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		VulkanUtility::CopyBufferToImage(mDevice, mCommandPool,
			mGraphicsQueue, stagingBuffer,
			mTexture, width, height);

		VulkanUtility::TransitionImageLayout(mDevice, mCommandPool,
			mTexture, mGraphicsQueue,
			VK_FORMAT_R8G8B8A8_SRGB, 
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		vkDestroyBuffer(mDevice, stagingBuffer, nullptr);
		vkFreeMemory(mDevice, stagingBufferMemory, nullptr);
	}

	void VulkanTextureLoader::ClearResources() {
		if (mDevice == VK_NULL_HANDLE) return;

		if (mTextureSampler != VK_NULL_HANDLE) {
			vkDestroySampler(mDevice, mTextureSampler, nullptr);
			mTextureSampler = VK_NULL_HANDLE;
		}
		if (mImageView != VK_NULL_HANDLE) {
			vkDestroyImageView(mDevice, mImageView, nullptr);
			mImageView = VK_NULL_HANDLE;
		}
		if (mTexture != VK_NULL_HANDLE) {
			vkDestroyImage(mDevice, mTexture, nullptr);
			mTexture = VK_NULL_HANDLE;
		}
		if (mTextureImageMemory != VK_NULL_HANDLE) {
			vkFreeMemory(mDevice, mTextureImageMemory, nullptr);
			mTextureImageMemory = VK_NULL_HANDLE;
		}
	}
}