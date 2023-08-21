#include <mkpch.h>

#include <vulkan/vk_enum_string_helper.h>
#include "vkState.h"
#include "presentView.h"
#include "MKEngine/Platform/PlatformBackend.h"
#include "vkExtern.h"
#include "../vertex.h"
#include "vkFunctions.h"

namespace MKEngine {
	
	std::vector<glm::vec3> trianglePositions;

	VulkanPresentView::VulkanPresentView()
	{
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

		vkDestroySurfaceKHR(vkState::API->Instance, Surface, nullptr);

		SwapChain = VK_NULL_HANDLE;
	}

	void VulkanPresentView::InitSurface(Window* window)
	{
		Surface = VkExtern::CreateWindowSurface(vkState::API->Instance, window);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(vkState::API->PhysicalDevice,
			Surface, &formatCount, nullptr);
		assert(formatCount > 0);

		std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(vkState::API->PhysicalDevice,
			Surface, &formatCount, surfaceFormats.data());

		if ((formatCount == 1) &&
			(surfaceFormats[0].format == VK_FORMAT_UNDEFINED))
		{
			ColorFormat = VK_FORMAT_B8G8R8A8_SRGB;
			ColorSpace = surfaceFormats[0].colorSpace;
		}
		else
		{
			bool found_B8G8R8A8_UNORM = false;
			for (auto&& surfaceFormat : surfaceFormats)
			{
				if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_SRGB)
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
		WaitDeviceIdle();
		VkSwapchainKHR oldSwapchain = SwapChain;

		auto [title, width, height, VSync] = m_windowRef->GetData();

		VkSurfaceCapabilitiesKHR surfCaps;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkState::API->PhysicalDevice,
			Surface, &surfCaps);

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(vkState::API->PhysicalDevice,
			Surface, &presentModeCount, nullptr);

		std::vector<VkPresentModeKHR> presentModes(presentModeCount);

		if (presentModeCount != 0) {
			presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(vkState::API->PhysicalDevice,
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

		if (vkState::API->QueueFamilyIndices.Present != vkState::API->QueueFamilyIndices.Graphics)
		{
			uint32_t queueFamilyIndices[] = { vkState::API->QueueFamilyIndices.Graphics , vkState::API->QueueFamilyIndices.Present };
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else {
			createInfo.queueFamilyIndexCount = 0;
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}

		createInfo.presentMode = presentMode;
		createInfo.oldSwapchain = oldSwapchain;
		createInfo.compositeAlpha = compositeAlpha;
		createInfo.clipped = VK_TRUE;

		if (surfCaps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
			createInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		if (surfCaps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT)
			createInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

		if (vkCreateSwapchainKHR(vkState::API->LogicalDevice, &createInfo,
			nullptr, &SwapChain) != VK_SUCCESS) {
			MK_LOG_CRITICAL("failed to create swap chain!");
		}


		if (oldSwapchain != VK_NULL_HANDLE) {
			
			vkDestroySwapchainKHR(vkState::API->LogicalDevice, oldSwapchain, VK_NULL_HANDLE);
		}


		std::vector<VkImage> images;
		vkGetSwapchainImagesKHR(vkState::API->LogicalDevice, SwapChain,
			&ImageCount, nullptr);
		images.resize(ImageCount);
		vkGetSwapchainImagesKHR(vkState::API->LogicalDevice, SwapChain,
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

			vkCreateImageView(vkState::API->LogicalDevice, &viewInfo, nullptr, &Buffers[i].View);

			
		}

		SwapChainExtent = swapchainExtent;

		MK_LOG_INFO("Created PresentViews {0} - {1}x{2}", ImageCount, width, height);
	}

	void VulkanPresentView::FinalizeCreation()
	{
		CreateUniformBuffers();
		CreateFrameBuffer();
		CreateCommandBuffers();

		CreateSync();
		CreateDescriptorPool();
		CreateDescriptorSets();
	}

	void VulkanPresentView::RecreateSwapChain()
	{
		m_renderIsBegin = false;
		MK_LOG_INFO("Recreate swapchain");

		int width = 0;
		int height = 0;

		MKEngine::PlatformBackend::GetWindowSize(m_windowRef, &width, &height);

		while (width == 0 || height == 0)
		{
			PlatformBackend::CurrentBackend->HandleEvents();

			MKEngine::PlatformBackend::GetWindowSize(m_windowRef, &width, &height);
		}

		CleanupSwapChain(false);

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
			createInfo.renderPass = vkState::API->RenderPass;
			createInfo.attachmentCount = 1;
			createInfo.pAttachments = attachments;
			createInfo.width = SwapChainExtent.width;
			createInfo.height = SwapChainExtent.height;
			createInfo.layers = 1;

			if (vkCreateFramebuffer(vkState::API->LogicalDevice, &createInfo, VK_NULL_HANDLE, &(Buffers[i].FrameBuffer)) != VK_SUCCESS) {
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
			sync.InFlightFence = VkExtern::CreateFence(vkState::API->LogicalDevice);
			sync.ImageAvailableSemaphore = VkExtern::CreateSemaphore(vkState::API->LogicalDevice);
			sync.RenderFinishedSemaphore = VkExtern::CreateSemaphore(vkState::API->LogicalDevice);
			Buffers[i].Sync = sync;
		}
	}

	VkResult VulkanPresentView::AcquireNextImage(const VkSemaphore presentCompleteSemaphore, uint32_t* imageIndex) const
	{
		return vkAcquireNextImageKHR(vkState::API->LogicalDevice, SwapChain, UINT64_MAX,
			presentCompleteSemaphore, VK_NULL_HANDLE, imageIndex);
	}

	VkResult VulkanPresentView::QueuePresent(const VkQueue queue, const uint32_t imageIndex, const VkSemaphore waitSemaphore) const
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
		if(m_renderIsBegin)
		{
			auto commandBuffer = m_currentBufferDraw;
			auto imageIndex = m_currentImageIndexDraw;

			vkCmdEndRenderPass(commandBuffer);

			if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
				MK_LOG_ERROR("failed to record command buffer!");
			}

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

			vkResetFences(vkState::API->LogicalDevice, 1, &(Buffers[FrameNumber].Sync.InFlightFence));
			if (vkQueueSubmit(vkState::API->GraphicsQueue, 1, &submitInfo, (Buffers[FrameNumber].Sync.InFlightFence)) != VK_SUCCESS) {
				MK_LOG_ERROR("failed to submit draw command buffer!");
			}

			VkPresentInfoKHR presentInfo{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };

			presentInfo.waitSemaphoreCount = 1;
			presentInfo.pWaitSemaphores = signalSemaphores;

			const VkSwapchainKHR swapChains[] = { SwapChain };
			presentInfo.swapchainCount = 1;
			presentInfo.pSwapchains = swapChains;
			presentInfo.pImageIndices = &imageIndex;

			VkResult result = vkQueuePresentKHR(vkState::API->PresentQueue, &presentInfo);

			if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
				RecreateSwapChain();
			}
			else if (result != VK_SUCCESS) {
				MK_LOG_ERROR("failed to present swap chain image!");
			}


			FrameNumber = (FrameNumber + 1) % MaxFramesInFlight;

			m_renderIsBegin = false;
		}

		//Wait CPU unlock 
		vkWaitForFences(vkState::API->LogicalDevice, 1, &(Buffers[FrameNumber].Sync.InFlightFence), TRUE, UINT64_MAX);
		uint32_t imageIndex;
		//Get image to draw
		VkResult result = vkAcquireNextImageKHR(vkState::API->LogicalDevice, SwapChain, UINT64_MAX, (Buffers[FrameNumber].Sync.ImageAvailableSemaphore), nullptr, &imageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR) {

			RecreateSwapChain();
			return;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			MK_LOG_ERROR("failed to acquire swap chain image!");
		}

		//Reset command buffer
		const VkCommandBuffer commandBuffer = Buffers[FrameNumber].CommandBuffer;

		vkResetCommandBuffer(commandBuffer, 0);

		m_currentImageIndexDraw = imageIndex;
		m_currentBufferDraw = commandBuffer;

		m_renderIsBegin = true;

		//Begin renderPass
		VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		beginInfo.flags = 0; // Optional
		beginInfo.pInheritanceInfo = nullptr; // Optional

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
			MK_LOG_ERROR("failed to begin recording command buffer!");
		}

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = vkState::API->RenderPass;
		renderPassInfo.framebuffer = Buffers[imageIndex].FrameBuffer;
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = SwapChainExtent;
		constexpr VkClearValue clearColor = { {{1.0f, 0.5f, 0.25f, 1.0f}} };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkState::API->GraphicsPipeline);

