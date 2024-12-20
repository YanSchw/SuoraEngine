#include "BuildTool.h"

#include "Common/Filesystem.h"
#include "Common/StringUtils.h"
#include "Common/Platform.h"
#include "ThirdParty/Yaml/Yaml.h"

namespace Suora::Tools
{
	/** Premake5 does not seem to like Backslashes.
	*   So this function will replace all \ with a / */
	static std::string FixPathForPremake5(std::string str)
	{
		for (int32_t i = 0; i < str.size(); i++)
		{
			if (str[i] == '\\')
			{
				str[i] = '/';
			}
		}
		return str;
	}

	void BuildTool::GenerateModules(const std::filesystem::path& projectRootPath, BuildCollection& collection, bool writeAllModules)
	{
		if (collection.allModulesPath == "")
		{
			collection.allModulesPath = projectRootPath.string() + "/Build/AllModules";
			std::filesystem::create_directories(std::filesystem::path(projectRootPath).append("Build").append("AllModules"));
			std::filesystem::create_directories(std::filesystem::path(projectRootPath).append("Build").append("Premake5Projects"));
		}
		std::string engineRootPath = projectRootPath.string();

		// Go through Enginepath...
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

				if (settings["Engine"]["Path"].IsNone())
				{
					// There is no EnginePath...
					break;
				}
				engineRootPath = settings["Engine"]["Path"].As<std::string>();
				GenerateModules(std::filesystem::path(engineRootPath), collection, false);
				break;
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
					GenerateModule(std::filesystem::canonical(module_.path()), engineRootPath, collection);
				}
			}
		}

		/** Fix EngineRootPath for Premake5 */
		engineRootPath = FixPathForPremake5(engineRootPath);

		std::string premake5 = "project \"AllModules\"\n\
		kind \"StaticLib\"\n\
		language \"C++\"\n\
		cppdialect \"C++20\"\n\
		staticruntime \"on\"\n\
\n\
		targetdir(\"%{wks.location}/Build/\" ..outputdir .. \"/%{prj.name}\")\n\
		objdir(\"%{wks.location}/Build/Intermediate/\" ..outputdir .. \"/%{prj.name}\")\n\
\n\
		files\n\
		{\n\
			\"Modules.generated.cpp\"\n\
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
		group \"EngineModules\"\n\
			" + collection.includesEngine + "\n\
		group \"\"\n\
		group \"ProjectModules\"\n\
			" + collection.includesProject + "\n\
		group \"\"\n\
\n\
			";

		std::string modulesCPP = "#include <iostream>\n\
		#include \"Suora/Core/Log.h\"\n\
		/**  This File was generated by SuoraBuildTool. Do not modify! */\n\
\n\
		inline extern void Modules_Init()\n\
		{\n\
			SUORA_LOG(Suora::LogCategory::Module, Suora::LogLevel::Info, \"Initializing all Engine/Project Modules:\");\n\
			" + collection.inits + "\n\
		}";

		if (writeAllModules)
		{
			Platform::WriteToFile(projectRootPath.string() + "/Build/AllModules/premake5.lua", premake5);
			std::string modulesCPP_Path = projectRootPath.string() + "/Build/AllModules/Modules.generated.cpp";
			if (Platform::ReadFromFile(modulesCPP_Path) != modulesCPP)
			{
				Platform::WriteToFile(modulesCPP_Path, modulesCPP);
			}
		}
	}

	void BuildTool::GenerateProjectPremake5(const std::filesystem::path& projectRootPath, const std::filesystem::path& engineRootPath)
	{
		std::string projectName = "";
		std::string temp = projectRootPath.string();
		for (int32_t i = temp.size() - 1; i >= 0; i--)
		{
			if (temp[i] == '\\' || temp[i] == '/')
			{
				break;
			}
			projectName.insert(0, 1, (char)temp[i]);
		}
		std::string enginePathStr = FixPathForPremake5(engineRootPath.string());

		std::filesystem::copy_file(std::filesystem::path(engineRootPath).append("Code").append("Engine").append("Engine.lua"), std::filesystem::path(projectRootPath).append("Build").append("Premake5Projects").append("Engine.lua"), std::filesystem::copy_options::overwrite_existing);
		std::filesystem::copy_file(std::filesystem::path(engineRootPath).append("Code").append("Editor").append("Editor.lua"), std::filesystem::path(projectRootPath).append("Build").append("Premake5Projects").append("Editor.lua"), std::filesystem::copy_options::overwrite_existing);
		std::filesystem::copy_file(std::filesystem::path(engineRootPath).append("Code").append("Runtime").append("Runtime.lua"), std::filesystem::path(projectRootPath).append("Build").append("Premake5Projects").append("Runtime.lua"), std::filesystem::copy_options::overwrite_existing);

		std::filesystem::copy_file(std::filesystem::path(engineRootPath).append("Code").append("Dependencies").append("assimp").append("assimp.lua"), std::filesystem::path(projectRootPath).append("Build").append("Premake5Projects").append("assimp.lua"), std::filesystem::copy_options::overwrite_existing);
		std::filesystem::copy_file(std::filesystem::path(engineRootPath).append("Code").append("Dependencies").append("Glad").append("Glad.lua"), std::filesystem::path(projectRootPath).append("Build").append("Premake5Projects").append("Glad.lua"), std::filesystem::copy_options::overwrite_existing);
		std::filesystem::copy_file(std::filesystem::path(engineRootPath).append("Code").append("Dependencies").append("GLFW").append("GLFW.lua"), std::filesystem::path(projectRootPath).append("Build").append("Premake5Projects").append("GLFW.lua"), std::filesystem::copy_options::overwrite_existing);

		std::string premake5 = "include \"" + enginePathStr + "/Code/Dependencies/premake/premake_customization/solution_items.lua\"\n\
\n\
			ENGINE_PATH = \"" + enginePathStr + "\"\n\
			SCRIPT_PATH = \"%{wks.location}/Scripts\"\
\n\
			workspace \"" + projectName + "\"\n\
			architecture \"x86_64\"\n\
			startproject \"Editor\"\n\
\n\
			configurations\n\
			{\n\
				\"Debug\",\n\
				\"Release\",\n\
				\"Dist\"\n\
			}\n\
\n\
			flags\n\
			{\n\
				\"MultiProcessorCompile\"\n\
			}\n\
\n\
			outputdir = \"%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}\"\n\
\n\
			--Include directories relative to root folder(solution directory)\n\
			IncludeDir = {}\n\
			IncludeDir[\"GLFW\"] = \"" + enginePathStr + "/Code/Dependencies/GLFW/include\"\n\
			IncludeDir[\"Glad\"] = \"" + enginePathStr + "/Code/Dependencies/Glad/include\"\n\
			IncludeDir[\"glm\"] = \"" + enginePathStr + "/Code/Dependencies/glm\"\n\
			IncludeDir[\"stb_image\"] = \"" + enginePathStr + "/Code/Dependencies/stb_image\"\n\
			IncludeDir[\"stb_truetype\"] = \"" + enginePathStr + "/Code/Dependencies/stb_truetype\"\n\
			IncludeDir[\"entt\"] = \"" + enginePathStr + "/Code/Dependencies/entt/include\"\n\
			IncludeDir[\"assimp\"] = \"" + enginePathStr + "/Code/Dependencies/assimp/include\"\n\
\n\
			group \"Dependencies\"\n\
				include \"Build/Premake5Projects/GLFW.lua\"\n\
				include \"Build/Premake5Projects/Glad.lua\"\n\
				include \"Build/Premake5Projects/assimp.lua\"\n\
\n\
				include \"Build/AllModules\"\n\
			group \"\"\n\
\n\
			include \"Build/Premake5Projects/Engine.lua\"\n\
			include \"Build/Premake5Projects/Editor.lua\"\n\
			include \"Build/Premake5Projects/Runtime.lua\"\n\
			";

		Platform::WriteToFile(projectRootPath.string() + "/premake5.lua", premake5);
	}

	void BuildTool::GenerateBatchScriptForPremake5Solution(const std::filesystem::path& projectRootPath, const std::filesystem::path& engineRootPath)
	{
		std::string batchScript = "@echo off\n\
			pushd %~dp0\\..\\\n\
			call " + FixPathForPremake5(engineRootPath.string()) + "/Code/Dependencies/premake/PremakeBinaries/premake5.exe vs2022\n\
			popd\n\
			";
		Platform::WriteToFile(projectRootPath.string() + "/Scripts/GenerateSolution.bat", batchScript);
	}

	static bool IsSubpath(const std::filesystem::path& base, const std::filesystem::path& path)
	{
		auto rel = std::filesystem::relative(path, base);
		return !rel.empty() && rel.native()[0] != '.';
	}
	void BuildTool::GenerateModule(const std::filesystem::path& modulePath, const std::filesystem::path& enginePath, BuildCollection& collection)
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
		std::string include = "include \"" + moduleName + ".generated.lua" + "\"\n";
		if (IsSubpath(enginePath, modulePath))
		{
			collection.includesEngine += include;
		}
		else
		{
			collection.includesProject += include;
		}
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
		cppdialect \"C++20\"\n\
		staticruntime \"on\"\n\
\n\
		MODULE_PATH = \"" + FixPathForPremake5(modulePath.string()) + "\"\n\
		AdditionalIncludeDir1 = \"\"\n\
		AdditionalIncludeDir2 = \"\"\n\
		AdditionalIncludeDir3 = \"\"\n\
		AdditionalIncludeDir4 = \"\"\n\
		dofileopt(\"" + (FixPathForPremake5(modulePath.string()) + "/" + moduleName + ".lua") + "\")\n\
\n\
		targetdir(\"%{wks.location}/Build/\" ..outputdir .. \"/%{prj.name}\")\n\
		objdir(\"%{wks.location}/Build/Intermediate/\" ..outputdir .. \"/%{prj.name}\")\n\
\n\
		files\n\
		{\n\
			\"" + FixPathForPremake5(modulePath.string()) + "/**.cpp\",\n\
			\"" + FixPathForPremake5(modulePath.string()) + "/**.h\",\n\
			\"" + FixPathForPremake5(modulePath.string()) + "/***.cpp\",\n\
			\"" + FixPathForPremake5(modulePath.string()) + "/***.h\",\n\
		}\n\
\n\
		includedirs\n\
		{\n\
			\"" + FixPathForPremake5(enginePath.string()) + "/Code/Engine/Source\",\n\
			\"" + FixPathForPremake5(enginePath.string()) + "/Code/Dependencies\",\n\
			\"" + FixPathForPremake5(enginePath.string()) + "/Code/Dependencies/spdlog/include\",\n\
			\"" + FixPathForPremake5(enginePath.string()) + "/Code/Dependencies/glm\",\n\
			\"" + FixPathForPremake5(enginePath.string()) + "/Code/Dependencies/entt/include\",\n\
			\"" + FixPathForPremake5(modulePath.string()) + "\",\n\
			\"" + FixPathForPremake5(modulePath.string()) + "/..\",\n\
			\"%{AdditionalIncludeDir1}\",\n\
			\"%{AdditionalIncludeDir2}\",\n\
			\"%{AdditionalIncludeDir3}\",\n\
			\"%{AdditionalIncludeDir4}\",\n\
		}\n\
\n\
		links\n\
		{\n\
			\"Engine\"\n\
		}\n\
\n\
		filter \"system:windows\"\n\
			systemversion \"latest\"\n\
			prebuildcommands {\"call %{SCRIPT_PATH}/SuoraBuildTool.exe\"}\n\
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
		dofileopt(\"" + (FixPathForPremake5(modulePath.string()) + "/" + moduleName + ".lua") + "\")\n\
\n\
			";

		std::string moduleCPP = "\
#include \"Suora/Core/Log.h\"\n\
/**  This File was generated by SuoraBuildTool. Do not modify! */\n\
//** Header Includes */\n\
#include \"" + modulePath.string() + "/" + moduleName + ".h" + "\"\n\
#ifdef SUORA_ENABLE_MODULE_AUTO_INCLUDE\n\
" + headerIncludes + "\n\
#endif\n\
\n\
inline extern void " + moduleName + "_Init()\n\
{\n\
	SUORA_LOG(Suora::LogCategory::Module, Suora::LogLevel::Info, \" - " + moduleName + "\");\n\
}";

		Platform::WriteToFile(collection.allModulesPath + "/" + moduleName + ".generated.lua", premake5Module);
		std::string moduleCPP_Path = modulePath.string() + "/" + moduleName + ".module.cpp";
		if (Platform::ReadFromFile(moduleCPP_Path) != moduleCPP)
		{
			Platform::WriteToFile(moduleCPP_Path, moduleCPP);
		}
	}

}
