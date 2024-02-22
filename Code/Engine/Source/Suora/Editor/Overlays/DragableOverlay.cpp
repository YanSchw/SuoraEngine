#include "Precompiled.h"
#include "DragableOverlay.h"

namespace Suora
{

	void DragableOverlay::Render(float deltaTime)
	{
		if (WasMousePressedOutsideOfOverlay())
		{
			Dispose();
			return;
		}
		EditorUI::DrawRect(x + 5, y - 5, width, height, 4, Color(0, 0, 0, 0.15f));

		EditorUI::DrawRect(x, y, width, height, 4, Color(Vec3(EditorPreferences::Get()->UiColor) * 0.8f, 1.0f));
		EditorUI::DrawRect(x + 2, y + 2, width - 4, height - 4, 4, EditorPreferences::Get()->UiColor);
		EditorUI::DrawRect(x, y + height - m_HeaderHeight, width, m_HeaderHeight, -4, Color(Vec3(EditorPreferences::Get()->UiColor) * 0.8f, 1.0f));
		EditorUI::Text(m_Title, Font::Instance, x + 10.0f + 2.0f, y + height - m_HeaderHeight - 2.0f, width - 10.0f, m_HeaderHeight, 28.0f, Vec2(-1, 0), Color(0, 0, 0, 0.85f));
		EditorUI::Text(m_Title, Font::Instance, x + 10.0f, y + height - m_HeaderHeight, width - 10.0f, m_HeaderHeight, 28.0f, Vec2(-1, 0), Color(1.0f));

		if (m_Dragged)
		{
			x += NativeInput::GetMouseDelta().x;
			y -= NativeInput::GetMouseDelta().y;
		}
		else
		{
			if (x < 0) x = 0;
			if (y < 0) y = 0;
			if (x + width > EditorUI::CurrentWindow->GetWindow()->GetWidth()) x = EditorUI::CurrentWindow->GetWindow()->GetWidth() - width;
			if (y + height > EditorUI::CurrentWindow->GetWindow()->GetHeight()) y = EditorUI::CurrentWindow->GetWindow()->GetHeight() - height;
		}

		if (NativeInput::GetMouseButtonDown(Mouse::ButtonLeft))
		{
			if ((EditorUI::CurrentWindow->GetWindow()->GetHeight() - NativeInput::GetMousePosition().y) > y + height - m_HeaderHeight)
			{
				m_Dragged = true;
			}
		}
		if (NativeInput::GetMouseButtonUp(Mouse::ButtonLeft))
		{
			m_Dragged = false;
		}
	}

	void DragableOverlay::OnDispose()
	{
		EditorUI::ConsumeInput();
	}

	void DragableOverlay::CenterOverlay()
	{
		x = EditorUI::CurrentWindow->GetWindow()->GetWidth() / 2 - width / 2;
		y = EditorUI::CurrentWindow->GetWindow()->GetHeight() / 2 - height / 2;
	}

}