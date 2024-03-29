#pragma once
#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <memory>
#include <filesystem>
#include <future>
#include <mutex>

namespace Suora::Tools
{
	template<class T> using Ref = std::shared_ptr<T>;

	struct BuildTool
	{
		struct BuildCollection
		{
			std::string allModulesPath = "";
			std::vector<std::filesystem::path> collectedModules;
			std::string links;
			std::string includesEngine;
			std::string includesProject;
			std::string inits;
		};

		/** Generates the 'AllModules' Project in /Build/AllModules
		  *  projectRootPath is not the /Code path, but the path containing /Code, /Content etc. */
		void GenerateModules(const std::filesystem::path& projectRootPath, BuildCollection& collection, bool writeAllModules = true);

		void GenerateProjectPremake5(const std::filesystem::path& projectRootPath, const std::filesystem::path& engineRootPath);

		void GenerateBatchScriptForPremake5Solution(const std::filesystem::path& projectRootPath, const std::filesystem::path& engineRootPath);
	private:
		void GenerateModule(const std::filesystem::path& modulePath, const std::filesystem::path& enginePath, BuildCollection& collection);
	};

}


#ifdef BUILDTOOL_IMPL
	#include "BuildTool.cpp"
#endif