#include "Precompiled.h"
#include "ScriptTypes.h"

namespace Suora
{
	template<>
	ScriptDataType TemplateToScriptDataType<void>()
	{
		return ScriptDataType::Void;
	}
	template<>
	ScriptDataType TemplateToScriptDataType<class Object*>()
	{
		return ScriptDataType::ObjectPtr;
	}

}