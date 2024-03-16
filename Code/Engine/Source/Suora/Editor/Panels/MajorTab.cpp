#include "Precompiled.h"
#include "MajorTab.h"
#include "MinorTab.h"
#include "Dockspace.h"
#include "Suora/Editor/EditorWindow.h"
#include "Suora/Editor/Util/EditorPreferences.h"
#include "Suora/Assets/SuoraProject.h"
#include "Suora/Editor/AssetPreview.h"
#include "Suora/Editor/Panels/Major/ExportProjectPanel.h"

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

		// Menu Bar
		Array<Class> implementations = Class::GetSubclassesOf(MajorMenuItem::StaticClass());
		Array<Ref<MajorMenuItem>> Items;
		for (const auto& It : implementations)
		{
			auto Impl = Ref<MajorMenuItem>(New(It)->As<MajorMenuItem>());
			if (Impl && Impl->Filter())
			{
				Items.Add(Impl);
			}
		}
		Items.Sort([](const Ref<MajorMenuItem>& a, const Ref<MajorMenuItem>& b)
		{
			return a->GetOrderIndex() < b->GetOrderIndex();
		});

		if (EditorUI::Button("File", x + 15, y + height * 0.2f, 70, height * 0.6f))
		{
			std::vector<EditorUI::ContextMenuElement> fileItems;
			for (const Ref<MajorMenuItem>& item : Items)
			{
				if (item->IsFileItem())
					fileItems.push_back(EditorUI::ContextMenuElement{ {}, item->m_Lambda, item->GetLabel(), nullptr });
			}
			fileItems.push_back(EditorUI::ContextMenuElement{ {}, [&]() { SaveAsset(); }, "Save Asset", nullptr });
			fileItems.push_back(EditorUI::ContextMenuElement{ {}, [&]() { OpenExportProjectTab(); }, "Export Project", nullptr });
			EditorUI::CreateContextMenu(fileItems, x + 15, y + height * 0.2f);
		}
		if (EditorUI::Button("Edit", x + 90, y + height * 0.2f, 55, height * 0.6f))
		{
			std::vector<EditorUI::ContextMenuElement> editItems;
			for (const Ref<MajorMenuItem>& item : Items)
			{
				if (item->IsEditItem())
					editItems.push_back(EditorUI::ContextMenuElement{ {}, item->m_Lambda, item->GetLabel(), nullptr });
			}
			editItems.push_back(EditorUI::ContextMenuElement{ {}, [&]() { GetEditorWindow()->OpenAsset(ProjectSettings::Get()); }, "Edit ProjectSettings", nullptr });
			editItems.push_back(EditorUI::ContextMenuElement{ {}, [&]() { GetEditorWindow()->OpenAsset(EditorPreferences::Get()); }, "Edit EditorPreferences", nullptr });
			EditorUI::CreateContextMenu(editItems, x + 90, y + height * 0.2f);
		}
		if (EditorUI::Button("Window", x + 150, y + height * 0.2f, 80, height * 0.6f))
		{
			std::vector<EditorUI::ContextMenuElement> windowItems;
			for (const Ref<MajorMenuItem>& item : Items)
			{
				if (item->IsWindowItem())
					windowItems.push_back(EditorUI::ContextMenuElement{ {}, item->m_Lambda, item->GetLabel(), nullptr });
			}
			EditorUI::CreateContextMenu(windowItems, x + 150, y + height * 0.2f);
		}

		x += 250.0f;
		EditorUI::Button("", x - 1, y, height, height, params);
		if (m_Asset)
		{
			EditorUI::ButtonParams SaveButtonParams;
			SaveButtonParams.ButtonRoundness = 4.0f;
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

		AssetPreview::ResetAssetPreview(m_Asset);
	}

	void MajorTab::OpenExportProjectTab()
	{
		Ref<ExportProjectPanel> major = CreateRef<ExportProjectPanel>();
		GetEditorWindow()->Tabs.Add(Ref<MajorTab>(major));
		GetEditorWindow()->SelectedMajorTab = GetEditorWindow()->Tabs.Last();
		major->m_EditorWindow = GetEditorWindow();
		major->Init();
	}

}