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

		CommandBufferDescription commandBufferDescription;
		commandBufferDescription.FamilyIndex = VkContext::API->QueueFamilyIndices.Graphics;

		VkContext::API->CommandBuffer = CommandBuffer::Create(commandBufferDescription);

		VkContext::API->Begin = reinterpret_cast<PFN_vkCmdBeginRendering>(vkGetInstanceProcAddr(VkContext::API->Instance, "vkCmdBeginRendering"));
		VkContext::API->End = reinterpret_cast<PFN_vkCmdEndRendering>(vkGetInstanceProcAddr(VkContext::API->Instance, "vkCmdEndRendering"));

		VmaAllocatorCreateInfo allocatorCreateInfo{};
		allocatorCreateInfo.device = VkContext::API->LogicalDevice;
		allocatorCreateInfo.instance = VkContext::API->Instance;
		allocatorCreateInfo.physicalDevice = VkContext::API->PhysicalDevice;

		vmaCreateAllocator(&allocatorCreateInfo, &VkContext::API->VmaAllocator);

		GraphicsPipelineDescription graphicsPipelineDescription{};
		ShaderCreateDescription vertDesc;
		vertDesc.Path = "shaders/vert.spv";
		ShaderCreateDescription fragDesc;
		fragDesc.Path = "shaders/frag.spv";

		DescriptorSetLayoutDescription descriptorSetLayoutDescription;
		descriptorSetLayoutDescription.AddBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT);
		descriptorSetLayoutDescription.AddBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);

		VkContext::API->DescriptorSetLayout = DescriptorSetLayout::CreateDescriptorSetLayout(descriptorSetLayoutDescription);

		const auto layout = CreatePipelineLayout(VkContext::API->LogicalDevice, VkContext::API->DescriptorSetLayout.Resource);

		graphicsPipelineDescription.Shaders.emplace_back(Shader::CreateShader(vertDesc));
		graphicsPipelineDescription.Shaders.emplace_back(Shader::CreateShader(fragDesc));

		graphicsPipelineDescription.VertexInput.DefineAttribute(0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, Position));
		graphicsPipelineDescription.VertexInput.DefineAttribute(0, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, Color));
		graphicsPipelineDescription.VertexInput.DefineAttribute(0, 2, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, TexCoord));
		graphicsPipelineDescription.FrontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		graphicsPipelineDescription.PipelineLayout = layout;

		graphicsPipelineDescription.ColorAttachment.SetDepthAttachment(VK_FORMAT_D32_SFLOAT_S8_UINT, VK_COMPARE_OP_LESS, true);

		graphicsPipelineDescription.VertexInput.VertexDefineSlot(0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX);

		GraphicsPipeline = GraphicsPipeline::CreateGraphicsPipeline(graphicsPipelineDescription);

		VkContext::API->GraphicsPipeline = GraphicsPipeline;

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
		testCamera.SetPerspective(75, 1, 0.1, 1000);
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
		if (GraphicsPipeline.Resource)
			GraphicsPipeline::DestroyGraphicsPipeline(GraphicsPipeline);
		if (VkContext::API->CommandBuffer.CommandPool)
			CommandBuffer::Destroy(VkContext::API->CommandBuffer);

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
