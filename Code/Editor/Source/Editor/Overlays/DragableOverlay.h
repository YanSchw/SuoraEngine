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
		}

		void Render(float deltaTime) override;
		void CenterOverlay();

	private:
		std::string m_Title;
		float m_HeaderHeight = 30.0f;
		bool m_Dragged = false;
	};

}