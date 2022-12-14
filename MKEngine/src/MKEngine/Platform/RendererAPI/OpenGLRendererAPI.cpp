#include "mkpch.h"
#include "mkengine.h"
#include "OpenGLRendererAPI.h"
#include "gl.h"

namespace MKEngine {

	OpenGLRendererAPI::OpenGLRendererAPI()
	{
		MK_LOG_WARN("OpenGL: Waiting for creation window for successfully init");
	}

	OpenGLRendererAPI::~OpenGLRendererAPI()
	{
	}

	void OpenGLRendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{

	}

	/*
	void OpenGLRendererAPI::SetClearColor(const glm::vec4& color)
	{

	}
	*/

	void OpenGLRendererAPI::Clear()
	{

	}

	void OpenGLRendererAPI::InitFunctions(void* procAddress)
	{
		const auto version = gladLoadGL(reinterpret_cast<GLADloadfunc>(procAddress));
		MK_LOG_INFO("OpenGL {0}.{1} loaded!", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));
	}


}