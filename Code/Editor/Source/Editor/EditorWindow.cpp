#include "EditorWindow.h"
#include "Suora/Core/Application.h"
#include "Suora/Assets/SuoraProject.h"
#include "Suora/Assets/ShaderGraph.h"
#include "Suora/Renderer/RenderCommand.h"
#include "Suora/Renderer/RenderPipeline.h"
#include "Suora/Renderer/Renderer3D.h"
#include "Panels/MajorTab.h"
#include "Panels/MinorTab.h"
#include "Panels/DockspacePanel.h"
#include "Panels/Minor/EditorConsolePanel.h"
#include "Panels/Major/NodeClassEditor.h"
#include "Panels/Major/ShaderGraphEditorPanel.h"
#include "Suora/Debug/VirtualConsole.h"
#include "Util/EditorPreferences.h"
#include "EditorUI.h"
#include "Launcher.h"
#include <Suora.h>
#include <optional>

namespace Suora
{
	static Ref<Texture> TexIcon;
	static Ref<Texture> TexDockingCenter;
	static bool TextureInit = false;

	static bool IsLauncher()
	{
		return AssetManager::GetProjectAssetPath() == "";
	}

	EditorWindow::EditorWindow()
	{
		s_Current = this;
		WindowProps props;
		props.Title = "Suora Editor " + std::to_string(SUORA_VERSION_MAJOR) + "." + std::to_string(SUORA_VERSION_MINOR) + "." + std::to_string(SUORA_VERSION_PATCH);
		props.isDecorated = true;
		props.hasTitlebar = false;
		m_Window = Application::Get().CreateAppWindow(props);
		m_Window->Maximize();
		m_Window->Maximize();
		m_Window->SetVSync(false);
		m_Window->m_WindowIconOverride = AssetManager::GetAsset<Texture2D>(SuoraID("059f71bb-7ad3-47a0-be84-6dfc910f7ca3"));
		m_Window->m_OnDesktopFilesDropped.Register([this](Array<std::string> paths) { this->HandleAssetFileDrop(paths); });

		if (!TextureInit)
		{
			TextureInit = true;
			TexIcon = Texture::Create(AssetManager::GetAssetRootPath() + "/EngineContent/Suora.png");

			TexDockingCenter = Texture::Create(AssetManager::GetAssetRootPath() + "/EditorContent/Textures/Docking/Center.png");
		}

		m_Launcher = CreateRef<Launcher>(this);

		m_PrivateMajorTab = Ref<MajorTab>(new MajorTab());
		m_PrivateMajorTab->m_EditorWindow = this;
		m_HeroTools.Add(Ref<MinorTab>(new ContentBrowser(m_PrivateMajorTab.get())));
		m_HeroTools.Add(Ref<MinorTab>(new EditorConsolePanel(m_PrivateMajorTab.get())));
	}

	EditorWindow::~EditorWindow()
	{
		delete m_Window;
	}

	void EditorWindow::Update(float deltaTime)
	{
		GraphicsContext* context = (GraphicsContext*)(m_Window->GetGraphicsContext());
		context->MakeCurrent();

		EditorUI::SetCurrentWindow(this);

		if (!IsLauncher() && !m_WasEditorStartupAssetOpened)
		{
			m_WasEditorStartupAssetOpened = true;
			if (ProjectSettings::Get()->m_EditorStartupAsset)
			{
				OpenAsset(ProjectSettings::Get()->m_EditorStartupAsset);
			}
		}

		Render(deltaTime);

		m_Window->SetVSync(false);
		m_Window->OnUpdate();

		if (NativeInput::GetKey(Key::KPAdd)) EditorPreferences::Get()->UiScale += deltaTime;
		if (NativeInput::GetKey(Key::KPSubtract)) EditorPreferences::Get()->UiScale -= deltaTime;
		if (NativeInput::GetKey(Key::LeftControl)) EditorPreferences::Get()->UiScale += NativeInput::GetMouseScrollDelta() * 0.1f;
		EditorPreferences::Get()->UiScale = Math::Clamp(EditorPreferences::Get()->UiScale, 0.5f, 3.0f);
		if (NativeInput::GetKey(Key::LeftControl) && NativeInput::GetKeyDown(Key::Tab)) SelectedMajorTab++; if (SelectedMajorTab >= Tabs.Size()) SelectedMajorTab = 0;

		if      (NativeInput::GetKey(Key::LeftControl) && NativeInput::GetKeyDown(Key::Y)) Tabs[SelectedMajorTab]->Undo();
		else if (NativeInput::GetKey(Key::LeftControl) && NativeInput::GetKeyDown(Key::Z)) Tabs[SelectedMajorTab]->Redo();

	}

