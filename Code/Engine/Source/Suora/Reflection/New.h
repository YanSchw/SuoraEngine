#pragma once
#include "inttypes.h"
#include "Class.h"

namespace Suora
{

	class Object;

	/** Internal use only... */
	extern inline Object* InlineNew(const Class& cls, bool includeCompositionData, bool deepestLayer);

	/** Use Suora::New(...) to allocated a new Instance of a SuoraClass */
	static Object* New(const Class& cls, bool includeCompositionData = false, bool deepestLayer = false)
	{
		return InlineNew(cls, includeCompositionData, deepestLayer);
	}

	/** See above, just templated... */
	template<class T>
	static T* New()
	{
		return (T*)New(T::StaticClass());
	}

}