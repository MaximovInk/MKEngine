#include "mkpch.h"

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#include "VulkanAPI.h"
#include "vkExtern.h"
#include "vkFunctions.h"

#include "vkState.h"

namespace MKEngine {

	void VulkanAPI::Initialize()
	{
		vkState::API = new vkState();

		vkState::API->Instance = VkExtern::CreateInstance();
#if VULKAN_VALIDATION
		DebugMessenger = VkExtern::CreateDebugMessenger(vkState::API->Instance);
#endif

		Device = CreateDevice();

		VmaAllocatorCreateInfo allocatorCreateInfo{};
		allocatorCreateInfo.device = vkState::API->LogicalDevice;
		allocatorCreateInfo.instance = vkState::API->Instance;
		allocatorCreateInfo.physicalDevice = vkState::API->PhysicalDevice;

		vmaCreateAllocator(&allocatorCreateInfo, &vkState::API->VMAAllocator);

		GraphicsPipelineDescescription description{  };
		description.SwapChainFormat = VK_FORMAT_B8G8R8A8_SRGB;
		
		GraphicsPipeline = CreateGraphicsPipeline(description);

		vkState::API->GraphicsPipeline = GraphicsPipeline.Reference;
		vkState::API->RenderPass = GraphicsPipeline.RenderPass;
		vkState::API->PipelineLayout = GraphicsPipeline.PipelineLayout;
		vkState::API->DescriptorSetLayout = GraphicsPipeline.DescriptorSetLayout;
	}

	void VulkanAPI::Finalize()
	{
		WaitDeviceIdle();

		for (const auto [id, view] : PresentViews) {
			delete view;
		}
		PresentViews.clear();

		if (GraphicsPipeline.Reference)
			vkDestroyPipeline(vkState::API->LogicalDevice, GraphicsPipeline.Reference, nullptr);
		if (GraphicsPipeline.PipelineLayout)
			vkDestroyPipelineLayout(vkState::API->LogicalDevice, GraphicsPipeline.PipelineLayout, nullptr);
		if (GraphicsPipeline.RenderPass)
			vkDestroyRenderPass(vkState::API->LogicalDevice, GraphicsPipeline.RenderPass, nullptr);
		if (GraphicsPipeline.DescriptorSetLayout)
			vkDestroyDescriptorSetLayout(vkState::API->LogicalDevice, GraphicsPipeline.DescriptorSetLayout, nullptr);

		for (const auto& [id, view] : PresentViews)
		{
			delete view;
		}

		PresentViews.clear();

		DestroyDevice(Device);

		if (vkState::API->CommandPool)
			vkDestroyCommandPool(vkState::API->LogicalDevice, vkState::API->CommandPool, nullptr);

		vmaDestroyAllocator(vkState::API->VMAAllocator);

		if (vkState::API->LogicalDevice)
			vkDestroyDevice(vkState::API->LogicalDevice, nullptr);

#if VULKAN_VALIDATION
		if (DebugMessenger)
			VkExtern::DestroyDebugMessenger(vkState::API->Instance, DebugMessenger);
#endif

		if (vkState::API->Instance)
			vkDestroyInstance(vkState::API->Instance, nullptr);
	}

	void VulkanAPI::OnWindowCreate(Window* window) {

		const int id = window->GetID();

		const auto presentView = new VulkanPresentView();
		presentView->InitSurface(window);

		presentView->CreateSwapChain();

		PresentViews[id] = presentView;

		presentView->FinalizeCreation();
	}

	void VulkanAPI::OnWindowDestroy(const Window* window) {
		const int id = window->GetID();
		if (!PresentViews.empty()) {
			delete PresentViews[id];
			int c = PresentViews.erase(id);
		}
	}

	void VulkanAPI::OnWindowResize(Window* window)
	{
		const int id = window->GetID();

		auto data = window->GetData();

		PresentViews[id]->RecreateSwapChain();
	}

	void VulkanAPI::OnWindowRender(const Window* window)
	{
		const int id = window->GetID();
		PresentViews[id]->BeginRender();
	}


}