	void EditorWindow::Render(float deltaTime)
	{
		float const ui = EditorPreferences::Get()->UiScale;
		RenderCommand::SetClearColor(EditorPreferences::Get()->UiBackgroundColor);
		RenderCommand::Clear();
		RenderCommand::SetAlphaBlending(true);
		EditorUI::DrawRect(0, 0, m_Window->GetWidth(), 0, 0, Color(1, 0, 0, 1));

		if (!IsLauncher())
		{
			if (SelectedMajorTab < 0)
			{
				EditorUI::Text("Use Ctrl + Space to open the ContentDrawer", Font::Instance, 0.0f, 0.0f, m_Window->GetWidth(), m_Window->GetHeight(), 28.0f, Vec2(0.0f), Color(1.0f));
				RenderBottomBarAndHeroTool(deltaTime);
				EditorUI::PushInput(NativeInput::GetMousePosition().x, m_Window->GetHeight() - NativeInput::GetMousePosition().y, 0, 0);
				EditorUI::RenderOverlays(deltaTime);
				return;
			}

			RenderSelectedMajorTab(deltaTime);
		}

		/// Draw the actual Launcher, if needed...
		if (IsLauncher())
		{
			EditorUI::PushInput(NativeInput::GetMouseX(), GetWindow()->GetHeight() - NativeInput::GetMouseY(), 0.0f, 0.0f);
			m_Launcher->Render(deltaTime);
		}
		else
		{
			m_Launcher = nullptr;
		}

#if defined(SUORA_DEBUG)
		EditorUI::Text("Debug", Font::Instance, 85*ui, m_Window->GetHeight() - (27 * ui), 75*ui, 20*ui, 22.0f, Vec2(0.0f), Color(0.65f, 0.75f, 0.75f, 1.0f));
#endif
		// Logo
		EditorUI::DrawRect(0, m_Window->GetHeight() - (36 * ui), m_Window->GetWidth(), 36 * ui, 0, EditorPreferences::Get()->UiTitlebarColor);
		EditorUI::DrawTexturedRect(TexIcon, 4 * ui, m_Window->GetHeight() - (27 * ui), 73 * ui, 20 * ui, 0, Color(0.8f));
		m_Window->RegisterOverTitlebar(NativeInput::GetMousePosition().y < (35 * ui) && NativeInput::GetMousePosition().x < m_Window->GetWidth() - (36 * ui) * 3);
		// Window Handling... (Iconify, Minimize, Maximize)
		{
			EditorUI::PushInput(NativeInput::GetMousePosition().x, m_Window->GetHeight() - NativeInput::GetMousePosition().y, 0, 0);
			EditorUI::ButtonParams Params;
			Params.ButtonRoundness = 0.0f;
			Params.ButtonColor = EditorPreferences::Get()->UiTitlebarColor;
			Params.ButtonOutlineColor = EditorPreferences::Get()->UiTitlebarColor;
			Params.ButtonColorClicked = EditorPreferences::Get()->UiTitlebarColor;
			Params.ButtonColorHover = Color(0.95f, 0.15f, 0.15f, 1.0f);
			Params.CenteredIcon = AssetManager::GetAsset<Texture2D>(SuoraID("c86ec48b-0c9a-489a-a082-ef0250a246ca"));
			if (EditorUI::Button("", m_Window->GetWidth() - (36 * ui) * 1, m_Window->GetHeight() - (36 * ui), 36 * ui, 36 * ui, Params))
			{
				std::exit(0);
			}
			Params.ButtonColorHover = Color(0.35f, 0.35f, 0.35f, 1.0f);
			Params.CenteredIcon = AssetManager::GetAsset<Texture2D>(SuoraID(m_Window->IsMaximized() ? "37489392-8840-44f8-b5d7-278452845669" : "425b4026-5db2-4398-ac6a-593524379285"));
			if (EditorUI::Button("", m_Window->GetWidth() - (36 * ui) * 2, m_Window->GetHeight() - (36 * ui), 36 * ui, 36 * ui, Params))
			{
				m_Window->Maximize();
			}
			Params.ButtonColorHover = Color(0.35f, 0.35f, 0.35f, 1.0f);
			Params.CenteredIcon = AssetManager::GetAsset<Texture2D>(SuoraID("47d123a4-eae7-4499-9a69-d6b8e99c05c4"));
			if (EditorUI::Button("", m_Window->GetWidth() - (36 * ui) * 3, m_Window->GetHeight() - (36 * ui), 36 * ui, 36 * ui, Params))
			{
				m_Window->Iconify();
			}

			if (!IsLauncher())
			{
				EditorUI::Button(ProjectSettings::GetProjectName(), m_Window->GetWidth() - (36 * ui) * 9, m_Window->GetHeight() - (17 * ui), 5 * 36 * ui, 18 * ui);
			}
		}

		if (!IsLauncher())
		{
			RenderBottomBarAndHeroTool(deltaTime);
		}

		// Toolbar
		if (!IsLauncher() && SelectedMajorTab >= 0)
		{
			EditorUI::DrawRect(0, m_Window->GetHeight() - (73 * ui), m_Window->GetWidth(), 40 * ui, 0, EditorPreferences::Get()->UiBackgroundColor);
			EditorUI::PushInput(NativeInput::GetMousePosition().x, m_Window->GetHeight() - NativeInput::GetMousePosition().y, 0, 0);
			float toolbarX = 0.0f;
			Tabs[SelectedMajorTab]->DrawToolbar(toolbarX, m_Window->GetHeight() - (73.0f * ui), 40.0f * ui);
		}

		EditorUI::DrawRectOutline(0, 0, m_Window->GetWidth(), m_Window->GetHeight(), 1, Color(0, 0, 0, 1));

		// Render Overlays
		EditorUI::PushInput(NativeInput::GetMousePosition().x, m_Window->GetHeight() - NativeInput::GetMousePosition().y, 0, 0);
		EditorUI::RenderOverlays(deltaTime);

		// Tooltip
		RenderTooltip(deltaTime);

	}

