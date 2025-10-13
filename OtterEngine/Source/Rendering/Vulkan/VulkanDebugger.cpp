#include "OtterPCH.h"

#include "Rendering/Vulkan/VulkanDebugger.h"

namespace OtterEngine {
	void VulkanDebugger::SetupDebugMessenger(VkInstance instance) {
		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		PopulateDebugMessengerCreateInfo(createInfo);

		if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr) != VK_SUCCESS) {
			OTTER_CORE_CRITICAL("[VULKAN RENDERER] Failed to set up debug messenger!")
				throw std::runtime_error("Failed to set up debug messenger!");
		}
	}

	void VulkanDebugger::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
	{
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;
	}

	VkResult VulkanDebugger::CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator)
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

	void VulkanDebugger::DestroyDebugUtilsMessengerEXT(VkInstance instance, const VkAllocationCallbacks* pAllocator)
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr) {
			func(instance, mDebugMessenger, pAllocator);
			mDebugMessenger = VK_NULL_HANDLE;
		}
	}

	VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugger::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {

		switch (messageSeverity) {
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
			OTTER_CORE_LOG("[VULKAN RENDERER DEBUG LOG CBK]\n{}", pCallbackData->pMessage);
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
			OTTER_CORE_WARNING("[VULKAN RENDERER DEBUG WARNING CBK]\n{}", pCallbackData->pMessage);
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
			OTTER_CORE_ERROR("[VULKAN RENDERER DEBUG ERROR CBK]\n{}", pCallbackData->pMessage);
			break;
		default:
			break;
		}

		return VK_FALSE;
	}
}