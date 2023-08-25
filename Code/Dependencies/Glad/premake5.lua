project "Glad"
    kind "StaticLib"
    language "C"
    staticruntime "on"
    
    targetdir ("%{wks.location}/Build/" .. outputdir .. "/Dependencies/%{prj.name}")
    objdir ("%{wks.location}/Build/Intermediate/" .. outputdir .. "/Dependencies/%{prj.name}")

    files
    {
        "include/glad/glad.h",
        "include/KHR/khrplatform.h",
        "src/glad.c"
    }

    includedirs
    {
        "include"
    }
    
    filter "system:windows"
        systemversion "latest"

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        runtime "Release"
        optimize "on"
