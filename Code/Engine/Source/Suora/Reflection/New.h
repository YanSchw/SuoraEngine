#pragma once
#include "inttypes.h"
#include "Class.h"

namespace Suora
{

	class Object;

	/** Internal use only... */
	extern Object* InlineNew(const Class& cls, const bool isRootNode);

	/** Use Suora::New(...) to allocated a new Instance of a SuoraClass */
	[[nodiscard]] static Object* New(const Class& cls, const bool isRootNode = false)
	{
		return InlineNew(cls, isRootNode);
	}

	/** See above, just templated... */
	template<class T>
	[[nodiscard]] static T* New()
	{
		return (T*)New(T::StaticClass());
	}

}