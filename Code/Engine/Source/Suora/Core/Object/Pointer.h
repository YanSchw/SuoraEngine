#pragma once
#include "Object.h"
#include "Suora/Common/Array.h"
#include <vector>
#include <unordered_map>
#include <memory>

namespace Suora
{
	template<class T, bool R> struct TemplatePtr;
	class Object;

	struct InternalPtr
	{
		static void Nullify(Object* obj);

		InternalPtr();
		~InternalPtr();

		Object* GetAddress() const;
		void Asign(Object* obj);

		Object* operator=(Object* obj);
		bool operator==(InternalPtr& other) const;

	private:
		inline static std::unordered_map<Object*, Array<InternalPtr*>> s_PtrTable;
		inline static std::unordered_map<Object*, uint32_t> s_RefCounts;

		Object* m_Value = nullptr;
		bool m_RefCounting;

		template<class T, bool R> friend struct TemplatePtr;
		friend class Object;
		friend class Node;
	};

	/** Runtime-Safe ObjectPtr implementation; automatically nullifies, if Object is deleted! */
	template<class T, bool REF_COUNTED>
	struct TemplatePtr
	{
		TemplatePtr()
		{
			m_InternalPtr.m_RefCounting = REF_COUNTED;
		}
		TemplatePtr(T& obj)
		{
			m_InternalPtr.m_RefCounting = REF_COUNTED;
			m_InternalPtr = (Object*) &obj;
		}
		TemplatePtr(T* obj)
		{
			m_InternalPtr.m_RefCounting = REF_COUNTED;
			m_InternalPtr = (Object*) obj;
		}
		TemplatePtr(const std::shared_ptr<T>& obj)
		{
			m_InternalPtr.m_RefCounting = REF_COUNTED;
			m_InternalPtr = (Object*) obj.get();
		}
		~TemplatePtr()
		{
			m_InternalPtr.Asign(nullptr);
		}

		inline T* Get() const
		{
			return (T*)m_InternalPtr.GetAddress();
		}
		inline T* operator->() const
		{
			return (T*)m_InternalPtr.GetAddress();
		}
		//Dereference operator
		inline T& operator*() const
		{
			return *(T*)m_InternalPtr.GetAddress();
		}


		template<class U, bool R>
		bool operator==(TemplatePtr<U, R>& other) const
		{
			return m_InternalPtr == other.m_InternalPtr;
		}
		TemplatePtr<T, REF_COUNTED>& operator=(T& obj)
		{
			m_InternalPtr = &obj;
			return *this;
		}
		TemplatePtr<T, REF_COUNTED>& operator=(T* obj)
		{
			m_InternalPtr = obj;
			return *this;
		}
		TemplatePtr<T, REF_COUNTED>& operator=(const std::shared_ptr<T>& obj)
		{
			m_InternalPtr = obj.get();
			return *this;
		}

		// Cast operator
		operator T* () const
		{
			return (T*)m_InternalPtr.GetAddress();
		}
		operator bool() const
		{
			return m_InternalPtr.GetAddress();
		}

	protected:
		InternalPtr m_InternalPtr;

		template<class U, bool R> friend struct TemplatePtr;
		friend struct InternalPtr;
	};

	template<class T> using Ptr			= TemplatePtr<T, false>;
	template<class T> using SharedPtr	= TemplatePtr<T, true>;

}