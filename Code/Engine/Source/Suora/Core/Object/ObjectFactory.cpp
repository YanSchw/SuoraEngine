#include "Precompiled.h"
#include "ObjectFactory.h"
#include "Suora/Common/Array.h"

namespace Suora
{
	
	
	Array<Class> ObjectFactory::GetAllNativeClasses()
	{
		Array<Class> Classes;

		for (auto it : s_ObjectAllocators)
		{
			Classes.Add(it.first);
		}

		return Classes;
	}

	Object* ObjectFactory::Allocate(NativeClassID id)
	{
		return s_ObjectAllocators.at(id)();
	}
}