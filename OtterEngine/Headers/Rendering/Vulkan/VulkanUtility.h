#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <array>
#include <vector>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "Rendering/Vertex.h"

namespace OtterEngine {
	class VulkanUtility {
	public:
		static uint32_t FindMemoryType(VkPhysicalDevice device, uint32_t filter, VkMemoryPropertyFlags properties);

		static void CreateNewBuffer(VkDevice device, VkPhysicalDevice physDevice, VkDeviceSize size, VkBufferUsageFlags usages, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

		static void CreateVkImage(VkDevice device, VkPhysicalDevice physDevice, VkImage& img, VkDeviceMemory& imgMem, uint32_t w, uint32_t h, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties);

		static VkImageView CreateImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

		static VkCommandBuffer BeginSingleTimeCommandBuffer(VkDevice device, VkCommandPool commandPool);

		static void EndSingleTimeCommandBuffer(VkDevice device, VkCommandBuffer buffer, VkCommandPool pool, VkQueue grQueue);

		static void CopyBuffer(VkDevice device, VkQueue queue, VkCommandPool cmdPool, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

		static void TransitionImageLayout(VkDevice device, VkCommandPool cmdPool, VkImage image, VkQueue grQueue, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

		static void CopyBufferToImage(VkDevice device, VkCommandPool commandPool, VkQueue grQueue, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

		static bool HasStencilComponent(VkFormat format) {
			return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
		}

		static VkFormat FindSupportedFormat(VkPhysicalDevice device, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

		static VkFormat FindDepthFormat(VkPhysicalDevice device);

		static uint32_t FindQueueFamilies(const vk::raii::PhysicalDevice& device);

		static const char* VkResultToString(VkResult res) {
			switch (res) {
			case VK_SUCCESS: return "VK_SUCCESS";
			case VK_NOT_READY: return "VK_NOT_READY";
			case VK_TIMEOUT: return "VK_TIMEOUT";
			case VK_EVENT_SET: return "VK_EVENT_SET";
			case VK_EVENT_RESET: return "VK_EVENT_RESET";
			case VK_INCOMPLETE: return "VK_INCOMPLETE";
			case VK_ERROR_OUT_OF_HOST_MEMORY: return "VK_ERROR_OUT_OF_HOST_MEMORY";
			case VK_ERROR_OUT_OF_DEVICE_MEMORY: return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
			case VK_ERROR_INITIALIZATION_FAILED: return "VK_ERROR_INITIALIZATION_FAILED";
			case VK_ERROR_DEVICE_LOST: return "VK_ERROR_DEVICE_LOST";
			case VK_ERROR_MEMORY_MAP_FAILED: return "VK_ERROR_MEMORY_MAP_FAILED";
			case VK_ERROR_LAYER_NOT_PRESENT: return "VK_ERROR_LAYER_NOT_PRESENT";
			case VK_ERROR_EXTENSION_NOT_PRESENT: return "VK_ERROR_EXTENSION_NOT_PRESENT";
			case VK_ERROR_FEATURE_NOT_PRESENT: return "VK_ERROR_FEATURE_NOT_PRESENT";
			case VK_ERROR_INCOMPATIBLE_DRIVER: return "VK_ERROR_INCOMPATIBLE_DRIVER";
			case VK_ERROR_TOO_MANY_OBJECTS: return "VK_ERROR_TOO_MANY_OBJECTS";
			case VK_ERROR_FORMAT_NOT_SUPPORTED: return "VK_ERROR_FORMAT_NOT_SUPPORTED";
			case VK_ERROR_FRAGMENTED_POOL: return "VK_ERROR_FRAGMENTED_POOL";
			case VK_ERROR_SURFACE_LOST_KHR: return "VK_ERROR_SURFACE_LOST_KHR";
			case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR: return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
			case VK_SUBOPTIMAL_KHR: return "VK_SUBOPTIMAL_KHR";
			case VK_ERROR_OUT_OF_DATE_KHR: return "VK_ERROR_OUT_OF_DATE_KHR";
			case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR: return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
			case VK_ERROR_VALIDATION_FAILED_EXT: return "VK_ERROR_VALIDATION_FAILED_EXT";
			case VK_ERROR_INVALID_SHADER_NV: return "VK_ERROR_INVALID_SHADER_NV";
			default: return "Unknown VkResult";
			}
		}

		static const char* VkPhysicalDeviceTypeToString(const vk::PhysicalDeviceType& type)
		{
			switch (type)
			{
			case vk::PhysicalDeviceType::eIntegratedGpu:
				return "Integrated GPU";
				case vk::PhysicalDeviceType::eDiscreteGpu:
				return "Discrete GPU";
				case vk::PhysicalDeviceType::eVirtualGpu:
				return "Virtual GPU";
			case vk::PhysicalDeviceType::eCpu:
				return "CPU";
			case vk::PhysicalDeviceType::eOther:
			default:
				return "Unknown device type!";
			}
		}

		static bool IsDeviceSuitable(vk::raii::PhysicalDevice device, VkSurfaceKHR surface, std::vector<const char*> deviceExtensions);

		static bool CheckDeviceExtensionSupport(VkPhysicalDevice device, std::vector<const char*> deviceExtensions);

		static SwapchainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);

		static VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

		static VkExtent2D ChooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities, GLFWwindow* window);

		static VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

		static bool CheckValidationLayerSupport(const std::vector<const char*>& validationLayers);
	};

	struct VertexLayout {
		static VkVertexInputBindingDescription GetBindingDescription() {
			VkVertexInputBindingDescription bindingDescription{};
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(Vertex);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			return bindingDescription;
		}

		static std::array<VkVertexInputAttributeDescription, 4> GetAttributeDescriptions() {
			std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions{};
			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT; // vec3
			attributeDescriptions[0].offset = offsetof(Vertex, mPosition);

			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT; // vec3
			attributeDescriptions[1].offset = offsetof(Vertex, mNormal);

			attributeDescriptions[2].binding = 0;
			attributeDescriptions[2].location = 2;
			attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT; // vec2
			attributeDescriptions[2].offset = offsetof(Vertex, mTexCoord);

			attributeDescriptions[3].binding = 0;
			attributeDescriptions[3].location = 3;
			attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT; // vec3
			attributeDescriptions[3].offset = offsetof(Vertex, mColor);

			return attributeDescriptions;
		}
	};

#define VK_CHECK(x)														 \
do {																	 \
    VkResult err = (x);													 \
    if (err != VK_SUCCESS) {											 \
        OTTER_FATAL("[VULKAN FATAL ERROR] {} in {}\nFile: {}\nLine: {}", \
                    (int)err, #x, __FILE__, __LINE__);					 \
    }																	 \
} while (0)

}