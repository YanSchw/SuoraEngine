project "GLFW"
	kind "StaticLib"
	language "C"

	targetdir ("%{wks.location}/Build/" .. outputdir .. "/Dependencies/%{prj.name}")
	objdir ("%{wks.location}/Build/Intermediate/" .. outputdir .. "/Dependencies/%{prj.name}")

	files
	{
		"%{ENGINE_PATH}/Code/Dependencies/GLFW/include/GLFW/glfw3.h",
		"%{ENGINE_PATH}/Code/Dependencies/GLFW/include/GLFW/glfw3native.h",
		"%{ENGINE_PATH}/Code/Dependencies/GLFW/src/glfw_config.h",
		"%{ENGINE_PATH}/Code/Dependencies/GLFW/src/context.c",
		"%{ENGINE_PATH}/Code/Dependencies/GLFW/src/init.c",
		"%{ENGINE_PATH}/Code/Dependencies/GLFW/src/input.c",
		"%{ENGINE_PATH}/Code/Dependencies/GLFW/src/monitor.c",
		
		"%{ENGINE_PATH}/Code/Dependencies/GLFW/src/null_init.c",
		"%{ENGINE_PATH}/Code/Dependencies/GLFW/src/null_joystick.c",
		"%{ENGINE_PATH}/Code/Dependencies/GLFW/src/null_monitor.c",
		"%{ENGINE_PATH}/Code/Dependencies/GLFW/src/null_window.c",
		
		"%{ENGINE_PATH}/Code/Dependencies/GLFW/src/platform.c",
		"%{ENGINE_PATH}/Code/Dependencies/GLFW/src/vulkan.c",
		"%{ENGINE_PATH}/Code/Dependencies/GLFW/src/window.c"
	}
	filter "system:linux"
		pic "On"

		systemversion "latest"
		staticruntime "On"

		files
		{
			"%{ENGINE_PATH}/Code/Dependencies/GLFW/src/x11_init.c",
			"%{ENGINE_PATH}/Code/Dependencies/GLFW/src/x11_monitor.c",
			"%{ENGINE_PATH}/Code/Dependencies/GLFW/src/x11_window.c",
			"%{ENGINE_PATH}/Code/Dependencies/GLFW/src/xkb_unicode.c",
			"%{ENGINE_PATH}/Code/Dependencies/GLFW/src/posix_module.c",
			"%{ENGINE_PATH}/Code/Dependencies/GLFW/src/posix_time.c",
			"%{ENGINE_PATH}/Code/Dependencies/GLFW/src/posix_thread.c",
			"%{ENGINE_PATH}/Code/Dependencies/GLFW/src/glx_context.c",
			"%{ENGINE_PATH}/Code/Dependencies/GLFW/src/egl_context.c",
			"%{ENGINE_PATH}/Code/Dependencies/GLFW/src/osmesa_context.c",
			"%{ENGINE_PATH}/Code/Dependencies/GLFW/src/linux_joystick.c"
		}

		defines
		{
			"_GLFW_X11"
		}

	filter "system:windows"
		systemversion "latest"
		staticruntime "On"

		files
		{
			"%{ENGINE_PATH}/Code/Dependencies/GLFW/src/win32_init.c",
			"%{ENGINE_PATH}/Code/Dependencies/GLFW/src/win32_joystick.c",
			"%{ENGINE_PATH}/Code/Dependencies/GLFW/src/win32_module.c",
			"%{ENGINE_PATH}/Code/Dependencies/GLFW/src/win32_monitor.c",
			"%{ENGINE_PATH}/Code/Dependencies/GLFW/src/win32_time.c",
			"%{ENGINE_PATH}/Code/Dependencies/GLFW/src/win32_thread.c",
			"%{ENGINE_PATH}/Code/Dependencies/GLFW/src/win32_window.c",
			"%{ENGINE_PATH}/Code/Dependencies/GLFW/src/wgl_context.c",
			"%{ENGINE_PATH}/Code/Dependencies/GLFW/src/egl_context.c",
			"%{ENGINE_PATH}/Code/Dependencies/GLFW/src/osmesa_context.c"
		}

		defines 
		{ 
			"_GLFW_WIN32",
			"_CRT_SECURE_NO_WARNINGS"
		}

	filter "system:macosx"
		systemversion "latest"
		staticruntime "On"

		files
		{
			"%{ENGINE_PATH}/Code/Dependencies/GLFW/src/cocoa_init.m",
			"%{ENGINE_PATH}/Code/Dependencies/GLFW/src/cocoa_monitor.m",
			"%{ENGINE_PATH}/Code/Dependencies/GLFW/src/cocoa_window.m",
			"%{ENGINE_PATH}/Code/Dependencies/GLFW/src/cocoa_time.c",
			"%{ENGINE_PATH}/Code/Dependencies/GLFW/src/posix_module.c",
			"%{ENGINE_PATH}/Code/Dependencies/GLFW/src/nsgl_context.m",
			"%{ENGINE_PATH}/Code/Dependencies/GLFW/src/egl_context.c",
			"%{ENGINE_PATH}/Code/Dependencies/GLFW/src/osmesa_context.c"
		}

		defines
		{
			"_GLFW_COCOA"
		}

		-- Ensure .m files are compiled as Objective-C
		buildoptions { "-x objective-c" }

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"