	void EditorWindow::DelegateChanges(MajorTab* majorTab)
	{
		for (Ref<MajorTab> It : Tabs)
		{
			if (It.get() != majorTab)
			{
				It->ApplyChangesOfOtherMajorTabs(majorTab);
			}
		}
	}

	void EditorWindow::OpenAsset(Asset* asset)
	{
		if (asset == nullptr || asset->IsFlagSet(AssetFlags::Missing))
		{
			return;
		}
		for (int32_t i = 0; i < Tabs.Size(); i++)
		{
			if (Tabs[i]->m_Asset == asset)
			{
				SelectedMajorTab = i;
				return;
			}
		}

		Array<Class> cls = Class::GetSubclassesOf(MajorTab::StaticClass());

		for (Class It : cls)
		{
			if (asset->IsA<ShaderGraph>()) It = ShaderGraphEditorPanel::StaticClass();

			Ref<MajorTab> major = Ref<MajorTab>(New(It)->As<MajorTab>());
			if (asset->GetClass().Inherits(major->m_AssetClass))
			{
				Tabs.Add(Ref<MajorTab>(major));
				SelectedMajorTab = Tabs.Last();
				major->m_EditorWindow = this;
				major->m_Asset = asset;
				major->Init();
				return;
			}
		}
	}
	void EditorWindow::CloseAsset(Asset* asset)
	{
		for (int32_t i = 0; i < Tabs.Size(); i++)
		{
			if (Tabs[i]->m_Asset == asset)
			{
				if (SelectedMajorTab == i)
				{
					SelectedMajorTab = -1;
				}
				Tabs.RemoveAt(i);
				if (Tabs.Size() >= 1)
				{
					SelectedMajorTab++;
				}
				return;
			}
		}
	}

	void EditorWindow::StopPlayInEditor()
	{
		if (Tabs[SelectedMajorTab]->IsA<NodeClassEditor>())
		{
			Tabs[SelectedMajorTab]->As<NodeClassEditor>()->StopPlayInEditor();
		}
	}

