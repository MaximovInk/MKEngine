#include "mkpch.h"

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#include "VulkanAPI.h"

#include "model.h"
#include "vkExtern.h"
#include "Pipeline/graphicsPipeline.h"

#include "VkContext.h"

#include "../camera.h"
#include "DescriptorSet/descriptorSet.h"

namespace MKEngine {

	Mesh VulkanAPI::testMesh;
	Texture VulkanAPI::testTexture;
	Model VulkanAPI::testModel;
	Camera VulkanAPI::testCamera;

	void VulkanAPI::Initialize()
	{
		VkContext::API = new VkContext();

		VkContext::API->Instance = VkExtern::CreateInstance();
#if VULKAN_VALIDATION
		DebugMessenger = VkExtern::CreateDebugMessenger(VkContext::API->Instance);
#endif

		Device = Device::Create();

		VmaAllocatorCreateInfo allocatorCreateInfo{};
		allocatorCreateInfo.device = VkContext::API->LogicalDevice;
		allocatorCreateInfo.instance = VkContext::API->Instance;
		allocatorCreateInfo.physicalDevice = VkContext::API->PhysicalDevice;

		vmaCreateAllocator(&allocatorCreateInfo, &VkContext::API->VmaAllocator);

		GraphicsPipelineDescription description{};
		ShaderCreateDescription vertDesc;
		vertDesc.Path = "shaders/vert.spv";
		ShaderCreateDescription fragDesc;
		fragDesc.Path = "shaders/frag.spv";

		DescriptorSetLayoutDescription desc;
		desc.AddBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT);
		desc.AddBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);

		VkContext::API->DescriptorSetLayout = DescriptorSetLayout::CreateDescriptorSetLayout(desc);
		const auto layout = CreatePipelineLayout(VkContext::API->LogicalDevice, VkContext::API->DescriptorSetLayout.Resource);

		description.Shaders.emplace_back(Shader::CreateShader(vertDesc));
		description.Shaders.emplace_back(Shader::CreateShader(fragDesc));

		description.VertexInput.DefineAttribute(0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, Position));
		description.VertexInput.DefineAttribute(0, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, Color));
		description.VertexInput.DefineAttribute(0, 2, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, TexCoord));
		description.FrontFace = VK_FRONT_FACE_CLOCKWISE;
		description.PipelineLayout = layout;

		description.ColorAttachment.SetDepthAttachment(VK_FORMAT_D32_SFLOAT, VK_COMPARE_OP_LESS, true);

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
		testTexture = Texture::CreateTexture(textureDescription);

		testModel = Model::LoadModel("models/scene.gltf");

		testCamera.FlipY = true;
		testCamera.SetPerspective(75, 1, 0.1, 100);
		testCamera.SetPosition(glm::vec3(0.0f, 2.5f, -10.25f));
		testCamera.SetRotation(glm::vec3(0.0f, 0.0f, 0.0f));

	}

	void VulkanAPI::Finalize()
	{
		WaitDeviceIdle();

		Mesh::Destroy(testMesh);
		Texture::DestroyTexture(testTexture);
		Model::DestroyModel(testModel);

		for (const auto [id, presentView] : PresentViews) {
			PresentView::Destroy(presentView);
		}
		PresentViews.clear();

		if (GraphicsPipeline.PipelineLayout)
			vkDestroyPipelineLayout(VkContext::API->LogicalDevice, GraphicsPipeline.PipelineLayout, nullptr);
		if (VkContext::API->DescriptorSetLayout.Resource)
			DescriptorSetLayout::DestroyDescriptorSetLayout(VkContext::API->DescriptorSetLayout);
		if (GraphicsPipeline.Reference)
			GraphicsPipeline::DestroyGraphicsPipeline(GraphicsPipeline);

		if (VkContext::API->CommandPool)
			vkDestroyCommandPool(VkContext::API->LogicalDevice, VkContext::API->CommandPool, nullptr);

		vmaDestroyAllocator(VkContext::API->VmaAllocator);

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

		const auto presentView = PresentView::Create();
		presentView->InitSurface(window);

		presentView->CreateSwapChain();

		PresentViews[id] = presentView;

		presentView->FinalizeCreation();
	}

	void VulkanAPI::OnWindowDestroy(const Window* window) {
		const int id = window->GetID();
		if (!PresentViews.empty()) {
			PresentView::Destroy(PresentViews[id]);
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
