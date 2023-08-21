#include "mkpch.h"
#include "VulkanRendererAPI.h"
#include "Vulkan/VulkanAPI.h"

namespace MKEngine {
	VulkanAPI API;
	Window* currentWindow;

	VulkanRendererAPI::VulkanRendererAPI()
	{
		API.Initialize();
	}

	VulkanRendererAPI::~VulkanRendererAPI()
	{
		API.Finalize();
	}

	void VulkanRendererAPI::OnWindowCreated(Window* window)
	{
		API.OnWindowCreate(window);
	}

	void VulkanRendererAPI::OnWindowDestroyed(Window* window)
	{
		API.OnWindowDestroy(window);
	}
	void VulkanRendererAPI::OnWindowResized(Window* window)
	{
		API.OnWindowResize(window);
	}
	void VulkanRendererAPI::OnWindowRender(Window* window)
	{
		API.OnWindowRender(window);
		currentWindow = window;
	}

}

bool MKEngine::VulkanRendererAPI::operator==(const VulkanRendererAPI& other) const
{
	return false;
}

