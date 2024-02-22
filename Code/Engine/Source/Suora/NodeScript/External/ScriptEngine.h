#pragma once

#include "Suora/Core/EngineSubSystem.h"
#include "ScriptEngine.generated.h"

namespace Suora
{

	class ScriptEngine : public EngineSubSystem
	{
		SUORA_CLASS(59378090843);
	public:

		/** Returns the 'ClassDomain' of ScriptClasses
		*   For C# Scripting it returns 'CSharp' because that is the Prefix of all C# ScriptClasses
		*	E.g. 'CSharp$Example.MyClass'	                                                        */
		virtual String GetScriptClassDomain() const = 0;

		virtual Array<Class> GetAllScriptClasses() = 0;

		virtual Class GetScriptParentClass(String scriptClass) = 0;

		virtual Object* CreateScriptClassInstance(const String& scriptClass, bool isRootNode) = 0;

		virtual void InvokeManagedEvent(Object* obj, size_t hash, ScriptStack& stack) = 0;

		static ScriptEngine* GetScriptEngineByDomain(const String& domain);

	protected:
		inline static Array<ScriptEngine*> s_ScriptEngines;

		friend struct Class;
	};

}