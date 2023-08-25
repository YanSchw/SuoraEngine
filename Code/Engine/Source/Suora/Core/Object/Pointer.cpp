#include "Precompiled.h"
#include "Pointer.h"

namespace Suora
{
	void PtrTable::Nullify(void* obj)
	{
		const Array<Ptr<void*>*>& pointers = s_PtrTable[obj];
		for (Ptr<void*>* ptr : pointers)
		{
			ptr->ptr = nullptr;
		}
		s_PtrTable.erase(obj);
	}
}