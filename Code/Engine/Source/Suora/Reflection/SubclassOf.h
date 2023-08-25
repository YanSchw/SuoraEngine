#pragma once
#include "Class.h"

namespace Suora
{

	/** Not used in C++, the Engine needs it internally, to do Scripting... */
	struct TSubclassOf
	{
		TSubclassOf()
		{
		}
		TSubclassOf(const Class& cls)
		{
			SetClass(cls);
		}

		Class GetBase() const
		{
			return m_BaseClass;
		}
		Class GetClass() const
		{
			return m_SubClass;
		}
		void SetClass(const Class& cls)
		{
			if (cls.Inherits(m_BaseClass))
			{
				m_SubClass = cls;
			}
		}

		operator Class() const
		{
			return m_SubClass;
		}

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