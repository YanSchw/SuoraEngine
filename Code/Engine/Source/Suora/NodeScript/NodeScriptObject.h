#pragma once
#include "Suora/Core/Object/Interface.h"
#include "NodeScriptObject.generated.h"

namespace Suora
{

	struct ScriptClassInternal;
	class World;
	class Blueprint;

	class INodeScriptObject : public Interface
	{
		SUORA_CLASS(62476);
	public:
		Class m_Class = Class::None;
		Array<Ref<ScriptClassInternal>> m_ScriptClasses;
		Array<Blueprint*> m_BlueprintLinks;
		World* m_World = nullptr;

		virtual ~INodeScriptObject();

		void InitializeBlueprintInstance(World& world);
		bool TryDispatchNodeEvent(size_t hash, ScriptStack& stack);

	};
}