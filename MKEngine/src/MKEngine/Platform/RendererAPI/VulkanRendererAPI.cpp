#include "mkpch.h"
#include "SDL_vulkan.h"
#include "VulkanRendererAPI.h"
#include "Vulkan/device.h"

namespace MKEngine {

	VulkanDevice* device;


	VulkanRendererAPI::VulkanRendererAPI()
	{
		device = new VulkanDevice();
	}

	VulkanRendererAPI::~VulkanRendererAPI()
	{
		delete device;
	}

	void VulkanRendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
	}

	void VulkanRendererAPI::Clear()
	{
	}

	void VulkanRendererAPI::InitFunctions(void* procAddress)
	{
	}
	void VulkanRendererAPI::OnWindowCreated(Window* window)
	{
		device->OnWindowCreate(window);
	}

	void VulkanRendererAPI::OnWindowDestroyed(Window* window)
	{
		
	}
}