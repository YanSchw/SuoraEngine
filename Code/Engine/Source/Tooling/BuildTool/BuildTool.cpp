#include "Precompiled.h"
#include "BuildTool.h"

#include "Suora/Common/Filesystem.h"
#include "Suora/Common/Common.h"
#include "Suora/Serialization/Yaml.h"

#ifndef BUILDTOOL_IMPL
#include "Platform/Platform.h"
#endif

namespace Suora::Tools
{

	void BuildTool::GenerateModules(const std::filesystem::path& projectRootPath, BuildCollection& collection, bool writeAllModules)
	{
		std::string engineRootPath = projectRootPath.string();

		// Go through Enginepath...
		if (writeAllModules)
		{
			std::vector<DirectoryEntry> entries = File::GetAllAbsoluteEntriesOfPath(std::filesystem::path(projectRootPath).append("Content"));
			for (auto file : entries)
			{
				const std::string ext = File::GetFileExtension(file);
				if (ext == ".suora")
				{
					const std::string str = Platform::ReadFromFile(file.path().string());
					Yaml::Node root;
					Yaml::Parse(root, str);
					Yaml::Node& settings = root["Settings"];

					if (!settings["Engine"]["Path"].IsNone())
					{
						engineRootPath = settings["Engine"]["Path"].As<std::string>();
					}
					GenerateModules(std::filesystem::path(engineRootPath), collection, false);
					break;
				}
			}
		}

		// Now collect all Modules
		std::filesystem::path modulePath = std::filesystem::path(projectRootPath).append("Code").append("Modules");
		if (std::filesystem::exists(modulePath))
		{
			for (auto& module_ : std::filesystem::directory_iterator(modulePath))
			{
				if (module_.is_directory())
				{
					collection.collectedModules.push_back(std::filesystem::canonical(module_.path()));
					GenerateModule(std::filesystem::canonical(module_.path()), collection);
				}
			}
		}


		std::string premake5 = "project \"AllModules\"\n\
		kind \"StaticLib\"\n\
		language \"C++\"\n\
		cppdialect \"C++17\"\n\
		staticruntime \"on\"\n\
\n\
		targetdir(\"%{wks.location}/Build/\" ..outputdir .. \"/%{prj.name}\")\n\
		objdir(\"%{wks.location}/Build/Intermediate/\" ..outputdir .. \"/%{prj.name}\")\n\
\n\
		files\n\
		{\n\
			\"Modules.cpp\"\n\
		}\n\
\n\
		includedirs\n\
		{\n\
			\"" + engineRootPath + "/Code/Engine/Source\",\n\
			\"" + engineRootPath + "/Code/Dependencies\",\n\
			\"" + engineRootPath + "/Code/Dependencies/spdlog/include\",\n\
			\"" + engineRootPath + "/Code/Dependencies/glm\",\n\
		}\n\
\n\
		links\n\
		{\n\
			\"Engine\",\n\
			" + collection.links + "\n\
		}\n\
\n\
		group \"Modules\"\n\
			" + collection.includes + "\n\
		group \"\"\n\
\n\
		filter \"system:windows\"\n\
			systemversion \"latest\"\n\
\n\
		filter \"configurations:Debug\"\n\
			defines \"SUORA_DEBUG\"\n\
			runtime \"Debug\"\n\
			symbols \"on\"\n\
\n\
		filter \"configurations:Release\"\n\
			defines \"SUORA_RELEASE\"\n\
			runtime \"Release\"\n\
			optimize \"on\"\n\
\n\
		filter \"configurations:Dist\"\n\
			defines \"SUORA_DIST\"\n\
			runtime \"Release\"\n\
			optimize \"on\"\n\
\n\
			";

		std::string modulesCPP = "#include <iostream>\n\
		#include \"Suora/Core/Log.h\"\n\
\n\
\n\
		inline extern void Modules_Init()\n\
		{\n\
			SUORA_LOG(Suora::LogCategory::Module, Suora::LogLevel::Info, \"Initializing all Engine/Project Modules:\");\n\
			" + collection.inits + "\n\
		}";

		if (writeAllModules)
		{
			std::filesystem::create_directories(std::filesystem::path(projectRootPath).append("Build").append("AllModules"));
			Platform::WriteToFile(projectRootPath.string() + "/Build/AllModules/premake5.lua", premake5);
			Platform::WriteToFile(projectRootPath.string() + "/Build/AllModules/Modules.cpp", modulesCPP);
		}
	}

