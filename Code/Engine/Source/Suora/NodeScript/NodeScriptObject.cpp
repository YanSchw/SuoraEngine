#include "Precompiled.h"
#include "NodeScriptObject.h"
#include "Suora/NodeScript/Scripting/ScriptVM.h"
#include "Suora/Assets/Blueprint.h"
#include "Suora/GameFramework/InputModule.h"
#include "Suora/GameFramework/GameInstance.h"
#include "Suora/GameFramework/World.h"
#include "Suora/GameFramework/Node.h"

namespace Suora
{

	INodeScriptObject::~INodeScriptObject()
	{
	}

	void INodeScriptObject::InitializeBlueprintInstance(World& world)
	{
		m_World = &world;
		/*if (world.GetGameInstance() && world.GetGameInstance()->GetInputModule() && m_BlueprintLinks.Size() > 0)
		{
			bool bBind = false;
			for (Blueprint* blueprint : m_BlueprintLinks)
			{
				if (blueprint->m_InputEventsToBeBound.Size() != 0) bBind = true;
			}
			if (bBind)
			{
				world.GetGameInstance()->GetInputModule()->RegisterBlueprintInstance(GetRootObject()->As<Node>(), m_BlueprintLinks[m_BlueprintLinks.Last()]->m_InputEventsAreForPawnsOnly);
				for (Blueprint* blueprint : m_BlueprintLinks)
				{
					for (auto& It : blueprint->m_InputEventsToBeBound)
					{
						world.GetGameInstance()->GetInputModule()->BindInputScriptEvent(GetRootObject()->As<Node>(), It.Label, It.Flags, It.ScriptFunctionHash);
					}
				}
			}
		}*/
	}

	bool INodeScriptObject::TryDispatchNodeEvent(size_t hash, ScriptStack& stack)
	{
		bool called = false;

		for (Ref<ScriptClassInternal> script : m_ScriptClasses)
		{
			for (ScriptFunction& func : script->m_Functions)
			{
				if (func.m_IsEvent && func.m_Hash == hash)
				{
					ScriptStack STACK = stack;
					func.Call(GetRootObject(), STACK);
					called = true;
				}
			}
		}

		return called;
	}
}