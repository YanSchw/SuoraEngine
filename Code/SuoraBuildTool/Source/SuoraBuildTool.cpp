
#include <filesystem>
#include <iostream>

#include "HeaderTool/HeaderTool.h"
#include "BuildTool/BuildTool.h"
#include "Common/Platform.h"
#include "ThirdParty/Yaml/Yaml.h"
#include "Common/Filesystem.h"
#include "Common/Log.h"

class SuoraBuildTool
{
public:

	static void ProccessHeaders()
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
			std::vector<DirectoryEntry> entries = File::GetAllAbsoluteEntriesOfPath(std::filesystem::path(projectCodePath).parent_path());
			for (auto file : entries)
			{
				const std::string ext = File::GetFileExtension(file);
				if (ext == ".suora")
				{
					const std::string str = Platform::ReadFromFile(file.path().string());
					Yaml::Node root;
					Yaml::Parse(root, str);
					Yaml::Node& settings = root["Settings"];
					enginePath = settings["Engine"]["Path"].As<std::string>();
					break;
				}
			}
		}

		Tools::HeaderTool headerTool;
		if (enginePath != "")
		{
			headerTool.FetchHeaders(enginePath + "/Code");
			headerTool.ParseHeaders(enginePath + "/Code", true);
		}
		headerTool.FetchHeaders(projectCodePath.string());
		headerTool.ParseHeaders(projectCodePath.string(), true);

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

int main()
{
	SuoraBuildTool::ProccessHeaders();

	return EXIT_SUCCESS;
}