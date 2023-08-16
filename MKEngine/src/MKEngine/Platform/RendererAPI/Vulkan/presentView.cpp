#include "mkpch.h"
#include "presentView.h"
#include "MKEngine/Platform/PlatformBackend.h"
#include "device.h"
#include "vkExtern.h"
#include "render_structs.h"

namespace MKEngine {
	
	std::vector<glm::vec3> trianglePositions;

	VulkanPresentView::VulkanPresentView(VulkanDevice* device)
	{
		this->m_device = device;

		MaxFramesInFlight = 2;
		FrameNumber = 0;

		for (float x = -1.0f; x < 1.0f; x+=0.2f)
		{
			for (float y = -1.0f; y < 1.0f; y += 0.2f)
			{
				trianglePositions.emplace_back(x,y,0.0f);
			}
		}
	}

	VulkanPresentView::~VulkanPresentView()
	{
		CleanupSwapChain();

		vkDestroySurfaceKHR(m_device->Instance, Surface, nullptr);

		SwapChain = VK_NULL_HANDLE;
	}

	void VulkanPresentView::InitSurface(Window* window)
	{
		Surface = VkExtern::CreateWindowSurface(m_device->Instance, window);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(m_device->PhysicalDevice,
			Surface, &formatCount, nullptr);
		assert(formatCount > 0);

		std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(m_device->PhysicalDevice,
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

		m_windowRef = window;

		MK_LOG_INFO("CHOOSED FORMAT: {0}, {1}", string_VkFormat(ColorFormat), string_VkColorSpaceKHR(ColorSpace));

	}
	
	void VulkanPresentView::CreateSwapChain()
	{
		m_device->WaitDeviceIdle();
		VkSwapchainKHR oldSwapchain = SwapChain;

		auto [title, width, height, VSync] = m_windowRef->GetData();

		VkSurfaceCapabilitiesKHR surfCaps;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_device->PhysicalDevice,
			Surface, &surfCaps);

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(m_device->PhysicalDevice, 
			Surface, &presentModeCount, nullptr);

		std::vector<VkPresentModeKHR> presentModes(presentModeCount);

		if (presentModeCount != 0) {
			presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(m_device->PhysicalDevice,
				Surface, &presentModeCount, presentModes.data());
		}

		VkExtent2D swapchainExtent = {};

		if (surfCaps.currentExtent.width == (uint32_t)-1)
		{
			swapchainExtent.width = width;
			swapchainExtent.height = height;
		}
		else
		{
			swapchainExtent = surfCaps.currentExtent;
			width = surfCaps.currentExtent.width;
			height = surfCaps.currentExtent.height;
		}

		VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;

		if (!VSync)
		{
			for (size_t i = 0; i < presentModeCount; i++)
			{
				if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
				{
					presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
					break;
				}

				if (presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR)
					presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
			}
		}

		MK_LOG_INFO("SELECTED PRESENT MODE: {0}", string_VkPresentModeKHR(presentMode));

		uint32_t desiredNumberOfSwapChainImages = surfCaps.minImageCount + 1;
		if ((surfCaps.maxImageCount > 0)
			&& (desiredNumberOfSwapChainImages > surfCaps.maxImageCount))
		{
			desiredNumberOfSwapChainImages = surfCaps.maxImageCount;
		}

		VkSurfaceTransformFlagsKHR preTransform;
		if (surfCaps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
			preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		else
			preTransform = surfCaps.currentTransform;

		VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		std::vector compositeAlphaFlags = {
		VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
		VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
		VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
		};
		for (auto& compositeAlphaFlag : compositeAlphaFlags) {
			if (surfCaps.supportedCompositeAlpha & compositeAlphaFlag) {
				compositeAlpha = compositeAlphaFlag;
				break;
			}
		}

		VkSwapchainCreateInfoKHR createInfo{ VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
		createInfo.surface = Surface;
		createInfo.minImageCount = desiredNumberOfSwapChainImages;
		createInfo.imageFormat = ColorFormat;
		createInfo.imageColorSpace = ColorSpace;
		createInfo.imageExtent =
		{ swapchainExtent.width, swapchainExtent.height };
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		createInfo.preTransform = static_cast<VkSurfaceTransformFlagBitsKHR>(preTransform);

		if (m_device->QueueFamilyIndices.Present != m_device->QueueFamilyIndices.Graphics)
		{
			uint32_t queueFamilyIndices[] = { m_device->QueueFamilyIndices.Graphics , m_device->QueueFamilyIndices.Present };
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else {
			createInfo.queueFamilyIndexCount = 0;
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}

		createInfo.presentMode = presentMode;
		//createInfo.oldSwapchain = oldSwapchain;
		createInfo.compositeAlpha = compositeAlpha;
		createInfo.clipped = VK_TRUE;

		if (surfCaps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
			createInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		if (surfCaps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT)
			createInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

		if (vkCreateSwapchainKHR(m_device->LogicalDevice, &createInfo,
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
		vkGetSwapchainImagesKHR(m_device->LogicalDevice, SwapChain,
			&ImageCount, nullptr);
		images.resize(ImageCount);
		vkGetSwapchainImagesKHR(m_device->LogicalDevice, SwapChain, 
			&ImageCount, images.data());

		Buffers.resize(ImageCount);

		for (uint32_t i = 0; i < ImageCount; i++)
		{
			VkImageViewCreateInfo viewInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
			viewInfo.pNext = nullptr;
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

			Buffers[i].Image = images[i];

			viewInfo.image = Buffers[i].Image;

			vkCreateImageView(m_device->LogicalDevice, &viewInfo, nullptr, &Buffers[i].View);
		}

		SwapChainExtent = swapchainExtent;

		MK_LOG_INFO("Created PresentViews {0} - {1}x{2}", ImageCount, width, height);
	}

	void VulkanPresentView::FinalizeCreation()
	{
		CreateFrameBuffer();
		CreateCommandBuffers();

		CreateSync();
	}

	void VulkanPresentView::RecreateSwapChain()
	{
		MK_LOG_INFO("Recreate swapchain");

		int width = 0;
		int height = 0;

		MKEngine::PlatformBackend::GetWindowSize(m_windowRef, &width, &height);

		while (width == 0 || height == 0)
		{
			PlatformBackend::CurrentBackend->HandleEvents();

			MKEngine::PlatformBackend::GetWindowSize(m_windowRef, &width, &height);
		}

		CleanupSwapChain();
		CreateSwapChain();
		//Graphics pipeline ()
		FinalizeCreation();
	}

	void VulkanPresentView::CreateFrameBuffer()
	{
		for (size_t i = 0; i < ImageCount; i++) {
			const VkImageView attachments[] = {
				Buffers[i].View
			};

			VkFramebufferCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			createInfo.renderPass = m_device->GraphicsPipeline.RenderPass;
			createInfo.attachmentCount = 1;
			createInfo.pAttachments = attachments;
			createInfo.width = SwapChainExtent.width;
			createInfo.height = SwapChainExtent.height;
			createInfo.layers = 1;

			if (vkCreateFramebuffer(m_device->LogicalDevice, &createInfo, VK_NULL_HANDLE, &(Buffers[i].FrameBuffer)) != VK_SUCCESS) {
				MK_LOG_ERROR("Failed to create framebuffer");
			}
		
		}
		MK_LOG_INFO("Created framebuffers x{0}", ImageCount);
	}

	void VulkanPresentView::CreateSync()
	{
		for (size_t i = 0; i < ImageCount; i++)
		{
			ViewSync sync{};
			sync.InFlightFence = VkExtern::CreateFence(m_device->LogicalDevice);
			sync.ImageAvailableSemaphore = VkExtern::CreateSemaphore(m_device->LogicalDevice);
			sync.RenderFinishedSemaphore = VkExtern::CreateSemaphore(m_device->LogicalDevice);
			Buffers[i].Sync = sync;
		}
	}

	VkResult VulkanPresentView::AcquireNextImage(const VkSemaphore presentCompleteSemaphore, uint32_t* imageIndex) const
	{
		return vkAcquireNextImageKHR(m_device->LogicalDevice, SwapChain, UINT64_MAX,
			presentCompleteSemaphore, VK_NULL_HANDLE, imageIndex);
	}

	VkResult VulkanPresentView::QueuePresent(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore) const
	{
		VkPresentInfoKHR presentInfo = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
		presentInfo.pNext = VK_NULL_HANDLE;
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

	void VulkanPresentView::BeginRender()
	{
		vkWaitForFences(m_device->LogicalDevice, 1, &(Buffers[FrameNumber].Sync.InFlightFence), TRUE, UINT64_MAX);
		uint32_t imageIndex;
		VkResult result = vkAcquireNextImageKHR(m_device->LogicalDevice, SwapChain, UINT64_MAX, (Buffers[FrameNumber].Sync.ImageAvailableSemaphore), nullptr, &imageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR) {

			RecreateSwapChain();
			return;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			MK_LOG_ERROR("failed to acquire swap chain image!");
		}

		const VkCommandBuffer commandBuffer = Buffers[FrameNumber].CommandBuffer;

		vkResetCommandBuffer(commandBuffer, 0);

		m_currentImageIndexDraw = imageIndex;
		m_currentBufferDraw = commandBuffer;
	}

	void VulkanPresentView::EndRender()
	{
		auto commandBuffer = m_currentBufferDraw;
		auto imageIndex = m_currentImageIndexDraw;

		VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };

		const VkSemaphore waitSemaphores[] = { Buffers[FrameNumber].Sync.ImageAvailableSemaphore };
		constexpr VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;
		const VkSemaphore signalSemaphores[] = { Buffers[FrameNumber].Sync.RenderFinishedSemaphore };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		vkResetFences(m_device->LogicalDevice, 1, &(Buffers[FrameNumber].Sync.InFlightFence));
		if (vkQueueSubmit(m_device->GraphicsQueue, 1, &submitInfo, (Buffers[FrameNumber].Sync.InFlightFence)) != VK_SUCCESS) {
			MK_LOG_ERROR("failed to submit draw command buffer!");
		}

		VkPresentInfoKHR presentInfo{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		const VkSwapchainKHR swapChains[] = { SwapChain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;

		VkResult result = vkQueuePresentKHR(m_device->PresentQueue, &presentInfo);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
			RecreateSwapChain();
		}
		else if (result != VK_SUCCESS) {
			MK_LOG_ERROR("failed to present swap chain image!");
		}


		FrameNumber = (FrameNumber + 1) % MaxFramesInFlight;
	}

	void VulkanPresentView::CleanupSwapChain() const
	{
		m_device->WaitDeviceIdle();

		for (uint32_t i = 0; i < ImageCount; i++)
		{
			vkDestroyImageView(m_device->LogicalDevice, Buffers[i].View, VK_NULL_HANDLE);
			vkDestroyFramebuffer(m_device->LogicalDevice, Buffers[i].FrameBuffer, VK_NULL_HANDLE);
			vkDestroyFence(m_device->LogicalDevice, Buffers[i].Sync.InFlightFence, VK_NULL_HANDLE);
			vkDestroySemaphore(m_device->LogicalDevice, Buffers[i].Sync.ImageAvailableSemaphore, VK_NULL_HANDLE);
			vkDestroySemaphore(m_device->LogicalDevice, Buffers[i].Sync.RenderFinishedSemaphore, VK_NULL_HANDLE);
			vkFreeCommandBuffers(m_device->LogicalDevice, m_device->CommandPool, 1, &Buffers[i].CommandBuffer);
		}

		vkDestroySwapchainKHR(m_device->LogicalDevice, SwapChain, VK_NULL_HANDLE);
	}

	void VulkanPresentView::CreateCommandBuffers() {
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = m_device->CommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;

		for (size_t i = 0; i < ImageCount; i++)
		{
			if (vkAllocateCommandBuffers(m_device->LogicalDevice, &allocInfo, &(Buffers[i].CommandBuffer)) != VK_SUCCESS) {
				MK_LOG_ERROR("Failed to allocate command buffers!");
			}
		}

		
	}

	void VulkanPresentView::RecordDrawCommands(VkCommandBuffer commandBuffer, uint32_t imageIndex, int testIndex)
	{
		VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		beginInfo.flags = 0; // Optional
		beginInfo.pInheritanceInfo = nullptr; // Optional

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
			MK_LOG_ERROR("failed to begin recording command buffer!");
		}

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_device->GraphicsPipeline.RenderPass;
		renderPassInfo.framebuffer = Buffers[imageIndex].FrameBuffer;
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = SwapChainExtent;
		constexpr VkClearValue clearColor = { {{1.0f, 0.5f, 0.25f, 1.0f}} };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_device->GraphicsPipeline.Reference);

		WindowData winData = m_windowRef->GetData();

		VkViewport viewport = {
			0,
			0,
			static_cast<float>(winData.Width),
			static_cast<float>(winData.Height)};
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = VkExtent2D{
			winData.Width,
			winData.Height
		};

		vkCmdSetScissor(commandBuffer,0,1,&scissor);

		if(testIndex == 0)
		{
			for (glm::vec3 position : trianglePositions) {
				const glm::mat4 model = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), glm::vec3(0.1, 0.1, 0.1));

				ObjectData data;
				data.Model = model;

				const VkBuffer vertexBuffers[] = { this->m_device->VertexBuffer.Resource };
				constexpr VkDeviceSize offsets[] = { 0 };
				vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
				vkCmdBindIndexBuffer(commandBuffer, this->m_device->IndicesBuffer.Resource, 0, VK_INDEX_TYPE_UINT16);

				vkCmdPushConstants(commandBuffer, m_device->GraphicsPipeline.PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(data), &data);
				vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(Indices.size()), 1, 0, 0, 0);
			}
		}
		else
		{
			const glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0)) * glm::scale(glm::mat4(1.0f), glm::vec3(1, 1, 1));

			ObjectData data;
			data.Model = model;

			const VkBuffer vertexBuffers[] = { this->m_device->VertexBuffer.Resource };
			constexpr VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
			vkCmdBindIndexBuffer(commandBuffer, this->m_device->IndicesBuffer.Resource, 0, VK_INDEX_TYPE_UINT16);

			vkCmdPushConstants(commandBuffer, m_device->GraphicsPipeline.PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(data), &data);
			vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(Indices.size()), 1, 0, 0, 0);
		}
	

		vkCmdEndRenderPass(commandBuffer);

		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
			MK_LOG_ERROR("failed to record command buffer!");
		}
	}

	void VulkanPresentView::Record(int testIndex)
	{
		RecordDrawCommands(m_currentBufferDraw, m_currentImageIndexDraw, testIndex);
	}
	/*
	 *
	void VulkanPresentView::Render()
	{
		

		//RecordDrawCommands(commandBuffer,imageIndex);

		
	}
	 *
	 */
}
