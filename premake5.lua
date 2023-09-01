include "./Code/Dependencies/premake/premake_customization/solution_items.lua"

ENGINE_PATH = "%{wks.location}"

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

group "Dependencies"
	include "Code/Dependencies/premake"
	include "Code/Dependencies/GLFW/GLFW.lua"
	include "Code/Dependencies/Glad/Glad.lua"
	include "Code/Dependencies/assimp/assimp.lua"
group ""

include "Build/AllModules"


include "Code/Engine/Engine.lua"
include "Code/Editor/Editor.lua"
include "Code/Runtime/Runtime.lua"
include "Code/SuoraBuildTool/project.lua"
