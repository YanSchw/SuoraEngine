#pragma once
#include <string>
#include "Suora/Editor/EditorUI.h"

namespace Suora
{

	struct DragableOverlay : public EditorUI::Overlay
	{
		DragableOverlay(const String& title) 
			: m_Title(title)
		{
			EditorUI::CurrentWindow->m_InputEvent = EditorInputEvent::EditorUI_Overlay;
		}

		void Render(float deltaTime) override;
		void OnDispose() override;
		void CenterOverlay();

	private:
		String m_Title;
		float m_HeaderHeight = 30.0f;
		bool m_Dragged = false;
	};

}