include "../Dependencies/premake/premake_customization/solution_items.lua"

ENGINE_PATH = "%{wks.location}/../.."

workspace "SuoraBuildToolStandaloneCompilation"
	architecture "x86_64"
	startproject "SuoraBuildTool"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
include "project.lua"
