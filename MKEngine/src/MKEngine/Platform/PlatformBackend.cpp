#include "mkpch.h"
#include "PlatformBackend.h"

#include "SDLbackend/SDLBackend.h"
//#include "GLFWbackend/GLFWBackend.h"

namespace MKEngine {

    UniquePointer<PlatformBackend> PlatformBackend::s_CurrentBackend;

    void PlatformBackend::Initialize()
    {
        //SDL has many platforms, so for now we only use it
       PlatformBackend::s_CurrentBackend = CreateUniquePointer<SDLBackend>();
       //PlatformBackend::s_CurrentBackend = CreateUniquePointer<GLFWBackend>();
    }

    void PlatformBackend::Finalize()
    {
        delete PlatformBackend::s_CurrentBackend.get();
    }
}