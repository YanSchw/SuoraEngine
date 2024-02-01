#include "Precompiled.h"
#include "ScriptEngine.h"

#include "Suora/Core/Engine.h"

namespace Suora
{

	ScriptEngine* ScriptEngine::GetScriptEngineByDomain(const String& domain)
	{
		for (auto It : s_ScriptEngines)
		{
			if (It->GetScriptClassDomain() == domain)
			{
				return It;
			}
		}

		return nullptr;
	}

}