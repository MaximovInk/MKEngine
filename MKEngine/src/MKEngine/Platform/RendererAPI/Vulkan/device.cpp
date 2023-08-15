#include "mkpch.h"
#include "device.h"

#include <utility>
#include "MKEngine/Core/Log.h"
#include "SDL.h"
#include "presentView.h"
#include "shaders.h"
#include "render_structs.h"
#include "buffer.h"

#include "vkExtern.h"

#if defined(_WIN32)
#include "vulkan/vulkan_win32.h"
#endif

namespace MKEngine {

	uint32_t VulkanDevice::GetPresentViewQueueFamilyIndex() const {

		std::vector<VkBool32> supportsPresent(QueueFamilyProperties.size());
		for (uint32_t i = 0; i < QueueFamilyProperties.size(); i++)
		{
#if defined(_WIN32)
			if (vkGetPhysicalDeviceWin32PresentationSupportKHR(PhysicalDevice, i))
				return i;
#else
			if (QueueFamilyProperties[i].queueFlags & VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT)
				return i;
#endif
		}

		MK_LOG_ERROR("Cannot find present view queue family index!");

		return 0;

	}

	uint32_t VulkanDevice::GetQueueFamilyIndex(const VkQueueFlags queueFlags) const
	{
		if ((queueFlags & VK_QUEUE_COMPUTE_BIT) == queueFlags)
		{
			for (uint32_t i = 0; i < static_cast<uint32_t>(QueueFamilyProperties.size()); i++)
			{
				if ((QueueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
					&& ((QueueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0))
				{
					return i;
				}
			}
		}

		if ((queueFlags & VK_QUEUE_TRANSFER_BIT) == queueFlags)
		{
			for (uint32_t i = 0; i < static_cast<uint32_t>(QueueFamilyProperties.size()); i++)
			{
				if ((QueueFamilyProperties[i].queueFlags & VK_QUEUE_TRANSFER_BIT)
					&& ((QueueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0)
					&& ((QueueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) == 0))
				{
					return i;
				}
			}
		}


		for (uint32_t i = 0; i < static_cast<uint32_t>(QueueFamilyProperties.size()); i++)
		{
			if ((QueueFamilyProperties[i].queueFlags & queueFlags) == queueFlags)
			{
				return i;
			}
		}

		MK_LOG_ERROR("Could not find a matching queue family index");

		return 0;
	}

	bool VulkanDevice::ExtensionSupported(const std::string& extension)
	{
		return (std::find(SupportedExtensions.begin(), SupportedExtensions.end(), extension) != SupportedExtensions.end());
	}

	VulkanDevice::VulkanDevice()
	{
		Instance = VkExtern::CreateInstance();

#if VULKAN_VALIDATION
		DebugMessenger = VkExtern::CreateDebugMessenger(Instance);
#endif
		std::vector<const char*> enabledExtensions;

		PhysicalDevice = VkExtern::CreatePhysicalDevice(Instance);

		vkGetPhysicalDeviceProperties(PhysicalDevice, &Properties);
		vkGetPhysicalDeviceFeatures(PhysicalDevice, &Features);
		vkGetPhysicalDeviceMemoryProperties(PhysicalDevice, &MemoryProperties);
		uint32_t queueFamilyCount;
		vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &queueFamilyCount, nullptr);
		MK_ASSERT(queueFamilyCount > 0, "QUEUE FAMILY COUNT MUST BE > 0");
		MK_LOG_INFO("QUEUE FAMILY COUNT: {0}", queueFamilyCount);
		QueueFamilyProperties.resize(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &queueFamilyCount, QueueFamilyProperties.data());

		uint32_t extensionCount = 0;
		vkEnumerateDeviceExtensionProperties(PhysicalDevice, nullptr, &extensionCount, nullptr);
		if (extensionCount > 0)
		{
			if (std::vector<VkExtensionProperties> extensions(extensionCount);
				vkEnumerateDeviceExtensionProperties(PhysicalDevice, nullptr, &extensionCount, &extensions.front()) == VK_SUCCESS)
			{
				for (const auto& [extensionName, specVersion] : extensions)
				{
					SupportedExtensions.emplace_back(extensionName);
				}
			}
		}

		const VkPhysicalDeviceFeatures deviceFeatures{};
		/*
		deviceFeatures.samplerAnisotropy = true;
		FEATURES OF DEVICE
		*/
		if (CreateLogicalDevice(deviceFeatures, DeviceExtensions, nullptr) != VK_SUCCESS)
			MK_LOG_CRITICAL("Failed to create logical device!");
		else
			MK_LOG_INFO("Logical device successfully created");

		GraphicsQueue = VkExtern::GetQueue(LogicalDevice, QueueFamilyIndices.Graphics, 0);
		PresentQueue = VkExtern::GetQueue(LogicalDevice, QueueFamilyIndices.Present, 0);

		CommandPool = CreateCommandPool(QueueFamilyIndices.Graphics, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

		CreateCommandBuffer();
	}

	VulkanDevice::~VulkanDevice()
	{
		WaitDeviceIdle();

		DestroyBuffer(VertexBuffer);

		for (const auto [id, view] : PresentViews) {
			delete view;
		}
		PresentViews.clear();

		if (CommandPool)
			vkDestroyCommandPool(LogicalDevice, CommandPool, nullptr);
		
		if (GraphicsPipeline.Reference)
			vkDestroyPipeline(LogicalDevice, GraphicsPipeline.Reference, nullptr);
		if (GraphicsPipeline.PipelineLayout)
			vkDestroyPipelineLayout(LogicalDevice, GraphicsPipeline.PipelineLayout, nullptr);
		if (GraphicsPipeline.RenderPass)
			vkDestroyRenderPass(LogicalDevice, GraphicsPipeline.RenderPass, nullptr);

		for (const auto& [id, view] : PresentViews)
		{
			delete view;
		}

		PresentViews.clear();

		if (LogicalDevice)
			vkDestroyDevice(LogicalDevice, nullptr);
#if VULKAN_VALIDATION
		if (DebugMessenger)
			VkExtern::DestroyDebugMessenger(Instance, DebugMessenger);
#endif
		if (Instance)
			vkDestroyInstance(Instance, nullptr);
	}

	void VulkanDevice::OnWindowCreate(MKEngine::Window* window) {

		auto sdlWindow = static_cast<SDL_Window*>(window->GetNativeWindow());
		const int id = window->GetID();

		const auto presentView = new VulkanPresentView(this);
		presentView->InitSurface(window);

		presentView->CreateSwapChain();

		PresentViews[id] = presentView;

		GraphicsPipelineDesc pipelineDesc{ };
		pipelineDesc.SwapChainExtent = presentView->SwapChainExtent;
		pipelineDesc.SwapChainFormat = presentView->ColorFormat;

		CreateGraphicsPipeline(pipelineDesc);

		presentView->FinalizeCreation();

		const auto bufferSize = sizeof(Vertices[0]) * Vertices.size();

		BufferDesciption description;
		description.Size = bufferSize;
		description.Data = (void*)Vertices.data();
		description.Usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		description.Access = DataAccess::Device;
		const auto buffer = CreateBuffer(description);
		VertexBuffer = buffer;
	}

	void VulkanDevice::OnWindowDestroy(const MKEngine::Window* window) {
		const int id = window->GetID();
		if (!PresentViews.empty()) {
			delete PresentViews[id];
			int c = PresentViews.erase(id);
		}
	}

	void VulkanDevice::OnWindowResize(MKEngine::Window* window) 
	{
		const int id = window->GetID();

		auto data = window->GetData();

		PresentViews[id]->RecreateSwapChain();
	}

	void VulkanDevice::OnWindowRender(const MKEngine::Window* window)
	{
		const int id = window->GetID();
		PresentViews[id]->Render();
	}

	VkResult VulkanDevice::CreateLogicalDevice(VkPhysicalDeviceFeatures enabledFeatures,
		std::vector<const char*> enabledExtensions, void* pNextChain, VkQueueFlags requestedQueueTypes)
	{
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};
		constexpr float defaultQueuePriority(0.0f);

		// Graphics queue
		if (requestedQueueTypes & VK_QUEUE_GRAPHICS_BIT)
		{
			QueueFamilyIndices.Graphics = GetQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT);
			VkDeviceQueueCreateInfo queueInfo{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
			queueInfo.queueFamilyIndex = QueueFamilyIndices.Graphics;
			queueInfo.queueCount = 1;
			queueInfo.pQueuePriorities = &defaultQueuePriority;
			queueCreateInfos.push_back(queueInfo);
		}
		else
			QueueFamilyIndices.Graphics = 0;

		MK_LOG_INFO("graphics family: {0}", QueueFamilyIndices.Graphics);

		// Dedicated compute queue
		if (requestedQueueTypes & VK_QUEUE_COMPUTE_BIT)
		{
			QueueFamilyIndices.Compute = GetQueueFamilyIndex(VK_QUEUE_COMPUTE_BIT);
			if (QueueFamilyIndices.Compute != QueueFamilyIndices.Graphics)
			{
				// If compute family index differs, 
				//we need an additional queue create info for the compute queue
				VkDeviceQueueCreateInfo queueInfo{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
				queueInfo.queueFamilyIndex = QueueFamilyIndices.Compute;
				queueInfo.queueCount = 1;
				queueInfo.pQueuePriorities = &defaultQueuePriority;
				queueCreateInfos.push_back(queueInfo);
			}
		}
		else
			// Else we use the same queue
			QueueFamilyIndices.Compute = QueueFamilyIndices.Graphics;

		MK_LOG_INFO("compute family: {0}", QueueFamilyIndices.Compute);

		// Dedicated transfer queue
		if (requestedQueueTypes & VK_QUEUE_TRANSFER_BIT)
		{
			QueueFamilyIndices.Transfer = GetQueueFamilyIndex(VK_QUEUE_TRANSFER_BIT);
			if ((QueueFamilyIndices.Transfer != QueueFamilyIndices.Graphics)
				&& (QueueFamilyIndices.Transfer != QueueFamilyIndices.Compute))
			{
				// If transfer family index differs, 
				//we need an additional queue create info for the transfer queue
				VkDeviceQueueCreateInfo queueInfo{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
				queueInfo.queueFamilyIndex = QueueFamilyIndices.Transfer;
				queueInfo.queueCount = 1;
				queueInfo.pQueuePriorities = &defaultQueuePriority;
				queueCreateInfos.push_back(queueInfo);
			}
		}
		else
			// Else we use the same queue
			QueueFamilyIndices.Transfer = QueueFamilyIndices.Graphics;

		MK_LOG_INFO("transfer family: {0}", QueueFamilyIndices.Transfer);

		//Present view queue
		{
			QueueFamilyIndices.Present = GetPresentViewQueueFamilyIndex();
			if (QueueFamilyIndices.Present != QueueFamilyIndices.Graphics)
			{
				VkDeviceQueueCreateInfo queueInfo{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
				queueInfo.queueFamilyIndex = QueueFamilyIndices.Present;
				queueInfo.queueCount = 1;
				queueInfo.pQueuePriorities = &defaultQueuePriority;
				queueCreateInfos.push_back(queueInfo);
			}

		}

		MK_LOG_INFO("present family: {0}", QueueFamilyIndices.Present);

		std::vector<const char*> deviceExtensions(std::move(enabledExtensions));

		/*
		EXTENSIONS FOR DEVICE
		*/
		deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

		VkDeviceCreateInfo deviceCreateInfo = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
		deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());;
		deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
		deviceCreateInfo.pEnabledFeatures = &enabledFeatures;
#if VULKAN_VALIDATION
		deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
#else
		deviceCreateInfo.enabledLayerCount = 0;
#endif

		VkPhysicalDeviceFeatures2 physicalDeviceFeatures2{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
		if (pNextChain) {
			physicalDeviceFeatures2.features = enabledFeatures;
			physicalDeviceFeatures2.pNext = pNextChain;
			deviceCreateInfo.pEnabledFeatures = nullptr;
			deviceCreateInfo.pNext = &physicalDeviceFeatures2;
		}

		if (!deviceExtensions.empty())
		{
			for (const char* enabledExtension : deviceExtensions)
			{
				if (!ExtensionSupported(enabledExtension)) {
					std::cerr << "Enabled device extension \"" << enabledExtension << "\" is not present at device level\n";
				}
			}

			deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
			deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
		}

		this->EnabledFeatures = enabledFeatures;

		return vkCreateDevice(PhysicalDevice, &deviceCreateInfo, nullptr, &LogicalDevice);
	}

	VkPipelineLayout CreatePipelineLayout(const VkDevice device) {
		VkPipelineLayout pipelineLayout;

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		VkPushConstantRange pushConstantInfo;
		pushConstantInfo.offset = 0;
		pushConstantInfo.size = sizeof(ObjectData);
		pushConstantInfo.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantInfo;


		if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
			MK_LOG_ERROR("Failed to create pipeline layout");
		}

		return pipelineLayout;
	}

	VkRenderPass CreateRenderPass(const VkDevice device, const VkFormat format) {
		VkRenderPass renderPass;
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = format;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		VkRenderPassCreateInfo renderPassInfo{ VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;

		if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
			MK_LOG_ERROR("Failed to create render pass");
		}

		return renderPass;
	}

	VkResult VulkanDevice::CreateGraphicsPipeline(const GraphicsPipelineDesc& description)
	{
		VkGraphicsPipelineCreateInfo createInfo{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };

		std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

		//Vertex Input
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{ VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
		auto bindingDescription = Vertex::GetBindingDescription();
		auto attributeDescriptions = Vertex::GetAttributeDescriptions();
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
		createInfo.pVertexInputState = &vertexInputInfo;

		//Input Assembly
		VkPipelineInputAssemblyStateCreateInfo inputAssembly{ VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;
		createInfo.pInputAssemblyState = &inputAssembly;

		//Vertex Shader
		ShaderCreateDesc vertexShaderDesc;
		vertexShaderDesc.Path = "shaders/vert.spv";
		Shader vertexShader = CreateShader(*this, vertexShaderDesc);
		VkPipelineShaderStageCreateInfo vertShaderStageInfo{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertexShader.Resource;
		vertShaderStageInfo.pName = "main";
		shaderStages.push_back(vertShaderStageInfo);

		//Viewport and scissor - Dynamic state
		VkViewport viewport = { 0.0, 0.0, 32.0, 32.0, 0.0, 1.0 };
		VkDynamicState dynamicState[] = { VK_DYNAMIC_STATE_VIEWPORT , VK_DYNAMIC_STATE_SCISSOR };

		VkPipelineViewportStateCreateInfo  viewportState{ VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
		viewportState.pViewports = nullptr;
		viewportState.viewportCount = 1;
		viewportState.pScissors = nullptr;
		viewportState.scissorCount = 1;

		VkPipelineDynamicStateCreateInfo  dynamicStateCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
		dynamicStateCreateInfo.dynamicStateCount = 2;
		dynamicStateCreateInfo.pDynamicStates = dynamicState;

		createInfo.pViewportState = &viewportState;
		createInfo.pDynamicState = &dynamicStateCreateInfo;

		//Rasterizer
		VkPipelineRasterizationStateCreateInfo rasterizer{ VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;
		createInfo.pRasterizationState = &rasterizer;

		//Fragment shader Shader
		ShaderCreateDesc fragmentShaderDesc;
		fragmentShaderDesc.Path = "shaders/frag.spv";
		Shader fragmentShader = CreateShader(*this, fragmentShaderDesc);
		VkPipelineShaderStageCreateInfo fragShaderStageInfo{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragmentShader.Resource;
		fragShaderStageInfo.pName = "main";
		shaderStages.push_back(fragShaderStageInfo);
		createInfo.stageCount = shaderStages.size();
		createInfo.pStages = shaderStages.data();

		//Multisampling
		VkPipelineMultisampleStateCreateInfo multisampling{ VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		createInfo.pMultisampleState = &multisampling;

		//Color blend
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;
		VkPipelineColorBlendStateCreateInfo colorBlending{ VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		createInfo.pColorBlendState = &colorBlending;
		colorBlending.blendConstants[0] = 0.0f;
		colorBlending.blendConstants[1] = 0.0f;
		colorBlending.blendConstants[2] = 0.0f;
		colorBlending.blendConstants[3] = 0.0f;

		//PipelineLayout
		VkPipelineLayout layout = CreatePipelineLayout(LogicalDevice);
		createInfo.layout = layout;

		//Renderpass
		VkRenderPass renderPass = CreateRenderPass(LogicalDevice, description.SwapChainFormat);
		createInfo.renderPass = renderPass;

		//Extra
		createInfo.basePipelineHandle = nullptr;

		//Create graphics pipeline
		VkPipeline vkPipeline;

		if (vkCreateGraphicsPipelines(LogicalDevice, nullptr, 1, &createInfo, nullptr, &vkPipeline) != VK_SUCCESS)
			MK_LOG_ERROR("Failed to create graphics pipeline");

		Pipeline output;
		output.Reference = vkPipeline;
		output.PipelineLayout = layout;
		output.RenderPass = renderPass;

		GraphicsPipeline = output;

		DestroyShader(*this, fragmentShader);
		DestroyShader(*this, vertexShader);

		return VK_SUCCESS;
	}

	VkCommandPool   VulkanDevice::CreateCommandPool(const uint32_t queueFamilyIndex,
	                                                const VkCommandPoolCreateFlags createFlags) const
	{
		VkCommandPoolCreateInfo commandPoolInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
		commandPoolInfo.queueFamilyIndex = queueFamilyIndex;
		commandPoolInfo.flags = createFlags;
		VkCommandPool commandPool;

		if (vkCreateCommandPool(LogicalDevice, &commandPoolInfo, nullptr, &commandPool) != VK_SUCCESS) {
			MK_LOG_CRITICAL("Failed to create command pool");
		}
		else
			MK_LOG_INFO("CommandPool device successfully created");

		return commandPool;
	}

	Buffer VulkanDevice::CreateBuffer(const BufferDesciption description)
	{
		Buffer buffer;

		if(description.Access == DataAccess::Host)
		{
			VkBufferCreateInfo bufferInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
			bufferInfo.size = description.Size;
			bufferInfo.usage = description.Usage;
			bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			if (vkCreateBuffer(LogicalDevice, &bufferInfo, nullptr, &buffer.Resource) != VK_SUCCESS) {
				MK_LOG_ERROR("failed to create vertex buffer!");
			}

			VkMemoryRequirements memRequirements;
			vkGetBufferMemoryRequirements(LogicalDevice, buffer.Resource, &memRequirements);

			VkMemoryAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
			allocInfo.allocationSize = memRequirements.size;
			allocInfo.memoryTypeIndex = VkExtern::FindMemoryType(PhysicalDevice,
				memRequirements.memoryTypeBits,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

			if (vkAllocateMemory(LogicalDevice, &allocInfo, nullptr, &buffer.Memory) != VK_SUCCESS) {
				MK_LOG_ERROR("failed to allocate vertex buffer memory!");
			}
			buffer.MappedData = description.Data;

			vkBindBufferMemory(LogicalDevice, buffer.Resource, buffer.Memory, 0);

			void* data;
			vkMapMemory(LogicalDevice, buffer.Memory, 0, description.Size, 0, &data);
			memcpy(data, description.Data, description.Size);
			//memcpy(data, Vertices.data(), description.Size);
			vkUnmapMemory(LogicalDevice, buffer.Memory);
		}
		if(description.Access == DataAccess::Device)
		{
			const VkDeviceSize bufferSize = description.Size;

			BufferDesciption stagingDescription;
			stagingDescription.Size = bufferSize;
			stagingDescription.Data = (void*)description.Data;
			stagingDescription.Usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			stagingDescription.Access = DataAccess::Host;

			const Buffer stagingBuffer = CreateBuffer(stagingDescription);

			VkBufferCreateInfo bufferInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
			bufferInfo.size = description.Size;
			bufferInfo.usage = description.Usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
			bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			if (vkCreateBuffer(LogicalDevice, &bufferInfo, nullptr, &buffer.Resource) != VK_SUCCESS) {
				MK_LOG_ERROR("failed to create vertex buffer!");
			}

			VkMemoryRequirements memRequirements;
			vkGetBufferMemoryRequirements(LogicalDevice, buffer.Resource, &memRequirements);

			VkMemoryAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
			allocInfo.allocationSize = memRequirements.size;
			allocInfo.memoryTypeIndex = VkExtern::FindMemoryType(PhysicalDevice,
				memRequirements.memoryTypeBits,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

			if (vkAllocateMemory(LogicalDevice, &allocInfo, nullptr, &buffer.Memory) != VK_SUCCESS) {
				MK_LOG_ERROR("failed to allocate vertex buffer memory!");
			}
			buffer.MappedData = description.Data;
			vkBindBufferMemory(LogicalDevice, buffer.Resource, buffer.Memory, 0);

			CopyBuffer(stagingBuffer.Resource, buffer.Resource, bufferSize);

			vkDestroyBuffer(LogicalDevice, stagingBuffer.Resource, nullptr);
			vkFreeMemory(LogicalDevice, stagingBuffer.Memory, nullptr);

		}

		return buffer;
	}

	void VulkanDevice::DestroyBuffer(const Buffer& buffer)
	{
		vkDestroyBuffer(LogicalDevice, buffer.Resource, nullptr);
		vkFreeMemory(LogicalDevice, buffer.Memory, nullptr);
	}

	void VulkanDevice::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = CommandPool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(LogicalDevice, &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		VkBufferCopy copyRegion{};
		copyRegion.srcOffset = 0; // Optional
		copyRegion.dstOffset = 0; // Optional
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
		vkEndCommandBuffer(commandBuffer);
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(GraphicsQueue);

		vkFreeCommandBuffers(LogicalDevice, CommandPool, 1, &commandBuffer);

	}

	void VulkanDevice::CreateCommandBuffer() {
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = CommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;

		if (vkAllocateCommandBuffers(LogicalDevice, &allocInfo, &MainCommandBuffer) != VK_SUCCESS) {
			MK_LOG_ERROR("failed to allocate command buffer!");
		}
	}

	void VulkanDevice::ImmediateSubmit(std::function<void(VkCommandBuffer)> const& callback) const
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = CommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		if (vkAllocateCommandBuffers(LogicalDevice, &allocInfo, &commandBuffer) != VK_SUCCESS) {
			MK_LOG_ERROR("failed to allocate command buffer for immediate submit!");
		}

		VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		callback(commandBuffer);

		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(GraphicsQueue);

		vkFreeCommandBuffers(LogicalDevice, CommandPool, 1, &commandBuffer);
	}

	void VulkanDevice::WaitDeviceIdle() const
	{
		vkDeviceWaitIdle(LogicalDevice);
	}
}