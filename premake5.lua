include "./Code/Dependencies/premake/premake_customization/solution_items.lua"

workspace "Suora"
	architecture "x86_64"
	startproject "Editor"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

	--solution_items
	--{
	--	".editorconfig"
	--}

	flags
	{
		"MultiProcessorCompile"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Include directories relative to root folder (solution directory)
IncludeDir = {}
IncludeDir["GLFW"] = "%{wks.location}/Code/Dependencies/GLFW/include"
IncludeDir["Glad"] = "%{wks.location}/Code/Dependencies/Glad/include"
IncludeDir["glm"] = "%{wks.location}/Code/Dependencies/glm"
IncludeDir["stb_image"] = "%{wks.location}/Code/Dependencies/stb_image"
IncludeDir["stb_truetype"] = "%{wks.location}/Code/Dependencies/stb_truetype"
IncludeDir["entt"] = "%{wks.location}/Code/Dependencies/entt/include"
IncludeDir["assimp"] = "%{wks.location}/Code/Dependencies/assimp/include"
IncludeDir["Reflection"] = "%{wks.location}/Code/Engine/src/Suora/Reflection"

group "Dependencies"
	include "Code/Dependencies/premake"
	include "Code/Dependencies/GLFW"
	include "Code/Dependencies/Glad"
	include "Code/Dependencies/assimp"
group ""

include "Build/AllModules"


include "Code/Engine"
include "Code/Editor"
include "Code/Runtime"
include "Code/SuoraBuildTool"
