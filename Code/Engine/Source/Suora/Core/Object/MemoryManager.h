#pragma once
#include "inttypes.h"
#include <vector>
#include <stack>

namespace Suora
{
	class Object;

	class MemoryManager
	{
		struct Pointer
		{
			Object* Src = nullptr;
			Object** Ptr = nullptr;
			bool PendingKill = false;
			Pointer(Object* src, Object** ptr) 
				: Src(src), Ptr(ptr) 
			{
			}
		};

	private:
		static inline uint32_t s_MemoryFootprint = 0;
		static inline MemoryManager* s_Instance = nullptr;
	public:
		std::vector<Pointer> m_ObservedPointers;
		MemoryManager()
		{
			s_Instance = this;
		}
		static MemoryManager* Instance()
		{
			return s_Instance ? s_Instance : new MemoryManager();
		}
		void ObservePointer(Object* src, Object** ptr)
		{
			m_ObservedPointers.push_back(Pointer(src, ptr));
		}
		void NullifyPtrTo(Object* obj)
		{
			for (int i = m_ObservedPointers.size() - 1; i >= 0; i--)
			{
				Pointer& PTR = m_ObservedPointers[i];
				if (PTR.Src == obj) PTR.PendingKill = true; // TODO: Remove Ptr from Vector
				if ((*PTR.Ptr) == obj) (*PTR.Ptr) = nullptr;

			}
		}
		void CleanUp()
		{

		}
		static inline uint32_t GetMemoryFootprint()
		{
			return s_MemoryFootprint;
		}
		friend class Object;
	};

}