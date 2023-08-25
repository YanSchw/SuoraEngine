#pragma once
#include "Object/Object.h"
#include "EngineSubSystem.generated.h"

namespace Suora
{
	class Engine;

	/** To be extended... */
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

		friend class Engine;
	};

}