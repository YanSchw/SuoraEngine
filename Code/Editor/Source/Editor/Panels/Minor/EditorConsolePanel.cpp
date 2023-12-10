#include "EditorConsolePanel.h"
#include "Suora/Debug/VirtualConsole.h"
#include "Editor/Util/Icon.h"
#include "Editor/Util/EditorPreferences.h"

namespace Suora
{

	EditorConsolePanel::EditorConsolePanel(MajorTab* majorTab)
		: MinorTab(majorTab)
	{
		Name = "Console";
	}

	void EditorConsolePanel::Render(float deltaTime)
	{
		EditorUI::DrawRect(0, 0, GetWidth(), GetHeight(), 0, EditorPreferences::Get()->UiBackgroundColor);

		EditorUI::ButtonParams Params;
		Params.TextOrientation = Vec2(-1.0f, 0.0f);
		Params.TextOffsetLeft = 25.0f;
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
				if (It.m_Level == LogLevel::Trace)
				{
					Params.ButtonColorHover = Color(0.46f, 0.46f, 0.46f, 0.2f);
					Params.TextColor = Color(0.63f, 0.63f, 0.63f, 1);
				}
				else if (It.m_Level == LogLevel::Debug)
				{
					Params.ButtonColorHover = Color(0.26862f, 0.45f, 0.62647f, 0.2f);
					Params.TextColor = Color(0.33725f, 0.60294f, 0.764117f, 1);
				}
				else if (It.m_Level == LogLevel::Info)
				{
					Params.ButtonColorHover = Color(0.16862f, 0.2f, 0.17647f, 0.2f);
					Params.TextColor = Color(0.33725f, 0.55294f, 0.294117f, 1);
				}
				else if (It.m_Level == LogLevel::Warn)
				{
					Params.ButtonColorHover = Color(0.46764f, 0.32196f, 0.23705f, 0.2f);
					Params.TextColor = Color(0.80784f, 0.65098f, 0.27058f, 1);
				}
				else if (It.m_Level == LogLevel::Error)
				{
					Params.ButtonColorHover = Color(0.41764f, 0.20196f, 0.14705f, 0.2f); // Params.ButtonColorHover = Color(0.51764f, 0.30196f, 0.24705f, 1);
					Params.TextColor = Color(0.6745098f, 0.2078431f, 0.2745098f, 1);
				}
				else if (It.m_Level == LogLevel::Critical)
				{
					Params.ButtonColorHover = Color(0.16862f, 0.2f, 0.37647f, 0.2f);
					Params.TextColor = Color(0.33725f, 0.25294f, 0.594117f, 1);
				}

				EditorUI::Button(It.m_Message, 20.0f, y, GetWidth() - 50.0f, LineHeight-1.0f, Params);
				EditorUI::DrawTexturedRect(EditorConsolePanel::GetLogLevelIcon(It.m_Level), 8.0f, y + 2.0f, LineHeight - 4.0f, LineHeight - 4.0f, 0.0f, Params.TextColor * Color(1, 1, 1, 0.5f));
			}

			y += LineHeight;
		}

		EditorUI::ButtonParams PanelButtonParam;
		PanelButtonParam.ButtonColor = EditorPreferences::Get()->UiForgroundColor;
		PanelButtonParam.ButtonOutlineColor = EditorPreferences::Get()->UiBackgroundColor;
		PanelButtonParam.ButtonColorHover = Math::Lerp(EditorPreferences::Get()->UiForgroundColor, Color(1.0f), 0.05f);
		PanelButtonParam.ButtonColorClicked = EditorPreferences::Get()->UiForgroundColor;
		PanelButtonParam.ButtonRoundness = 0;
		PanelButtonParam.TextDropShadow = false;
		PanelButtonParam.TextOrientation = Vec2(-0.1f, 0);
		PanelButtonParam.TextSize = 24.0f;

		EditorUI::Button("", 0, GetHeight() - 35.0f, GetWidth(), 35.0f, PanelButtonParam);

		EditorUI::TextField(&s_InputCommand, 5.0f, GetHeight() - 30.0f, GetWidth() - 250.0f, 25.0f, EditorUI::TextFieldButtonParams(), [](String cmd)
		{
			if (NativeInput::GetKeyDown(Key::Enter))
			{
				VirtualConsole::IssueCommand(cmd);
				s_InputCommand = "";
			}
		});
		if (s_InputCommand == "")
		{
			EditorUI::Text("Enter a Command here...", Font::Instance, 15.0f, GetHeight() - 30.0f, GetWidth() - 265.0f, 25.0f, 18.0f, Vec2(-1, 0), Color(1));
		}

		const float scrollUp = y - (GetHeight() - 35.0f) - m_ScrollY;
		EditorUI::ScrollbarVertical(GetWidth() - 10, 0, 10, GetHeight() - 35.0f, 0, 0, GetWidth(), GetHeight() - 35.0f, scrollUp < 0 ? 0 : Math::Abs(scrollUp), 0, &m_ScrollY);
	}
	Texture* EditorConsolePanel::GetLogLevelIcon(const LogLevel level)
	{
		switch (level)
		{
		case LogLevel::Trace: return Icon::TextBubble;
		case LogLevel::Debug: return Icon::Bug;
		case LogLevel::Info: return Icon::Info;
		case LogLevel::Warn: return Icon::Warning;
		case LogLevel::Error: return Icon::Error;
		case LogLevel::Critical: return Icon::Critical;
		default: break;
		}
		return nullptr;
	}

}