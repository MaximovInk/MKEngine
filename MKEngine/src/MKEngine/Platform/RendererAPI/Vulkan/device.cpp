#include "mkpch.h"
#include "device.h"
#include "MKEngine/Core/Log.h"
#include "SDL.h"
#include "SDL_vulkan.h"
#include "vkExtern.h"
#include "presentView.h"
#include "shaders.h"
#include "render_structs.h"

namespace MKEngine {

	//std::map<std::int16_t, VkSurfaceKHR> surfaces;

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
			/*
			vkGetPhysicalDeviceSurfaceSupportKHR(PhysicalDevice, i,
				Surface, &supportsPresent[i]);
			*/
		}



	}

	uint32_t VulkanDevice::GetQueueFamilyIndex(VkQueueFlags queueFlags) const
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

		/*


		for (uint32_t i = static_cast<uint32_t>(QueueFamilyProperties.size())-1; i >= 0; i--)
		{
			if ((QueueFamilyProperties[i].queueFlags & queueFlags) == queueFlags)
			{
				return i;
			}
		}
		*/


		MK_LOG_ERROR("Could not find a matching queue family index");
	}

	bool VulkanDevice::ExtensionSupported(std::string extension)
	{
		return (std::find(SupportedExtensions.begin(), SupportedExtensions.end(), extension) != SupportedExtensions.end());
	}

	/*
	bool bInitSuccess = CreateInstance()
		&& CreatePhysicalDevice()
		&& CreateWindowSurface()
		&& CreateQueueFamily()
		&& CreateLogicalDevice()
		&& CreateSwapChain();
	*/

	VulkanDevice::VulkanDevice()
	{
		Instance = VkExtern::CreateInstance();

		DebugMessenger = VkExtern::CreateDebugMessenger(Instance);

		VkPhysicalDeviceFeatures enabledFeatures;
		std::vector<const char*> enabledExtensions;
		void* pNextChain = nullptr;

		PhysicalDevice = VkExtern::CreatePhysicalDevice(Instance);

		vkGetPhysicalDeviceProperties(PhysicalDevice, &Properties);
		vkGetPhysicalDeviceFeatures(PhysicalDevice, &Features);
		vkGetPhysicalDeviceMemoryProperties(PhysicalDevice, &MemoryProperties);
		uint32_t queueFamilyCount;
		vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &queueFamilyCount, nullptr);
		MK_ASSERT(queueFamilyCount > 0, "QUEUE FAMILY COUNT MUST BE > 0");
		MK_LOG_INFO("[VULKAN] QUEUE FAMILY COUNT: {0}", queueFamilyCount);
		QueueFamilyProperties.resize(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &queueFamilyCount, QueueFamilyProperties.data());

		uint32_t extenshionCount = 0;
		vkEnumerateDeviceExtensionProperties(PhysicalDevice, nullptr, &extenshionCount, nullptr);
		if (extenshionCount > 0)
		{
			std::vector<VkExtensionProperties> extensions(extenshionCount);
			if (vkEnumerateDeviceExtensionProperties(PhysicalDevice, nullptr, &extenshionCount, &extensions.front()) == VK_SUCCESS)
			{
				for (VkExtensionProperties extenshion : extensions)
				{
					SupportedExtensions.push_back(extenshion.extensionName);
				}
			}
		}

		VkPhysicalDeviceFeatures deviceFeatures{};
		/*
		deviceFeatures.samplerAnisotropy = true;
		FEATURES OF DEVICE
		*/
		if (CreateLogicalDevice(deviceFeatures, DeviceExtensions, nullptr) != VK_SUCCESS)
			MK_LOG_CRITICAL("[VULKAN] Failed to create logical device!");
#if TRACE_INITIALIZATION_RENDERER
		else
			MK_LOG_INFO("[VULKAN] Logical device successfully created");
