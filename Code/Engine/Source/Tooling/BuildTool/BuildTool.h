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
			std::vector<std::filesystem::path> collectedModules;
			std::string links;
			std::string includes;
			std::string inits;
		};

		/** Generates the 'AllModules' Project in /Build/AllModules
		  *  projectRootPath is not the /Code path, but the path containing /Code, /Content etc. */
		void GenerateModules(const std::filesystem::path& projectRootPath, BuildCollection& collection = BuildCollection(), bool writeAllModules = true);
	private:
		void GenerateModule(const std::filesystem::path& modulePath, BuildCollection& collection);
	};

}


#ifdef BUILDTOOL_IMPL
	#include "BuildTool.cpp"
#endif