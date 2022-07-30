project "MKEditor"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "On"

    targetdir  ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

    files{
        "src/**.h",
        "src/**.cpp"
    }

    includedirs{
        "%{wks.location}/MKEngine/src",
        "%{wks.location}/MKEngine/vendor/spdlog/include",
    }

    links {
        "MKEngine"
    }
    
    filter "system:windows"
        systemversion "latest"

        defines{
            "MK_PLATFORM_WINDOWS",
            "MK_ENABLE_LOG"
        }

    filter "configurations:Debug"
        defines {"MK_DEBUG","_DEBUG", "MK_ENABLE_ASSERTS"}
        symbols "On"
        buildoptions "/MDd"

    filter "configurations:Release"
        defines "MK_RELEASE"
        buildoptions "/MD"
        optimize "On"