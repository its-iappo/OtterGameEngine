#include "OtterPCH.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtc/matrix_transform.hpp>

#include <map>
#include <string>
#include <string.h> // C header
#include <algorithm>

#define VULKAN_HPP_NO_EXCEPTIONS
#define VULKAN_HPP_NO_CONSTRUCTORS
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <vulkan/vulkan_structs.hpp>

#include "Utils/OtterIO.h"
#include "Rendering/Vulkan/VulkanUtility.h"
#include "Rendering/Vulkan/VulkanMeshLoader.h" 
#include "Rendering/Vulkan/VulkanTextureLoader.h"
#include "Rendering/Vulkan/VulkanRenderer.h"


namespace OtterEngine {
	VulkanRenderer::VulkanRenderer(GLFWwindow* window) :
		pWindow(window),
		mCurrentFrame(0),
		mSurface(VK_NULL_HANDLE),
		mInstance(VK_NULL_HANDLE),
		mSwapchain(VK_NULL_HANDLE),
		mDepthImage(VK_NULL_HANDLE),
		mRenderPass(VK_NULL_HANDLE),
		mCommandPool(VK_NULL_HANDLE),
		mPhysicalDevice(VK_NULL_HANDLE),
		mPipelineLayout(VK_NULL_HANDLE),
		mDepthImageView(VK_NULL_HANDLE),
		mDepthImageMemory(VK_NULL_HANDLE),
		mGraphicsPipeline(VK_NULL_HANDLE),
		mDescriptorSetLayout(VK_NULL_HANDLE),
		mSwapchainImageFormat(VK_FORMAT_UNDEFINED),
		mSwapchainExtent{} {
	}

	VulkanRenderer::~VulkanRenderer() {
		if (mIsCleared) return;
		Clear();
	}

	/// <summary>
	/// IRenderer Init() override
	/// </summary>
	void VulkanRenderer::Init() {
		CreateVulkanInstance();

		SetupDebugMessenger();
		CreateSurface();
		
		PickPhysicalDevice();
		CreateLogicalDevice();

		CreateSwapchain();
		
		CreateImageViews();
		CreateRenderPass();
		CreateDescriptorSetLayout();
		CreateGraphicsPipeline();
		CreateCommandPool();
		CreateDepthResources();
		CreateFramebuffers();

		CreateTextureLoader();
		mTextureLoader->LoadTexture("viking_room.png");

		CreateMeshLoader();
		mMeshLoader->LoadMesh("viking_room.obj");

		CreateUniformBuffers();
		CreateDescriptorPool();
		CreateDescriptorSets();
		CreateCommandBuffers();
		CreateSyncObjects();

		OTTER_CORE_LOG("[VULKAN RENDERER] Otter Vulkan Renderer initialized!");
	}

	/// <summary>
	/// IRenderer Clear() override
	/// </summary>
	void VulkanRenderer::Clear() {
		if (mIsCleared) return;

		//if (mDevice != VK_NULL_HANDLE) {
		//	vkDeviceWaitIdle(mDevice);
		//}
		CleanupSwapchainResources();

		mTextureLoader->ClearResources();

		if (mDescriptorPool != VK_NULL_HANDLE) {
			// vkDestroyDescriptorPool(mDevice, mDescriptorPool, nullptr);
			mDescriptorPool = VK_NULL_HANDLE; // DESCRIPTOR POOL RESET
		}
		if (mDescriptorSetLayout != VK_NULL_HANDLE) {
			//vkDestroyDescriptorSetLayout(mDevice, mDescriptorSetLayout, nullptr);
			mDescriptorSetLayout = VK_NULL_HANDLE; // DESCRIPTOR SET LAYOUT RESET
		}

		// Buffers cleanup

		for (uint32_t i = 0; i < MAX_ONGOING_FRAMES; ++i) {
			//if (mUniformBuffers[i] != VK_NULL_HANDLE) {
			//	vkDestroyBuffer(mDevice, mUniformBuffers[i], nullptr);
			//	mUniformBuffers[i] = VK_NULL_HANDLE; // UNIFORM BUFFER RESET
			//}
			//if (mUniformBuffersMemory[i] != VK_NULL_HANDLE) {
			//	vkFreeMemory(mDevice, mUniformBuffersMemory[i], nullptr);
			//	mUniformBuffersMemory[i] = VK_NULL_HANDLE; // UNIFORM BUFFER MEMORY RESET
			//}
		}

		mMeshLoader->ClearResources();

		// Semaphores and fences cleanup

		for (size_t i = 0; i < mRenderFinishedSemaphores.size(); ++i) {
			if (mRenderFinishedSemaphores[i] != VK_NULL_HANDLE) {
				//vkDestroySemaphore(mDevice, mRenderFinishedSemaphores[i], nullptr);
			}
		}
		mRenderFinishedSemaphores.clear();

		for (size_t i = 0; i < MAX_ONGOING_FRAMES; ++i) {
			if (mImageAvailableSemaphores[i] != VK_NULL_HANDLE) {
				//vkDestroySemaphore(mDevice, mImageAvailableSemaphores[i], nullptr);
			}
			if (mActiveFences[i] != VK_NULL_HANDLE) {
				//vkDestroyFence(mDevice, mActiveFences[i], nullptr);
			}
		}
		mImageAvailableSemaphores.clear();
		mActiveFences.clear();

		//if (mCommandPool != VK_NULL_HANDLE) {
		//	vkDestroyCommandPool(mDevice, mCommandPool, nullptr);
		//	mCommandPool = VK_NULL_HANDLE; // COMMAND POOL RESET
		//}

		//if (mDevice != VK_NULL_HANDLE) {
		//	vkDestroyDevice(mDevice, nullptr);
		//	mDevice = VK_NULL_HANDLE; // DEVICE RESET
		//}

		//if (mEnableValidationLayers) {
		//	mVkDebugger->DestroyDebugUtilsMessengerEXT(mInstance, nullptr);
		//	mVkDebugger.reset(); // DEBUG MESSENGER RESET
		//}
		//if (mSurface != VK_NULL_HANDLE) {
		//	vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
		//	mSurface = VK_NULL_HANDLE; // SURFACE RESET
		//}
		//if (mInstance != VK_NULL_HANDLE) {
		//	vkDestroyInstance(mInstance, nullptr);
		//	mInstance = VK_NULL_HANDLE; // INSTANCE RESET
		//}

		mIsCleared = true;
	}