	void EditorWindow::RenderSelectedMajorTab(float deltaTime)
	{
		float const ui = EditorPreferences::Get()->UiScale;
		Tabs[SelectedMajorTab]->m_DockspacePanel.Update(deltaTime);

		// MinorTabs
		Tabs[SelectedMajorTab]->m_EditorWindow = this;
		Tabs[SelectedMajorTab]->x = 0;
		Tabs[SelectedMajorTab]->y = (35.0f + m_HeroToolHeight) * ui;
		Tabs[SelectedMajorTab]->width = GetWindow()->GetWidth();
		Tabs[SelectedMajorTab]->height = GetWindow()->GetHeight() - (73.0f * ui) - (35.0f + m_HeroToolHeight) * ui;
		Tabs[SelectedMajorTab]->Update(deltaTime);
		for (int i = 0; i < Tabs[SelectedMajorTab]->m_DockspacePanel.m_DockingSpaces.Size(); i++)
		{
			DockspacePanel& tab = Tabs[SelectedMajorTab]->m_DockspacePanel;
			DockingSpace& space = *Tabs[SelectedMajorTab]->m_DockspacePanel.m_DockingSpaces[i];
			EditorUI::DrawRect(tab.m_PanelX + tab.m_PanelWidth * space.x + 2, tab.m_PanelY + tab.m_PanelHeight * space.y + 2, tab.m_PanelWidth * space.GetWidth() - 4, tab.m_PanelHeight * space.GetHeight() - 4, 0, Math::Lerp(EditorPreferences::Get()->UiBackgroundColor, EditorPreferences::Get()->UiColor, 0.45f));
			uint32_t x = tab.m_PanelX + tab.m_PanelWidth * space.x + 2;
			for (int t = 0; t < space.m_MinorTabs.Size(); t++)
			{
				EditorUI::DrawRect(x, tab.m_PanelY + tab.m_PanelHeight * space.y + tab.m_PanelHeight * space.GetHeight() - 2 - (22 * ui), 150 * ui, 22 * ui, -4, (t == space.m_SelectedMinorTab) ? EditorPreferences::Get()->UiColor * 1.2f : EditorPreferences::Get()->UiColor);
				EditorUI::Text(space.m_MinorTabs[t]->Name, Font::Instance, x + (25 * ui) + (6 * ui), tab.m_PanelY + tab.m_PanelHeight * space.y + tab.m_PanelHeight * space.GetHeight() - 2 - (22 * ui), 140 * ui, 22 * ui, 24 * ui, Vec2(-1, 0), Color(.75f, .75f, .75f, 1));
				if (space.m_MinorTabs[t]->m_IconTexture) EditorUI::DrawTexturedRect(space.m_MinorTabs[t]->m_IconTexture->GetTexture(), x, tab.m_PanelY + tab.m_PanelHeight * space.y + tab.m_PanelHeight * space.GetHeight() - 2 - (22 * ui), 24.0f * ui, 24.0f * ui, 0.0f, Color(0.5f));
				x += 165 * ui;
			}
			EditorUI::PushInput(NativeInput::GetMousePosition().x - (tab.m_PanelX + tab.m_PanelWidth * space.x + 2), m_Window->GetHeight() - NativeInput::GetMousePosition().y - (tab.m_PanelY + tab.m_PanelHeight * space.y + 2), tab.m_PanelX + tab.m_PanelWidth * space.x + 2, tab.m_PanelY + tab.m_PanelHeight * space.y + 2);
			space.m_MinorTabs[space.m_SelectedMinorTab]->Update(deltaTime, tab.m_PanelWidth * space.GetWidth() - 4, tab.m_PanelHeight * space.GetHeight() - 4 - 22 * ui);
			space.m_MinorTabs[space.m_SelectedMinorTab]->m_Framebuffer->Bind();
			space.m_MinorTabs[space.m_SelectedMinorTab]->Render(deltaTime);
			space.m_MinorTabs[space.m_SelectedMinorTab]->m_Framebuffer->Unbind();
			space.m_MinorTabs[space.m_SelectedMinorTab]->m_Framebuffer->DrawToScreen(tab.m_PanelX + tab.m_PanelWidth * space.x + 2, tab.m_PanelY + tab.m_PanelHeight * space.y + 2, tab.m_PanelWidth * space.GetWidth() - 4, tab.m_PanelHeight * space.GetHeight() - 4 - 22 * ui);

			if (m_InputEvent == EditorInputEvent::Dockspace_Dragging)
			{
				EditorUI::DrawTexturedRect(TexDockingCenter, (tab.m_PanelX + tab.m_PanelWidth * space.x + 2) + (tab.m_PanelWidth * space.GetWidth() - 4) * .5f - 25 * ui, (tab.m_PanelY + tab.m_PanelHeight * space.y + 2) + (tab.m_PanelHeight * space.GetHeight() - 4 - 22 * ui) * .5f - 25 * ui, 50 * ui, 50 * ui, 0, Color(1));
			}
		}
	}

