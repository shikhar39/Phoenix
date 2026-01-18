outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
workspace "Phoenix"
    architecture "x86_64"
    startproject "PhoenixEngine"
    location "build/"

    configurations
    {
        "Debug",
        "Release"
    }

    flags
    {
        "MultiProcessorCompile"
    }



project "PhoenixEngine"
    location "build"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    staticruntime "off"
    characterset "Unicode"

    targetdir ("build/bin/" .. outputdir .. "/%{prj.name}")
    objdir    ("build/bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "src/**.h",
        "src/**.hpp",
        "src/**.cpp"
    }
    includedirs
    {
        "src",
        "ext/vulkan/include",
    }

    libdirs
    {
        "ext/vulkan/Lib"
    }

    local shaderPatterns = {
        "shaders/**.vert",
        "shaders/**.frag"
    }

    -- Shader compiler
    GLSLANG = "glslangValidator"

    filter "system:windows"
		if os.getenv("VULKAN_SDK") then
	        GLSLANG = "%{os.getenv('VULKAN_SDK')}/Bin/glslangValidator.exe"
	    end

		includedirs
		{
		    "ext",
		    "ext/glfw/include",
			"ext/vulkan/include"
		}
		libdirs
		{
		    "ext/vulkan/Lib",
		}
		link
		{
		    "glfw3",
			"vulkan-1"
		}
	    systemversion "latest"
	    defines
	    {
	        "PHX_PLATFORM_WINDOWS",
	        "VK_USE_PLATFORM_WIN32_KHR"
	    }
        filter "action:vs*"
            buildoptions { "/utf-8" }
            libdirs {
                "ext/glfw/lib-vc2022"
            }

    filter "system:linux"
   		systemversion "latest"
        toolset "gcc"
        buildoptions { "-fPIC" }

        links
        {
            "glfw",
            "vulkan",
			"spdlog",
            "fmt"
        }
    filter{}

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"
        defines "PHX_DEBUG"

    filter "configurations:Release"
        runtime "Release"
        optimize "on"
        defines "PHX_RELEASE"

    filter {}
