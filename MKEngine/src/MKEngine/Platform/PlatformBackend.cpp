#include "mkpch.h"
#include "PlatformBackend.h"
#include "SDLbackend/SDLBackend.h"

namespace MKEngine {

    PlatformBackend* PlatformBackend::s_CurrentBackend;

    void PlatformBackend::Initialize()
    {
       PlatformBackend::s_CurrentBackend = new SDLBackend();
    }

    void PlatformBackend::Finalize()
    {
        delete PlatformBackend::s_CurrentBackend;
    }
}