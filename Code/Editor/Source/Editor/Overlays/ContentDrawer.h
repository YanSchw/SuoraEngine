#pragma once
#include "../EditorUI.h"
#include "../Panels/MajorTab.h"
#include "DragableOverlay.h"

namespace Suora
{
	class ContentBrowser;
	class EditorConsolePanel;

	struct ContentDrawer : public EditorUI::Overlay
	{
	public:
		ContentDrawer(MajorTab* majorTab);
		void Render(float deltaTime) override;

	private:
		Ref<ContentBrowser> m_ContentBrowser = nullptr;

	};
	struct ConsoleOverlay : public EditorUI::Overlay
	{
	public:
		ConsoleOverlay(MajorTab* majorTab);
		void Render(float deltaTime) override;

	private:
		Ref<EditorConsolePanel> m_EditorConsolePanel = nullptr;

	};

}