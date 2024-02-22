#pragma once
#include "Object/Object.h"
#include "EngineSubSystem.generated.h"

namespace Suora
{
	class Engine;

	/** EngineSubSystems extend the Core Engine with anonymous Subsystems and Functionality.
	*   The Engine always has to work independent of EngineSubsystems. */
	class EngineSubSystem : public Object
	{
		SUORA_CLASS(7564982331);
	public:
		EngineSubSystem()
		{
		}
		virtual ~EngineSubSystem()
		{
		}

		virtual bool Initialize() = 0;
		virtual void Shutdown() = 0;
		virtual void Tick(float deltaTime) = 0;

		friend class Engine;
	};

}