//#include <Suora/Core/Log.h>
//#include <Suora/Core/Log.cpp>
#include <filesystem>
#include <iostream>
#define HEADERTOOL_IMPL
#include "Tooling/HeaderTool/HeaderTool.h"

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

		Tools::HeaderTool headerTool;
		headerTool.FetchHeaders(projectCodePath.string());
		headerTool.ParseHeaders(projectCodePath.string(), true);
	}

};

int main()
{
	SuoraBuildTool::ProccessHeaders();

	/*SUORA_INFO("Press Enter to close the Console...");
	std::cin.get();*/
	return EXIT_SUCCESS;
}