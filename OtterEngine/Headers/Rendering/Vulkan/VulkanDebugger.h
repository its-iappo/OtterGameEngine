#pragma once

#include <vulkan/vulkan.h>

namespace OtterEngine {
	class VulkanDebugger {
	private:
		VkDebugUtilsMessengerEXT mDebugMessenger;

		VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator);

		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

	public:
		void SetupDebugMessenger(VkInstance instance);

		void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

		void DestroyDebugUtilsMessengerEXT(VkInstance instance, const VkAllocationCallbacks* pAllocator);

	};
}