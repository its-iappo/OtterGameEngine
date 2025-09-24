#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
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

		VkDebugUtilsMessengerEXT mDebugMessenger = VK_NULL_HANDLE;

		void CreateVulkanInstance();
		void CreateSurface();
		void PickPhysicalDevice();
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

		bool CheckValidationLayerSupport();

		void CreateShaderModule(const std::vector<char>& shader, VkShaderModule& shaderModule);

		void CreateGraphicsPipeline();

		// Debugging utilities

		static const char* VkResultToString(VkResult res);
		static const char* VkPhysicalDeviceTypeToString(VkPhysicalDeviceType type);

		void SetupDebugMessenger();

		void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

		VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);

		void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);


	public:
		explicit VulkanRenderer(GLFWwindow* window);
		~VulkanRenderer() override;

		void Init() override; 
		void Clear() override;
		void DrawFrame() override;
	};
}
