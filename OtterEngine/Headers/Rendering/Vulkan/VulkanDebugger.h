#pragma once

#include <vulkan/vulkan_raii.hpp>
#include <vulkan/vulkan_enums.hpp>

namespace OtterEngine {
	class VulkanDebugger {
	private:
		vk::raii::DebugUtilsMessengerEXT mDebugMessenger;

		VkResult CreateDebugUtilsMessengerEXT(vk::raii::Instance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator);

		static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity, vk::DebugUtilsMessageTypeFlagsEXT messageType, const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

	public:
		void SetupDebugMessenger(const vk::raii::Instance& instance);

		void DestroyDebugUtilsMessengerEXT(vk::raii::Instance instance, const VkAllocationCallbacks* pAllocator);

		VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity, vk::DebugUtilsMessageTypeFlagsEXT messageType, const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

	};
}