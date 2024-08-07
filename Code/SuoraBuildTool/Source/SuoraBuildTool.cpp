
#include <filesystem>
#include <iostream>

#include "SuoraBuildToolParams.h"
#include "HeaderTool/HeaderTool.h"
#include "BuildTool/BuildTool.h"
#include "Common/Platform.h"
#include "ThirdParty/Yaml/Yaml.h"
#include "Common/Filesystem.h"
#include "Common/Log.h"

class SuoraBuildTool
{
public:

	static void ProccessHeaders(SuoraBuildToolParams& params)
	{
		using namespace Suora;

		std::filesystem::path projectCodePath = std::filesystem::current_path();
		while (!std::filesystem::is_directory(projectCodePath.append("Code")))
		{
			projectCodePath = projectCodePath.parent_path().parent_path();
		}

		// Search For EnginePath...
		std::string enginePath = "";
		{
			std::vector<DirectoryEntry> entries = File::GetAllAbsoluteEntriesOfPath(std::filesystem::path(projectCodePath).parent_path() / "Content");
			for (auto file : entries)
			{
				const std::string ext = File::GetFileExtension(file);
				if (ext == ".suora")
				{
					const std::string str = Platform::ReadFromFile(file.path().string());
					Yaml::Node root;
					Yaml::Parse(root, str);
					Yaml::Node& settings = root["Settings"];
					std::string possibleEnginePath = settings["Engine"]["Path"].As<std::string>();
					if (std::filesystem::exists(std::filesystem::path(possibleEnginePath)))
					{
						enginePath = possibleEnginePath;
						BUILD_INFO("Engine located in {0}", enginePath);
					}
					break;
				}
			}
		}

		Tools::HeaderTool headerTool;
		if (enginePath != "")
		{
			headerTool.FetchHeaders(enginePath + "/Code");
			headerTool.ParseHeaders(enginePath + "/Code", params.CacheHeaderWriteTimes);
		}
		headerTool.FetchHeaders(projectCodePath.string());
		headerTool.ParseHeaders(projectCodePath.string(), params.CacheHeaderWriteTimes);

		BUILD_INFO("Generating Modules...");
		Tools::BuildTool buildTool;
		Tools::BuildTool::BuildCollection collection;
		buildTool.GenerateModules(std::filesystem::path(projectCodePath).parent_path(), collection);
		BUILD_INFO("Generated AllModules!");

		if (enginePath != "")
		{
			buildTool.GenerateProjectPremake5(projectCodePath.parent_path(), enginePath);
			buildTool.GenerateBatchScriptForPremake5Solution(projectCodePath.parent_path(), enginePath);
		}

		BUILD_DEBUG("Done!");
	}

};

int main(int argc, char** argv)
{
	SuoraBuildToolParams params;

	for (int i = 1; i < argc; i++)
	{
		std::string str = argv[i];
		if (str == "-?")
		{
			BUILD_DEBUG("[-v] | [-version] to show the Engine Version SuoraBuildTool supports.");
			BUILD_DEBUG("[-donotcache] to regenerate all Headers.");
		}
		else if (str == "-v" || str == "-version")
		{
			BUILD_DEBUG("This compilation of SuoraBuildTool targets SuoraEngine Version 0.11.0");
		}
		else if (str == "-donotcache")
		{
			params.CacheHeaderWriteTimes = false;
		}
		else
		{
			BUILD_ERROR("Could not parse argument '{0}'!", str);
			return EXIT_FAILURE;
		}
	}

	SuoraBuildTool::ProccessHeaders(params);

	return EXIT_SUCCESS;
}