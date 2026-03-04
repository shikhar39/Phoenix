outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

workspace "Phoenix"
    architecture "x86_64"
    startproject "PhoenixEngine"

    configurations { "Debug", "Release" }

    flags { "MultiProcessorCompile" }

project "PhoenixEngine"
    location "build"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    staticruntime "off"
    characterset "Unicode"

    targetdir ("build/bin/" .. outputdir .. "/%{prj.name}")
    objdir    ("build/bin-int/" .. outputdir .. "/%{prj.name}")

    files {
        "src/**.h",
        "src/**.hpp",
        "src/**.cpp",
        "shaders/**.vert",
        "shaders/**.frag",
    }

    includedirs {
        "src",
    }

    shaderdir = "shaders"
    shaderout = "shaders/out"
    GLSLANG   = "glslangValidator"

    -------------------------
    -- WINDOWS
    -------------------------
    filter "system:windows"
        if os.getenv("VULKAN_SDK") and os.target() == "windows" then
            GLSLANG = "%{os.getenv('VULKAN_SDK')}/Bin/glslangValidator.exe"
        end

        includedirs {
            "ext",
            "ext/glfw/include",
            "ext/vulkan/include",
        }

        libdirs {
            "ext/glfw/lib-vc2022",
            "ext/vulkan/lib",
        }

        links {
            "glfw3",
            "vulkan-1",
        }

        systemversion "latest"
        defines {
            "PHX_PLATFORM_WINDOWS",
            "VK_USE_PLATFORM_WIN32_KHR",
        }

        filter "action:vs*"
            buildoptions { "/utf-8" }
        filter {}
    filter {}

    -------------------------
    -- LINUX
    -------------------------
    filter "system:linux"
        systemversion "latest"
        toolset "gcc"
        buildoptions { "-fPIC" }
        if os.getenv("VULKAN_SDK") then
            includedirs {
                "/usr/include",           -- normal system includes
                "/usr/include/GLFW",      -- system GLFW headers
                "%{os.getenv('VULKAN_SDK')}/include", -- Vulkan SDK headers
            }

            -- We usually don't *need* libdirs for Vulkan on Linux;
            -- let the linker find system libvulkan.so in /usr/lib.
            GLSLANG = "%{os.getenv('VULKAN_SDK')}/bin/glslangValidator"
        else
            includedirs {
                "/usr/include",
                "/usr/include/GLFW",
                "/usr/include/vulkan",
            }

            GLSLANG = "glslangValidator"
        end
        links {
            "glfw",   -- Arch’s lib name is libglfw.so
            "vulkan",
            "spdlog",
            "fmt",
        }

        shadercommand = GLSLANG .. " -V %{file.path} -o ../" .. shaderout .. "/%{file.name}.spv"
        filter "files:**.vert or files:**.frag"
            buildmessage "Compiling shader %{file.relpath}"
            buildcommands {
                "echo " .. shadercommand,
                shadercommand,
            }
            buildoutputs {
                shaderout .. "/%{file.name}.spv",
            }
        filter {}
    filter {}

    -------------------------
    -- CONFIGS
    -------------------------
    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"
        defines "PHX_DEBUG"
    filter {}

    filter "configurations:Release"
        runtime "Release"
        optimize "on"
        defines "PHX_RELEASE"
    filter {}
