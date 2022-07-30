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
        "./vendor/spdlog/include"
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
        symbols "On"
        buildoptions "/MDd"

    filter "configurations:Release"
        defines "MK_RELEASE"
        optimize "On"
        buildoptions "/MD"
		optimize "On"

        