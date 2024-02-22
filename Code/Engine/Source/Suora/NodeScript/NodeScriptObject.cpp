#include "Precompiled.h"
#include "NodeScriptObject.h"
#include "Suora/NodeScript/Scripting/ScriptVM.h"
#include "Suora/NodeScript/External/ScriptEngine.h"
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
	}

	bool INodeScriptObject::TryDispatchNodeEvent(size_t hash, ScriptStack& stack)
	{
		bool called = false;

		if (m_ScriptEngine)
		{
			ScriptStack STACK = stack;
			m_ScriptEngine->InvokeManagedEvent(GetRootObject(), hash, STACK);
		}

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