#include <mkpch.h>

#include <vulkan/vk_enum_string_helper.h>

#include "presentView.h"
#include "MKEngine/Platform/PlatformBackend.h"
#include "vkExtern.h"
#include "VulkanAPI.h"
#include "MKEngine/Input/input.h"
#include "Rendering/RenderingInfo.h"

namespace MKEngine {

	PresentView* PresentView::Create()
	{
		const auto presentView = new PresentView();

		presentView->MaxFramesInFlight = 2;
		presentView->FrameNumber = 0;

		return presentView;
	}

	void PresentView::Destroy(PresentView* presentView)
	{
		presentView->CleanupSwapChain();

		vkDestroySurfaceKHR(VkContext::API->Instance, presentView->Surface, nullptr);

		presentView->SwapChain = VK_NULL_HANDLE;
	}

	void PresentView::InitSurface(Window* window)
	{
		Surface = VkExtern::CreateWindowSurface(VkContext::API->Instance, window);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(VkContext::API->PhysicalDevice,
			Surface, &formatCount, nullptr);
		assert(formatCount > 0);

		std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(VkContext::API->PhysicalDevice,
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
	
	void PresentView::CreateSwapChain()
	{
		WaitDeviceIdle();
		VkSwapchainKHR oldSwapchain = SwapChain;

		auto [title, width, height, VSync] = m_windowRef->GetData();

		VkSurfaceCapabilitiesKHR surfCaps;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(VkContext::API->PhysicalDevice,
			Surface, &surfCaps);

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(VkContext::API->PhysicalDevice,
			Surface, &presentModeCount, nullptr);

		std::vector<VkPresentModeKHR> presentModes(presentModeCount);

		if (presentModeCount != 0) {
			presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(VkContext::API->PhysicalDevice,
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

		if (VkContext::API->QueueFamilyIndices.Present != VkContext::API->QueueFamilyIndices.Graphics)
		{
			uint32_t queueFamilyIndices[] = { VkContext::API->QueueFamilyIndices.Graphics , VkContext::API->QueueFamilyIndices.Present };
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

		if (vkCreateSwapchainKHR(VkContext::API->LogicalDevice, &createInfo,
			nullptr, &SwapChain) != VK_SUCCESS) {
			MK_LOG_CRITICAL("failed to create swap chain!");
		}

		if (oldSwapchain != VK_NULL_HANDLE) {
			
			vkDestroySwapchainKHR(VkContext::API->LogicalDevice, oldSwapchain, VK_NULL_HANDLE);
		}

		std::vector<VkImage> images;
		vkGetSwapchainImagesKHR(VkContext::API->LogicalDevice, SwapChain,
			&ImageCount, nullptr);
		images.resize(ImageCount);
		vkGetSwapchainImagesKHR(VkContext::API->LogicalDevice, SwapChain,
			&ImageCount, images.data());

		Buffers.resize(ImageCount);

		for (uint32_t i = 0; i < ImageCount; i++)
		{
			ImageViewDescription imageViewDescription;
			imageViewDescription.Format = ColorFormat;
			imageViewDescription.IsSwapchain = true;
			imageViewDescription.Image = Image::Create(images[i], VK_IMAGE_ASPECT_COLOR_BIT);

			Buffers[i].View = ImageView::Create(imageViewDescription);
			//Buffers[i].View.Image.TransitionImageLayout(ColorFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		}

		SwapChainExtent = swapchainExtent;

		MK_LOG_INFO("Created PresentViews {0} - {1}x{2}", ImageCount, width, height);
	}

	void PresentView::FinalizeCreation()
	{
		CreateUniformBuffers();
		CreateCommandBuffers();
		CreateSync();
		CreateDescriptorSets();

		const auto [Title, Width, Height, VSync] = m_windowRef->GetData();

		for (size_t i = 0; i < Buffers.size(); i++)
		{
			ImageDescription imageDescription;
			imageDescription.Width = Width;
			imageDescription.Height = Height;
			imageDescription.Format = VK_FORMAT_D32_SFLOAT_S8_UINT;
			imageDescription.Usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

			const Image depthImage = Image::Create(imageDescription);

			ImageViewDescription imageViewDescription;
			imageViewDescription.Format = VK_FORMAT_D32_SFLOAT_S8_UINT;
			imageViewDescription.Aspect = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
			imageViewDescription.Image = depthImage;

			Buffers[i].Depth = ImageView::Create(imageViewDescription);

			Buffers[i].Depth.Image.TransitionImageLayout(VK_FORMAT_D32_SFLOAT_S8_UINT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
		}
	}

	void PresentView::RecreateSwapChain()
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
	
	void PresentView::CreateSync()
	{
		for (size_t i = 0; i < ImageCount; i++)
		{
			ViewSync sync{};
			sync.ImageAvailableSemaphore = VkExtern::CreateSemaphore(VkContext::API->LogicalDevice);
			sync.RenderFinishedSemaphore = VkExtern::CreateSemaphore(VkContext::API->LogicalDevice);
			Buffers[i].Sync = sync;
		}
	}

	VkResult PresentView::AcquireNextImage(const VkSemaphore presentCompleteSemaphore, uint32_t* imageIndex) const
	{
		return vkAcquireNextImageKHR(VkContext::API->LogicalDevice, SwapChain, UINT64_MAX,
			presentCompleteSemaphore, VK_NULL_HANDLE, imageIndex);
	}

	VkResult PresentView::QueuePresent(const VkQueue queue, const uint32_t imageIndex, const VkSemaphore waitSemaphore) const
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

	void PresentView::Render()
	{
		if(m_renderIsBegin)
		{
			EndRender();
			m_renderIsBegin = false;
		}

		BeginRender();

		const glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0)) * glm::scale(glm::mat4(1.0f), glm::vec3(1, 1, 1));
		ObjectData data;
		data.Model = model;

		vkCmdPushConstants(m_currentBufferDraw.Resource, VkContext::API->GraphicsPipeline.PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(data), &data);

		VulkanAPI::testModel.Draw(m_currentBufferDraw.Resource);

		m_renderIsBegin = true;
	}
	
	void PresentView::CleanupSwapChain(const bool destroySwapChain) const
	{
		WaitDeviceIdle();

		for (uint32_t i = 0; i < ImageCount; i++)
		{
			ImageView::Destroy(Buffers[i].Depth);
			Image::Destroy(Buffers[i].Depth.Image);
			ImageView::Destroy(Buffers[i].View);
			vkDestroySemaphore(VkContext::API->LogicalDevice, Buffers[i].Sync.ImageAvailableSemaphore, VK_NULL_HANDLE);
			vkDestroySemaphore(VkContext::API->LogicalDevice, Buffers[i].Sync.RenderFinishedSemaphore, VK_NULL_HANDLE);
			CommandBuffer::Destroy(Buffers[i].CommandBuffer);
			Buffer::Destroy(Buffers[i].UniformBuffer);
			DescriptorSet::Destroy(Buffers[i].DescriptorSet);
		}

		if(destroySwapChain)
			vkDestroySwapchainKHR(VkContext::API->LogicalDevice, SwapChain, VK_NULL_HANDLE);
	}

	void PresentView::CreateCommandBuffers() {

		CommandBufferDescription description;
		description.Level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		description.FamilyIndex = VkContext::API->QueueFamilyIndices.Graphics;
		description.Flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		//description.CommandPool = VkContext::API->CommandBuffer.CommandPool;

		for (size_t i = 0; i < ImageCount; i++)
		{
			Buffers[i].CommandBuffer = CommandBuffer::Create(description);
		}
	}

	void PresentView::CreateUniformBuffers()
	{
		UniformBuffers.resize(ImageCount);
		BufferDescription bufferDescription;
		bufferDescription.Access = ACCESS_HOST;
		bufferDescription.Usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		bufferDescription.Size = sizeof(UniformBufferObject);


		for (size_t i = 0; i < ImageCount; i++)
		{
			Buffers[i].UniformBuffer = Buffer::Create(bufferDescription);

			//vmaMapMemory(VkContext::API->VMAAllocator, Buffers[i].UniformBuffer.Allocation, &Buffers[i].UniformBuffer.MappedData);

		}
	}

	void PresentView::UpdateUniformBuffer() const
	{
		const auto [Title, Width, Height, VSync] = m_windowRef->GetData();
		static auto startTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();
		const float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

		static float clickedTime;

		if(!Input::getMouseButton(Mouse::Button0))
			clickedTime = time;

		UniformBufferObject ubo{};
		ubo.model = glm::rotate(glm::mat4(1.0f), clickedTime * glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

		const auto wnd = m_windowRef->GetData();

		VulkanAPI::testCamera.UpdateAspectRatio(static_cast<float>(wnd.Width) / wnd.Height);

		CameraInput input;

		if (Input::getKey(Key::A))
			input.moveInput.x -= 1;
		if (Input::getKey(Key::D))
			input.moveInput.x += 1;
		if (Input::getKey(Key::W))
			input.moveInput.y += 1;
		if (Input::getKey(Key::S))
			input.moveInput.y -= 1;

		VulkanAPI::testCamera.Update(Application::DeltaTime, input);

		const auto wheelDelta = Input::getMouseScrollDelta();

		float deltaY = 0;
		float deltaX = 0;

		if (Input::getMouseButton(Mouse::LeftButton))
		{
			deltaX = Input::getMouseDeltaX();
			deltaY = -Input::getMouseDeltaY();
		}
		if (Input::getMouseButton(Mouse::MiddleButton))
		{
			VulkanAPI::testCamera.Translate(glm::vec3(0.0f, Input::getMouseDeltaY() *0.05f,0));
		}

		VulkanAPI::testCamera.Rotate(glm::vec3(deltaY, 0.0f, 0.0f));
		VulkanAPI::testCamera.Rotate(glm::vec3(0.0f, deltaX, 0.0f));
		VulkanAPI::testCamera.Translate(glm::vec3(0.0f, 0.0f, (float)wheelDelta * 0.5f));

		ubo.view = VulkanAPI::testCamera.Matrices.View;
		ubo.proj = VulkanAPI::testCamera.Matrices.Perspective;
		//ubo.model = glm::mat4(1.0);

		memcpy(Buffers[m_currentImageIndexDraw].UniformBuffer.MappedData, &ubo, sizeof(ubo));
	}
	
	void PresentView::CreateDescriptorSets()
	{
		DescriptorSetDescription desc;
		desc.Layout = VkContext::API->DescriptorSetLayout;

		for (size_t i = 0; i < ImageCount; i++)
		{
			auto descriptorSet = DescriptorSet::Create(desc);
			descriptorSet.BindBuffer(0, Buffers[i].UniformBuffer.Resource, 0, sizeof(UniformBufferObject));
			descriptorSet.BindCombinedImageSampler(1, VulkanAPI::testTexture.View, VulkanAPI::testTexture.Sampler);

			Buffers[i].DescriptorSet = descriptorSet;
		}
	}

	void PresentView::EndRender()
	{
		m_currentBufferDraw.EndRendering();

		m_currentBufferDraw.ImageMemoryBarrier(
			Buffers[m_currentImageIndexDraw].View.Image,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
			0, 
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
		);

		m_currentBufferDraw.End();

		VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };

		const VkSemaphore waitSemaphores[] = { Buffers[FrameNumber].Sync.ImageAvailableSemaphore };
		constexpr VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_currentBufferDraw.Resource;
		const VkSemaphore signalSemaphores[] = { Buffers[FrameNumber].Sync.RenderFinishedSemaphore };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		m_currentBufferDraw.WaitFence.Reset();

		if (vkQueueSubmit(VkContext::API->GraphicsQueue, 1, &submitInfo, m_currentBufferDraw.WaitFence.Resource) != VK_SUCCESS) {
			MK_LOG_ERROR("failed to submit draw command buffer!");
		}

		VkPresentInfoKHR presentInfo{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &SwapChain;
		presentInfo.pImageIndices = &m_currentImageIndexDraw;

		const VkResult result = vkQueuePresentKHR(VkContext::API->PresentQueue, &presentInfo);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
			RecreateSwapChain();
		}
		else if (result != VK_SUCCESS) {
			MK_LOG_ERROR("failed to present swap chain image!");
		}

		FrameNumber = (FrameNumber + 1) % MaxFramesInFlight;
	}

	void PresentView::BeginRender()
	{
		m_currentBufferDraw = Buffers[FrameNumber].CommandBuffer;

		//Wait CPU unlock 
		m_currentBufferDraw.WaitForExecute();

		//Get image to draw

		if (const VkResult result =
			vkAcquireNextImageKHR(VkContext::API->LogicalDevice, SwapChain, UINT64_MAX, (Buffers[FrameNumber].Sync.ImageAvailableSemaphore), nullptr, &m_currentImageIndexDraw);
			result == VK_ERROR_OUT_OF_DATE_KHR) {
			RecreateSwapChain();
			return;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			MK_LOG_ERROR("failed to acquire swap chain image!");
		}

		//Reset command buffer
		m_currentBufferDraw.Reset();

		const auto [Title, Width, Height, VSync] = m_windowRef->GetData();

		//Begin command buffer
		m_currentBufferDraw.Begin();

		m_currentBufferDraw.ImageMemoryBarrier(
			Buffers[m_currentImageIndexDraw].View.Image,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			0,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
			);

		m_currentBufferDraw.ImageMemoryBarrier(
			Buffers[m_currentImageIndexDraw].Depth.Image,
			VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
			0,
			VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
			VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
			);

		

		RenderingInfo info({0,0,Width,Height});
		info.AddColorAttachment(Buffers[m_currentImageIndexDraw].View.Resource);
		info.SetDepthAttachment(Buffers[m_currentImageIndexDraw].Depth.Resource);

		m_currentBufferDraw.BeginRendering(info);

		m_currentBufferDraw.BindPipeline(VkContext::API->GraphicsPipeline);

		m_currentBufferDraw.SetViewport(0, 0, Width, Height);

		m_currentBufferDraw.SetScissor(0, 0, Width, Height);

		UpdateUniformBuffer();

		m_currentBufferDraw.BindDescriptorSet(Buffers[m_currentImageIndexDraw].DescriptorSet);

	}

}