	void EditorWindow::RenderTooltip(float deltaTime)
	{
		EditorUI::tooltipFrames += deltaTime;
		if (EditorUI::tooltipFrames <= 0.2f) EditorUI::tooltipAlpha = Math::Lerp<float>(EditorUI::tooltipAlpha, 1, 25.f * deltaTime);
		else  EditorUI::tooltipAlpha = Math::Lerp<float>(EditorUI::tooltipAlpha, 0, 35.f * deltaTime);

		std::vector<std::string> lines = Util::SplitString(EditorUI::tooltipText, '\n');

		float x = NativeInput::GetMousePosition().x + 15 + (1 - EditorUI::tooltipAlpha) * 30.f;
		float height = 25.0f + 25.0f * lines.size();
		float y = m_Window->GetHeight() - NativeInput::GetMousePosition().y - (height * 0.5f) - (1 - EditorUI::tooltipAlpha) * 10;
		float width = 0.f;

		for (const std::string& line : lines)
		{
			const float currentWidth = Font::Instance->GetStringWidth(line, 22.0f) * 0.55f;
			if (currentWidth > width) width = currentWidth;
		}

		{
			float overShot = x + width - m_Window->GetWidth();
			if (overShot > 0.0f) x -= overShot;
		}
		{
			if (y < 0.0f) y -= y;
		}

		Color toolTipBackground = EditorPreferences::Get()->UiColor; toolTipBackground.a = EditorUI::tooltipAlpha;
		Color toolTipOutline = EditorPreferences::Get()->UiForgroundColor; toolTipOutline.a = EditorUI::tooltipAlpha;
		EditorUI::DrawRect(x, y, width, height, 0, toolTipBackground);
		EditorUI::DrawRectOutline(x, y, width, height, 2, toolTipOutline);
		for (int i = 0; i < lines.size(); i++)
		{
			EditorUI::Text(lines[lines.size() - 1 - i], Font::Instance, x + 5.0f, y + 25.0f * i, width, 50.0f, 22, Vec2(-1, 0), Color(1, 1, 1, EditorUI::tooltipAlpha));
		}
	}

