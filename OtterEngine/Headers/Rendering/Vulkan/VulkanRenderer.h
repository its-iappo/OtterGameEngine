#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include <optional>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"

#include "Rendering/Vertex.h"
#include "Rendering/Vulkan/VulkanDebugger.h"

#include "Rendering/IRenderer.h"

namespace OtterEngine {
	class VulkanRenderer : public IRenderer { 
	public:
		struct UniformBufferObject {
			alignas(16) glm::mat4 model;
			alignas(16) glm::mat4 view;
			alignas(16) glm::mat4 proj;
		};

	private:
		GLFWwindow* pWindow;
		VkInstance mInstance = VK_NULL_HANDLE;

		VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;
		VkDevice mDevice = VK_NULL_HANDLE;
		
		VkSurfaceKHR mSurface = VK_NULL_HANDLE;

		static constexpr uint32_t MAX_ONGOING_FRAMES = 2;

#ifdef NDEBUG
		const bool mEnableValidationLayers = false;
#else
		const bool mEnableValidationLayers = true;
#endif
		const std::vector<const char*> mValidationLayers = {
		"VK_LAYER_KHRONOS_validation"
		};
		const std::vector<const char*> mDeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

		VkQueue mGraphicsQueue = VK_NULL_HANDLE;
		VkQueue mPresentQueue = VK_NULL_HANDLE;

		VkSwapchainKHR mSwapchain = VK_NULL_HANDLE;
		std::vector<VkImage> mSwapchainImages;
		std::vector<VkImageView> mSwapchainImageViews;
		std::vector<VkFramebuffer> mSwapchainFramebuffers;
		VkFormat mSwapchainImageFormat;
		VkExtent2D mSwapchainExtent;

		VkRenderPass mRenderPass;
		VkDescriptorSetLayout mDescriptorSetLayout = VK_NULL_HANDLE;
		VkPipelineLayout mPipelineLayout;
		VkPipeline mGraphicsPipeline;

		VkCommandPool mCommandPool = VK_NULL_HANDLE;

		std::vector<VkBuffer> mUniformBuffers;
		std::vector<VkDeviceMemory> mUniformBuffersMemory;
		std::vector<void*> mUniformBuffersMapped;

		VkDescriptorPool mDescriptorPool = VK_NULL_HANDLE;
		std::vector<VkDescriptorSet> mDescriptorSets;
		std::vector<VkCommandBuffer> mCommandBuffers;

		std::vector<VkSemaphore> mImageAvailableSemaphores;
		std::vector<VkSemaphore> mRenderFinishedSemaphores;
		std::vector<VkFence> mActiveFences;
		std::vector<VkFence> mImagesInFlight;
		uint32_t mCurrentFrame = 0;

		bool mIsCleared = false;

		std::unique_ptr<class VulkanTextureLoader> mTextureLoader;
		std::unique_ptr<class VulkanMeshLoader> mMeshLoader;

		VkImage mDepthImage;
		VkDeviceMemory mDepthImageMemory;
		VkImageView mDepthImageView;

		std::unique_ptr<VulkanDebugger> mVkDebugger;

		void CreateVulkanInstance();
		void CreateSurface();
		void PickPhysicalDevice();
		void CreateLogicalDevice();
		void CreateSwapchain();
		void CreateImageViews();
		void CreateRenderPass();
		void CreateFramebuffers();
		void CreateCommandPool();
		void CreateDepthResources();

		void CreateTextureLoader();
		void CreateMeshLoader();
		
		void CreateUniformBuffers();
		void CreateDescriptorPool();
		void CreateDescriptorSets();
		void CreateCommandBuffers();
		void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
		void CreateSyncObjects();

		void CleanupSwapchainResources();
		void RecreateSwapchain();

		VkShaderModule CreateShaderModule(const std::vector<char>& shader) const;

		void CreateGraphicsPipeline();

		void CreateDescriptorSetLayout();

		void UpdateUniformBuffer(uint32_t currentImage);

		// Debugging and utilities
		void SetupDebugMessenger();
	public:
		explicit VulkanRenderer(GLFWwindow* window);
		~VulkanRenderer() override;

		void Init() override; 
		void Clear() override;
		void DrawFrame() override;
	};
}
