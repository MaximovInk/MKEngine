#include "mkpch.h"
#include "GLFWBackend.h"
#include "MKEngine/Core/Log.h"
#include <GLFW/glfw3.h>

namespace MKEngine {

	GLFWBackend::GLFWBackend()
	{
		MK_LOG_TRACE("Loading glfw");
		if (!glfwInit())
		{
			MK_LOG_ERROR("Error loading glfw");
		}

		MK_LOG_INFO("Loaded glfw successfully!");
	}

	GLFWBackend::~GLFWBackend()
	{
		glfwTerminate();
	}

	void GLFWBackend::OnUpdate()
	{
		glfwPollEvents();
	}

	void GLFWBackend::SetEventCallback(const EventCallbackFn& callback)
	{

	}

	void GLFWBackend::MakeWindow(Window* window, const WindowSettings& settings)
	{
		GLFWwindow* wnd = glfwCreateWindow(640, 480, settings.Title.c_str(), NULL, NULL);
		if (!window) {
			MK_LOG_ERROR("creating window error!");
		}
	}
}