#pragma once
#include "Class.h"

namespace Suora
{
	struct Class;

	/** Not used in C++, the Engine needs it internally, to do Scripting... */
	struct TSubclassOf
	{
		TSubclassOf();
		TSubclassOf(const Class& cls);

		Class GetBase() const;
		Class GetClass() const;
		void SetClass(const Class& cls);

		operator Class() const;

	private:
		Class m_BaseClass = Class::None;
		Class m_SubClass = Class::None;

		template<class T> friend struct SubclassOf;
	};

	/** Used, to require a certain Baseclase */
	template<class T>
	struct SubclassOf : public TSubclassOf
	{
		SubclassOf()
		{
			m_BaseClass = T::StaticClass();
		}
		SubclassOf(const Class& cls)
		{
			m_BaseClass = T::StaticClass();
			SetClass(cls);
		}

		operator Class()
		{
			return GetClass();
		}

	};

}