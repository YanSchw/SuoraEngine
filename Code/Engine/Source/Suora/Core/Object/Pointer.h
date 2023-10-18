#pragma once
#include "Object.h"
#include "Suora/Common/Array.h"
#include <vector>
#include <unordered_map>
#include <memory>

namespace Suora
{
	template<class T, bool R> struct TObjectPtr;
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

		template<class T, bool R> friend struct TObjectPtr;
		friend class Object;
		friend class Node;
	};

	/** Runtime-Safe ObjectPtr implementation; automatically nullifies, if Object is deleted! */
	template<class T, bool REF_COUNTED>
	struct TObjectPtr
	{
		TObjectPtr()
		{
			m_InternalPtr.m_RefCounting = REF_COUNTED;
		}
		TObjectPtr(T& obj)
		{
			m_InternalPtr.m_RefCounting = REF_COUNTED;
			m_InternalPtr = (Object*) &obj;
		}
		TObjectPtr(T* obj)
		{
			m_InternalPtr.m_RefCounting = REF_COUNTED;
			m_InternalPtr = (Object*) obj;
		}
		template<class U, bool R>
		TObjectPtr(const TObjectPtr<U, R>& obj)
		{
			static_assert(std::is_base_of<T, U>::value);

			m_InternalPtr.m_RefCounting = REF_COUNTED;
			m_InternalPtr = (Object*)obj.Get();
		}
		TObjectPtr(const std::shared_ptr<T>& obj)
		{
			m_InternalPtr.m_RefCounting = REF_COUNTED;
			m_InternalPtr = (Object*) obj.get();
		}
		~TObjectPtr()
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
		bool operator==(TObjectPtr<U, R>& other) const
		{
			return m_InternalPtr == other.m_InternalPtr;
		}
		TObjectPtr<T, REF_COUNTED>& operator=(T& obj)
		{
			m_InternalPtr = (Object*) &obj;
			return *this;
		}
		TObjectPtr<T, REF_COUNTED>& operator=(T* obj)
		{
			m_InternalPtr = (Object*) obj;
			return *this;
		}

		template<class U, bool R>
		TObjectPtr<T, REF_COUNTED>& operator=(const TObjectPtr<U, R>& obj)
		{
			static_assert(std::is_base_of<T, U>::value);
			m_InternalPtr = obj.Get();
			return *this;
		}
		TObjectPtr<T, REF_COUNTED>& operator=(const std::shared_ptr<T>& obj)
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

		template<class U, bool R> friend struct TObjectPtr;
		friend struct InternalPtr;
	};

	template<class T> using Ptr         = TObjectPtr<T, false>;
	template<class T> using WeakPtr     = TObjectPtr<T, false>;
	template<class T> using SharedPtr   = TObjectPtr<T, true>;
	template<class T> using Ref         = std::shared_ptr<T>;
	template<class T> using Scope       = std::unique_ptr<T>;

}