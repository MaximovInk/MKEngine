#include "mkpch.h"
#include "RendererAPI.h"
//#include "RendererAPI/OpenGLRendererAPI.h"
#include "RendererAPI/VulkanRendererAPI.h"

namespace MKEngine {
    RendererAPI* RendererAPI::s_API;
    void* RendererAPI::s_Context;

    void RendererAPI::Make()
    {
        //s_API = new OpenGLRendererAPI();
        s_API = new VulkanRendererAPI();
    }
    void RendererAPI::Destroy()
    {
        delete s_API;
    }
}