		const auto [Title, Width, Height, VSync] = m_windowRef->GetData();

		const VkViewport viewport = {
			0,
			0,
			static_cast<float>(Width),
			static_cast<float>(Height) };
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = VkExtent2D{
			Width,
			Height
		};

		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		UpdateUniformBuffer(imageIndex);

		vkCmdBindDescriptorSets(
			commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			vkState::API->PipelineLayout,
			0,
			1,
			&Buffers[imageIndex].DescriptorSet,
			0,
			nullptr);
	}

	void VulkanPresentView::CleanupSwapChain(bool destroySwapChain) const
	{
		WaitDeviceIdle();

		for (uint32_t i = 0; i < ImageCount; i++)
		{
			vkDestroyImageView(vkState::API->LogicalDevice, Buffers[i].View, VK_NULL_HANDLE);
			vkDestroyFramebuffer(vkState::API->LogicalDevice, Buffers[i].FrameBuffer, VK_NULL_HANDLE);
			vkDestroyFence(vkState::API->LogicalDevice, Buffers[i].Sync.InFlightFence, VK_NULL_HANDLE);
			vkDestroySemaphore(vkState::API->LogicalDevice, Buffers[i].Sync.ImageAvailableSemaphore, VK_NULL_HANDLE);
			vkDestroySemaphore(vkState::API->LogicalDevice, Buffers[i].Sync.RenderFinishedSemaphore, VK_NULL_HANDLE);
			vkFreeCommandBuffers(vkState::API->LogicalDevice, vkState::API->CommandPool, 1, &Buffers[i].CommandBuffer);
			DestroyBuffer(Buffers[i].UniformBuffer);
		}

		vkDestroyDescriptorPool(vkState::API->LogicalDevice, DescriptorPool, nullptr);
		if(destroySwapChain)
			vkDestroySwapchainKHR(vkState::API->LogicalDevice, SwapChain, VK_NULL_HANDLE);


	}

	void VulkanPresentView::CreateCommandBuffers() {
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = vkState::API->CommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;

		for (size_t i = 0; i < ImageCount; i++)
		{
			if (vkAllocateCommandBuffers(vkState::API->LogicalDevice, &allocInfo, &(Buffers[i].CommandBuffer)) != VK_SUCCESS) {
				MK_LOG_ERROR("Failed to allocate command buffers!");
			}
		}
	}

	void VulkanPresentView::CreateUniformBuffers()
	{
		UniformBuffers.resize(ImageCount);
		BufferDescription bufferDescription{};
		bufferDescription.Access = DataAccess::Host;
		bufferDescription.Usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		bufferDescription.Size = sizeof(UniformBufferObject);

		for (size_t i = 0; i < ImageCount; i++)
		{
			Buffers[i].UniformBuffer = CreateBuffer(bufferDescription);

			vkMapMemory(vkState::API->LogicalDevice, Buffers[i].UniformBuffer.Memory, 0, bufferDescription.Size, 0, &Buffers[i].UniformBuffer.MappedData);

		}
	}

	void VulkanPresentView::UpdateUniformBuffer(const uint32_t currentImage) {
		static auto startTime = std::chrono::high_resolution_clock::now();

		const auto currentTime = std::chrono::high_resolution_clock::now();
		const float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

		UniformBufferObject ubo{};
		ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		ubo.view = glm::lookAtRH(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));

		ubo.proj = glm::perspectiveRH_ZO(glm::radians(45.0f), SwapChainExtent.width / (float)SwapChainExtent.height, 0.1f, 10.0f);
		//ubo.proj[1][1] *= -1;
		memcpy(Buffers[currentImage].UniformBuffer.MappedData, &ubo, sizeof(ubo));
	}

	void VulkanPresentView::CreateDescriptorPool()
	{
		std::array<VkDescriptorPoolSize, 2> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = ImageCount;
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = ImageCount;

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = ImageCount;
		if (vkCreateDescriptorPool(vkState::API->LogicalDevice, &poolInfo, nullptr, &DescriptorPool) != VK_SUCCESS) {
			MK_LOG_ERROR("Failed to create descriptor pool!");
		}
	}

	void VulkanPresentView::CreateDescriptorSets()
	{
		VkDescriptorSetAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
		allocInfo.descriptorPool = DescriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &vkState::API->DescriptorSetLayout;

		for (size_t i = 0; i < ImageCount; i++)
		{
			if (vkAllocateDescriptorSets(vkState::API->LogicalDevice, &allocInfo, &Buffers[i].DescriptorSet) != VK_SUCCESS) {
				MK_LOG_ERROR("failed to allocate descriptor sets!");
			}
		}

		for (size_t i = 0; i < ImageCount; i++) {
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = Buffers[i].UniformBuffer.Resource;
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(UniformBufferObject);

			//VkDescriptorImageInfo imageInfo{};
			//imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			//imageInfo.imageView = vkState::API->TestTexture.View;
			//imageInfo.sampler = vkState::API->TestTexture.Sampler;

			std::array<VkWriteDescriptorSet, 1> descriptorWrites{};

			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = Buffers[i].DescriptorSet;
			descriptorWrites[0].dstBinding = 0;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &bufferInfo;

			/*
			 *descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[1].dstSet = Buffers[i].DescriptorSet;
			descriptorWrites[1].dstBinding = 1;
			descriptorWrites[1].dstArrayElement = 0;
			descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[1].descriptorCount = 1;
			descriptorWrites[1].pImageInfo = &imageInfo;
			 *
			 */
		

			vkUpdateDescriptorSets(vkState::API->LogicalDevice, descriptorWrites.size(),
				descriptorWrites.data(), 0, nullptr);
		}
	}

}
