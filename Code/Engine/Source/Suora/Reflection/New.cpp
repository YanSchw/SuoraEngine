#include "Precompiled.h"
#include "New.h"

#include "Suora/Assets/ScriptClass.h"
#include "Suora/Assets/Blueprint.h"
#include "Suora/Core/Object/ObjectFactory.h"

namespace Suora
{

	inline Object* InlineNew(const Class& cls, const bool isRootNode)
	{
		SuoraAssert(cls != Class::None);

		Object* obj = cls.IsNative() ?									ObjectFactory::Allocate(cls.GetNativeClassID()) 
					: cls.IsScriptClass() ?								cls.GetScriptClass()->CreateInstance(isRootNode)
					:													cls.GetBlueprintClass()->CreateInstance(isRootNode);

		return obj;
	}

}