#include "Precompiled.h"
#include "SubclassOf.h"
#include "Class.h"

namespace Suora
{
	TSubclassOf::TSubclassOf()
	{
	}
	TSubclassOf::TSubclassOf(const Class& cls)
	{
		SetClass(cls);
	}

	Class TSubclassOf::GetBase() const
	{
		return m_BaseClass;
	}
	Class TSubclassOf::GetClass() const
	{
		return m_SubClass;
	}
	void TSubclassOf::SetClass(const Class& cls)
	{
		if (cls.Inherits(m_BaseClass))
		{
			m_SubClass = cls;
		}
	}

	TSubclassOf::operator Class() const
	{
		return m_SubClass;
	}
}