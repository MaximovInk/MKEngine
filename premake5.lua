workspace "MKEngine"
    startproject "MKEditor"
    
    architecture "x64"
    
    configurations{
        "Debug",
        "Release"
    }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

include "MKEngine"
include "MKEditor"