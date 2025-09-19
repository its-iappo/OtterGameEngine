#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <vector>

#include "Rendering/IRenderer.h"

namespace OtterEngine {

	class VulkanRenderer : public IRenderer {
	private:
		GLFWwindow* pWindow;
		VkInstance mInstance = VK_NULL_HANDLE;
		VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;
		VkDevice mDevice = VK_NULL_HANDLE;
		VkSurfaceKHR mSurface = VK_NULL_HANDLE;

		VkSwapchainKHR mSwapchain = VK_NULL_HANDLE;
		VkFormat mSwapchainImageFormat;
		VkExtent2D mSwapchainExtent;
		std::vector<VkImage> mSwapchainImages;
		std::vector<VkImageView> mSwapchainImageViews;
		std::vector<VkFramebuffer> mSwapchainFramebuffers;

		uint32_t mGraphicsFamily = UINT32_MAX;
		uint32_t mPresentFamily = UINT32_MAX;

		VkQueue mGraphicsQueue = VK_NULL_HANDLE;
		VkQueue mPresentQueue = VK_NULL_HANDLE;

		VkRenderPass mRenderPass;
		VkCommandPool mCommandPool = VK_NULL_HANDLE;

		VkPipeline mGraphicsPipeline;
		VkPipelineLayout mPipelineLayout;

		std::vector<VkCommandBuffer> mCommandBuffers;
		std::vector<VkSemaphore> mImageAvailableSemaphores;
		std::vector<VkSemaphore> mImageDoneSemaphores;
		std::vector<VkFence> mOnGoingFences;

		uint32_t mCurrentFrame = 0;

		void Create();
		void CreateSurface();
		void GetFirstAvailablePhysicalDevice();
		void CreateLogicalDevice();
		void CreateCommandPool();
		void CreateSwapchain();
		void CreateImageViews();
		void CreateRenderPass();
		void CreateFramebuffers();
		void CreateCommandBuffers();
		void CreateSyncObjects();

		void CleanupSwapchainResources();
		void RecreateSwapchain();

		VkShaderModule CreateShaderModule(const std::vector<char>& shader);

		void CreateGraphicsPipeline();

	public:
		explicit VulkanRenderer(GLFWwindow* window);
		~VulkanRenderer() override;

		void Init() override; 
		void Clear() override;
		void DrawFrame() override;
	};
}
