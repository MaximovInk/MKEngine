#include "mkpch.h"
#include "vkState.h"

namespace MKEngine {
	vkState* vkState::API;

	void WaitDeviceIdle()
	{
		vkDeviceWaitIdle(vkState::API->LogicalDevice);
	}
}