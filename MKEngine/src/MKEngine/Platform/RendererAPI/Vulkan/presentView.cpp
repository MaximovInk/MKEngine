#include "mkpch.h"
#include "presentView.h"
#include "device.h"
#include "vkExtern.h"
#include "render_structs.h"

namespace MKEngine {
	
	std::vector<glm::vec3> trianglePositions;

	VulkanPresentView::VulkanPresentView(VulkanDevice* device)
	{
		this->device = device;

		maxFramesInFlight = 2;
		frameNumber = 0;

		for (float x = -1.0f; x < 1.0f; x+=0.2f)
		{
			for (float y = -1.0f; y < 1.0f; y += 0.2f)
			{
				trianglePositions.push_back(glm::vec3(x,y,0.0f));
			}
		}
	}

	VulkanPresentView::~VulkanPresentView()
	{
		cleanupSwapChain();

		vkDestroySurfaceKHR(device->Instance, Surface, nullptr);

		SwapChain = VK_NULL_HANDLE;
	}

	void VulkanPresentView::InitSurface(Window* window)
	{
		Surface = VkExtern::CreateWindowSurface(device->Instance, window);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device->PhysicalDevice,
			Surface, &formatCount, NULL);
		assert(formatCount > 0);

		std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device->PhysicalDevice,
			Surface, &formatCount, surfaceFormats.data());

		if ((formatCount == 1) &&
			(surfaceFormats[0].format == VK_FORMAT_UNDEFINED))
		{
			ColorFormat = VK_FORMAT_B8G8R8A8_UNORM;
			ColorSpace = surfaceFormats[0].colorSpace;
		}
		else
		{
			bool found_B8G8R8A8_UNORM = false;
			for (auto&& surfaceFormat : surfaceFormats)
			{
				if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_UNORM)
				{
					ColorFormat = surfaceFormat.format;
					ColorSpace = surfaceFormat.colorSpace;
					found_B8G8R8A8_UNORM = true;
					break;
				}
			}

			if (!found_B8G8R8A8_UNORM)
			{
				ColorFormat = surfaceFormats[0].format;
				ColorSpace = surfaceFormats[0].colorSpace;
			}
		}

		windowRef = window;

		MK_LOG_INFO("CHOOSED FORMAT: {0}, {1}", string_VkFormat(ColorFormat), string_VkColorSpaceKHR(ColorSpace));

	}
	
	void VulkanPresentView::CreateSwapChain()
	{
		device->WaitDeviceIdle();
		VkSwapchainKHR oldSwapchain = SwapChain;

		WindowData data = windowRef->GetData();

		VkSurfaceCapabilitiesKHR surfCaps;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device->PhysicalDevice,
			Surface, &surfCaps);
