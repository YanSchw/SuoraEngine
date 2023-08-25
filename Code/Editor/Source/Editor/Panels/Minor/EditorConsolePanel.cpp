#include "EditorConsolePanel.h"
#include "../../Util/EditorPreferences.h"
#include "Suora/Debug/VirtualConsole.h"

namespace Suora
{

	EditorConsolePanel::EditorConsolePanel(MajorTab* majorTab)
		: MinorTab(majorTab)
	{
		Name = "Console";
	}

	EditorConsolePanel::~EditorConsolePanel()
	{

	}

	void EditorConsolePanel::Render(float deltaTime)
	{
		EditorUI::DrawRect(0, 0, GetWidth(), GetHeight(), 0, EditorPreferences::Get()->UiBackgroundColor);

		EditorUI::ButtonParams Params;
		Params.TextOrientation = Vec2(-0.95f, 0.0f);
		Params.ButtonRoundness = 0;
		Params.ButtonColor = EditorPreferences::Get()->UiBackgroundColor;
		Params.ButtonOutlineColor = Color(0);
		Params.TextSize = 24.0f * EditorPreferences::Get()->UiScale;

		const float LineHeight = 20.0f * EditorPreferences::Get()->UiScale;
		float y = 0.0f + m_ScrollY;// GetHeight() - LineHeight + m_ScrollY;

		for (int32_t i = VirtualConsole::m_Messages.size() - 1; i >= 0; i--)
		{
			auto& It = VirtualConsole::m_Messages[i];
			if (y < GetHeight() && y > -200.0f)
			{
				Params.ButtonColor = i % 2 == 0 ? EditorPreferences::Get()->UiBackgroundColor : Math::Lerp(EditorPreferences::Get()->UiBackgroundColor, EditorPreferences::Get()->UiForgroundColor, 0.3f);
				if (It.m_Level == LogLevel::Info)
				{
					Params.ButtonColorHover = Color(0.16862f, 0.2f, 0.17647f, 0.2f);
					Params.TextColor = Color(0.33725f, 0.55294f, 0.294117f, 1);
				}
				else if (It.m_Level == LogLevel::Warn)
				{
					Params.ButtonColorHover = Color(0.46764f, 0.32196f, 0.23705f, 0.2f);
					Params.TextColor = Color(0.80784f, 0.65098f, 0.27058f, 1);
				}
				else
				{
					Params.ButtonColorHover = Color(0.41764f, 0.20196f, 0.14705f, 0.2f); // Params.ButtonColorHover = Color(0.51764f, 0.30196f, 0.24705f, 1);
					Params.TextColor = Color(0.80784f, 0.43921f, 0.27058f, 1);
				}

				EditorUI::Button(It.m_Message, 20.0f, y, GetWidth() - 50.0f, LineHeight-1.0f, Params);
			}

			y += LineHeight;
		}


		float scrollUp = y - GetHeight() - m_ScrollY;
		EditorUI::ScrollbarVertical(GetWidth() - 10, 0, 10, GetHeight(), 0, 0, GetWidth(), GetHeight(), scrollUp < 0 ? 0 : Math::Abs(scrollUp), 0, &m_ScrollY);
	}

}