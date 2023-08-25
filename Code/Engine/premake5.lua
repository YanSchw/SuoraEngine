project "Engine"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir ("%{wks.location}/Build/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/Build/Intermediate/" .. outputdir .. "/%{prj.name}")

	pchheader "Precompiled.h"
	pchsource "Source/Precompiled.cpp"

	files
	{
		"Source/**.h",
		"Source/**.cpp",
		"../Dependencies/stb_image/**.h",
		"../Dependencies/stb_image/**.cpp",
		"../Dependencies/stb_truetype/**.h",
		"../Dependencies/stb_truetype/**.cpp",
		"../Dependencies/glm/glm/**.hpp",
		"../Dependencies/glm/glm/**.inl",
	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS",
		"GLFW_INCLUDE_NONE"
	}

	includedirs
	{
		"Source",
		"../Dependencies/spdlog/include",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.stb_image}",
		"%{IncludeDir.stb_truetype}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.assimp}",
		"%{IncludeDir.Reflection}"
	}

	links
	{
		"GLFW",
		"Glad",
		"assimp",
		"opengl32.lib"
	}

	filter "system:windows"
		systemversion "latest"

		defines
		{
		}

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
