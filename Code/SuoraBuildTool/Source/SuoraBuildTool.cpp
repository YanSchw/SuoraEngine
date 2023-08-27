//#include <Suora/Core/Log.h>
//#include <Suora/Core/Log.cpp>
#include "Suora/Serialization/Yaml.cpp"
#include <filesystem>
#include <iostream>
#define HEADERTOOL_IMPL
#include "Tooling/HeaderTool/HeaderTool.h"
#define BUILDTOOL_IMPL
#include "Tooling/BuildTool/BuildTool.h"


class SuoraBuildTool
{
public:

	SuoraBuildTool()
	{
	}

	~SuoraBuildTool()
	{
	}

	inline static void ProccessHeaders()
	{
		using namespace Suora;

		std::filesystem::path projectCodePath;
		projectCodePath = std::filesystem::current_path();
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

		std::cout << "Generating Modules..." << std::endl;
		Tools::BuildTool buildTool;
		buildTool.GenerateModules(std::filesystem::path(projectCodePath).parent_path());
		std::cout << "Generated AllModules!" << std::endl;
	}

};

int main()
{
	SuoraBuildTool::ProccessHeaders();

	/*SUORA_INFO("Press Enter to close the Console...");
	std::cin.get();*/
	return EXIT_SUCCESS;
}