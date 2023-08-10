#include "mkpch.h"
#include "RendererAPI.h"
#include "RendererAPI/VulkanRendererAPI.h"

namespace MKEngine {
    RendererAPI* RendererAPI::s_API;
    void* RendererAPI::s_Context;
    RenderBackendType RendererAPI::s_RenderBackend;

    void RendererAPI::Make(RenderBackendType backend)
    {
        s_API = new VulkanRendererAPI();
        s_RenderBackend = backend;
    }
    void RendererAPI::Destroy()
    {
        delete s_API;
    }
}