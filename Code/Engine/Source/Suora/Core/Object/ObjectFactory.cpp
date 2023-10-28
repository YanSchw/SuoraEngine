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
			Classes.Add(Class(it.first));
		}

		return Classes;
	}

	Object* ObjectFactory::Allocate(const NativeClassID id)
	{
		if (s_ObjectAllocators.find(id) == s_ObjectAllocators.end())
		{
			return nullptr;
		}
		return s_ObjectAllocators.at(id)();
	}
}