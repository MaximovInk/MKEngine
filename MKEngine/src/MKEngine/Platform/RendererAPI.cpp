#include "mkpch.h"
#include "RendererAPI.h"
#include "RendererAPI/VulkanRendererAPI.h"

namespace MKEngine {
    RendererAPI* RendererAPI::CurrentAPI;
    void* RendererAPI::m_context;

    void RendererAPI::Make()
    {
        CurrentAPI = new VulkanRendererAPI();
    }
    void RendererAPI::Destroy()
    {
        delete CurrentAPI;
    }
}