/*

		MK_LOG_INFO("[VULKAN] Swapchain caps: MIN_IMG: {0}, MAX_IMG: {1}, SIZE {2}x{3}",
			surfCaps.minImageCount,
			surfCaps.maxImageCount,
			surfCaps.currentExtent.width,
			surfCaps.currentExtent.height
			);

		MK_LOG_INFO("[VULKAN] max layers array images {0}", surfCaps.maxImageArrayLayers);

		MK_LOG_INFO("\tsupported transforms:");
		std::vector<std::string> stringList = VkExtern::log_transform_bits(surfCaps.supportedTransforms);
		for (std::string line : stringList) {
			MK_LOG_INFO("\t\t {0}", line);
		}

		MK_LOG_INFO("\tsupported alpha operations:");
		stringList = VkExtern::log_alpha_composite_bits(surfCaps.supportedTransforms);
		for (std::string line : stringList) {
			MK_LOG_INFO("\t\t {0}", line);
		}

		MK_LOG_INFO("\tsupported image usage:\n");
		stringList = VkExtern::log_image_usage_bits(surfCaps.supportedUsageFlags);
		for (std::string line : stringList) {
			MK_LOG_INFO("\t\t {0}", line);
		}

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device->PhysicalDevice,
			Surface, &formatCount, NULL);
		assert(formatCount > 0);

		std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device->PhysicalDevice,
			Surface, &formatCount, surfaceFormats.data());


		for (auto&& surfaceFormat : surfaceFormats)
		{
			MK_LOG_INFO("supported pixel format: {0}", string_VkFormat(surfaceFormat.format));
			MK_LOG_INFO("supported color space: {0}", string_VkColorSpaceKHR(surfaceFormat.colorSpace));

		}

*/

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device->PhysicalDevice, 
			Surface, &presentModeCount, nullptr);

		std::vector<VkPresentModeKHR> presentModes(presentModeCount);

		if (presentModeCount != 0) {
			presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device->PhysicalDevice,
				Surface, &presentModeCount, presentModes.data());
		}
		/*
		
		for (auto presentMode : presentModes) {
			MK_LOG_INFO("present mode: {0}", VkExtern::log_present_mode(presentMode));
		}
		*/

		VkExtent2D swapchainExtent = {};

		if (surfCaps.currentExtent.width == (uint32_t)-1)
		{
			swapchainExtent.width = data.Width;
			swapchainExtent.height = data.Height;
		}
		else
		{
			swapchainExtent = surfCaps.currentExtent;
			data.Width = surfCaps.currentExtent.width;
			data.Height = surfCaps.currentExtent.height;
		}

		VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;

		if (!data.VSync)
		{
			for (size_t i = 0; i < presentModeCount; i++)
			{
				if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
				{
					swapchainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
					break;
				}

				if (presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR)
					swapchainPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
			}
		}

		MK_LOG_INFO("SELECTED PRESENT MODE: {0}", string_VkPresentModeKHR(swapchainPresentMode));

		uint32_t desiredNumberOfSwapchainImages = surfCaps.minImageCount + 1;
		if ((surfCaps.maxImageCount > 0)
			&& (desiredNumberOfSwapchainImages > surfCaps.maxImageCount))
		{
			desiredNumberOfSwapchainImages = surfCaps.maxImageCount;
		}

		VkSurfaceTransformFlagsKHR preTransform;
		if (surfCaps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
			preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		else
			preTransform = surfCaps.currentTransform;

		VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		std::vector<VkCompositeAlphaFlagBitsKHR> compositeAlphaFlags = {
		VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
		VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
		VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
		};
		for (auto& compositeAlphaFlag : compositeAlphaFlags) {
			if (surfCaps.supportedCompositeAlpha & compositeAlphaFlag) {
				compositeAlpha = compositeAlphaFlag;
				break;
			};
		}

		VkSwapchainCreateInfoKHR createInfo{ VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
		createInfo.surface = Surface;
		createInfo.minImageCount = desiredNumberOfSwapchainImages;
		createInfo.imageFormat = ColorFormat;
		createInfo.imageColorSpace = ColorSpace;
		createInfo.imageExtent =
		{ swapchainExtent.width, swapchainExtent.height };
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		createInfo.preTransform = (VkSurfaceTransformFlagBitsKHR)preTransform;

		if (device->QueueFamilyIndices.present != device->QueueFamilyIndices.graphics)
		{
			uint32_t queueFamilyIndices[] = { device->QueueFamilyIndices.graphics , device->QueueFamilyIndices.present };
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else {
			createInfo.queueFamilyIndexCount = 0;
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}

		createInfo.presentMode = swapchainPresentMode;
		//createInfo.oldSwapchain = oldSwapchain;
		createInfo.compositeAlpha = compositeAlpha;
		createInfo.clipped = VK_TRUE;

		if (surfCaps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
			createInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		if (surfCaps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT)
			createInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

		if (vkCreateSwapchainKHR(device->LogicalDevice, &createInfo,
			nullptr, &SwapChain) != VK_SUCCESS) {
			MK_LOG_CRITICAL("failed to create swap chain!");
		}
/*

		if (oldSwapchain != VK_NULL_HANDLE) {
			for (uint32_t i = 0; i < ImageCount; i++)
			{
				vkDestroyImageView(device->LogicalDevice, Buffers[i].view, nullptr);
			}
			vkDestroySwapchainKHR(device->LogicalDevice, oldSwapchain, nullptr);
		}
*/

		std::vector<VkImage> images;
		vkGetSwapchainImagesKHR(device->LogicalDevice, SwapChain,
			&ImageCount, nullptr);
		images.resize(ImageCount);
		vkGetSwapchainImagesKHR(device->LogicalDevice, SwapChain, 
			&ImageCount, images.data());

		Buffers.resize(ImageCount);

		for (uint32_t i = 0; i < ImageCount; i++)
		{
			VkImageViewCreateInfo viewInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
			viewInfo.pNext = NULL;
			viewInfo.format = ColorFormat;
			viewInfo.components = {
				VK_COMPONENT_SWIZZLE_R,
				VK_COMPONENT_SWIZZLE_G,
				VK_COMPONENT_SWIZZLE_B,
				VK_COMPONENT_SWIZZLE_A
			};
			viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			viewInfo.subresourceRange.baseMipLevel = 0;
			viewInfo.subresourceRange.levelCount = 1;
			viewInfo.subresourceRange.baseArrayLayer = 0;
			viewInfo.subresourceRange.layerCount = 1;
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			viewInfo.flags = 0;

			Buffers[i].image = images[i];

			viewInfo.image = Buffers[i].image;

			vkCreateImageView(device->LogicalDevice, &viewInfo, nullptr, &Buffers[i].view);
		}

		SwapChainExtent = swapchainExtent;

		MK_LOG_INFO("[VULKAN] Created PresentViews {0} - {1}x{2}", ImageCount, data.Width, data.Height);
	}

	void VulkanPresentView::FinalizeCreation()
	{
		createFramebuffer();
		createCommandBuffers();

		createSync();
	}

	void VulkanPresentView::RecreateSwapchain()
	{
		MK_LOG_INFO("RECREATE swapchain");

		int width = 0;
		int height = 0;

		PlatformBackend::s_CurrentBackend->GetWindowSize(windowRef, &width, &height);

		while (width == 0 || height == 0)
		{
			PlatformBackend::s_CurrentBackend->HandleEvents();
			PlatformBackend::s_CurrentBackend->GetWindowSize(windowRef, &width, &height);
		}

		WindowData* data = &windowRef->GetData();
		data->Width = width;
		data->Height = height;

		cleanupSwapChain();
		CreateSwapChain();
		//Graphics pipeline ()
		FinalizeCreation();
	}

	void VulkanPresentView::createFramebuffer()
	{
		for (size_t i = 0; i < ImageCount; i++) {
			VkImageView attachments[] = {
				Buffers[i].view
			};

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = device->GraphicsPipeline.renderPass;
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.width = SwapChainExtent.width;
			framebufferInfo.height = SwapChainExtent.height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(device->LogicalDevice, &framebufferInfo, nullptr, &(Buffers[i].framebuffer)) != VK_SUCCESS) {
				MK_LOG_ERROR("Failed to create framebuffer");
			}
		
		}
		MK_LOG_INFO("created framebuffers x{0}", ImageCount);
	}

	void VulkanPresentView::createSync()
	{
		for (size_t i = 0; i < ImageCount; i++)
		{
			ViewSync sync{};
			sync.inFlightFence = VkExtern::createFence(device->LogicalDevice);
			sync.imageAvailableSemaphore = VkExtern::createSemaphore(device->LogicalDevice);
			sync.renderFinishedSemaphore = VkExtern::createSemaphore(device->LogicalDevice);
			Buffers[i].sync = sync;
		}
	}

	VkResult VulkanPresentView::AcquireNextImage(VkSemaphore presentCompleteSemaphore, uint32_t* imageIndex)
	{
		return vkAcquireNextImageKHR(device->LogicalDevice, SwapChain, UINT64_MAX,
			presentCompleteSemaphore, (VkFence)nullptr, imageIndex);
	}

	VkResult VulkanPresentView::QueuePresent(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore)
	{
		VkPresentInfoKHR presentInfo = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
		presentInfo.pNext = NULL;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &SwapChain;
		presentInfo.pImageIndices = &imageIndex;

		if (waitSemaphore != VK_NULL_HANDLE)
		{
			presentInfo.pWaitSemaphores = &waitSemaphore;
			presentInfo.waitSemaphoreCount = 1;

		}
		return vkQueuePresentKHR(queue, &presentInfo);
	}

	void VulkanPresentView::cleanupSwapChain()
	{
		device->WaitDeviceIdle();

		for (uint32_t i = 0; i < ImageCount; i++)
		{
			vkDestroyImageView(device->LogicalDevice, Buffers[i].view, nullptr);
			vkDestroyFramebuffer(device->LogicalDevice, Buffers[i].framebuffer, nullptr);
			vkDestroyFence(device->LogicalDevice, Buffers[i].sync.inFlightFence, nullptr);
			vkDestroySemaphore(device->LogicalDevice, Buffers[i].sync.imageAvailableSemaphore, nullptr);
			vkDestroySemaphore(device->LogicalDevice, Buffers[i].sync.renderFinishedSemaphore, nullptr);
			vkFreeCommandBuffers(device->LogicalDevice, device->CommandPool, 1, &Buffers[i].commandBuffer);
		}

		vkDestroySwapchainKHR(device->LogicalDevice, SwapChain, nullptr);
	}

	void VulkanPresentView::createCommandBuffers() {
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = device->CommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;

		for (size_t i = 0; i < ImageCount; i++)
		{
			if (vkAllocateCommandBuffers(device->LogicalDevice, &allocInfo, &(Buffers[i].commandBuffer)) != VK_SUCCESS) {
				MK_LOG_ERROR("Failed to allocate command buffers!");
			}
		}

		
	}

	void VulkanPresentView::RecordDrawCommands(VkCommandBuffer commandBuffer, uint32_t imageIndex)
	{
		VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		beginInfo.flags = 0; // Optional
		beginInfo.pInheritanceInfo = nullptr; // Optional

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
			MK_LOG_ERROR("failed to begin recording command buffer!");
		}

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = device->GraphicsPipeline.renderPass;
		renderPassInfo.framebuffer = Buffers[imageIndex].framebuffer;
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = SwapChainExtent;
		VkClearValue clearColor = { {{1.0f, 0.5f, 0.25f, 1.0f}} };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, device->GraphicsPipeline.pipeline);

		/*
		for (glm::vec3 position : trianglePositions) {
			glm::mat4 model = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), glm::vec3(0.1,0.1,0.1));

			ObjectData data;
			data.model = model;

			vkCmdPushConstants(commandBuffer, device->GraphicsPipeline.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(data), &data);

			//vkCmdDraw(commandBuffer, 3, 1, 0, 0);
		}

		*/
		glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0,0,0)) * glm::scale(glm::mat4(1.0f), glm::vec3(1, 1, 1));

		ObjectData data;
		data.model = model;

		vkCmdPushConstants(commandBuffer, device->GraphicsPipeline.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(data), &data);
		vkCmdDraw(commandBuffer, 3, 1, 0, 0);

		vkCmdEndRenderPass(commandBuffer);

		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
			MK_LOG_ERROR("failed to record command buffer!");
		}
	}

	void VulkanPresentView::Render()
	{
		vkWaitForFences(device->LogicalDevice, 1, &(Buffers[frameNumber].sync.inFlightFence), TRUE, UINT64_MAX);
		uint32_t imageIndex;
		VkResult result = vkAcquireNextImageKHR(device->LogicalDevice, SwapChain, UINT64_MAX, (Buffers[frameNumber].sync.imageAvailableSemaphore), nullptr, &imageIndex);
			
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {

			RecreateSwapchain();
			return;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			MK_LOG_ERROR("failed to acquire swap chain image!");
		}

		VkCommandBuffer commandBuffer = Buffers[frameNumber].commandBuffer;

		vkResetCommandBuffer(commandBuffer, 0);

		RecordDrawCommands(commandBuffer,imageIndex);

		VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };

		VkSemaphore waitSemaphores[] = { Buffers[frameNumber].sync.imageAvailableSemaphore };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;
		VkSemaphore signalSemaphores[] = { Buffers[frameNumber].sync.renderFinishedSemaphore };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		vkResetFences(device->LogicalDevice, 1, &(Buffers[frameNumber].sync.inFlightFence));
		if (vkQueueSubmit(device->GraphicsQueue, 1, &submitInfo, (Buffers[frameNumber].sync.inFlightFence)) != VK_SUCCESS) {
			MK_LOG_ERROR("failed to submit draw command buffer!");
		}

		VkPresentInfoKHR presentInfo{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { SwapChain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;

		result = vkQueuePresentKHR(device->PresentQueue, &presentInfo);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
			RecreateSwapchain();
		}
		else if (result != VK_SUCCESS) {
			MK_LOG_ERROR("failed to present swap chain image!");
		}


		frameNumber = (frameNumber + 1) % maxFramesInFlight;
	}
}