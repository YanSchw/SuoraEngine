project "Editor"
	kind "ConsoleApp"
	filter "configurations:Dist"
		kind "WindowedApp"
	filter {}
	
	language "C++"
	cppdialect "C++20"
	staticruntime "on"

	targetdir ("%{wks.location}/Build/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/Build/Intermediate/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{ENGINE_PATH}/Code/Editor/Source/**.h",
		"%{ENGINE_PATH}/Code/Editor/Source/**.cpp"
	}

	includedirs
	{
		"%{ENGINE_PATH}/Code/Dependencies/spdlog/include",
		"%{ENGINE_PATH}/Code/Engine/Source",
		"%{ENGINE_PATH}/Code/Dependencies",
		"%{IncludeDir.glm}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.assimp}"
	}

	links
	{
		"Engine",
		"AllModules"
	}

	filter "system:windows"
		systemversion "latest"
		prebuildcommands {"call %{SCRIPT_PATH}/SuoraBuildTool.exe"}

	filter "configurations:Debug"
		defines "SUORA_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "SUORA_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "SUORA_DIST"
		runtime "Release"
		optimize "on"