	/// <summary>
	/// IRenderer DrawFrame() override
	/// </summary> 
	void VulkanRenderer::DrawFrame() {
		// Do not draw anything if window is minimized
		int width = 0, height = 0;
		glfwGetFramebufferSize(pWindow, &width, &height);
		if (width == 0 || height == 0) return;

		//vkWaitForFences(mDevice, 1, &mActiveFences[mCurrentFrame], VK_TRUE, UINT64_MAX);

		uint32_t imageIndex = 0;
		//VkResult nextImage = vkAcquireNextImageKHR(
		//	mDevice,
		//	mSwapchain,
		//	UINT64_MAX,
		//	mImageAvailableSemaphores[mCurrentFrame],
		//	VK_NULL_HANDLE,
		//	&imageIndex);

		//if (nextImage == VK_ERROR_OUT_OF_DATE_KHR) {
		//	RecreateSwapchain();
		//	return;
		//}

		//if (nextImage == VK_SUBOPTIMAL_KHR) {
		//	OTTER_CORE_WARNING("[VULKAN RENDERER] Failed to acquire optimal swapchain image!");
		//}
		//else if (nextImage != VK_SUCCESS) {
		//	OTTER_FATAL("[VULKAN RENDERER] Swapchain image acquisition failed! VkResult: {}", VulkanUtility::VkResultToString(nextImage));
		//}

		if (mImagesInFlight[imageIndex] != VK_NULL_HANDLE) {
			//vkWaitForFences(mDevice, 1, &mImagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
		}

		mImagesInFlight[imageIndex] = mActiveFences[mCurrentFrame];

		UpdateUniformBuffer(mCurrentFrame);

		//vkResetFences(mDevice, 1, &mActiveFences[mCurrentFrame]);

		vkResetCommandBuffer(mCommandBuffers[imageIndex], 0);
		RecordCommandBuffer(mCommandBuffers[imageIndex], imageIndex);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { mImageAvailableSemaphores[mCurrentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &mCommandBuffers[imageIndex];

		VkSemaphore signalSemaphores[] = { mRenderFinishedSemaphores[imageIndex] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		VK_CHECK(vkQueueSubmit(mGraphicsQueue, 1, &submitInfo, mActiveFences[mCurrentFrame]));

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapchains[] = { mSwapchain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapchains;
		presentInfo.pImageIndices = &imageIndex;


		VkResult presentResult = vkQueuePresentKHR(mPresentQueue, &presentInfo);

		if (presentResult == VK_ERROR_OUT_OF_DATE_KHR) {
			OTTER_CORE_WARNING("[VULKAN RENDERER] Swapchain is out of date, recreating...");
			RecreateSwapchain();
		}
		else if (presentResult == VK_SUBOPTIMAL_KHR) {
			OTTER_CORE_WARNING("[VULKAN RENDERER] Swapchain is suboptimal after presentation.");
			RecreateSwapchain();
		}
		else if (presentResult != VK_SUCCESS) {
			OTTER_FATAL("[VULKAN RENDERER] Failed to present swapchain image! VkResult {}", VulkanUtility::VkResultToString(presentResult));
		}

		mCurrentFrame = (mCurrentFrame + 1) % MAX_ONGOING_FRAMES;
	}

	void VulkanRenderer::CreateVulkanInstance() {
		constexpr vk::ApplicationInfo appInfo{ .pApplicationName = "Otter VkRenderer Engine",
											  .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
											  .pEngineName = "Otter Engine",
											  .engineVersion = VK_MAKE_VERSION(1, 0, 0),
											  .apiVersion = vk::ApiVersion14 };

		// char* const is an immutable pointer to char
		std::vector<char* const> requiredLayers;
		if (mEnableValidationLayers) {
			requiredLayers.assign(mValidationLayers.begin(), mValidationLayers.end());
		}

		auto layerProps = mContext.enumerateInstanceLayerProperties();

		// Are these layers supported?
		for (auto const& requiredLayer : requiredLayers)
		{
			if (std::ranges::none_of(layerProps,
				[requiredLayer](auto const& layerProperty) { return strcmp(layerProperty.layerName, requiredLayer) == 0; }))
			{
				OTTER_CORE_CRITICAL("[VULKAN RENDERER] Required layer not supported: " + std::string(requiredLayer));
			}
		}

		// Get the required instance extensions from GLFW.
		auto reqExtensions = GetRequiredExtensions();

		// Are these GLFW extensions supported?
		auto extensionProperties = mContext.enumerateInstanceExtensionProperties();
		for (auto const& ext : reqExtensions)
		{
			if (std::ranges::none_of(extensionProperties, [ext](auto const& extProp)
				{return strcmp(extProp.extensionName, ext) == 0; }))
			{
				OTTER_CORE_CRITICAL("[VULKAN RENDERER] Required GLFW extension not supported: " + std::string(ext));
			}
		}

		vk::InstanceCreateInfo createInfo{
			.pApplicationInfo = &appInfo,
			.enabledLayerCount = static_cast<uint32_t>(requiredLayers.size()),
			.enabledExtensionCount = static_cast<uint32_t>(reqExtensions.size()),
			.ppEnabledExtensionNames = reqExtensions.data() };

		mInstance = vk::raii::Instance(mContext, createInfo);
	}

	void VulkanRenderer::CreateSurface() {
		if (pWindow == nullptr) {
			OTTER_FATAL("[VULKAN RENDERER] GLFW Window pointer is null!");
		}

		VkSurfaceKHR surface;
		if (glfwCreateWindowSurface(*mInstance, pWindow, nullptr, &surface) != 0) {
			OTTER_FATAL("[VULKAN RENDERER] Unable to create Vulkan surface!");
		}
		mSurface = vk::raii::SurfaceKHR(mInstance, surface);
	}

	void VulkanRenderer::PickPhysicalDevice() {

		auto devices = mInstance.enumeratePhysicalDevices();

		OTTER_ASSERT(!devices.empty(), "[VULKAN RENDERER] No device supporting Vulkan found!");

		const auto iterator = std::ranges::find_if(devices, 
			[&](auto const& device) {

				bool isSuitable = device.getProperties().apiVersion >= VK_API_VERSION_1_3;
				auto queueFams = device.getQueueFamilyProperties();
				
				bool supportsGraphics = std::ranges::any_of(queueFams, [](auto const& qfp) 
					{return !!(qfp.queueFlags & vk::QueueFlagBits::eGraphics); });
				
				const auto queueFamIter = std::ranges::find_if(queueFams,
					[](vk::QueueFamilyProperties const& qfp) {
						return (qfp.queueFlags & vk::QueueFlagBits::eGraphics) != static_cast<vk::QueueFlags>(0);
					});
				
				auto availableDeviceExtensions = device.enumerateDeviceExtensionProperties();
				bool supportsAllRequiredExtensions =
					std::ranges::all_of(mDeviceExtensions,
						[&availableDeviceExtensions](auto const& requiredDeviceExtension) {
							return std::ranges::any_of(availableDeviceExtensions,
								[requiredDeviceExtension](auto const& availableDeviceExtension) \
							{ return strcmp(availableDeviceExtension.extensionName, requiredDeviceExtension) == 0; });
						});

				// The two .template tell the compiler that getFeatures2 and get are template functions

				auto features = device.template getFeatures2<
					vk::PhysicalDeviceFeatures2, 
					vk::PhysicalDeviceVulkan13Features, 
					vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();
				bool supportsRequiredFeatures = features.template get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering &&
					features.template get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState;

				return isSuitable && supportsGraphics && supportsAllRequiredExtensions && supportsRequiredFeatures;
			});

		OTTER_ASSERT(iterator != devices.end(), "[VULKAN RENDERER] Failed to find a suitable GPU for Vulkan rendering!");


		if (mPhysicalDevice == VK_NULL_HANDLE) {
			OTTER_FATAL("");
		}

		OTTER_CORE_LOG("[VULKAN RENDERER] | ================= Selected GPU for Vulkan rendering! ================= |");

		auto deviceProperties = mPhysicalDevice.getProperties();

		OTTER_CORE_LOG("[VULKAN RENDERER] | Device name: {}", deviceProperties.deviceName);
		OTTER_CORE_LOG("[VULKAN RENDERER] | Device type: {}", VulkanUtility::VkPhysicalDeviceTypeToString(deviceProperties.deviceType));
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
		OTTER_CORE_LOG("[VULKAN RENDERER] | ================= Device memory flags found for GPU: ================= |");

		auto memProperties = mPhysicalDevice.getMemoryProperties();

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			OTTER_CORE_LOG("[VULKAN RENDERER] [{}] Memory Type: flags=0x{:x}",
				i, memProperties.memoryTypes[i].propertyFlags);
		}

		OTTER_CORE_LOG("[VULKAN RENDERER] | ================= ++++++++++++++++++++++++++++++++++ ================= |");
	}

	void VulkanRenderer::CreateLogicalDevice() {
		std::vector<vk::QueueFamilyProperties> queueFamilyProperties = mPhysicalDevice.getQueueFamilyProperties();
		
		uint32_t queueIndex = ~0; // All bits to 1 -> Invalid flag!

		for (uint32_t qfpIndex = 0; qfpIndex < queueFamilyProperties.size(); qfpIndex++)
		{
			if ((queueFamilyProperties[qfpIndex].queueFlags & vk::QueueFlagBits::eGraphics) &&
				mPhysicalDevice.getSurfaceSupportKHR(qfpIndex, *mSurface))
			{
				// Found a queue family that supports both graphics and present
				queueIndex = qfpIndex;
				break;
			}
		}
		if (queueIndex == ~0) {
			OTTER_FATAL("[VULKAN RENDERER] Cannot find a queue for graphics or present!");

		}

		vk::StructureChain<
			vk::PhysicalDeviceFeatures2, 
			vk::PhysicalDeviceVulkan13Features, 
			vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT> 
			featureChain = {
			{},                                   // vk::PhysicalDeviceFeatures2
			{.dynamicRendering = true},           // vk::PhysicalDeviceVulkan13Features
			{.extendedDynamicState = true}        // vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT
		};

		float priority = 0.5f;
		vk::DeviceQueueCreateInfo devQueueCreateInfo{
			.queueFamilyIndex = queueIndex, 
			.queueCount = 1, 
			.pQueuePriorities = &priority};

		vk::DeviceCreateInfo devCreateInfo{
			.pNext = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
			.queueCreateInfoCount = 1,
			.pQueueCreateInfos = &devQueueCreateInfo,
			.enabledExtensionCount = static_cast<uint32_t>(mDeviceExtensions.size()),
			.ppEnabledExtensionNames = mDeviceExtensions.data()
		};

		mDevice = vk::raii::Device(mPhysicalDevice, devCreateInfo);
		mGraphicsQueue = vk::raii::Queue(mDevice, grIndex, 0);

		OTTER_CORE_LOG("[VULKAN RENDERER] Logical Device and Graphics Queue created succesfully!");
	}

	void VulkanRenderer::CreateCommandPool() {
		QueueFamilyIndices indices = VulkanUtility::FindQueueFamilies(mPhysicalDevice, mSurface);

		VkCommandPoolCreateInfo info{};

		info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		info.queueFamilyIndex = indices.mGraphicsFamily.value();

		VK_CHECK(vkCreateCommandPool(mDevice, &info, nullptr, &mCommandPool));
	}

	void VulkanRenderer::CreateDepthResources()
	{
		VkFormat depthFormat = VulkanUtility::FindDepthFormat(mPhysicalDevice);

		VulkanUtility::CreateVkImage(mDevice, mPhysicalDevice,
			mDepthImage, mDepthImageMemory,
			mSwapchainExtent.width, mSwapchainExtent.height,
			depthFormat, VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		mDepthImageView = VulkanUtility::CreateImageView(mDevice,
			mDepthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

		VulkanUtility::TransitionImageLayout(mDevice, mCommandPool,
			mDepthImage, mGraphicsQueue, depthFormat,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
	}

	void VulkanRenderer::CreateTextureLoader()
	{
		mTextureLoader = std::make_unique<VulkanTextureLoader>(mDevice, mPhysicalDevice, mCommandPool, mGraphicsQueue);
	}

	void VulkanRenderer::CreateMeshLoader()
	{
		mMeshLoader = std::make_unique<VulkanMeshLoader>(mDevice, mPhysicalDevice, mCommandPool, mGraphicsQueue);
	}

	void VulkanRenderer::CreateSwapchain()
	{
		auto surfCap = mPhysicalDevice.getSurfaceCapabilitiesKHR(*mSurface);
		mSwapchainExtent = VulkanUtility::ChooseSwapExtent(surfCap);

		VkExtent2D extent = VulkanUtility::ChooseSwapExtent(swapchainSupport.mCapabilities, pWindow);
		VkPresentModeKHR presentMode = VulkanUtility::ChooseSwapPresentMode(swapchainSupport.mPresentModes);
		VkSurfaceFormatKHR surfaceFormat = VulkanUtility::ChooseSwapSurfaceFormat(swapchainSupport.mFormats);

		uint32_t imageCount = swapchainSupport.mCapabilities.minImageCount + 1;
		if (swapchainSupport.mCapabilities.maxImageCount > 0 && imageCount > swapchainSupport.mCapabilities.maxImageCount) {
			imageCount = swapchainSupport.mCapabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = mSurface;
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		QueueFamilyIndices indices = VulkanUtility::FindQueueFamilies(mPhysicalDevice, mSurface);
		uint32_t queueFamilyIndices[] = { indices.mGraphicsFamily.value(), indices.mPresentFamily.value() };

		if (indices.mGraphicsFamily != indices.mPresentFamily) {
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else {
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}

		createInfo.preTransform = swapchainSupport.mCapabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;

		VK_CHECK(vkCreateSwapchainKHR(mDevice, &createInfo, nullptr, &mSwapchain));

		vkGetSwapchainImagesKHR(mDevice, mSwapchain, &imageCount, nullptr);
		mSwapchainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(mDevice, mSwapchain, &imageCount, mSwapchainImages.data());

		mImagesInFlight.resize(imageCount, VK_NULL_HANDLE);

		mSwapchainImageFormat = surfaceFormat.format;
		mSwapchainExtent = extent;

		OTTER_CORE_LOG("[VULKAN RENDERER] Vulkan swapchain created succesfully!");
	}

	void VulkanRenderer::CreateImageViews() {
		mSwapchainImageViews.resize(mSwapchainImages.size());

		for (size_t i = 0; i < mSwapchainImages.size(); ++i) {
			mSwapchainImageViews[i] = VulkanUtility::CreateImageView(mDevice,
				mSwapchainImages[i],
				mSwapchainImageFormat,
				VK_IMAGE_ASPECT_COLOR_BIT);
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

		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		dependency.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		VkAttachmentDescription depthAttachment{};
		depthAttachment.format = VulkanUtility::FindDepthFormat(mPhysicalDevice);
		depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;

		std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		VK_CHECK(vkCreateRenderPass(mDevice, &renderPassInfo, nullptr, &mRenderPass));
	}

	void VulkanRenderer::CreateFramebuffers() {
		mSwapchainFramebuffers.resize(mSwapchainImageViews.size());

		for (size_t i = 0; i < mSwapchainImageViews.size(); ++i) {
			std::array<VkImageView, 2> attachments = { mSwapchainImageViews[i], mDepthImageView };

			VkFramebufferCreateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			info.renderPass = mRenderPass;
			info.attachmentCount = static_cast<uint32_t>(attachments.size());
			info.pAttachments = attachments.data();
			info.width = mSwapchainExtent.width;
			info.height = mSwapchainExtent.height;
			info.layers = 1;

			VK_CHECK(vkCreateFramebuffer(mDevice, &info, nullptr, &mSwapchainFramebuffers[i]));
		}
	}

	void VulkanRenderer::CreateUniformBuffers() {
		VkDeviceSize bufferSize = sizeof(UniformBufferObject);

		mUniformBuffers.resize(MAX_ONGOING_FRAMES);
		mUniformBuffersMemory.resize(MAX_ONGOING_FRAMES);
		mUniformBuffersMapped.resize(MAX_ONGOING_FRAMES);

		for (uint32_t i = 0; i < MAX_ONGOING_FRAMES; ++i) {
			VulkanUtility::CreateNewBuffer(mDevice, mPhysicalDevice,
				bufferSize,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
				VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				mUniformBuffers[i],
				mUniformBuffersMemory[i]);

			vkMapMemory(mDevice, mUniformBuffersMemory[i], 0, bufferSize, 0, &mUniformBuffersMapped[i]);
		}
	}

	void VulkanRenderer::CreateDescriptorPool() {
		std::array<VkDescriptorPoolSize, 2> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = MAX_ONGOING_FRAMES;
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = MAX_ONGOING_FRAMES;

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = MAX_ONGOING_FRAMES;

		VK_CHECK(vkCreateDescriptorPool(mDevice, &poolInfo, nullptr, &mDescriptorPool));
	}

	void VulkanRenderer::CreateDescriptorSets()
	{
		std::vector<VkDescriptorSetLayout> layouts(MAX_ONGOING_FRAMES, mDescriptorSetLayout);

		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = mDescriptorPool;
		allocInfo.descriptorSetCount = MAX_ONGOING_FRAMES;
		allocInfo.pSetLayouts = layouts.data();

		mDescriptorSets.resize(MAX_ONGOING_FRAMES);

		VK_CHECK(vkAllocateDescriptorSets(mDevice, &allocInfo, mDescriptorSets.data()));

		for (uint32_t i = 0; i < MAX_ONGOING_FRAMES; ++i) {
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = mUniformBuffers[i];
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(UniformBufferObject);

			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = mTextureLoader->GetCurrentImageView();
			imageInfo.sampler = mTextureLoader->GetCurrentSampler();

			std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = mDescriptorSets[i];
			descriptorWrites[0].dstBinding = 0;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &bufferInfo;

			descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[1].dstSet = mDescriptorSets[i];
			descriptorWrites[1].dstBinding = 1;
			descriptorWrites[1].dstArrayElement = 0;
			descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[1].descriptorCount = 1;
			descriptorWrites[1].pImageInfo = &imageInfo;

			vkUpdateDescriptorSets(mDevice,
				static_cast<uint32_t>(descriptorWrites.size()),
				descriptorWrites.data(),
				0, nullptr);
		}
	}

	void VulkanRenderer::CreateCommandBuffers() {
		mCommandBuffers.resize(mSwapchainFramebuffers.size());

		VkCommandBufferAllocateInfo cbAllocInfo{};
		cbAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cbAllocInfo.commandPool = mCommandPool;
		cbAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cbAllocInfo.commandBufferCount = static_cast<uint32_t>(mCommandBuffers.size());

		VK_CHECK(vkAllocateCommandBuffers(mDevice, &cbAllocInfo, mCommandBuffers.data()));
	}

	void VulkanRenderer::RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		VK_CHECK(vkBeginCommandBuffer(commandBuffer, &beginInfo));

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = mRenderPass;
		renderPassInfo.framebuffer = mSwapchainFramebuffers[imageIndex];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = mSwapchainExtent;

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
		clearValues[1].depthStencil = { 1.0f, 0 };

		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mGraphicsPipeline);

		// Viewport and scissor
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)mSwapchainExtent.width;
		viewport.height = (float)mSwapchainExtent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = mSwapchainExtent;
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		if (mMeshLoader &&
			mMeshLoader->GetVertexBuffer() != VK_NULL_HANDLE &&
			mMeshLoader->GetIndexBuffer() != VK_NULL_HANDLE &&
			mMeshLoader->GetIndexCount() > 0) {

			VkBuffer vertexBuffers[] = { mMeshLoader->GetVertexBuffer() };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

			vkCmdBindIndexBuffer(commandBuffer, mMeshLoader->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);

			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayout, 0, 1, &mDescriptorSets[mCurrentFrame], 0, nullptr);

			vkCmdDrawIndexed(commandBuffer, mMeshLoader->GetIndexCount(), 1, 0, 0, 0);
		}
		else {
			OTTER_CORE_WARNING("[COMMAND] No mesh to render - clearing screen only");
		}

		vkCmdEndRenderPass(commandBuffer);

		VK_CHECK(vkEndCommandBuffer(commandBuffer));
	}

	void VulkanRenderer::CreateSyncObjects() {
		mImageAvailableSemaphores.resize(MAX_ONGOING_FRAMES);
		mRenderFinishedSemaphores.resize(mSwapchainImages.size());
		mActiveFences.resize(MAX_ONGOING_FRAMES);

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < MAX_ONGOING_FRAMES; i++) {
			VK_CHECK(vkCreateSemaphore(mDevice, &semaphoreInfo, nullptr, &mImageAvailableSemaphores[i]));
		}

		for (size_t i = 0; i < mSwapchainImages.size(); i++) {
			VK_CHECK(vkCreateSemaphore(mDevice, &semaphoreInfo, nullptr, &mRenderFinishedSemaphores[i]));
		}

		for (size_t i = 0; i < MAX_ONGOING_FRAMES; i++) {
			VK_CHECK(vkCreateFence(mDevice, &fenceInfo, nullptr, &mActiveFences[i]));

		}

		mImagesInFlight.resize(mSwapchainImages.size(), VK_NULL_HANDLE);
	}

	void VulkanRenderer::CleanupSwapchainResources() {
		// Clear framebuffer
		for (VkFramebuffer buffer : mSwapchainFramebuffers) {
			if (buffer != VK_NULL_HANDLE) {
				vkDestroyFramebuffer(mDevice, buffer, nullptr);
			}
		}
		mSwapchainFramebuffers.clear();

		// Clear depth resources
		if (mDepthImageView != VK_NULL_HANDLE) {
			vkDestroyImageView(mDevice, mDepthImageView, nullptr);
			mDepthImageView = VK_NULL_HANDLE;
		}
		if (mDepthImage != VK_NULL_HANDLE) {
			vkDestroyImage(mDevice, mDepthImage, nullptr);
			mDepthImage = VK_NULL_HANDLE;
		}
		if (mDepthImageMemory != VK_NULL_HANDLE) {
			vkFreeMemory(mDevice, mDepthImageMemory, nullptr);
			mDepthImageMemory = VK_NULL_HANDLE;
		}

		// Clear image views
		for (VkImageView imageView : mSwapchainImageViews) {
			if (imageView != VK_NULL_HANDLE) vkDestroyImageView(mDevice, imageView, nullptr);
		}
		mSwapchainImageViews.clear();

		if (mGraphicsPipeline != VK_NULL_HANDLE) {
			vkDestroyPipeline(mDevice, mGraphicsPipeline, nullptr);
			mGraphicsPipeline = VK_NULL_HANDLE;
		}

		if (mPipelineLayout != VK_NULL_HANDLE) {
			vkDestroyPipelineLayout(mDevice, mPipelineLayout, nullptr);
			mPipelineLayout = VK_NULL_HANDLE;
		}

		// Clear render pass
		if (mRenderPass != VK_NULL_HANDLE) {
			vkDestroyRenderPass(mDevice, mRenderPass, nullptr);
			mRenderPass = VK_NULL_HANDLE;
		}

		// Free and clear command buffers
		if (!mCommandBuffers.empty() && mCommandPool != VK_NULL_HANDLE) {
			vkFreeCommandBuffers(mDevice, mCommandPool,
				static_cast<uint32_t>(mCommandBuffers.size()),
				mCommandBuffers.data());
			mCommandBuffers.clear();
		}

		if (mSwapchain != VK_NULL_HANDLE) {
			vkDestroySwapchainKHR(mDevice, mSwapchain, nullptr);
			mSwapchain = VK_NULL_HANDLE;
		}
	}

	void VulkanRenderer::RecreateSwapchain() {
		int width = 0, height = 0;
		while (width == 0 || height == 0) {
			glfwGetFramebufferSize(pWindow, &width, &height);
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(mDevice);

		CleanupSwapchainResources();

		CreateSwapchain();
		CreateImageViews();
		CreateRenderPass();
		CreateGraphicsPipeline();
		CreateDepthResources();
		CreateFramebuffers();
		CreateCommandBuffers();

		mImagesInFlight.resize(mSwapchainImages.size(), VK_NULL_HANDLE);
	}

	VkShaderModule VulkanRenderer::CreateShaderModule(const std::vector<char>& shader) const
	{
		if (shader.empty()) {
			OTTER_FATAL("[VULKAN RENDERER] Shader code is empty!");
		}

		// Ensure the size is a multiple of 4, as required by Vulkan for pCode
		if (shader.size() % 4 != 0) {
			OTTER_FATAL("[VULKAN RENDERER] Shader SPIR-V size not multiple of 4 (size: {})", shader.size());
		}

		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = shader.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(shader.data());

		VkShaderModule shaderModule = VK_NULL_HANDLE;

		VK_CHECK(vkCreateShaderModule(mDevice, &createInfo, nullptr, &shaderModule));

		return shaderModule;
	}

	void VulkanRenderer::CreateGraphicsPipeline() {
		auto vertShaderCode = OtterIO::ReadFile("../Shaders/triangle.vert.spv");
		auto fragShaderCode = OtterIO::ReadFile("../Shaders/triangle.frag.spv");

		OTTER_CORE_LOG("[VULKAN RENDERER] Vert size is: {}", vertShaderCode.size());
		OTTER_CORE_LOG("[VULKAN RENDERER] Frag size is: {}", fragShaderCode.size());

		VkShaderModule vertShaderModule = CreateShaderModule(vertShaderCode);
		VkShaderModule fragShaderModule = CreateShaderModule(fragShaderCode);

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

		// Vertex input
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		auto bindingDescription = VertexLayout::GetBindingDescription();
		auto attributeDescriptions = VertexLayout::GetAttributeDescriptions();

		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());

		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

		// Input Assembly
		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;

		// Rasterizer
		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT; // VK_CULL_MODE_NONE -> Should be used only for debugging
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE; // Depends on how vertices are drawn
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
		colorBlending.blendConstants[0] = 0.0f;
		colorBlending.blendConstants[1] = 0.0f;
		colorBlending.blendConstants[2] = 0.0f;
		colorBlending.blendConstants[3] = 0.0f;

		// Dynamic viewport and scissor
		std::vector<VkDynamicState> dynamicStates = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};

		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicState.pDynamicStates = dynamicStates.data();

		// Pipeline Layout
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &mDescriptorSetLayout;
		pipelineLayoutInfo.pushConstantRangeCount = 0;

		VK_CHECK(vkCreatePipelineLayout(mDevice, &pipelineLayoutInfo, nullptr, &mPipelineLayout));

		// Depth Stencil
		VkPipelineDepthStencilStateCreateInfo depthStencil{};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = VK_TRUE;
		depthStencil.depthWriteEnable = VK_TRUE;
		depthStencil.depthCompareOp = VK_COMPARE_OP_LESS; // Lower depth -> Closer
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.minDepthBounds = 0.0f;
		depthStencil.maxDepthBounds = 1.0f;
		depthStencil.stencilTestEnable = VK_FALSE;
		depthStencil.front = {};
		depthStencil.back = {};

		// Pipeline
		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = &dynamicState;
		pipelineInfo.pDepthStencilState = &depthStencil;
		pipelineInfo.layout = mPipelineLayout;
		pipelineInfo.renderPass = mRenderPass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

		//OTTER_CORE_LOG("[PIPELINE] Creating graphics pipeline with:");
		//OTTER_CORE_LOG("[PIPELINE] - Binding: binding={}, stride={}, inputRate={}",
		//	bindingDescription.binding, bindingDescription.stride, bindingDescription.inputRate);

		//for (size_t i = 0; i < attributeDescriptions.size(); ++i) {
		//	const auto& attr = attributeDescriptions[i];
		//	OTTER_CORE_LOG("[PIPELINE] - Attribute[{}]: location={}, binding={}, format={}, offset={}",
		//		i, attr.location, attr.binding, attr.format, attr.offset);
		//}

		VK_CHECK(vkCreateGraphicsPipelines(mDevice, nullptr, 1, &pipelineInfo, nullptr, &mGraphicsPipeline));

		OTTER_CORE_LOG("[VULKAN RENDERER] Graphics pipeline created!");

		// Clean up shader modules
		vkDestroyShaderModule(mDevice, fragShaderModule, nullptr);
		vkDestroyShaderModule(mDevice, vertShaderModule, nullptr);
	}

	void VulkanRenderer::CreateDescriptorSetLayout()
	{
		VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		uboLayoutBinding.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutBinding samplerLayoutBinding{};
		samplerLayoutBinding.binding = 1;
		samplerLayoutBinding.descriptorCount = 1;
		samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding.pImmutableSamplers = nullptr;
		samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();

		VK_CHECK(vkCreateDescriptorSetLayout(mDevice, &layoutInfo, nullptr, &mDescriptorSetLayout));
	}

	void VulkanRenderer::UpdateUniformBuffer(uint32_t currentImage) {

		// This variable is only initialized once, on the first call
		// to this function, then it retains its value between calls.
		static auto startTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();

		// Elapsed time since rendering has started
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

		UniformBufferObject ubo{};
		// Model matrix: rotate around Z axis
		ubo.model = glm::rotate(glm::mat4(1.0f), // Identity matrix
			time * glm::radians(90.0f), // Create a smooth rotation over time (90 deg/s)
			glm::vec3(0.0f, 0.0f, 1.0f)); // Rotate around Z axis

		// View matrix: camera at (2,2,2), looking at origin, with up-vector pointing along positive Z axis
		ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), // Camera position in World space (eye position) 
			glm::vec3(0.0f, 0.0f, 0.0f), // Look at origin (center position)
			glm::vec3(0.0f, 0.0f, 1.0f)); // Up vector (Z-Axis)

		// Perspective projection matrix
		ubo.proj = glm::perspective(glm::radians(45.0f), // FOV
			mSwapchainExtent.width / (float)mSwapchainExtent.height, // Aspect ratio (adapt to window resize)
			0.5f, 20.0f);// Near and far planes

		// Invert Y axis
		ubo.proj[1][1] *= -1;

		memcpy(mUniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
	}

	// Debug methods and utilities
	void VulkanRenderer::SetupDebugMessenger()
	{
		mVkDebugger = std::make_unique<VulkanDebugger>();
		mVkDebugger->SetupDebugMessenger(mInstance);
	}

	std::vector<const char*> VulkanRenderer::GetRequiredExtensions() {
		uint32_t count = 0;
		auto glfwExtensions = glfwGetRequiredInstanceExtensions(&count);

		std::vector extensions(glfwExtensions, glfwExtensions + count);
		if (mEnableValidationLayers) {
			extensions.push_back(vk::EXTDebugUtilsExtensionName);
		}

		return extensions;
	}
}
