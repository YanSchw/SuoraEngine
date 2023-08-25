#include "Precompiled.h"
#include "MajorTab.h"
#include "MinorTab.h"
#include "Dockspace.h"
#include "../EditorWindow.h"
#include "../Util/EditorPreferences.h"
#include "Suora/Assets/SuoraProject.h"

namespace Suora
{
	
	void MajorTab::Init()
	{

	}
	void MajorTab::Update(float deltaTime)
	{
		if ((NativeInput::GetKey(Key::LeftControl) || NativeInput::GetKey(Key::RightControl)) && NativeInput::GetKeyDown(Key::S))
		{
			SaveAsset();
		}

		m_DockspacePanel.m_PanelX = x;
		m_DockspacePanel.m_PanelY = y;
		m_DockspacePanel.m_PanelWidth = width;
		m_DockspacePanel.m_PanelHeight = height;
	}

	void MajorTab::DrawToolbar(float& x, float y, float height)
	{
		EditorUI::ButtonParams params;
		params.ButtonRoundness = 0.0f;
		params.ButtonColorClicked = Color(0.0f);
		params.ButtonOutlineColor = EditorPreferences::Get()->UiColor;
		params.ButtonColor = EditorPreferences::Get()->UiForgroundColor;
		params.ButtonColorHover = EditorPreferences::Get()->UiForgroundColor;
		params.HoverCursor = Cursor::Default;

		EditorUI::Button("", x, y, 250.0f, height, params);

		if (EditorUI::Button("File", x + 15, y + height * 0.2f, 70, height * 0.6f))
		{
			EditorUI::CreateContextMenu({ EditorUI::ContextMenuElement{{}, [&]() { SaveAsset(); }, "Save Asset", nullptr}}, x + 15, y + height * 0.2f);
		}
		if (EditorUI::Button("Edit", x + 90, y + height * 0.2f, 55, height * 0.6f))
		{
			EditorUI::CreateContextMenu({ EditorUI::ContextMenuElement{{}, [&]() { GetEditorWindow()->OpenAsset(ProjectSettings::Get()); }, "Edit ProjectSettings", nullptr},
										  EditorUI::ContextMenuElement{{}, [&]() { GetEditorWindow()->OpenAsset(EditorPreferences::Get()); }, "Edit EditorPreferences", nullptr} }, x + 90, y + height * 0.2f);
		}
		if (EditorUI::Button("Window", x + 150, y + height * 0.2f, 80, height * 0.6f))
		{
			EditorUI::CreateContextMenu({ EditorUI::ContextMenuElement{{}, []() {}, "", nullptr} }, x + 150, y + height * 0.2f);
		}

		x += 250.0f;
		EditorUI::Button("", x - 1, y, height, height, params);
		if (m_Asset)
		{
			EditorUI::ButtonParams SaveButtonParams;
			SaveButtonParams.ButtonRoundness = 0.0f;
			SaveButtonParams.ButtonColorClicked = Color(0.0f);
			SaveButtonParams.ButtonColorHover = EditorPreferences::Get()->UiForgroundColor * 1.2f;
			SaveButtonParams.CenteredIcon = AssetManager::GetAsset<Texture2D>(SuoraID("617d1c03-b141-4d71-9d73-993188ff011f"));
			SaveButtonParams.TooltipText = "Save Asset (" + m_Asset->GetAssetName() + ")";
			if (EditorUI::Button("", x + 4, y + 1 + 4, height - 2 - 8, height - 2 - 8, SaveButtonParams))
			{
				SaveAsset();
			}
		}

		x += height;
		float leftOverWidth = GetEditorWindow()->GetWindow()->GetWidth() - x;
		EditorUI::Button("", x - 2, y, leftOverWidth, height, params);
	}

	Texture* MajorTab::GetIconTexture()
	{
		return AssetManager::GetAsset<Texture2D>(SuoraID("bsv3no9876"))->GetTexture();
	}
	void MajorTab::SaveAsset()
	{
		SUORA_LOG(LogCategory::AssetManagement, LogLevel::Info, "Saving Asset: {0}", m_Asset->GetAssetName());
	}

}