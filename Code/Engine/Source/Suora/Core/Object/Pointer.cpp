#include "Precompiled.h"
#include "Pointer.h"

namespace Suora
{
	void InternalPtr::Nullify(Object* obj)
	{
		const Array<InternalPtr*>& pointers = s_PtrTable[obj];
		for (InternalPtr* ptr : pointers)
		{
			ptr->m_Value = nullptr;
		}
		s_PtrTable.erase(obj);
		s_RefCounts.erase(obj);
	}

	InternalPtr::InternalPtr()
		: m_RefCounting(false)
	{
	}

	InternalPtr::~InternalPtr()
	{
		if (m_Value == nullptr)
		{
			return;
		}

		if (InternalPtr::s_PtrTable[m_Value].Contains(this))
		{
			InternalPtr::s_PtrTable[m_Value].Remove(this);
		}
	}

	Object* InternalPtr::GetAddress() const
	{
		return m_Value;
	}

	void InternalPtr::Asign(Object* obj)
	{
		if (m_Value == obj)
		{
			// No change needed...
			return;
		}

		if (m_Value)
		{
			if (InternalPtr::s_PtrTable[m_Value].Contains(this))
			{
				InternalPtr::s_PtrTable[m_Value].Remove(this);
			}

			if (m_RefCounting)
			{
				InternalPtr::s_RefCounts[m_Value]--;
				if (s_RefCounts[m_Value] <= 0)
				{
					delete m_Value;
					if (s_RefCounts.find(m_Value) != s_RefCounts.end())
					{
						InternalPtr::Nullify(m_Value);
					}
				}
			}
		}

		m_Value = obj;

		if (m_Value)
		{
			if (!InternalPtr::s_PtrTable[m_Value].Contains(this))
			{
				InternalPtr::s_PtrTable[m_Value].Add(this);
			}
			if (m_RefCounting)
			{
				InternalPtr::s_RefCounts[m_Value]++;
			}
		}

	}

	Object* InternalPtr::operator=(Object* obj)
	{
		Asign(obj);
		return obj;
	}

	bool InternalPtr::operator==(InternalPtr& other) const
	{
		return (m_Value == other.m_Value);
	}

}