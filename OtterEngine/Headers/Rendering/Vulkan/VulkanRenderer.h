#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <vector>
#include <optional>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include "glm/glm.hpp"

#include "Rendering/IRenderer.h"

namespace OtterEngine {
	class VulkanRenderer : public IRenderer { 
	public:
		struct Vertex {
			glm::vec2 pos;
			glm::vec3 color;

			static VkVertexInputBindingDescription GetBindingDescription() {
				VkVertexInputBindingDescription bindingDescription{};
				bindingDescription.binding = 0;
				bindingDescription.stride = sizeof(Vertex);
				bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
				return bindingDescription;
			}

			static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions() {
				std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);
				attributeDescriptions[0].binding = 0;
				attributeDescriptions[0].location = 0;
				attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT; // vec2
				attributeDescriptions[0].offset = offsetof(Vertex, pos);
				attributeDescriptions[1].binding = 0;
				attributeDescriptions[1].location = 1;
				attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT; // vec3
				attributeDescriptions[1].offset = offsetof(Vertex, color);
				return attributeDescriptions;
			}
		};

		struct UniformBufferObject {
			alignas(16) glm::mat4 model;
			alignas(16) glm::mat4 view;
			alignas(16) glm::mat4 proj;
		};

		struct QueueFamilyIndices {
			std::optional<uint32_t> graphicsFamily = UINT32_MAX;
			std::optional<uint32_t> presentFamily = UINT32_MAX;
			bool IsComplete() const {
				return graphicsFamily != UINT32_MAX && presentFamily != UINT32_MAX;
			}
		};

		struct SwapChainSupportDetails {
			VkSurfaceCapabilitiesKHR capabilities{};
			std::vector<VkSurfaceFormatKHR> formats;
			std::vector<VkPresentModeKHR> presentModes;
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
		const std::vector<const char*> validationLayers = {
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

		// Vertex buffer
		VkBuffer mVertexBuffer = VK_NULL_HANDLE;
		VkDeviceMemory mVertexBufferMemory = VK_NULL_HANDLE;
		const std::vector<Vertex> mVertices = {
			{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
			{{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
			{{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
			{{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
		};

		// Index buffer
		VkBuffer mIndexBuffer = VK_NULL_HANDLE;
		VkDeviceMemory mIndexBufferMemory = VK_NULL_HANDLE;
		const std::vector<uint32_t> mIndices = {
			0, 1, 2, 2, 3, 0
		};

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

		// Debug
		VkDebugUtilsMessengerEXT mDebugMessenger = VK_NULL_HANDLE;

		void CreateVulkanInstance();
		void CreateSurface();
		void PickPhysicalDevice();
		void CreateLogicalDevice();
		void CreateSwapchain();
		void CreateImageViews();
		void CreateRenderPass();
		void CreateFramebuffers();
		void CreateCommandPool();
		void CreateVertexBuffer();
		void CreateIndexBuffer();
		void CreateUniformBuffers();
		void CreateDescriptorPool();
		void CreateDescriptorSets();
		void CreateCommandBuffers();
		void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
		void CreateSyncObjects();

		void CleanupSwapchainResources();
		void RecreateSwapchain();

		void CreateNewBuffer(VkDeviceSize size, VkBufferUsageFlags usages, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

		bool CheckValidationLayerSupport();

		VkShaderModule CreateShaderModule(const std::vector<char>& shader) const;

		void CreateGraphicsPipeline();

		void CreateDescriptorSetLayout();

		void UpdateUniformBuffer(uint32_t currentImage);

		// Debugging utilities

		static const char* VkResultToString(VkResult res);
		static const char* VkPhysicalDeviceTypeToString(VkPhysicalDeviceType type);

		void SetupDebugMessenger();

		void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

		VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);

		void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

		bool IsDeviceSuitable(VkPhysicalDevice device);
		SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
		bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
		QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);

		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

	public:
		explicit VulkanRenderer(GLFWwindow* window);
		~VulkanRenderer() override;

		void Init() override; 
		void Clear() override;
		void DrawFrame() override;
	};
}
