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
IncludeDir["spirvcross"] = "FusionShaderTools/vendor/SPIRV-Cross"

group "Dependencies"
	include "FusionShaderTools/vendor/glslang"
	include "FusionShaderTools/vendor/SPIRV-Cross"

-- Reset Group to Default 
group ""

project "FusionShaderTools"
	location "FusionShaderTools"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files 
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
	}

	includedirs 
	{
		"%{prj.name}/src",
		"%{prj.name}/vendor",
		"%{IncludeDir.glslang}",
		"%{IncludeDir.spirvcross}",
	}

	links 
	{
		"glslang",
		"SPIRV-Cross",
	}

	filter "system:windows"
		systemversion "latest"

		defines
		{
			"FE_PLATFORM_WINDOWS",
		}

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