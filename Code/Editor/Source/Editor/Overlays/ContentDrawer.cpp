#include "ContentDrawer.h"
#include "../Panels/Minor/ContentBrowser.h"
#include "../Panels/Minor/EditorConsolePanel.h"
#include "../EditorUI.h"

namespace Suora
{

	ContentDrawer::ContentDrawer(MajorTab* majorTab)
	{
		m_ContentBrowser = CreateRef<ContentBrowser>(majorTab);
	}

	void ContentDrawer::Render(float deltaTime)
	{
		y += deltaTime * 3000.0f;
		if (y >= 0.0f) y = 0.0f;

		EditorUI::DrawRect(x, y, width, height, -4, Color(1.0f));

		EditorUI::PushInput(NativeInput::GetMousePosition().x, EditorUI::CurrentWindow->GetWindow()->GetHeight() - NativeInput::GetMousePosition().y, x, y);
		m_ContentBrowser->Update(deltaTime, width, height);
		m_ContentBrowser->m_Framebuffer->Bind();
		m_ContentBrowser->Render(deltaTime);
		m_ContentBrowser->m_Framebuffer->Unbind();
		m_ContentBrowser->m_Framebuffer->DrawToScreen(x, y, width, height);

		if (WasMousePressedOutsideOfOverlay())
		{
			Dispose();
			return;
		}
	}

	ConsoleOverlay::ConsoleOverlay(MajorTab* majorTab)
	{
		m_EditorConsolePanel = CreateRef<EditorConsolePanel>(majorTab);
	}

	void ConsoleOverlay::Render(float deltaTime)
	{
		y += deltaTime * 3000.0f;
		if (y >= 0.0f) y = 0.0f;

		EditorUI::DrawRect(x, y, width, height, -4, Color(1.0f));

		EditorUI::PushInput(NativeInput::GetMousePosition().x, EditorUI::CurrentWindow->GetWindow()->GetHeight() - NativeInput::GetMousePosition().y, x, y);
		m_EditorConsolePanel->Update(deltaTime, width, height);
		m_EditorConsolePanel->m_Framebuffer->Bind();
		m_EditorConsolePanel->Render(deltaTime);
		m_EditorConsolePanel->m_Framebuffer->Unbind();
		m_EditorConsolePanel->m_Framebuffer->DrawToScreen(x, y, width, height);

		if (WasMousePressedOutsideOfOverlay())
		{
			Dispose();
			return;
		}
	}

}
