project "Premake"
	kind "Utility"

	targetdir ("%{wks.location}/Build/" .. outputdir .. "/Dependencies/%{prj.name}")
	objdir ("%{wks.location}/Build/Intermediate/" .. outputdir .. "/Dependencies/%{prj.name}")

	files
	{
		"%{wks.location}/**premake5.lua"
	}

	postbuildmessage "Regenerating project files with Premake5!"
	postbuildcommands
	{
		"%{prj.location}bin/premake5 %{_ACTION} --file=\"%{wks.location}premake5.lua\""
	}
