#pragma once

#include <functional>
#include <Suora.h>

namespace Suora
{

	struct EditorTransaction
	{
		EditorTransaction() {}
		EditorTransaction(void* memory) { m_MemoryDependency = memory; }
		virtual void Undo() { };
		virtual void Redo() { };

		void TickMemory(void* from, void* to);

		void* m_MemoryDependency = nullptr;
	private:
		class MajorTab* m_MajorTab = nullptr;
		friend class MajorTab;
	};

	struct EditorLambdaChange : public EditorTransaction
	{

		void Undo() override { m_Undo(*this); }
		void Redo() override { m_Redo(*this); }

		std::function<void(EditorLambdaChange&)> m_Undo;
		std::function<void(EditorLambdaChange&)> m_Redo;

		EditorLambdaChange(std::function<void(EditorLambdaChange&)> undo, std::function<void(EditorLambdaChange&)> redo) : m_Undo(undo), m_Redo(redo) {}
		EditorLambdaChange(void* memory, std::function<void(EditorLambdaChange&)> undo, std::function<void(EditorLambdaChange&)> redo) : m_Undo(undo), m_Redo(redo) { m_MemoryDependency = memory; }

	};

}