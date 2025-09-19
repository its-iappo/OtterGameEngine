#include <stdexcept>
#include <vector>
#include <set>
#include <iostream>

#include "Core/Logger.h"
#include "Rendering/VulkanRenderer.h"

namespace OtterEngine {

	static constexpr uint32_t MAX_ONGOING_FRAMES = 2;

	VulkanRenderer::VulkanRenderer(GLFWwindow* window) :
		pWindow(window),
		mInstance(VK_NULL_HANDLE),
		mPhysicalDevice(VK_NULL_HANDLE),
		mDevice(VK_NULL_HANDLE),
		mSurface(VK_NULL_HANDLE),
		mSwapchain(VK_NULL_HANDLE),
		mSwapchainImageFormat(VK_FORMAT_UNDEFINED),
		mCommandPool(VK_NULL_HANDLE),
		mCurrentFrame(0) {
	}

	VulkanRenderer::~VulkanRenderer() {
		Clear();
	}

	/// <summary>
	/// IRenderer Init() override
	/// </summary>
	void VulkanRenderer::Init() {
		Create();
		CreateSurface();
		GetFirstAvailablePhysicalDevice(); // Find first available GPU that supports Vulkan
		CreateLogicalDevice(); // Create logical device from the physical device
		CreateCommandPool();
		CreateSwapchain();
		CreateImageViews();
		CreateRenderPass();
		CreateFramebuffers();
		CreateCommandBuffers();
		CreateSyncObjects();

		OTTER_CORE_LOG("Otter Vulkan Renderer initialized!");
	}

	/// <summary>
	/// IRenderer Clear() override
	/// </summary>
	void VulkanRenderer::Clear() {
		vkDeviceWaitIdle(mDevice);
		CleanupSwapchainResources();
		if (mCommandPool != VK_NULL_HANDLE) {
			vkDestroyCommandPool(mDevice, mCommandPool, nullptr);
			mCommandPool = VK_NULL_HANDLE;
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
		if (mInstance != VK_NULL_HANDLE) {
			vkDestroyInstance(mInstance, nullptr);
			mInstance = VK_NULL_HANDLE; // INSTANCE RESET
		}
	}

	/// <summary>
	/// IRenderer DrawFrame() override
	/// </summary>
	void VulkanRenderer::DrawFrame() {
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

	void VulkanRenderer::Create() {
		VkApplicationInfo appInfo{};

		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;

		appInfo.pApplicationName = "Otter Game Engine";
		appInfo.pEngineName = "Otter Game Engine";

		appInfo.applicationVersion = VK_MAKE_API_VERSION(0, 0, 1, 0);
		appInfo.engineVersion = VK_MAKE_API_VERSION(0, 0, 1, 0);
		appInfo.apiVersion = VK_API_VERSION_1_3;

		// Get required extensions from GLFW for Vulkan surface creation
		uint32_t glfwExtensionsCount = 0;
		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionsCount); // C-style array of strings for extensions' names

		if (!glfwExtensions) {
			throw std::runtime_error("Failed to get GLFW required instance extensions for Vulkan!");
		}

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionsCount);

		// extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
		// extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME); // Enable debug utils extension for validation layers

		VkInstanceCreateInfo createInfo{};

		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		// Disable validation layers
		createInfo.enabledLayerCount = 0;
		createInfo.ppEnabledLayerNames = nullptr;

		if (vkCreateInstance(&createInfo, nullptr, &mInstance) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create Vulkan renderer!");
		}
	}

	void VulkanRenderer::CreateSurface() {
		if (pWindow == nullptr) {
			throw std::runtime_error("GLFW window is null! Unable to create Vulkan surface!");
		}

		if (glfwCreateWindowSurface(mInstance, pWindow, nullptr, &mSurface) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create Vulkan surface!");
		}
	}

	void VulkanRenderer::GetFirstAvailablePhysicalDevice() {
		uint32_t deviceCount = 0;

		VkResult r = vkEnumeratePhysicalDevices(mInstance, &deviceCount, nullptr);

		if (deviceCount == 0) {
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

			OTTER_CORE_LOG("Selected GPU for Vulkan rendering!");
			return;
		}

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
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pEnabledFeatures = &deviceFeatures;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();
		createInfo.enabledLayerCount = 0;
		createInfo.ppEnabledLayerNames = nullptr;

		if (vkCreateDevice(mPhysicalDevice, &createInfo, nullptr, &mDevice) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create logical device for Vulkan!");
		}

		vkGetDeviceQueue(mDevice, mGraphicsFamily, 0, &mGraphicsQueue);
		vkGetDeviceQueue(mDevice, mPresentFamily, 0, &mPresentQueue);

		OTTER_CORE_LOG("Logical device and queues created succesfully!");
	}

	void VulkanRenderer::CreateCommandPool() {
		if (mCommandPool != VK_NULL_HANDLE) return;

		VkCommandPoolCreateInfo info{};

		info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		info.queueFamilyIndex = mGraphicsFamily;

		if (vkCreateCommandPool(mDevice, &info, nullptr, &mCommandPool) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create command pool!");
		}
	}

	void VulkanRenderer::CreateSwapchain()
	{
		VkSwapchainKHR oldSwapchain = mSwapchain;

		VkSurfaceCapabilitiesKHR capabilities{};
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mPhysicalDevice, mSurface, &capabilities);

		uint32_t formatCount = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(mPhysicalDevice, mSurface, &formatCount, nullptr);
		if (formatCount == 0) {
			throw std::runtime_error("Failed to get Vulkan surface formats!");
		}
		std::vector<VkSurfaceFormatKHR> formats(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(mPhysicalDevice, mSurface, &formatCount, formats.data());

		uint32_t presentModeCount = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(mPhysicalDevice, mSurface, &presentModeCount, nullptr);
		if (presentModeCount == 0) {
			throw std::runtime_error("Failed to find present modes!");
		}
		std::vector<VkPresentModeKHR> presentModes(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(mPhysicalDevice, mSurface, &presentModeCount, presentModes.data());

		VkSurfaceFormatKHR surfaceFormat = formats[0];
		for (const VkSurfaceFormatKHR& format : formats) {
			if (format.format == VK_FORMAT_B8G8R8A8_SRGB &&
				format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				surfaceFormat = format;
				break;
			}
		}
		mSwapchainImageFormat = surfaceFormat.format;

		VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;

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

		OTTER_CORE_LOG("Vulkan swapchain created succesfully!");
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
			throw std::runtime_error("Failed to allocate command buffers!");
		}

		for (size_t i = 0; i < mCommandBuffers.size(); ++i) {
			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

			vkBeginCommandBuffer(mCommandBuffers[i], &beginInfo);

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

			// Clear command buffer
			vkCmdEndRenderPass(mCommandBuffers[i]);

			if (vkEndCommandBuffer(mCommandBuffers[i]) != VK_SUCCESS) {
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
		CreateFramebuffers();
		CreateCommandBuffers();
	}
}