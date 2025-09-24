#include <set>
#include <memory>
#include <vector>
#include <string>
#include <iostream>
#include <stdexcept>

#include "Core/Logger.h"
#include "Utils/OtterIO.h"
#include "Rendering/VulkanRenderer.h"

namespace OtterEngine {

	static constexpr uint32_t MAX_ONGOING_FRAMES = 2;

	const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
	};

#ifdef NDEBUG
	const bool enableValidationLayers = false;
#else
	const bool enableValidationLayers = true;
#endif

	VulkanRenderer::VulkanRenderer(GLFWwindow* window) :
		pWindow(window),
		mCurrentFrame(0),
		mDevice(VK_NULL_HANDLE),
		mSurface(VK_NULL_HANDLE),
		mInstance(VK_NULL_HANDLE),
		mSwapchain(VK_NULL_HANDLE),
		mRenderPass(VK_NULL_HANDLE),
		mCommandPool(VK_NULL_HANDLE),
		mPhysicalDevice(VK_NULL_HANDLE),
		mPipelineLayout(VK_NULL_HANDLE),
		mGraphicsPipeline(VK_NULL_HANDLE),
		mSwapchainImageFormat(VK_FORMAT_UNDEFINED),
		mSwapchainExtent{} {
	}

	VulkanRenderer::~VulkanRenderer() {
		Clear();
	}

	/// <summary>
	/// IRenderer Init() override
	/// </summary>
	void VulkanRenderer::Init() {
		CreateVulkanInstance();
		SetupDebugMessenger();
		CreateSurface();
		PickPhysicalDevice(); // Find first available GPU that supports Vulkan
		CreateLogicalDevice(); // Create logical device from the physical device
		CreateCommandPool();
		CreateSwapchain();
		CreateImageViews();
		CreateRenderPass();
		CreateGraphicsPipeline();
		CreateFramebuffers();
		CreateCommandBuffers();
		CreateSyncObjects();

		OTTER_CORE_LOG("[VULKAN RENDERER] Otter Vulkan Renderer initialized!");
	}

	/// <summary>
	/// IRenderer Clear() override
	/// </summary>
	void VulkanRenderer::Clear() {
		vkDeviceWaitIdle(mDevice);
		CleanupSwapchainResources();
		if (mCommandPool != VK_NULL_HANDLE) {
			vkDestroyCommandPool(mDevice, mCommandPool, nullptr);
			mCommandPool = VK_NULL_HANDLE; // COMMAND POOL RESET
		}

		// Clearup semaphores and fences
		for (size_t i = 0; i < mImageAvailableSemaphores.size(); ++i) {
			if (mImageAvailableSemaphores[i] != VK_NULL_HANDLE)
				vkDestroySemaphore(mDevice, mImageAvailableSemaphores[i], nullptr);
			if (mImageDoneSemaphores[i] != VK_NULL_HANDLE)
				vkDestroySemaphore(mDevice, mImageDoneSemaphores[i], nullptr);
			if (mOnGoingFences[i] != VK_NULL_HANDLE)
				vkDestroyFence(mDevice, mOnGoingFences[i], nullptr);
		}

		mImageAvailableSemaphores.clear();
		mImageDoneSemaphores.clear();
		mOnGoingFences.clear();

		if (mSwapchain != VK_NULL_HANDLE) {
			vkDestroySwapchainKHR(mDevice, mSwapchain, nullptr);
			mSwapchain = VK_NULL_HANDLE; // SWAPCHAIN RESET
		}
		if (mDevice != VK_NULL_HANDLE) {
			vkDestroyDevice(mDevice, nullptr);
			mDevice = VK_NULL_HANDLE; // DEVICE RESET
		}
		if (mSurface != VK_NULL_HANDLE) {
			vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
			mSurface = VK_NULL_HANDLE; // SURFACE RESET
		}
		if (enableValidationLayers && mDebugMessenger != VK_NULL_HANDLE) {
			DestroyDebugUtilsMessengerEXT(mInstance, mDebugMessenger, nullptr);
			mDebugMessenger = VK_NULL_HANDLE; // DEBUG MESSENGER RESET
		}
		if (mInstance != VK_NULL_HANDLE) {
			vkDestroyInstance(mInstance, nullptr);
			mInstance = VK_NULL_HANDLE; // INSTANCE RESET
		}
	}

	/// <summary>
	/// IRenderer DrawFrame() override
	/// </summary>
	void VulkanRenderer::DrawFrame() {
		// Do not draw anything if window is minimized
		int width = 0, height = 0;
		glfwGetFramebufferSize(pWindow, &width, &height);
		if (width == 0 || height == 0) return;

		// Ensure previous frame is done
		vkWaitForFences(mDevice, 1, &mOnGoingFences[mCurrentFrame], VK_TRUE, UINT64_MAX);

		// Acquire next swapchain image
		uint32_t imageIndex = 0;
		VkResult nextImage = vkAcquireNextImageKHR(mDevice,
			mSwapchain,
			UINT64_MAX,
			mImageAvailableSemaphores[mCurrentFrame],
			VK_NULL_HANDLE,
			&imageIndex);

		if (nextImage == VK_ERROR_OUT_OF_DATE_KHR) {
			RecreateSwapchain();
			return;
		}
		else if (nextImage != VK_SUCCESS && nextImage != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("Acquisition of next swapchain image failed!");
		}

		vkResetFences(mDevice, 1, &mOnGoingFences[mCurrentFrame]);

		VkSemaphore waitSemaphores[] = { mImageAvailableSemaphores[mCurrentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		VkSemaphore signalSemaphores[] = { mImageDoneSemaphores[mCurrentFrame] };

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &mCommandBuffers[imageIndex];
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		if (vkQueueSubmit(mGraphicsQueue, 1, &submitInfo, mOnGoingFences[mCurrentFrame]) != VK_SUCCESS) {
			throw std::runtime_error("Submit to Vulkan queue failed!");
		}

		VkPresentInfoKHR presentInfo{};
		VkSwapchainKHR swapchains[] = { mSwapchain };

		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapchains;
		presentInfo.pImageIndices = &imageIndex;
		presentInfo.pResults = nullptr;

		VkResult presentResult = vkQueuePresentKHR(mPresentQueue, &presentInfo);

		if (presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR)
		{
			RecreateSwapchain();
		}
		else if (presentResult != VK_SUCCESS) {
			throw std::runtime_error("Failed to execute queue present!");
		}

		// Go to next frame (alternate between 0 and 1)
		mCurrentFrame = (mCurrentFrame + 1) % MAX_ONGOING_FRAMES;
	}

	void VulkanRenderer::CreateVulkanInstance() {
		VkApplicationInfo appInfo{};

		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;

		appInfo.pApplicationName = "Otter VKRender Engine";
		appInfo.pEngineName = "Otter Engine";

		appInfo.applicationVersion = VK_MAKE_API_VERSION(0, 0, 1, 0);
		appInfo.engineVersion = VK_MAKE_API_VERSION(0, 0, 1, 0);
		appInfo.apiVersion = VK_API_VERSION_1_3;

		uint32_t glfwExtensionsCount = 0;
		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionsCount); // C-style array of strings for extensions' names

		if (!glfwExtensions) {
			OTTER_CORE_CRITICAL("[VULKAN RENDERER] Failed to get GLFW required instance extensions for Vulkan!");
			throw std::runtime_error("Failed to get GLFW required instance extensions for Vulkan!");
		}

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionsCount);

		if (enableValidationLayers) {
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME); // Enable debug utils extension for validation layers
		}
		bool enableValidation = CheckValidationLayerSupport();

		VkInstanceCreateInfo createInfo{};

		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
		if (enableValidation) {
			// Enable validation layers
			OTTER_CORE_LOG("[VULKAN RENDERER] Validation layers enabled! Populating debug messenger.");
			PopulateDebugMessengerCreateInfo(debugCreateInfo);
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else {
			// Disable validation layers
			createInfo.enabledLayerCount = 0;
			createInfo.ppEnabledLayerNames = nullptr;
			OTTER_CORE_WARNING("[VULKAN RENDERER]Validation layers not found. To see debug logs, install the Vulkan SDK from LunarG.");
		}

		if (vkCreateInstance(&createInfo, nullptr, &mInstance) != VK_SUCCESS) {
			OTTER_CORE_CRITICAL("[VULKAN RENDERER] Failed to create Vulkan renderer!");
			throw std::runtime_error("Failed to create Vulkan renderer!");
		}
	}

	void VulkanRenderer::CreateSurface() {
		if (pWindow == nullptr) {
			OTTER_CORE_CRITICAL("[VULKAN RENDERER] GLFW window is null! Unable to create Vulkan surface!");
			throw std::runtime_error("GLFW window is null! Unable to create Vulkan surface!");
		}

		if (glfwCreateWindowSurface(mInstance, pWindow, nullptr, &mSurface) != VK_SUCCESS) {
			OTTER_CORE_CRITICAL("[VULKAN RENDERER] Failed to create Vulkan surface!");
			throw std::runtime_error("Failed to create Vulkan surface!");
		}
	}

	void VulkanRenderer::PickPhysicalDevice() {
		uint32_t deviceCount = 0;

		vkEnumeratePhysicalDevices(mInstance, &deviceCount, nullptr);

		if (deviceCount == 0) {
			OTTER_CORE_CRITICAL("[VULKAN RENDERER] No GPU supporting Vulkan found!");
			throw std::runtime_error("No GPU supporting Vulkan found!");
		}

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(mInstance, &deviceCount, devices.data());

		for (const VkPhysicalDevice& device : devices) {
			uint32_t queueCount = 0;

			vkGetPhysicalDeviceQueueFamilyProperties(device, &queueCount, nullptr);
			std::vector<VkQueueFamilyProperties> queueFamilies(queueCount);
			vkGetPhysicalDeviceQueueFamilyProperties(device, &queueCount, queueFamilies.data());

			int foundGraphics = -1;
			int foundPresent = -1;

			for (uint32_t i = 0; i < queueCount; i++) {
				if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
					foundGraphics = (int)i;
				}
				VkBool32 presentSupport = VK_FALSE;
				vkGetPhysicalDeviceSurfaceSupportKHR(device, i, mSurface, &presentSupport);
				if (presentSupport) {
					foundPresent = (int)i;
				}
				if (foundGraphics != -1 && foundPresent != -1) {
					break;
				}
			}

			// Incompatible device
			if (foundGraphics == -1 || foundPresent == -1) {
				continue;
			}

			uint32_t extensionCount = 0;
			vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
			std::vector<VkExtensionProperties> props(extensionCount);
			vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, props.data());

			bool isSwapchainSupported = false;

			for (const VkExtensionProperties& prop : props) {
				if (strcmp(prop.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0) {
					isSwapchainSupported = true;
					break;
				}
			}

			// Incompatible swapchain for this device
			if (!isSwapchainSupported) {
				continue;
			}

			mPhysicalDevice = device;
			mGraphicsFamily = static_cast<uint32_t>(foundGraphics);
			mPresentFamily = static_cast<uint32_t>(foundPresent);

			OTTER_CORE_LOG("[VULKAN RENDERER] | ================= Selected GPU for Vulkan rendering! ================= |");

			VkPhysicalDeviceProperties deviceProperties{};
			vkGetPhysicalDeviceProperties(mPhysicalDevice, &deviceProperties);

			OTTER_CORE_LOG("[VULKAN RENDERER] | Device name: {}", deviceProperties.deviceName);
			OTTER_CORE_LOG("[VULKAN RENDERER] | Device type: {}", VkPhysicalDeviceTypeToString(deviceProperties.deviceType));
			OTTER_CORE_LOG("[VULKAN RENDERER] | Device id: {}", deviceProperties.deviceID);
			OTTER_CORE_LOG("[VULKAN RENDERER] | API version: {}.{}.{}", VK_VERSION_MAJOR(deviceProperties.apiVersion), VK_VERSION_MINOR(deviceProperties.apiVersion), VK_VERSION_PATCH(deviceProperties.apiVersion));
			OTTER_CORE_LOG("[VULKAN RENDERER] | Raw driver version: {}", deviceProperties.driverVersion);
			OTTER_CORE_LOG("[VULKAN RENDERER] | Decoded driver version: {}.{}.{}", VK_VERSION_MAJOR(deviceProperties.driverVersion), VK_VERSION_MINOR(deviceProperties.driverVersion), VK_VERSION_PATCH(deviceProperties.driverVersion));

			std::string pipelineCacheUUID;
			for (size_t i = 0; i < VK_UUID_SIZE; i++)
			{
				char buf[4];
				snprintf(buf, sizeof(buf), "%02x", deviceProperties.pipelineCacheUUID[i]);
				pipelineCacheUUID += buf;
				if (i != VK_UUID_SIZE - 1) pipelineCacheUUID += "-";
			}
			OTTER_CORE_LOG("[VULKAN RENDERER] | Pipeline cache UUID: {}", pipelineCacheUUID);
			OTTER_CORE_LOG("[VULKAN RENDERER] | ================= ++++++++++++++++++++++++++++++++++ ================= |");

			return;
		}

		OTTER_CORE_CRITICAL("[VULKAN RENDERER] Failed to find a suitable GPU for Vulkan rendering!");
		throw std::runtime_error("Failed to find a suitable GPU for Vulkan rendering!");
	}

	void VulkanRenderer::CreateLogicalDevice() {
		float queuePriority = 1.0f;

		std::set<uint32_t> uniqueQueueFamilies = { mGraphicsFamily, mPresentFamily };
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

		for (uint32_t queueFamily : uniqueQueueFamilies) {
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

		VkPhysicalDeviceFeatures deviceFeatures{};

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.pEnabledFeatures = &deviceFeatures;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();
		createInfo.enabledLayerCount = 0;
		createInfo.ppEnabledLayerNames = nullptr;

		if (vkCreateDevice(mPhysicalDevice, &createInfo, nullptr, &mDevice) != VK_SUCCESS) {
			OTTER_CORE_CRITICAL("[VULKAN RENDERER] Failed to create logical device for Vulkan!");
			throw std::runtime_error("Failed to create logical device for Vulkan!");
		}

		vkGetDeviceQueue(mDevice, mGraphicsFamily, 0, &mGraphicsQueue);
		vkGetDeviceQueue(mDevice, mPresentFamily, 0, &mPresentQueue);

		OTTER_CORE_LOG("[VULKAN RENDERER] Logical device and queues created succesfully!");
	}

	void VulkanRenderer::CreateCommandPool() {
		if (mCommandPool != VK_NULL_HANDLE) return;

		VkCommandPoolCreateInfo info{};

		info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		info.queueFamilyIndex = mGraphicsFamily;

		if (vkCreateCommandPool(mDevice, &info, nullptr, &mCommandPool) != VK_SUCCESS) {
			OTTER_CORE_CRITICAL("[VULKAN RENDERER] Failed to create command pool!");
			throw std::runtime_error("Failed to create command pool!");
		}
	}

	void VulkanRenderer::CreateSwapchain()
	{
		VkSwapchainKHR oldSwapchain = mSwapchain;

		// Query swapchain support capabilities for the physical device and surface
		VkSurfaceCapabilitiesKHR capabilities{};
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mPhysicalDevice, mSurface, &capabilities);

		// Query supported surface formats
		uint32_t formatCount = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(mPhysicalDevice, mSurface, &formatCount, nullptr);
		if (formatCount == 0) {
			OTTER_CORE_CRITICAL("[VULKAN RENDERER] Failed to get Vulkan surface formats!");
			throw std::runtime_error("Failed to get Vulkan surface formats!");
		}
		std::vector<VkSurfaceFormatKHR> formats(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(mPhysicalDevice, mSurface, &formatCount, formats.data());

		VkSurfaceFormatKHR surfaceFormat = formats[0];
		for (const VkSurfaceFormatKHR& format : formats) {
			// VK_FORMAT_R8G8B8A8_SRGB || VK_FORMAT_B8G8R8A8_SRGB
			if (format.format == VK_FORMAT_B8G8R8A8_SRGB &&
				format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				surfaceFormat = format;
				break;
			}
		}
		mSwapchainImageFormat = surfaceFormat.format;

		// Query supported presentation modes
		uint32_t presentModeCount = 0;
		VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
		vkGetPhysicalDeviceSurfacePresentModesKHR(mPhysicalDevice, mSurface, &presentModeCount, nullptr);
		if (presentModeCount == 0) {
			OTTER_CORE_CRITICAL("[VULKAN RENDERER] Failed to find present modes!");
			throw std::runtime_error("Failed to find present modes!");
		}
		std::vector<VkPresentModeKHR> presentModes(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(mPhysicalDevice, mSurface, &presentModeCount, presentModes.data());

		for (const VkPresentModeKHR& mode : presentModes) {
			if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
				presentMode = mode;
				break;
			}
		}

		VkExtent2D extent = capabilities.currentExtent;
		if (extent.width == UINT32_MAX) {
			int width, height;
			glfwGetFramebufferSize(pWindow, &width, &height);
			extent.width = static_cast<uint32_t>(width);
			extent.height = static_cast<uint32_t>(height);
		}
		mSwapchainExtent = extent;

		uint32_t imageCount = capabilities.minImageCount + 1;
		if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
			imageCount = capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = mSurface;
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = mSwapchainImageFormat;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = mSwapchainExtent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		if (mGraphicsFamily != mPresentFamily) {
			uint32_t queueFamilyIndices[] = { mGraphicsFamily, mPresentFamily };
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else {
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0;
			createInfo.pQueueFamilyIndices = nullptr;
		}

		createInfo.preTransform = capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = oldSwapchain;

		if (vkCreateSwapchainKHR(mDevice, &createInfo, nullptr, &mSwapchain) != VK_SUCCESS) {
			OTTER_CORE_CRITICAL("[VULKAN RENDERER] Failed to create Vulkan swapchain!");
			throw std::runtime_error("Failed to create Vulkan swapchain!");
		}

		// Destroy old swapchain if a new one was created succesfully
		if (oldSwapchain != VK_NULL_HANDLE) {
			vkDestroySwapchainKHR(mDevice, oldSwapchain, nullptr);
		}

		uint32_t currImageCount = 0;
		vkGetSwapchainImagesKHR(mDevice, mSwapchain, &currImageCount, nullptr);
		mSwapchainImages.resize(currImageCount);
		vkGetSwapchainImagesKHR(mDevice, mSwapchain, &currImageCount, mSwapchainImages.data());

		OTTER_CORE_LOG("[VULKAN RENDERER] Vulkan swapchain created succesfully!");
	}

	void VulkanRenderer::CreateImageViews() {
		mSwapchainImageViews.resize(mSwapchainImages.size());

		for (size_t i = 0; i < mSwapchainImages.size(); ++i) {
			VkImageViewCreateInfo viewInfo{};
			viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewInfo.image = mSwapchainImages[i];
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			viewInfo.format = mSwapchainImageFormat;
			viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			viewInfo.subresourceRange.baseMipLevel = 0;
			viewInfo.subresourceRange.levelCount = 1;
			viewInfo.subresourceRange.baseArrayLayer = 0;
			viewInfo.subresourceRange.layerCount = 1;

			if (vkCreateImageView(mDevice, &viewInfo, nullptr, &mSwapchainImageViews[i]) != VK_SUCCESS) {
				OTTER_CORE_CRITICAL("[VULKAN RENDERER] Failed to create image views!");
				throw std::runtime_error("Failed to create image views!");
			}
		}
	}

	void VulkanRenderer::CreateRenderPass() {
		VkAttachmentDescription colorAttachment{};

		colorAttachment.format = mSwapchainImageFormat;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(mDevice, &renderPassInfo, nullptr, &mRenderPass) != VK_SUCCESS) {
			OTTER_CORE_CRITICAL("[VULKAN RENDERER] Failed to create render pass!");
			throw std::runtime_error("Failed to create render pass!");
		}
	}

	void VulkanRenderer::CreateFramebuffers() {
		mSwapchainFramebuffers.resize(mSwapchainImageViews.size());

		for (size_t i = 0; i < mSwapchainImageViews.size(); ++i) {
			VkImageView attachments[] = { mSwapchainImageViews[i] };

			VkFramebufferCreateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			info.renderPass = mRenderPass;
			info.attachmentCount = 1;
			info.pAttachments = attachments;
			info.width = mSwapchainExtent.width;
			info.height = mSwapchainExtent.height;
			info.layers = 1;

			if (vkCreateFramebuffer(mDevice, &info, nullptr, &mSwapchainFramebuffers[i]) != VK_SUCCESS) {
				OTTER_CORE_CRITICAL("[VULKAN RENDERER] Failed to create framebuffer!");
				throw std::runtime_error("Failed to create framebuffer!");
			}
		}
	}

	void VulkanRenderer::CreateCommandBuffers() {
		mCommandBuffers.resize(mSwapchainFramebuffers.size());

		VkCommandBufferAllocateInfo cbAllocInfo{};
		cbAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cbAllocInfo.commandPool = mCommandPool;
		cbAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cbAllocInfo.commandBufferCount = static_cast<uint32_t>(mCommandBuffers.size());

		if (vkAllocateCommandBuffers(mDevice, &cbAllocInfo, mCommandBuffers.data()) != VK_SUCCESS) {
			OTTER_CORE_CRITICAL("[VULKAN RENDERER] Failed to allocate command buffers!");
			throw std::runtime_error("Failed to allocate command buffers!");
		}

		for (size_t i = 0; i < mCommandBuffers.size(); ++i) {
			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

			if (vkBeginCommandBuffer(mCommandBuffers[i], &beginInfo) != VK_SUCCESS) {
				OTTER_CORE_CRITICAL("[VULKAN RENDERER] Failed to begin recording command buffer!");
				throw std::runtime_error("Failed to begin recording command buffer!");
			}

			VkRenderPassBeginInfo rpInfo{};
			rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			rpInfo.renderPass = mRenderPass;
			rpInfo.framebuffer = mSwapchainFramebuffers[i];
			rpInfo.renderArea.offset = { 0, 0 };
			rpInfo.renderArea.extent = mSwapchainExtent;

			VkClearValue clearColor = { {0.0f, 0.0f, 0.0f, 1.0f} };
			rpInfo.clearValueCount = 1;
			rpInfo.pClearValues = &clearColor;

			vkCmdBeginRenderPass(mCommandBuffers[i], &rpInfo, VK_SUBPASS_CONTENTS_INLINE);

			// Bind pipeline
			vkCmdBindPipeline(mCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, mGraphicsPipeline);

			// Issue draw command (3 vertices, 1 instance)
			vkCmdDraw(mCommandBuffers[i], 3, 1, 0, 0);

			vkCmdEndRenderPass(mCommandBuffers[i]);

			if (vkEndCommandBuffer(mCommandBuffers[i]) != VK_SUCCESS) {
				OTTER_CORE_CRITICAL("[VULKAN RENDERER] Failed to record command buffer!");
				throw std::runtime_error("Failed to record command buffer!");
			}
		}
	}

	void VulkanRenderer::CreateSyncObjects() {
		mImageAvailableSemaphores.resize(MAX_ONGOING_FRAMES);
		mImageDoneSemaphores.resize(MAX_ONGOING_FRAMES);
		mOnGoingFences.resize(MAX_ONGOING_FRAMES);

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (uint32_t i = 0; i < MAX_ONGOING_FRAMES; ++i) {
			if (vkCreateSemaphore(mDevice, &semaphoreInfo, nullptr, &mImageAvailableSemaphores[i]) != VK_SUCCESS ||
				vkCreateSemaphore(mDevice, &semaphoreInfo, nullptr, &mImageDoneSemaphores[i]) != VK_SUCCESS ||
				vkCreateFence(mDevice, &fenceInfo, nullptr, &mOnGoingFences[i]) != VK_SUCCESS) {
				OTTER_CORE_CRITICAL("[VULKAN RENDERER] Failed to create sync objects for a frame!");
				throw std::runtime_error("Failed to create sync objects for a frame!");
			}
		}
	}

	void VulkanRenderer::CleanupSwapchainResources() {

		// Clear framebuffer
		for (VkFramebuffer buffer : mSwapchainFramebuffers) {
			if (buffer != VK_NULL_HANDLE) {
				vkDestroyFramebuffer(mDevice, buffer, nullptr);
			}
		}
		mSwapchainFramebuffers.clear();

		// Clear image views
		for (VkImageView imageView : mSwapchainImageViews) {
			if (imageView != VK_NULL_HANDLE) vkDestroyImageView(mDevice, imageView, nullptr);
		}
		mSwapchainImageViews.clear();

		// Clear render pass
		if (mRenderPass != VK_NULL_HANDLE) {
			vkDestroyRenderPass(mDevice, mRenderPass, nullptr);
			mRenderPass = VK_NULL_HANDLE;
		}

		// Free and clear command buffers
		if (!mCommandBuffers.empty()) {
			vkFreeCommandBuffers(mDevice, mCommandPool, static_cast<uint32_t>(mCommandBuffers.size()), mCommandBuffers.data());
			mCommandBuffers.clear();
		}

		if (mPipelineLayout != VK_NULL_HANDLE) {
			vkDestroyPipelineLayout(mDevice, mPipelineLayout, nullptr);
			mPipelineLayout = VK_NULL_HANDLE;
		}

		if (mGraphicsPipeline != VK_NULL_HANDLE) {
			vkDestroyPipeline(mDevice, mGraphicsPipeline, nullptr);
			mGraphicsPipeline = VK_NULL_HANDLE;
		}
	}

	void VulkanRenderer::RecreateSwapchain() {
		int width = 0, height = 0;
		while (true) {
			glfwGetFramebufferSize(pWindow, &width, &height);

			// Ignore minimized window
			if (width != 0 && height != 0) break;

			glfwWaitEvents();
		}

		vkDeviceWaitIdle(mDevice);

		CleanupSwapchainResources();

		CreateSwapchain();

		CreateImageViews();
		CreateRenderPass();

		CreateGraphicsPipeline();

		CreateFramebuffers();
		CreateCommandBuffers();
	}

	void VulkanRenderer::CreateShaderModule(const std::vector<char>& shader, VkShaderModule& shaderModule)
	{
		// Ensure the size is a multiple of 4, as required by Vulkan for pCode
		size_t codeSize = shader.size();
		if (codeSize % 4 != 0) {
			// Copy into a temporary padded buffer (shouldn't normally happen for .spv files)
			std::vector<char> padded(codeSize + (4 - (codeSize % 4)));
			std::copy(shader.begin(), shader.end(), padded.begin());
			codeSize = padded.size();

			VkShaderModuleCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			createInfo.codeSize = codeSize;
			createInfo.pCode = reinterpret_cast<const uint32_t*>(padded.data());

			VkResult r = vkCreateShaderModule(mDevice, &createInfo, nullptr, &shaderModule);
			if (r != VK_SUCCESS) {
				OTTER_CORE_CRITICAL("[VULKAN RENDERER] Failed to create shader module (padded)!");
				throw std::runtime_error("Failed to create shader module (padded). VkResult = " + std::to_string(r));
			}
		}
		else {
			VkShaderModuleCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			createInfo.codeSize = codeSize;
			createInfo.pCode = reinterpret_cast<const uint32_t*>(shader.data());

			VkResult r = vkCreateShaderModule(mDevice, &createInfo, nullptr, &shaderModule);
			if (r != VK_SUCCESS) {
				OTTER_CORE_CRITICAL("[VULKAN RENDERER] Failed to create shader module!");
				throw std::runtime_error("Failed to create shader module. VkResult = " + std::to_string(r));
			}
		}
	}

	void VulkanRenderer::CreateGraphicsPipeline() {
		auto vertShaderCode = OtterIO::ReadFile("../Shaders/triangle.vert.spv");
		auto fragShaderCode = OtterIO::ReadFile("../Shaders/triangle.frag.spv");

		OTTER_CORE_LOG("[VULKAN RENDERER] Vert size is: {}", vertShaderCode.size());
		OTTER_CORE_LOG("[VULKAN RENDERER] Frag size is: {}", fragShaderCode.size());

		VkShaderModule vertShaderModule;
		CreateShaderModule(vertShaderCode, vertShaderModule);

		VkShaderModule fragShaderModule;
		CreateShaderModule(fragShaderCode, fragShaderModule);

		// Select pipeline stages for each shader

		VkPipelineShaderStageCreateInfo vertShaderStageInfo{}; // Vertex shader
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule;
		vertShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo fragShaderStageInfo{}; // Fragment shader
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShaderModule;
		fragShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(float) * 5; // vec2 pos, vec3 color
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		VkVertexInputAttributeDescription attributeDescription{};
		attributeDescription.binding = 0;
		attributeDescription.location = 0;
		attributeDescription.format = VK_FORMAT_R32G32_SFLOAT; // vec2
		attributeDescription.offset = 0;

		// Vertex input
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputInfo.pVertexAttributeDescriptions = nullptr;

		// Input Assembly
		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		// Dynamic viewport and scissor
		std::vector<VkDynamicState> dynamicStates = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};

		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicState.pDynamicStates = dynamicStates.data();

		// Rasterizer
		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE; // Depends on how vertices are drawn
		rasterizer.depthBiasEnable = VK_FALSE;

		// Multisampling
		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		// Color blending
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask =
			VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
			VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;

		// Pipeline Layout
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pushConstantRangeCount = 0;

		if (vkCreatePipelineLayout(mDevice, &pipelineLayoutInfo, nullptr, &mPipelineLayout) != VK_SUCCESS) {
			OTTER_CORE_CRITICAL("[VULKAN RENDERER] Failed to create pipeline layout!");
			throw std::runtime_error("Failed to create pipeline layout!");
		}

		// Pipeline
		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		//pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.layout = mPipelineLayout;
		pipelineInfo.renderPass = mRenderPass;
		pipelineInfo.pDynamicState = &dynamicState;
		pipelineInfo.subpass = 0;

		VkResult res = vkCreateGraphicsPipelines(mDevice, nullptr, 1, &pipelineInfo, nullptr, &mGraphicsPipeline);

		OTTER_CORE_LOG("[VULKAN RENDERER] Graphics pipeline result: {}", VkResultToString(res));

		if (res != VK_SUCCESS) {
			OTTER_CORE_CRITICAL("[VULKAN RENDERER] Failed to create graphics pipeline!");
			throw std::runtime_error("Failed to create graphics pipeline!");
		}

		// Clean up shader modules
		vkDestroyShaderModule(mDevice, fragShaderModule, nullptr);
		vkDestroyShaderModule(mDevice, vertShaderModule, nullptr);
	}

	bool VulkanRenderer::CheckValidationLayerSupport() {
		uint32_t layerCount = 0;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
		std::vector<VkLayerProperties> available(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, available.data());

		OTTER_CORE_LOG("[VULKAN RENDERER] ====================");
		OTTER_CORE_LOG("[VULKAN RENDERER] Available GPU layers:");
		for (const auto& prop : available) {
			OTTER_CORE_LOG("[VULKAN RENDERER] {}", prop.layerName);
		}
		OTTER_CORE_LOG("[VULKAN RENDERER] ====================");

		for (const char* layerName : validationLayers) {
			bool found = false;
			for (const auto& prop : available) {
				if (strcmp(prop.layerName, layerName) == 0) { found = true; break; }
			}
			if (!found) return false;
		}
		return true;
	}

	// Debug methods and utilities

	void VulkanRenderer::SetupDebugMessenger() {
		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		PopulateDebugMessengerCreateInfo(createInfo);

		if (CreateDebugUtilsMessengerEXT(mInstance, &createInfo, nullptr, &mDebugMessenger) != VK_SUCCESS) {
			OTTER_CORE_CRITICAL("[VULKAN RENDERER] Failed to set up debug messenger!")
				throw std::runtime_error("failed to set up debug messenger!");
		}
	}

	void VulkanRenderer::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
	{
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;
	}

	VkResult VulkanRenderer::CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
	{
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr) {
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else {
			OTTER_CORE_WARNING("[VULKAN RENDERER] Debug Utils Messenger extension not present, unable to create it!");
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	void VulkanRenderer::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr) {
			func(instance, debugMessenger, pAllocator);
		}
	}

	VKAPI_ATTR VkBool32 VKAPI_CALL VulkanRenderer::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
		OTTER_CORE_ERROR("[VULKAN RENDERER] Validation layer: {}", pCallbackData->pMessage);

		return VK_FALSE;
	}


	const char* VulkanRenderer::VkResultToString(VkResult res) {
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

	const char* VulkanRenderer::VkPhysicalDeviceTypeToString(VkPhysicalDeviceType type)
	{
		switch (type)
		{
		case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
			return "Integrated GPU";
		case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
			return "Discrete GPU";
		case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
			return "Virtual GPU";
		case VK_PHYSICAL_DEVICE_TYPE_CPU:
			return "CPU";
		case VK_PHYSICAL_DEVICE_TYPE_OTHER:
		case VK_PHYSICAL_DEVICE_TYPE_MAX_ENUM:
		default:
			return "Unknown device type!";
		}
	}


}
