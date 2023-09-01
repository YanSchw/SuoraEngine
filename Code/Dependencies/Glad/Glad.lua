project "Glad"
    kind "StaticLib"
    language "C"
    staticruntime "on"
    
    targetdir ("%{wks.location}/Build/" .. outputdir .. "/Dependencies/%{prj.name}")
    objdir ("%{wks.location}/Build/Intermediate/" .. outputdir .. "/Dependencies/%{prj.name}")

    files
    {
        "%{ENGINE_PATH}/Code/Dependencies/Glad/include/glad/glad.h",
        "%{ENGINE_PATH}/Code/Dependencies/Glad/include/KHR/khrplatform.h",
        "%{ENGINE_PATH}/Code/Dependencies/Glad/src/glad.c"
    }

    includedirs
    {
        "%{ENGINE_PATH}/Code/Dependencies/Glad/include"
    }
    
    filter "system:windows"
        systemversion "latest"

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        runtime "Release"
        optimize "on"
