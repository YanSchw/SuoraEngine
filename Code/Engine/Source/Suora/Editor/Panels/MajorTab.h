#pragma once
#include <string>
#include "DockspacePanel.h"
#include "Suora/Core/Base.h"
#include "Suora/Common/Array.h"
#include "Minor/LevelOutliner.h"
#include "Minor/ContentBrowser.h"
#include "Minor/ViewportPanel.h"
#include "Minor/DetailsPanel.h"
#include "Minor/NodeGraphEditor.h"
#include "Suora/GameFramework/Nodes/MeshNode.h"
#include "Suora/Editor/Util/EditorTransaction.h"
#include "MajorTab.generated.h"

namespace Suora
{
	class Texture2D;
	class EditorWindow;
	class MinorTab;
	class MajorTab;
	class DockspacePanel;
	struct Dockspace;

	class MajorTab : public Object
	{
		SUORA_CLASS(689754);

	public:
		String m_Name = "MajorTab";

		Ptr<Asset> m_Asset = nullptr;

		DockspacePanel m_DockspacePanel;

		uint32_t x = 0, y = 0, width = 10, height = 10;
		float m_BottomBarWidth = 35.0f;

		int UndoStackIndex = -1;
		Array<Ref<EditorTransaction>> UndoStack;

		MajorTab() { }

		virtual void Init();
		virtual void Update(float deltaTime);
		virtual void DrawToolbar(float& x, float y, float height);
		virtual void ApplyChangesOfOtherMajorTabs(MajorTab* other) { }
		virtual Texture* GetIconTexture();

		virtual void SaveAsset();

		EditorWindow* GetEditorWindow() const
		{
			return m_EditorWindow;
		}

		template<class T>
		bool RegisterMinorTab(T* t)
		{
			return false;
		}

		void Undo()
		{
			if (UndoStackIndex < 0) return;
			
			UndoStack[UndoStackIndex--]->Undo();
		}
		void Redo()
		{
			if (UndoStackIndex+1 >= UndoStack.Size()) return;

			UndoStack[++UndoStackIndex]->Redo();
		}
		void MakeEditorChange(Ref<EditorTransaction> change)
		{

			for (int i = UndoStack.Size()-1; i >= 0; i--)
			{
				if (i > UndoStackIndex) UndoStack.RemoveAt(i);
			}

			change->m_MajorTab = this;
			UndoStack.Add(change);
			UndoStackIndex++;
		}
	private:
		EditorWindow* m_EditorWindow = nullptr;
	protected:
		Class m_AssetClass = Class::None;

		friend class EditorWindow;
	};
}