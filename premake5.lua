-- Thumper Editor
workspace "aurora"
architecture "x86_64"
configurations { "debug", "release" }
startproject "aurora"

flags "MultiProcessorCompile"
language "C++"
cppdialect "C++latest"
cdialect "C17"
staticruntime "On"
stringpooling "On"

kind "StaticLib"
targetdir "%{wks.location}/bin/%{cfg.system}_%{cfg.buildcfg}"
objdir "%{wks.location}/bin_obj/%{cfg.system}_%{cfg.buildcfg}"

filter "configurations:debug"
runtime "Debug"
optimize "Debug"
symbols "On"
defines { "_DEBUG", "TE_DEBUG" }

filter "configurations:release"
runtime "Release"
optimize "Speed"
symbols "On"
defines { "NDEBUG", "TE_RELEASE" }
flags { "LinkTimeOptimization", "NoBufferSecurityCheck" }

filter "system:windows"
systemversion "latest"
defines { "NOMINMAX", "TE_WINDOWS" }
buildoptions { "/EHsc", "/Zc:preprocessor", "/Zc:__cplusplus", "/experimental:c11atomics" }

filter "system:linux"
defines "TE_LINUX"

group "dependencies"
for _, matchedfile in ipairs(os.matchfiles("premake/*.lua")) do
    include(matchedfile)
end
group ""

include "aurora/build.lua"