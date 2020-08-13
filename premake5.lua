workspace "FusionShaderTools"
	architecture "x64"
	startproject "FusionShaderTools"

	configurations
	{
		"Debug",
		"Release"
	}
	
	flags
	{ 
		"MultiProcessorCompile" 
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Include directories relative to root folder (solution directory)
IncludeDir = {}
IncludeDir["glslang"] = "FusionShaderTools/vendor/glslang"

group "Dependencies"
	include "FusionShaderTools/vendor/glslang"

group ""

project "FusionShaderTools"
	location "FusionShaderTools"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	-- pchheader "fepch.h"
	-- pchsource "Fusion/src/fepch.cpp"

	files 
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		-- "%{prj.name}/vendor/stb_image/**.h",
		-- "%{prj.name}/vendor/stb_image/**.cpp",
		-- "%{prj.name}/vendor/glm/glm/**.hpp",
		-- "%{prj.name}/vendor/glm/glm/**.inl",
	}

	defines
	{
		-- Disable ImGui warnings
		-- "_CRT_SECURE_NO_WARNINGS"
	}

	includedirs 
	{
		"%{prj.name}/src",
		"%{prj.name}/vendor",
		"%{IncludeDir.glslang}",
	}

	libdirs 
	{ 
		-- "%{prj.name}/vendor/glslang/lib/",
	}

	links 
	{
		"glslang",
		-- "HLSL.lib",
		-- "OGLCompiler.lib",
		-- "OSDependent.lib",
		-- "SPIRV.lib",
		-- "SPIRV-Tools.lib",
		-- "SPIRV-Tools-opt.lib",
		-- "SPVRemapper.lib",
	}

	filter "system:windows"
		systemversion "latest"

		defines
		{
			"FE_PLATFORM_WINDOWS",
		}

		-- No longer required with static build?
		-- postbuildcommands
		-- {
		-- 	("{COPY} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/Sandbox"),
		-- }

	filter "configurations:Debug"
		defines "FE_DEBUG"
		runtime "Debug"
		buildoptions "/MDd"
		symbols "on"

	filter "configurations:Release"
		defines "FE_RELEASE"
		runtime "Release"
		buildoptions "/MD"
		optimize "on"