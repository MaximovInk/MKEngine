#include "mkpch.h"

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#include "VulkanAPI.h"
#include "vkExtern.h"
#include "vkFunctions.h"
#include "Pipeline/graphicsPipeline.h"

#include "VkContext.h"

#include "../camera.h"

namespace MKEngine {

	Mesh VulkanAPI::testMesh;
	VkTexture VulkanAPI::testTexture;
	Model VulkanAPI::testModel;
	Camera VulkanAPI::testCamera;

	void VulkanAPI::Initialize()
	{
		VkContext::API = new VkContext();

		VkContext::API->Instance = VkExtern::CreateInstance();
#if VULKAN_VALIDATION
		DebugMessenger = VkExtern::CreateDebugMessenger(VkContext::API->Instance);
#endif

		Device = CreateDevice();

		VmaAllocatorCreateInfo allocatorCreateInfo{};
		allocatorCreateInfo.device = VkContext::API->LogicalDevice;
		allocatorCreateInfo.instance = VkContext::API->Instance;
		allocatorCreateInfo.physicalDevice = VkContext::API->PhysicalDevice;

		vmaCreateAllocator(&allocatorCreateInfo, &VkContext::API->VMAAllocator);

		GraphicsPipelineDescription description{};
		ShaderCreateDescription vertDesc;
		vertDesc.Path = "shaders/vert.spv";
		ShaderCreateDescription fragDesc;
		fragDesc.Path = "shaders/frag.spv";
		VkContext::API->DescriptorSetLayout = CreateDescriptorSetLayout(VkContext::API->LogicalDevice);
		const auto layout = CreatePipelineLayout(VkContext::API->LogicalDevice, VkContext::API->DescriptorSetLayout);

		description.Shaders.emplace_back(CreateShader(vertDesc));
		description.Shaders.emplace_back(CreateShader(fragDesc));

		description.VertexInput.DefineAttribute(0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, Position));
		description.VertexInput.DefineAttribute(0, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, Color));
		description.VertexInput.DefineAttribute(0, 2, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, TexCoord));
		description.FrontFace = VK_FRONT_FACE_CLOCKWISE;
		description.PipelineLayout = layout;

		description.VertexInput.VertexDefineSlot(0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX);

		GraphicsPipeline = GraphicsPipeline::CreateGraphicsPipeline(description);

		VkContext::API->GraphicsPipeline = GraphicsPipeline.Reference;
		VkContext::API->PipelineLayout = GraphicsPipeline.PipelineLayout;

		testMesh.Indices = INDICES;
		testMesh.Vertices = VERTICES;
		testMesh.Apply();

		TextureDescription textureDescription{};
		textureDescription.Path = "textures/uvcheck.png";
		textureDescription.Usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		textureDescription.Format = VK_FORMAT_R8G8B8A8_SRGB;
		testTexture = CreateTexture(textureDescription);

		testModel = Model::LoadModel("models/untitled.obj");

		testCamera.FlipY = true;
		testCamera.SetPerspective(75, 1, 0.1, 100);
		testCamera.SetPosition(glm::vec3(0.0f, 2.5f, -10.25f));
		testCamera.SetRotation(glm::vec3(0.0f, 0.0f, 0.0f));
	}

	void VulkanAPI::Finalize()
	{
		WaitDeviceIdle();

		Mesh::Destroy(testMesh);
		DestroyTexture(testTexture);
		Model::DestroyModel(testModel);

		for (const auto [id, view] : PresentViews) {
			delete view;
		}
		PresentViews.clear();

		if (GraphicsPipeline.PipelineLayout)
			vkDestroyPipelineLayout(VkContext::API->LogicalDevice, GraphicsPipeline.PipelineLayout, nullptr);
		if (VkContext::API->DescriptorSetLayout)
			vkDestroyDescriptorSetLayout(VkContext::API->LogicalDevice, VkContext::API->DescriptorSetLayout, nullptr);
		if (GraphicsPipeline.Reference)
			GraphicsPipeline::DestroyGraphicsPipeline(GraphicsPipeline);

		for (const auto& [id, view] : PresentViews)
		{
			delete view;
		}

		PresentViews.clear();

		if (VkContext::API->CommandPool)
			vkDestroyCommandPool(VkContext::API->LogicalDevice, VkContext::API->CommandPool, nullptr);

		vmaDestroyAllocator(VkContext::API->VMAAllocator);

		if (VkContext::API->LogicalDevice)
			vkDestroyDevice(VkContext::API->LogicalDevice, nullptr);

#if VULKAN_VALIDATION
		if (DebugMessenger)
			VkExtern::DestroyDebugMessenger(VkContext::API->Instance, DebugMessenger);
#endif

		if (VkContext::API->Instance)
			vkDestroyInstance(VkContext::API->Instance, nullptr);
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
		PresentViews[id]->Render();
	}


}
