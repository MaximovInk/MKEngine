#include "mkpch.h"
#include "Graphics.h"

#include "Platform/RendererAPI.h"

namespace MKEngine {
	void Graphics::DrawTest(int testIndex)
	{
		RendererAPI::CurrentAPI->DrawTest(testIndex);
	}
}
