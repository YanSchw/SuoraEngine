#pragma once
#include "Dockspace.h"

namespace Suora
{
	class MinorTab;
	class EditorWindow;

	class DockspacePanel
	{
	public:
		Array<Ref<DockingSpace>> m_DockingSpaces;
		Ref<MinorTab> m_CurrentDraggedTab = nullptr;
		int32_t m_DockspaceCurrentResize = 0;
		class EditorWindow* m_Window = nullptr;

		float m_PanelX = 50, m_PanelY = 50, m_PanelWidth = 996, m_PanelHeight = 560;

		DockspacePanel();

		void Update(float deltaTime);
		void Resize();

		class EditorWindow* GetEditorWindow() const;

	};

}