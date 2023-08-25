#pragma once
#include "Object.h"
#include "Suora/Common/Array.h"
#include <vector>
#include <unordered_map>
#include <memory>

namespace Suora
{
	template<class T> struct Ptr;

	struct PtrTable
	{
	private:
		inline static std::unordered_map<void*, Array<Ptr<void*>*>> s_PtrTable;

		static void Nullify(void* obj);

		template<class T> friend struct Ptr;
		friend class Object;
	};

	/** Runtime-Safe WeakObjectPtr implementation; automatically nullifies, if Object is deleted! */
	template<class T>
	struct Ptr
	{
		Ptr()
		{
		}
		Ptr(T& obj)
		{
			ptr = &obj;
		}
		Ptr(T* obj)
		{
			ptr = obj;
		}
		Ptr(const std::shared_ptr<T>& obj)
		{
			ptr = obj.get();
		}
		~Ptr()
		{
			if (!ptr) return;
			if (PtrTable::s_PtrTable[ptr].Contains((Ptr<void*>*)this)) PtrTable::s_PtrTable[ptr].Remove((Ptr<void*>*)this);
		}

		inline T* Get() const
		{
			return ptr;
		}
		inline T* operator->() const
		{
			return ptr;
		}
		//Dereference operator
		inline T& operator*() const
		{
			return *ptr;
		}


		template<class U>
		bool operator==(Ptr<U>& other) const
		{
			return ptr == other.ptr;
		}
		Ptr<T>& operator=(T& obj)
		{
			ptr = &obj;
			return *this;
		}
		Ptr<T>& operator=(T* obj)
		{
			ptr = obj;
			return *this;
		}
		Ptr<T>& operator=(const std::shared_ptr<T>& obj)
		{
			ptr = obj.get();
			return *this;
		}

		// Cast operator
		operator T* () const
		{
			return ptr;
		}
		operator bool() const
		{
			return ptr;
		}

		/*inline Class GetTemplateClass()
		{
			return T::StaticClass();
		}*/

	private:
		T* ptr = nullptr;
		template<class U> friend struct Ptr;
		friend struct PtrTable;
	};

}