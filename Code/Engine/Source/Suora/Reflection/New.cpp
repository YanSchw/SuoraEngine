#include "Precompiled.h"
#include "New.h"

#include "Suora/Assets/ScriptClass.h"
#include "Suora/Assets/Blueprint.h"
#include "Suora/Core/Object/ObjectFactory.h"
#include "Suora/Serialization/CompositionLayer.h"

namespace Suora
{

	inline Object* InlineNew(const Class& cls, bool includeCompositionData, bool deepestLayer)
	{
		Object* obj = cls.IsNative() ?									ObjectFactory::Allocate(cls.GetNativeClassID()) 
					: cls.IsScriptClass() ?								cls.GetScriptClass()->CreateInstance(includeCompositionData)
					:													cls.GetBlueprintClass()->CreateInstance(includeCompositionData, deepestLayer);

		if (includeCompositionData) obj->Implement<IObjectCompositionData>();
		return obj;
	}

}