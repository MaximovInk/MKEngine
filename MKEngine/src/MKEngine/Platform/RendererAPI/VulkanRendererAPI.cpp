#include "mkpch.h"
#include "SDL_vulkan.h"
#include "VulkanRendererAPI.h"
#include "Vulkan/device.h"

namespace MKEngine {

	VulkanDevice* device;

	/*
	vulkanexamplebase.h


	int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)							
{																								
	for (int32_t i = 0; i < __argc; i++) { VulkanExample::args.push_back(__argv[i]); };  		
	vulkanExample = new VulkanExample(); <- settings											
	vulkanExample->initVulkan(); <- instance, debug, physDevice, vks::VulkanDevice, deviceQueue, semaphoers														
	vulkanExample->setupWindow(hInstance, WndProc);												
	vulkanExample->prepare();																	
	vulkanExample->renderLoop();																
	delete(vulkanExample);																		
	return 0;																					
}
	*/

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
		device->OnWindowDestroy(window);
	}
	void VulkanRendererAPI::OnWindowResized(Window* window)
	{
		device->OnWindowResize(window);
	}
	void VulkanRendererAPI::OnWindowRender(Window* window)
	{
		device->OnWindowRender(window);
	}
}