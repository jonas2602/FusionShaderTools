workspace "FusionShaderTools"
	architecture "x64"
	startproject "FusionShaderTools"
	location ".."

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
IncludeDir["json"] = "%{wks.location}/vendor/json/include"
IncludeDir["glslang"] = "%{wks.location}/vendor/glslang"
IncludeDir["spirvcross"] = "%{wks.location}/vendor/SPIRV-Cross"

group "Dependencies"
	include "../vendor/glslang"
	include "../vendor/SPIRV-Cross"

-- Reset Group to Default 
group ""

project "FusionShaderTools"
	location ".."
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	files 
	{
		"%{wks.location}/include/**.h",
		"%{wks.location}/src/**.cpp",
		"%{wks.location}/standalone/**.h",
		"%{wks.location}/standalone/**.cpp",
	}

	includedirs 
	{
		"%{wks.location}/include",
		"%{wks.location}/standalone",
		"%{wks.location}/vendor",
		"%{IncludeDir.json}",
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

	filter "configurations:Debug"
		runtime "Debug"
		buildoptions "/MDd"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		buildoptions "/MD"
		optimize "on"