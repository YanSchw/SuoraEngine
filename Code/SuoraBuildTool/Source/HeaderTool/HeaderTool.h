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

	struct HeaderOutput
	{
		std::string m_OriginalHeaderPath;
		std::string m_GeneratedHeaderPath;
		bool operator<(const HeaderOutput& other) const { return m_OriginalHeaderPath < other.m_OriginalHeaderPath; }
	};

	struct FunctionMeta
	{
		std::string m_Helper;
		std::string m_Registor;
		FunctionMeta(const std::string& helper, const std::string& registor)
			: m_Helper(helper), m_Registor(registor)
		{
		}
	};

	struct HeaderTool
	{
		HeaderTool() = default;
		~HeaderTool() = default;
		void FetchHeaders(const std::string& path);
		void ParseHeaders(const std::filesystem::path& outPath, bool cacheWriteTime);

	private:
		void ParseSingleHeader(const std::filesystem::path& path);

	private:
		std::vector<std::filesystem::path> m_HeaderPaths;
		std::map<HeaderOutput, std::string> m_Output;

		struct Header
		{
			Header(HeaderTool* tool) : m_HeaderTool(tool)
			{
			}
			// To be pulled from HeaderFile
			std::string m_ClassID;
			std::string m_ClassName;
			std::string m_ParentClass = "None";
			bool m_IsStruct = false;

			// To be generated
			std::vector<std::string> m_ParentNames;
			std::vector<std::string> m_ParentIDs;

		private:
			// Metadata from HeaderFile
			std::string str;
			std::string m_HeaderPath;
			size_t m_ClassBodyBegin = 0;
			size_t m_ClassBodyEnd = 0;
			HeaderTool* m_HeaderTool = nullptr;

			friend struct HeaderTool;
		};

		void GenerateClassSymbols(HeaderTool::Header* header, std::map<HeaderOutput, std::string>* output);
		void GenerateTemplateInnerMember(std::string& str, std::string inner, const std::string& index);
		void GeneratePropertyReflection(const std::string& className, std::string& str, const std::string& meta, std::string member);
		FunctionMeta GenerateFunctionReflection(const std::string& className, const std::string& classID, const std::string& meta, const std::string& func);

		void AddHeader(Ref<Header> ref);
		std::vector<Ref<Header>> m_Headers;

		double m_InitialTime = 0.0;
	};
}


#ifdef HEADERTOOL_IMPL
	#include "HeaderTool.cpp"
#endif