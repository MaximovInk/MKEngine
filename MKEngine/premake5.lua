project "MKEngine"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
	staticruntime "on"

    pchheader "mkpch.h"
    pchsource "src/mkpch.cpp"

    targetdir  ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

    files
	{
		"src/**.h",
		"src/**.cpp"
    }

    includedirs{
        "./src",
        "./vendor/spdlog/include",
        "./vendor/sdl/include",

        "./vendor/glfw/include"
    }

    libdirs{
        "./vendor/sdl/lib/x64",

        --"./vendor/glfw/lib-vc2022"
    }

    links{
        "SDL2main.lib",
        "SDL2.lib",

        --"glfw3_mt.lib"
        --"glfw3.lib",
        --"glfw3dll.lib",

    }

    filter "system:windows"
        staticruntime "On"
        systemversion "latest"

        defines{
            "MK_PLATFORM_WINDOWS",
            "MK_BUILD_DLL",
            "MK_ENGINE",
        }

    filter "configurations:Debug"
        defines {"MK_DEBUG","_DEBUG", "MK_ENABLE_ASSERTS"}
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        defines "MK_RELEASE"
        runtime "Release"
        optimize "On"

        