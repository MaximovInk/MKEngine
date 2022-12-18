#include "mkpch.h"
#include "RendererAPI.h"
//#include "RendererAPI/OpenGLRendererAPI.h"
#include "RendererAPI/VulkanRendererAPI.h"

namespace MKEngine {
    RendererAPI* RendererAPI::s_API;
    void* RendererAPI::s_Context;
    RenderBackendType RendererAPI::s_RenderBackend;

    void RendererAPI::Make(RenderBackendType backend)
    {
        //TODO: Backend init changed
        //s_API = new OpenGLRendererAPI();
        s_API = new VulkanRendererAPI();
        s_RenderBackend = backend;
    }
    void RendererAPI::Destroy()
    {
        delete s_API;
    }
}