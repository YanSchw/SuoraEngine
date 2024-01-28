#pragma once

#include "Suora/Core/EngineSubSystem.h"
#include "ScriptEngine.generated.h"

namespace Suora
{

	class ScriptEngine : public EngineSubSystem
	{
		SUORA_CLASS(59378090843);
	public:

		virtual void Initialize() = 0;
		virtual void Shutdown() = 0;
	};

}