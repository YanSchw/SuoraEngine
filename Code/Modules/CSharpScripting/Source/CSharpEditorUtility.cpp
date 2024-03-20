#include "CSharpEditorUtility.h"
#include "CSharpScriptEngine.h"
#include "CSharpProjectGenerator.h"

namespace Suora
{

	CSharpGenerateSolutionMenuItem::CSharpGenerateSolutionMenuItem()
	{
		m_Lambda = []() { CSharpProjectGenerator::GenerateVisualStudioSolution(); };
	}

	CSharpBuildAndReloadMenuItem::CSharpBuildAndReloadMenuItem()
	{
		m_Lambda = []() { CSharpScriptEngine::Get()->BuildAndReloadAllCSProjects(); };
	}

}