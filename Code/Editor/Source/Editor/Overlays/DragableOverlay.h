#pragma once
#include <string>
#include "../EditorUI.h"

namespace Suora
{

	struct DragableOverlay abstract : public EditorUI::Overlay
	{
		DragableOverlay(const std::string& title) 
			: m_Title(title)
		{
			EditorUI::CurrentWindow->m_InputEvent = EditorInputEvent::EditorUI_Overlay;
		}

		void Render(float deltaTime) override;
		void OnDispose() override;
		void CenterOverlay();

	private:
		std::string m_Title;
		float m_HeaderHeight = 30.0f;
		bool m_Dragged = false;
	};

}