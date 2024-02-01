#include "Precompiled.h"
#include "ScriptEngine.h"

#include "Suora/Core/Engine.h"

namespace Suora
{

	ScriptEngine* ScriptEngine::GetScriptEngineByDomain(const String& domain)
	{
		auto subsystems = Engine::Get()->GetEngineSubsystems();

		for (auto It : subsystems)
		{
			ScriptEngine* scriptEngine = It->As<ScriptEngine>();
			if (scriptEngine)
			{
				if (scriptEngine->GetScriptClassDomain() == domain)
				{
					return scriptEngine;
				}
			}
		}

		return nullptr;
	}

}