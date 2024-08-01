#include "Precompiled.h"
#include "New.h"

#include "Suora/Assets/Blueprint.h"
#include "Suora/Core/Object/ObjectFactory.h"

#include "Suora/Core/Engine.h"
#include "Suora/NodeScript/External/ScriptEngine.h"

namespace Suora
{

	static Object* NewScriptClassInstance(const Class& cls, const bool isRootNode)
	{
		Array<String> split = StringUtil::SplitString(cls.GetScriptClass(), '$');
		ScriptEngine* scriptEngine = ScriptEngine::GetScriptEngineByDomain(split[0]);

		return scriptEngine ? scriptEngine->CreateScriptClassInstance(split[1], isRootNode) : nullptr;
	}

	inline Object* InlineNew(const Class& cls, const bool isRootNode)
	{
		SuoraAssert(cls != Class::None);

		Object* obj = cls.IsNative() ?									ObjectFactory::Allocate(cls.GetNativeClassID()) 
					: cls.IsScriptClass() ?								NewScriptClassInstance(cls, isRootNode)
					:													cls.GetBlueprintClass()->CreateInstance(isRootNode);

		return obj;
	}

}