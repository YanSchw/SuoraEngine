#include "CSharpEditorUtility.h"
#include "CSharpScriptEngine.h"
#include "CSharpProjectGenerator.h"

namespace Suora
{

	bool CSharpGenerateSolutionMenuItem::Filter()
	{
		return CSharpScriptEngine::Get();
	}
	CSharpGenerateSolutionMenuItem::CSharpGenerateSolutionMenuItem()
	{
		m_Lambda = []() { CSharpProjectGenerator::GenerateVisualStudioSolution(); };
	}

	bool CSharpBuildAndReloadMenuItem::Filter()
	{
		return CSharpScriptEngine::Get();
	}
	CSharpBuildAndReloadMenuItem::CSharpBuildAndReloadMenuItem()
	{
		m_Lambda = []() { CSharpScriptEngine::Get()->BuildAndReloadAllCSProjects(); };
	}

}