#pragma once
#include "mkpch.h"
#include "MKEngine/Core/core.h"
#include "MKEngine/Core/Log.h"
#include "MKEngine/Platform/Window.h"
#include "vulkan/vulkan.h"
#if defined(_WIN32)
#include "vulkan/vulkan_win32.h"
#endif


#include <vulkan/vk_enum_string_helper.h>

#define TRACE_INITIALIZATION_RENDERER 1

/*
	bool bInitSuccess = CreateInstance()
	&& CreatePhysicalDevice()
	&& CreateWindowSurface()
	&& CreateQueueFamily()
	&& CreateLogicalDevice()
	&& CreateSwapChain();
	*/

namespace MKEngine {

	
	const std::vector<const char*> ValidationLayers = {
"VK_LAYER_KHRONOS_validation"
	};

	class VkExtern {
	public:
#if _DEBUG
		static const bool EnableValidationLayers = true;
#else
		static const bool EnableValidationLayers = false;
#endif

		static std::vector<std::string> SupportedInstanceExtensions;

		static VkInstance CreateInstance();
		static VkDebugUtilsMessengerEXT CreateDebugMessenger(VkInstance instance);
		static VkPhysicalDevice CreatePhysicalDevice(VkInstance instance);

		static VkShaderModule CreateShaderModule(VkDevice device, const std::vector<char>& code);

		static void DestroyDebugMessanger(VkInstance instance, VkDebugUtilsMessengerEXT messenger);
		static VkSurfaceKHR CreateWindowSurface(VkInstance instance, MKEngine::Window* window);

		static void WaitFence(VkDevice logicalDevice, VkFence fence);
		static void ResetFence(VkDevice logicalDevice, VkFence fence);
		static VkBool32 getSupportedDepthStencilFormat(VkPhysicalDevice physicalDevice, VkFormat* depthStencilFormat);
		static VkBool32 getSupportedDepthFormat(VkPhysicalDevice physicalDevice, VkFormat* depthFormat);

		static VkQueue GetQueue(VkDevice logicalDeivce, int familyIndex, int queueIndex);

		static std::vector<std::string> log_transform_bits(VkSurfaceTransformFlagsKHR bits) {
			std::vector<std::string> result;

			if (bits & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
				result.push_back("identity");
			}
			if (bits & VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR) {
				result.push_back("90 degree rotation");
			}
			if (bits & VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR) {
				result.push_back("180 degree rotation");
			}
			if (bits & VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR) {
				result.push_back("270 degree rotation");
			}
			if (bits & VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_BIT_KHR) {
				result.push_back("horizontal mirror");
			}
			if (bits & VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_90_BIT_KHR) {
				result.push_back("horizontal mirror, then 90 degree rotation");
			}
			if (bits & VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_180_BIT_KHR) {
				result.push_back("horizontal mirror, then 180 degree rotation");
			}
			if (bits & VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_270_BIT_KHR) {
				result.push_back("horizontal mirror, then 270 degree rotation");
			}
			if (bits & VK_SURFACE_TRANSFORM_INHERIT_BIT_KHR) {
				result.push_back("inherited");
			}

			return result;
		}
		static std::vector<std::string> log_alpha_composite_bits(VkCompositeAlphaFlagsKHR bits) {
			std::vector<std::string> result;

			if (bits & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR) {
				result.push_back("opaque (alpha ignored)");
			}
			if (bits & VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR) {
				result.push_back("pre multiplied (alpha expected to already be multiplied in image)");
			}
			if (bits & VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR) {
				result.push_back("post multiplied (alpha will be applied during composition)");
			}
			if (bits & VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR) {
				result.push_back("inherited");
			}

			return result;
		}
		static std::vector<std::string> log_image_usage_bits(VkImageUsageFlags bits) {
			std::vector<std::string> result;
			if (bits & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) {
				result.push_back("transfer src");
			}
			if (bits & VK_IMAGE_USAGE_TRANSFER_DST_BIT) {
				result.push_back("transfer dst");
			}
			if (bits & VK_IMAGE_USAGE_SAMPLED_BIT) {
				result.push_back("sampled");
			}
			if (bits & VK_IMAGE_USAGE_STORAGE_BIT) {
				result.push_back("storage");
			}
			if (bits & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) {
				result.push_back("color attachment");
			}
			if (bits & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) {
				result.push_back("depth/stencil attachment");
			}
			if (bits & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT) {
				result.push_back("transient attachment");
			}
			if (bits & VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT) {
				result.push_back("input attachment");
			}
			if (bits & VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT) {
				result.push_back("fragment density map");
			}
			if (bits & VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR) {
				result.push_back("fragment shading rate attachment");
			}
			return result;
		}
		static std::string log_present_mode(VkPresentModeKHR presentMode) {
		
			if (presentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
				return "immediate";
			}
			if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
				return "mailbox";
			}
			if (presentMode == VK_PRESENT_MODE_FIFO_KHR) {
				return "fifo";
			}
			if (presentMode == VK_PRESENT_MODE_FIFO_RELAXED_KHR) {
				return "relaxed fifo";
			}
			if (presentMode == VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR) {
				return "shared demand refresh";
			}
			if (presentMode == VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR) {
				return "shared continuous refresh";
			}
			return "none/undefined";
		}
		
		static VkShaderModule createShaderModule(VkDevice logicalDevice, const std::vector<char>& code);

		static VkSemaphore createSemaphore(VkDevice _device);
		static VkFence createFence(VkDevice device, VkFenceCreateFlags flags = VK_FENCE_CREATE_SIGNALED_BIT);
	};

}