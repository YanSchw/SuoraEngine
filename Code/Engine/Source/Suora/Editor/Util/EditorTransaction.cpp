#include "Precompiled.h"
#include "EditorTransaction.h"
#include "Suora/Editor/Panels/MajorTab.h"

namespace Suora
{

	void EditorTransaction::TickMemory(void* from, void* to)
	{
		for (auto& change : m_MajorTab->UndoStack.GetData())
		{
			if (change->m_MemoryDependency == from)
				change->m_MemoryDependency = to;
		}
	}

}