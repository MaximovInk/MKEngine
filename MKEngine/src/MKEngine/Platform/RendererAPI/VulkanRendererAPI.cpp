#include "mkpch.h"
#include "SDL_vulkan.h"
#include "VulkanRendererAPI.h"
#include "Vulkan/device.h"

namespace MKEngine {

	VulkanDevice* device;
	static Window* currentWindow;

	VulkanRendererAPI::VulkanRendererAPI()
	{
		device = new VulkanDevice();
	}

	VulkanRendererAPI::~VulkanRendererAPI()
	{
		delete device;
	}

	void VulkanRendererAPI::OnWindowCreated(Window* window)
	{
		device->OnWindowCreate(window);
	}

	void VulkanRendererAPI::OnWindowDestroyed(Window* window)
	{
		device->OnWindowDestroy(window);
	}
	void VulkanRendererAPI::OnWindowResized(Window* window)
	{
		device->OnWindowResize(window);
	}
	void VulkanRendererAPI::OnWindowRender(Window* window)
	{
		device->OnWindowRender(window);
		currentWindow = window;
	}
	void VulkanRendererAPI::OnWindowEndRender(Window* window)
	{
		device->OnWindowRenderEnd(window);
	}
}

bool MKEngine::VulkanRendererAPI::operator==(const VulkanRendererAPI& other) const
{
	return false;
}

void MKEngine::VulkanRendererAPI::DrawTest(int index)
{
	if (currentWindow == nullptr)return;

	device->OnWindowDrawTest(currentWindow, index);
}