	void EditorWindow::RenderBottomBarAndHeroTool(float deltaTime)
	{
		float const ui = EditorPreferences::Get()->UiScale;
		EditorUI::PushInput(NativeInput::GetMousePosition().x, m_Window->GetHeight() - NativeInput::GetMousePosition().y, 0, 0);
		// Bottombar
		{
			EditorUI::ButtonParams SideBarParams;
			SideBarParams.ButtonRoundness = 0.0f;
			SideBarParams.ButtonColorClicked = Color(0.0f);
			SideBarParams.ButtonOutlineColor = EditorPreferences::Get()->UiForgroundColor;
			SideBarParams.ButtonColor = EditorPreferences::Get()->UiColor;
			SideBarParams.ButtonColorHover = EditorPreferences::Get()->UiForgroundColor;


			int x = 250.0f * ui + 2;
			//m_SidebarWidth = Math::Lerp(m_SidebarWidth, !disableSidebarInput && EditorUI::GetInput().x <= m_SidebarWidth * ui ? 35.0f + 150.0f : 35.0f, 8.0f * deltaTime);
			m_HeroToolHeight = Math::Lerp(m_HeroToolHeight, m_HeroToolOpened ? 350.0f : 0.0f, 8.0f * deltaTime);
			for (int i = 0; i < Tabs.Size(); i++)
			{
				EditorUI::ButtonParams Params = EditorUI::ButtonParams::Invisible();
				bool Hovering = false;
				Params.OutHover = &Hovering;
				if (EditorUI::Button("", x, 0, Tabs[i]->m_BottomBarWidth * ui, 35.0f * ui, Params))
				{
					SelectedMajorTab = i;
				}
				Tabs[i]->m_BottomBarWidth = Math::Lerp(Tabs[i]->m_BottomBarWidth, Hovering ? 135.0f : 35.0f, 8.0f * deltaTime);
				
				EditorUI::Button("", x, 0, Tabs[i]->m_BottomBarWidth * ui - 1, 35.0f * ui, SideBarParams);
				EditorUI::DrawTexturedRect(Tabs[i]->GetIconTexture(), x + 5.0f * ui, 5.0f * ui, 25.0f * ui, 25.0f * ui, 0.0f, Color(1.0f));
				EditorUI::Text(Tabs[i]->m_Name, Font::Instance, x + 35.0f * ui + 3.0f, 0, (Tabs[i]->m_BottomBarWidth - 35.0f) * ui, 35.0f * ui, 24.0f, Vec2(-1, 0), Color(1));
				if (i == SelectedMajorTab)
				{
					EditorUI::DrawRect(x, 0, Tabs[i]->m_BottomBarWidth * ui - 1, 3.0f, 0, EditorPreferences::Get()->UiHighlightColor);
				}
				x += Tabs[i]->m_BottomBarWidth * ui;
			}

			//y -= 4.0f * ui;
			if (EditorUI::Button("", 0.0f, 0.0f, 150.0f * ui, 35.0f * ui, SideBarParams) || (NativeInput::GetKey(Key::LeftControl) && NativeInput::GetKeyDown(Key::Space)))
			{
				if (!(m_HeroToolOpened && m_SelectedHeroTool != 0))
				{
					m_HeroToolOpened = !m_HeroToolOpened;
				}
				m_SelectedHeroTool = 0;
			}
			Texture* openIcon = AssetManager::GetAsset<Texture2D>(SuoraID("98548865-889f-4705-abe7-37a83fdd652d"))->GetTexture();
			Texture* closeIcon = AssetManager::GetAsset<Texture2D>(SuoraID("8742cec8-9ee5-4645-b036-577146904b41"))->GetTexture();
			EditorUI::DrawTexturedRect(m_HeroToolOpened ? closeIcon : openIcon, 5.0f * ui, 5.0f * ui, 25.0f * ui, 25.0f * ui, 0.0f, Color(1.0f));
			EditorUI::Text("ContentDrawer", Font::Instance, 35.0f * ui + 3.0f, 0.0f, (150.0f - 35.0f) * ui, 35.0f * ui - 1, 24.0f, Vec2(-1, 0), Color(1));

			// Console
			{
				if (EditorUI::Button("", 150.0f * ui + 1, 0.0f, 100.0f * ui, 35.0f * ui, SideBarParams) || (NativeInput::GetKey(Key::LeftControl) && NativeInput::GetKeyDown(Key::Period)))
				{
					if (!(m_HeroToolOpened && m_SelectedHeroTool != 1))
					{
						m_HeroToolOpened = !m_HeroToolOpened;
					}
					m_SelectedHeroTool = 1;
				}
				m_ConsoleLogs = VirtualConsole::GetLogMessages().size();
				m_ConsoleWarnings = VirtualConsole::GetWarnMessages().size();
				m_ConsoleErrors = VirtualConsole::GetErrorMessages().size();
				const std::string textErrors = m_ConsoleErrors >= 10 ? std::to_string(m_ConsoleErrors) : "0" + std::to_string(m_ConsoleErrors);
				const Color colorErrors = m_ConsoleErrors > 0 ? Color(0.6745098f, 0.2078431f, 0.2745098f, 1) : EditorPreferences::Get()->UiBackgroundColor * 0.5f;
				EditorUI::Text(textErrors, Font::Instance, 150.0f * ui + 10.0f * ui, 3.0f, 25.0f * ui, 9.0f * ui, 18.0f, Vec2(), colorErrors);
				EditorUI::DrawTexturedRect(AssetManager::GetAsset<Texture2D>(SuoraID("7224f801-1370-4f82-ac37-0cecf2cc35b9"))->GetTexture(), 150.0f * ui + 13.5f * ui, 13.0f * ui, 18.0f * ui, 18.0f * ui, 0, colorErrors);

				const std::string textWarnings = m_ConsoleWarnings >= 10 ? std::to_string(m_ConsoleWarnings) : "0" + std::to_string(m_ConsoleWarnings);
				const Color colorWarnings = m_ConsoleWarnings > 0 ? Color(0.7764706f, 0.6941176f, 0.2431373f, 1) : EditorPreferences::Get()->UiBackgroundColor * 0.5f;
				EditorUI::Text(textWarnings, Font::Instance, 150.0f * ui + 40.0f * ui, 3.0f, 25.0f * ui, 9.0f * ui, 18.0f, Vec2(), colorWarnings);
				EditorUI::DrawTexturedRect(AssetManager::GetAsset<Texture2D>(SuoraID("cf027e56-ead0-4e61-ac31-b3d9125a03d9"))->GetTexture(), 150.0f * ui + 43.5f * ui, 13.0f * ui, 18.0f * ui, 18.0f * ui, 0, colorWarnings);

				const std::string textLogs = m_ConsoleLogs >= 10 ? std::to_string(m_ConsoleLogs) : "0" + std::to_string(m_ConsoleLogs);
				const Color colorLogs = m_ConsoleLogs > 0 ? Color(1) : EditorPreferences::Get()->UiBackgroundColor * 0.5f;
				EditorUI::Text(textLogs, Font::Instance, 150.0f * ui + 70.0f * ui, 3.0f, 25.0f * ui, 9.0f * ui, 18.0f, Vec2(), colorLogs);
				EditorUI::DrawTexturedRect(AssetManager::GetAsset<Texture2D>(SuoraID("b3758660-1e0a-4a6a-9223-1a25966bdefd"))->GetTexture(), 150.0f * ui + 73.5f * ui, 13.0f * ui, 18.0f * ui, 18.0f * ui, 0, colorLogs);
			}

			SideBarParams.ButtonColorHover = SideBarParams.ButtonColor;
			SideBarParams.HoverCursor = Cursor::Default;
			EditorUI::Button("", x, 0.0f, m_Window->GetWidth() - x, 35.0f * ui, SideBarParams);
		}

		// Draw HeroTool
		if (m_HeroToolHeight >= 1.0f)
		{
			EditorUI::PushInput(NativeInput::GetMousePosition().x, m_Window->GetHeight() - NativeInput::GetMousePosition().y - 35.0f * ui, 0.0f, 35.0f * ui);
			if (m_SelectedHeroTool >= 0 && m_SelectedHeroTool <= m_HeroTools.Last())
			{
				m_HeroTools[m_SelectedHeroTool]->Update(deltaTime, m_Window->GetWidth(), m_HeroToolHeight * ui);
				m_HeroTools[m_SelectedHeroTool]->m_Framebuffer->Bind();
				m_HeroTools[m_SelectedHeroTool]->Render(deltaTime);
				m_HeroTools[m_SelectedHeroTool]->m_Framebuffer->Unbind();
				m_HeroTools[m_SelectedHeroTool]->m_Framebuffer->DrawToScreen(0.0f, 35.0f * ui, m_Window->GetWidth(), m_HeroToolHeight * ui);
			}
		}

		// Draw Stats
		EditorUI::Text(std::string("FPS: ").append(std::to_string(Engine::GetFramerate())), Font::Instance, m_Window->GetWidth() - 160, 0, 150, 35.0f * ui, 20.0f, Vec2(1, +1), Color(1));
		EditorUI::Text(std::to_string(deltaTime * 1000.0f) + "ms", Font::Instance, m_Window->GetWidth() - 160, 0, 150, 35.0f * ui, 20.0f, Vec2(1, -1), Color(1));
	}