	void BuildTool::GenerateModule(const std::filesystem::path& modulePath, BuildCollection& collection)
	{
		std::string moduleName = "";
		std::string temp = modulePath.string();
		for (int32_t i = temp.size() - 1; i >= 0; i--)
		{
			if (temp[i] == '\\' || temp[i] == '/')
			{
				break;
			}
			moduleName.insert(0, 1, (char)temp[i]);
		}

		collection.links += "\"" + moduleName + "\",\n";
		collection.includes += "include \"" + modulePath.string() + "\",\n";
		collection.inits += "extern void " + moduleName + "_Init(); " + moduleName + "_Init();\n";

		std::string headerIncludes = "";
		std::vector<DirectoryEntry> entries = File::GetAllAbsoluteEntriesOfPath(std::filesystem::path(modulePath));
		for (auto file : entries)
		{
			const std::string ext = File::GetFileExtension(file);
			if (ext == ".h")
			{
				headerIncludes += "#include \"" + std::filesystem::canonical(file.path()).string() + "\"\n";
			}
		}

		std::string premake5Module = "project \"" + moduleName + "\"\n\
		kind \"StaticLib\"\n\
		language \"C++\"\n\
		cppdialect \"C++17\"\n\
		staticruntime \"on\"\n\
\n\
		targetdir(\"%{wks.location}/Build/\" ..outputdir .. \"/%{prj.name}\")\n\
		objdir(\"%{wks.location}/Build/Intermediate/\" ..outputdir .. \"/%{prj.name}\")\n\
\n\
		files\n\
		{\n\
			\"**.cpp\",\n\
			\"**.h\",\n\
			\"***.cpp\",\n\
			\"***.h\",\n\
		}\n\
\n\
		includedirs\n\
		{\n\
			\"%{wks.location}/Code/Engine/Source\",\n\
			\"%{wks.location}/Code/Dependencies\",\n\
			\"%{wks.location}/Code/Dependencies/spdlog/include\",\n\
			\"%{wks.location}/Code/Dependencies/glm\",\n\
		}\n\
\n\
		links\n\
		{\n\
			\"Engine\"\n\
		}\n\
\n\
		filter \"system:windows\"\n\
			systemversion \"latest\"\n\
\n\
		filter \"configurations:Debug\"\n\
			defines \"SUORA_DEBUG\"\n\
			runtime \"Debug\"\n\
			symbols \"on\"\n\
\n\
		filter \"configurations:Release\"\n\
			defines \"SUORA_RELEASE\"\n\
			runtime \"Release\"\n\
			optimize \"on\"\n\
\n\
		filter \"configurations:Dist\"\n\
			defines \"SUORA_DIST\"\n\
			runtime \"Release\"\n\
			optimize \"on\"\n\
			";

		std::string moduleCPP = "#include \"Suora/Core/Log.h\"\n\
\n\
			//** Header Includes */\n\
			" + headerIncludes + "\n\
\n\
			inline extern void MyModule_Init()\n\
			{\n\
				SUORA_LOG(Suora::LogCategory::Module, Suora::LogLevel::Info, \" - " + moduleName + "\");\n\
			}";

		Platform::WriteToFile(modulePath.string() + "/premake5.lua", premake5Module);
		Platform::WriteToFile(modulePath.string() + "/" + moduleName + ".module.cpp", moduleCPP);
	}

}
