project "Runtime"
	kind "WindowedApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir ("%{wks.location}/Build/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/Build/Intermediate/" .. outputdir .. "/%{prj.name}")

	files 
	{
		"%{ENGINE_PATH}/Code/Runtime/Source/**.h",
		"%{ENGINE_PATH}/Code/Runtime/Source/**.cpp"
	}

	includedirs 
	{
		"%{ENGINE_PATH}/Code/Dependencies/spdlog/include",
		"%{ENGINE_PATH}/Code/Engine/Source",
		"%{ENGINE_PATH}/Code/Dependencies",
		"%{IncludeDir.glm}",
		"%{IncludeDir.entt}"
	}

	links 
	{
		"Engine",
		"AllModules"
	}

	filter "system:windows"
		systemversion "latest"

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
		