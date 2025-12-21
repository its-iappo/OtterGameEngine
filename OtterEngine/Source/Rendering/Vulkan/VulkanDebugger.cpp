#include "OtterPCH.h"

#define VULKAN_HPP_NO_EXCEPTIONS
#define VULKAN_HPP_NO_CONSTRUCTORS
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

#include "Rendering/Vulkan/VulkanDebugger.h"

namespace OtterEngine {
	void VulkanDebugger::SetupDebugMessenger(const vk::raii::Instance& instance) {

		vk::DebugUtilsMessageSeverityFlagsEXT sevFlags(
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);

		vk::DebugUtilsMessageTypeFlagsEXT msgTypeFlags(
			vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
			vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
			vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);

		vk::DebugUtilsMessengerCreateInfoEXT  debugUtilsMessengerCreateInfoEXT{
			 .messageSeverity = sevFlags,
			 .messageType = msgTypeFlags,
			 .pfnUserCallback = &debugCallback };

		mDebugMessenger = instance.createDebugUtilsMessengerEXT(debugUtilsMessengerCreateInfoEXT);
	}

	VkResult VulkanDebugger::CreateDebugUtilsMessengerEXT(vk::raii::Instance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator)
	{
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr) {
			return func(instance, pCreateInfo, pAllocator, &mDebugMessenger);
		}
		else {
			OTTER_CORE_WARNING("[VULKAN RENDERER] Debug Utils Messenger extension not present, unable to create it!");
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	void VulkanDebugger::DestroyDebugUtilsMessengerEXT(vk::raii::Instance instance, const VkAllocationCallbacks* pAllocator)
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr) {
			func(instance, mDebugMessenger, pAllocator);
			mDebugMessenger = VK_NULL_HANDLE;
		}
	}

	VKAPI_ATTR vk::Bool32 VKAPI_CALL VulkanDebugger::debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity, vk::DebugUtilsMessageTypeFlagsEXT messageType, const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
		
		if (severity <= vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo) {
			OTTER_CORE_LOG("[VULKAN RENDERER DEBUG LOG CBK]\n{}", pCallbackData->pMessage);
		}
		else if (severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning) {
			OTTER_CORE_WARNING("[VULKAN RENDERER DEBUG WARNING CBK]\n{}", pCallbackData->pMessage);
		}
		else {
			OTTER_CORE_ERROR("[VULKAN RENDERER DEBUG ERROR CBK]\n{}", pCallbackData->pMessage);
		}

		return vk::False;
	}
}