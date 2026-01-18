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
    location "build/PhoenixEngine"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    staticruntime "off"
    characterset "Unicode"

    targetdir ("build/bin/" .. outputdir .. "/%{prj.name}")
    objdir    ("build//bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "src/**.h",
        "src/**.hpp",
        "src/**.cpp"
    }
    includedirs
    {
        "src",
        "ext/glfw/include",
        "ext",
        "ext/vulkan/include"
    }
    links
    {
        "PhoenixEngine",
        "glfw3",
        "vulkan-1"
    }

    libdirs
    {
        "ext/vulkan/Lib"
    }

    -- Shader compiler
    GLSLANG = "glslangValidator"

    filter "system:windows"
        if os.getenv("VULKAN_SDK") then
            GLSLANG = "%{os.getenv('VULKAN_SDK')}/Bin/glslangValidator.exe"
        end

        -- Shader files
        shaderFiles =
        {
            "shaders/**.vert",
            "shaders/**.frag"
        }

        files(shaderFiles)

        for i, shader in ipairs(shaderFiles) do
            shaderfiles1 = os.matchfiles(shader)
            print(shaderfiles1)
            for j, filename in ipairs(shaderfiles1) do
                print(i, j)
                print(filename)    
                filter("files:" .. filename)
                    
                    prebuildcommands
                    {
                        'echo helloworld',
                        '{ECHO} Compiling %{file.relpath} >> log.txt',
                        '"' .. GLSLANG .. '" -V "%{file.relpath}" -o "%{file.relpath}.spv"'
                    }
                filter {}
            end
        end

        
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
    
    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"
        defines "PHX_DEBUG"

    filter "configurations:Release"
        runtime "Release"
        optimize "on"
        defines "PHX_RELEASE"

    filter {}
    