#endif

		GraphicsQueue = VkExtern::GetQueue(LogicalDevice, QueueFamilyIndices.graphics, 0);
		PresentQueue = VkExtern::GetQueue(LogicalDevice, QueueFamilyIndices.present, 0);



		CommandPool = CreateCommandPool(QueueFamilyIndices.graphics, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

	}

	VulkanDevice::~VulkanDevice()
	{
		WaitDeviceIdle();

		if (CommandPool)
			vkDestroyCommandPool(LogicalDevice, CommandPool, nullptr);
		
		if (GraphicsPipeline.pipeline)
			vkDestroyPipeline(LogicalDevice, GraphicsPipeline.pipeline, nullptr);
		if (GraphicsPipeline.pipelineLayout)
			vkDestroyPipelineLayout(LogicalDevice, GraphicsPipeline.pipelineLayout, nullptr);
		if (GraphicsPipeline.renderPass)
			vkDestroyRenderPass(LogicalDevice, GraphicsPipeline.renderPass, nullptr);

		for (size_t i = 0; i < PresentViews.size(); i++)
		{
			delete PresentViews[i];
		}

		PresentViews.clear();

		if (LogicalDevice)
			vkDestroyDevice(LogicalDevice, nullptr);

		if (DebugMessenger)
			VkExtern::DestroyDebugMessanger(Instance, DebugMessenger);

		if (Instance)
			vkDestroyInstance(Instance, nullptr);
	}

	void VulkanDevice::OnWindowCreate(MKEngine::Window* window) {

		auto sdlWindow = (SDL_Window*)window->GetNativeWindow();
		int id = window->GetID();

		auto presentView = new VulkanPresentView(this);
		presentView->InitSurface(window);
		int w, h;
		SDL_Vulkan_GetDrawableSize(sdlWindow, &w, &h);
		uint32_t width = w;
		uint32_t height = h;
		presentView->CreateSwapChain(&width, &height);

		PresentViews[id] = presentView;

		GraphicsPipelineDesc pipelineDesc{ };
		pipelineDesc.swapChainExtent = presentView->SwapChainExtent;
		pipelineDesc.swapChainFormat = presentView->ColorFormat;

		CreateGraphicsPipeline(pipelineDesc);

		presentView->CreateFramebuffer();
		presentView->CreateCommandBuffers();
		CreateCommandBuffer();

		presentView->CreateSync();
	}

	void VulkanDevice::OnWindowDestroy(MKEngine::Window* window) {
		int id = window->GetID();
		if (PresentViews.size() > 0) {
			delete PresentViews[id];
			int c = PresentViews.erase(id);
		}
	}

	void VulkanDevice::OnWindowResize(MKEngine::Window* window)
	{
		int id = window->GetID();

		auto data = window->GetData();
		PresentViews[id]->CreateSwapChain(&data.Width, &data.Height);
	}

	void VulkanDevice::OnWindowRender(MKEngine::Window* window)
	{
		int id = window->GetID();
		PresentViews[id]->Render();
	}

	VkResult VulkanDevice::CreateLogicalDevice(VkPhysicalDeviceFeatures enabledFeatures,
		std::vector<const char*> enabledExtensions, void* pNextChain, VkQueueFlags requestedQueueTypes)
	{
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};
		const float defaultQueuePriority(0.0f);

		// Graphics queue
		if (requestedQueueTypes & VK_QUEUE_GRAPHICS_BIT)
		{
			QueueFamilyIndices.graphics = GetQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT);
			VkDeviceQueueCreateInfo queueInfo{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
			queueInfo.queueFamilyIndex = QueueFamilyIndices.graphics;
			queueInfo.queueCount = 1;
			queueInfo.pQueuePriorities = &defaultQueuePriority;
			queueCreateInfos.push_back(queueInfo);
		}
		else
			QueueFamilyIndices.graphics = 0;

		MK_LOG_INFO("[VULKAN] graphics family: {0}", QueueFamilyIndices.graphics);

		// Dedicated compute queue
		if (requestedQueueTypes & VK_QUEUE_COMPUTE_BIT)
		{
			QueueFamilyIndices.compute = GetQueueFamilyIndex(VK_QUEUE_COMPUTE_BIT);
			if (QueueFamilyIndices.compute != QueueFamilyIndices.graphics)
			{
				// If compute family index differs, 
				//we need an additional queue create info for the compute queue
				VkDeviceQueueCreateInfo queueInfo{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
				queueInfo.queueFamilyIndex = QueueFamilyIndices.compute;
				queueInfo.queueCount = 1;
				queueInfo.pQueuePriorities = &defaultQueuePriority;
				queueCreateInfos.push_back(queueInfo);
			}
		}
		else
			// Else we use the same queue
			QueueFamilyIndices.compute = QueueFamilyIndices.graphics;

		MK_LOG_INFO("[VULKAN] compute family: {0}", QueueFamilyIndices.compute);

		// Dedicated transfer queue
		if (requestedQueueTypes & VK_QUEUE_TRANSFER_BIT)
		{
			QueueFamilyIndices.transfer = GetQueueFamilyIndex(VK_QUEUE_TRANSFER_BIT);
			if ((QueueFamilyIndices.transfer != QueueFamilyIndices.graphics)
				&& (QueueFamilyIndices.transfer != QueueFamilyIndices.compute))
			{
				// If transfer family index differs, 
				//we need an additional queue create info for the transfer queue
				VkDeviceQueueCreateInfo queueInfo{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
				queueInfo.queueFamilyIndex = QueueFamilyIndices.transfer;
				queueInfo.queueCount = 1;
				queueInfo.pQueuePriorities = &defaultQueuePriority;
				queueCreateInfos.push_back(queueInfo);
			}
		}
		else
			// Else we use the same queue
			QueueFamilyIndices.transfer = QueueFamilyIndices.graphics;

		MK_LOG_INFO("[VULKAN] transfer family: {0}", QueueFamilyIndices.transfer);

		//Present view queue
		{
			QueueFamilyIndices.present = GetPresentViewQueueFamilyIndex();
			if (QueueFamilyIndices.present != QueueFamilyIndices.graphics)
			{
				VkDeviceQueueCreateInfo queueInfo{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
				queueInfo.queueFamilyIndex = QueueFamilyIndices.present;
				queueInfo.queueCount = 1;
				queueInfo.pQueuePriorities = &defaultQueuePriority;
				queueCreateInfos.push_back(queueInfo);
			}

		}

		MK_LOG_INFO("[VULKAN] present family: {0}", QueueFamilyIndices.present);

		std::vector<const char*> deviceExtensions(enabledExtensions);

		/*
		EXTENSHIONS FOR DEVICE
		*/
		deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

		VkDeviceCreateInfo deviceCreateInfo = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
		deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());;
		deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
		deviceCreateInfo.pEnabledFeatures = &enabledFeatures;
		if (VkExtern::EnableValidationLayers) {
			deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayers.size());
			deviceCreateInfo.ppEnabledLayerNames = ValidationLayers.data();
		}
		else {
			deviceCreateInfo.enabledLayerCount = 0;
		}

		VkPhysicalDeviceFeatures2 physicalDeviceFeatures2{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
		if (pNextChain) {
			physicalDeviceFeatures2.features = enabledFeatures;
			physicalDeviceFeatures2.pNext = pNextChain;
			deviceCreateInfo.pEnabledFeatures = nullptr;
			deviceCreateInfo.pNext = &physicalDeviceFeatures2;
		}

		if (deviceExtensions.size() > 0)
		{
			for (const char* enabledExtension : deviceExtensions)
			{
				if (!ExtensionSupported(enabledExtension)) {
					std::cerr << "Enabled device extension \"" << enabledExtension << "\" is not present at device level\n";
				}
			}

			deviceCreateInfo.enabledExtensionCount = (uint32_t)deviceExtensions.size();
			deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
		}

		this->EnabledFeatures = enabledFeatures;

		return vkCreateDevice(PhysicalDevice, &deviceCreateInfo, nullptr, &LogicalDevice);
	}

	VkPipelineLayout createPipelineLayout(VkDevice device) {
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
			MK_LOG_ERROR("[VULKAN] Failed to create pipeline layout");
		}

		return pipelineLayout;
	}

	VkRenderPass createRenderPass(VkDevice device, VkFormat format) {
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
			MK_LOG_ERROR("[VULKAN] Failed to create render pass");
		}

		return renderPass;
	}

	VkResult VulkanDevice::CreateGraphicsPipeline(GraphicsPipelineDesc description)
	{
		VkGraphicsPipelineCreateInfo createInfo{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };

		std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

		//Vertex Input
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{ VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
		vertexInputInfo.vertexBindingDescriptionCount = 0;
		vertexInputInfo.vertexAttributeDescriptionCount = 0;
		createInfo.pVertexInputState = &vertexInputInfo;

		//Input Assembly

		VkPipelineInputAssemblyStateCreateInfo inputAssembly{ VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;
		createInfo.pInputAssemblyState = &inputAssembly;

		//auto vertShaderCode = readFile("shaders/vert.spv");
		//auto fragShaderCode = readFile("shaders/frag.spv");

		//Vertex Shader
		ShaderCreateDesc vertexShaderDesc;
		vertexShaderDesc.pPath = "shaders/vert.spv";
		Shader vertexShader = createShader(*this, vertexShaderDesc);
		VkPipelineShaderStageCreateInfo vertShaderStageInfo{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertexShader.resource;
		vertShaderStageInfo.pName = "main";
		shaderStages.push_back(vertShaderStageInfo);

		//Viewport and scissor

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		//TODO multiply windows
		viewport.width = (float)description.swapChainExtent.width;
		viewport.height = (float)description.swapChainExtent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = description.swapChainExtent;
		VkPipelineViewportStateCreateInfo viewportState{ VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;
		createInfo.pViewportState = &viewportState;

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
		fragmentShaderDesc.pPath = "shaders/frag.spv";
		Shader fragmentShader = createShader(*this, fragmentShaderDesc);
		VkPipelineShaderStageCreateInfo fragShaderStageInfo{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragmentShader.resource;
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
		VkPipelineLayout layout = createPipelineLayout(LogicalDevice);
		createInfo.layout = layout;

		//Renderpass
		VkRenderPass renderPass = createRenderPass(LogicalDevice, description.swapChainFormat);
		createInfo.renderPass = renderPass;

		//Extra
		createInfo.basePipelineHandle = nullptr;

		//Create grpahics pipeline
		VkPipeline vkPipeline;

		if (vkCreateGraphicsPipelines(LogicalDevice, nullptr, 1, &createInfo, nullptr, &vkPipeline) != VK_SUCCESS)
			MK_LOG_ERROR("Failed to create graphics pipeline");

		Pipeline output;
		output.pipeline = vkPipeline;
		output.pipelineLayout = layout;
		output.renderPass = renderPass;

		GraphicsPipeline = output;

		destroyShader(*this, fragmentShader);
		destroyShader(*this, vertexShader);

		return VK_SUCCESS;
	}

	VkCommandPool   VulkanDevice::CreateCommandPool(uint32_t queueFamilyIndex,
		VkCommandPoolCreateFlags createFlags)
	{
		VkCommandPoolCreateInfo commandPoolInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
		commandPoolInfo.queueFamilyIndex = queueFamilyIndex;
		commandPoolInfo.flags = createFlags;
		VkCommandPool commandPool;

		if (vkCreateCommandPool(LogicalDevice, &commandPoolInfo, nullptr, &commandPool) != VK_SUCCESS) {
			MK_LOG_CRITICAL("[VULKAN] Failed to create command pool");
		}
#if TRACE_INITIALIZATION_RENDERER
		else
			MK_LOG_INFO("[VULKAN] CommandPool device successfully created");
#endif

		return commandPool;
	}

	VkCommandBuffer VulkanDevice::CreateCommandBuffer() {
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = CommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;

		if (vkAllocateCommandBuffers(LogicalDevice, &allocInfo, &MainCommandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffer!");
		}
	}

	void VulkanDevice::WaitDeviceIdle() {
		vkDeviceWaitIdle(LogicalDevice);
	}

	
}