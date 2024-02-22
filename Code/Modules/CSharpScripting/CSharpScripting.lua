
pchheader "CoralPCH.hpp"
pchsource "%{ENGINE_PATH}/Code/Modules/CSharpScripting/ThirdParty/Coral/Coral.Native/Source/CoralPCH.cpp"

forceincludes { "CoralPCH.hpp" }

includedirs
{
	"%{ENGINE_PATH}/Code/Modules/CSharpScripting/ThirdParty/Coral/Coral.Native/",
	"%{ENGINE_PATH}/Code/Modules/CSharpScripting/ThirdParty/Coral/Coral.Native/Source",
	"%{ENGINE_PATH}/Code/Modules/CSharpScripting/ThirdParty/Coral/Coral.Native/Source/Coral",
}
files
{
    "%{ENGINE_PATH}/Code/Modules/CSharpScripting/ThirdParty/Coral/Coral.Native/Source/**.cpp",
    "%{ENGINE_PATH}/Code/Modules/CSharpScripting/ThirdParty/Coral/Coral.Native/Source/**.hpp",
}
externalincludedirs
{ 
	"%{ENGINE_PATH}/Code/Modules/CSharpScripting/ThirdParty/Coral/NetCore/"
}