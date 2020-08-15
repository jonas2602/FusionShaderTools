-- Include directories relative to root folder (solution directory)
IncludeDir = {}
IncludeDir["json"] = "vendor/json/include"
IncludeDir["glslang"] = "vendor/glslang"
IncludeDir["spirvcross"] = "vendor/SPIRV-Cross"

include "vendor/glslang"
include "vendor/SPIRV-Cross"

project "FusionShaderTools"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files 
	{
		"include/**.h",
		"src/**.cpp",
	}

	includedirs 
	{
		"src",
		"vendor",
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