	Window* EditorWindow::GetWindow()
	{
		return m_Window;
	}

	void EditorWindow::CopyAssetFilesToDirectory(const std::filesystem::path& directory, const std::filesystem::path& toBeCopied)
	{
		if (!std::filesystem::is_directory(toBeCopied))
		{
			Platform::CreateDirectory(directory);
			std::filesystem::copy(toBeCopied, std::filesystem::path(directory).append(toBeCopied.filename().string()));
		}
		else
		{
			for (auto It : std::filesystem::directory_iterator(toBeCopied))
			{
				CopyAssetFilesToDirectory(std::filesystem::path(directory).append(toBeCopied.filename().string()), It);
			}
		}
	}

	void EditorWindow::HandleAssetFileDrop(Array<std::string> paths)
	{
		m_HeroToolOpened = true;
		m_SelectedHeroTool = 0;

		ContentBrowser* contentDrawer = (ContentBrowser*)m_HeroTools[0].get();

		for (auto& It : paths)
		{
			CopyAssetFilesToDirectory(contentDrawer->GetCurrentDirectory(), It);
		}

		AssetManager::HotReload(contentDrawer->GetCurrentDirectory());

	}

	void EditorWindow::ForceOpenContentDrawer()
	{
		m_HeroToolOpened = true;
		m_SelectedHeroTool = 0;
	}

}