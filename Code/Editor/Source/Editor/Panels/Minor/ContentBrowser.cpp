#include "ContentBrowser.h"
#include "../../Util/EditorPreferences.h"
#include "../../Util/EditorCamera.h"
#include "../../Overlays/AssetActionsOverlays.h"

#include "../Major/MaterialEditorPanel.h"
#include "../Major/ShaderGraphEditorPanel.h"
#include "../Major/Texture2DEditorPanel.h"
#include "../Major/MeshEditorPanel.h"
#include "../Major/NodeClassEditor.h"
#include "../Major/SettingsTabs.h"
#include "Suora/Assets/ShaderGraph.h"
#include "Suora/Assets/Blueprint.h"

#define ITEM_CORNER 4.0f

namespace Suora
{

	struct AssetImporter : EditorUI::Overlay
	{
		Array<UnresolvedAsset> m_UnresolvedAsset;
		Array<int> m_AssetImported;
		int Index = 0;
		float m_ScrollY = 0.0f;
		Texture2D* TickTexture = nullptr;
		Ref<Texture> m_Texture; std::string texturePath = "";

		AssetImporter(const Array<UnresolvedAsset>& assets)
			: m_UnresolvedAsset(assets)
		{
			TickTexture = AssetManager::GetAssetByName<Texture2D>("CheckboxTick.texture");
			for (int i = 0; i < m_UnresolvedAsset.Size(); i++) m_AssetImported.Add(false);
		}
		void Render(float deltaTime) override
		{
			if (m_UnresolvedAsset.Size() == 0)
			{
				Dispose();
				return;
			}

			EditorUI::DrawRect(0, 0, EditorWindow::GetCurrent()->GetWindow()->GetWidth(), EditorWindow::GetCurrent()->GetWindow()->GetHeight(), 0.0f, Color(0, 0, 0, 0.2f));
			EditorUI::DrawRect(x - 2, y + 2, width, height, 4.0f, Color(1, 1, 1, 0.2f));
			EditorUI::DrawRect(x + 5, y - 5, width, height, 4.0f, Color(0, 0, 0, 0.5f));
			EditorUI::DrawRect(x, y, width, height, 4.0f, EditorPreferences::Get()->UiForgroundColor);

			if (EditorUI::Button("Finish", x + width - 175.0f, y + 10.0f, 150.0f, 25.0f))
			{
				Dispose();
			}

			EditorUI::DrawRect(x + width * 0.025f, y + height * 0.1f, width * 0.25f, height * 0.8f, 4.0f, EditorPreferences::Get()->UiBackgroundColor);
			float ItY = y + height * 0.9f + m_ScrollY;
			for (int i = 0; i < m_UnresolvedAsset.Size(); i++)
			{
				ItY -= 28.0f;
				if (ItY + m_ScrollY >= y + height * 0.9f || ItY + m_ScrollY <= y + height * 0.1f) continue;
				EditorUI::ButtonParams Params = EditorUI::ButtonParams::Invisible();
				Params.TextOrientation = Vec2(-0.95f, 0);
				Params.ButtonColorClicked = Params.ButtonColorHover = Params.ButtonColor = (Index == i) ? EditorPreferences::Get()->UiHighlightColor : Color(0);
				Params.TextColor = (Index == i) ? EditorPreferences::Get()->UiBackgroundColor : Color(1);
				if (EditorUI::Button(m_UnresolvedAsset[i].m_Path.stem().string(), x + width * 0.025f, ItY + m_ScrollY, width * 0.25f, 28.0f, Params))
				{
					Index = i;
				}
				EditorUI::Text(m_UnresolvedAsset[i].m_Class.GetNativeClassName(), Font::Instance, x + width * 0.025f, ItY + m_ScrollY, width * 0.25f, 28.0f, 24.0f, Vec2(0.75f, 0), Color(1));
				if (m_AssetImported[i]) EditorUI::DrawTexturedRect(TickTexture->GetTexture(), x + width * 0.275f - 28.0f, ItY + m_ScrollY, 28.0f, 28.0f, 0, Color(1));
			}
			float scrollDown = (y + height * 0.1f) - ItY + 100.0f;
			if (scrollDown <= 0.0f) scrollDown = 0.0f;
			EditorUI::ScrollbarVertical(x + width * 0.275f, y + height * 0.1f, 10.0f, height * 0.8f, x + width * 0.025f, y + height * 0.1f, width * 0.25f, height * 0.8f, 0.0f, scrollDown, &m_ScrollY);

			if (m_AssetImported[Index]) return;

			EditorUI::DrawRect(x + width * 0.29f - 1.0f, y + height * 0.2f - 1.0f, height * 0.6f + 2.0f, height * 0.6f + 2.0f, 4.0f, EditorPreferences::Get()->UiTextColor);
			EditorUI::DrawRect(x + width * 0.29f, y + height * 0.2f, height * 0.6f, height * 0.6f, 4.0f, EditorPreferences::Get()->UiBackgroundColor);

			if (m_UnresolvedAsset[Index].m_Class == Texture2D::StaticClass())
			{
				if (texturePath != m_UnresolvedAsset[Index].m_Path.string())
				{
					texturePath = m_UnresolvedAsset[Index].m_Path.string();
					m_Texture = Texture::Create(m_UnresolvedAsset[Index].m_Path.string());
				}
				EditorUI::DrawTexturedRect(m_Texture, x + width * 0.29f, y + height * 0.2f, height * 0.6f, height * 0.6f, 4.0f, Color(1));
				if (EditorUI::Button("Import", x + width - 175.0f - 175.0f, y + 10.0f, 150.0f, 25.0f, EditorUI::ButtonParams::Highlight()))
				{
					m_AssetImported[Index] = true;
					Texture2D* newAsset = AssetManager::CreateAsset<Texture2D>(m_UnresolvedAsset[Index].m_Path.stem().string(), m_UnresolvedAsset[Index].m_Path.parent_path().string());
					Yaml::Node serialized;
					newAsset->Serialize(serialized);
					std::string str;
					Yaml::Serialize(serialized, str);
					SuoraLog("Imported {0}", newAsset->m_Path.string());
					Platform::WriteToFile(newAsset->m_Path.string(), str);
				}
			}
			else if (m_UnresolvedAsset[Index].m_Class == Mesh::StaticClass())
			{
				if (EditorUI::Button("Import", x + width - 175.0f - 175.0f, y + 10.0f, 150.0f, 25.0f, EditorUI::ButtonParams::Highlight()))
				{
					m_AssetImported[Index] = true;
					Mesh* newAsset = AssetManager::CreateAsset<Mesh>(m_UnresolvedAsset[Index].m_Path.stem().string(), m_UnresolvedAsset[Index].m_Path.parent_path().string());
					Yaml::Node serialized;
					newAsset->Serialize(serialized);
					std::string str;
					Yaml::Serialize(serialized, str);
					SuoraLog("Imported {0}", newAsset->m_Path.string());
					Platform::WriteToFile(newAsset->m_Path.string(), str);
				}
			}
			
		}
	};

	static bool IsPathDirectSubpathOf(const std::string& directory, const FilePath& file)
	{
		return file.parent_path() == std::filesystem::path(directory);
	}

	ContentBrowser::ContentBrowser(MajorTab* majorTab)
		: MinorTab(majorTab)
	{
		Name = "Content Browser";
		m_IconTexture = AssetManager::GetAsset<Texture2D>(SuoraID("99898caa-a2b2-4fc4-9db7-5baacaed03e5"));
	}

	ContentBrowser::~ContentBrowser()
	{
	}

	void ContentBrowser::Render(float deltaTime)
	{
		EditorUI::DrawRect(0, 0, GetWidth(), GetHeight(), 0, EditorPreferences::Get()->UiColor);

		if (s_BrowseToAsset)
		{
			m_CurrentPath = s_BrowseToAsset->m_Path.parent_path().string();
			s_BrowseToAsset = nullptr;
		}

		Array<Asset*> assets = AssetManager::GetAssets<Asset>();
		Array<Folder> folders;
		const Vec2 size = Vec2(96, 144) * EditorPreferences::Get()->UiScale * m_ScaleBrowser;

		for (auto file : std::filesystem::directory_iterator(m_CurrentPath))
		{
			if (file.is_directory())
			{
				folders.Add(Folder{file.path().filename().string(), file.path().string()});
			}
		}

		float x = 15, y = GetHeight() - size.y - 15 - 30 + m_ScrollY;
		bool rightClickPossible = true;
		int index = 0;
		for (int i = 0; i < folders.Size(); i++)
		{
			DrawFolder(folders[i], x, y, index, size, rightClickPossible);
			index++;

			x += size.x + 5;
			if (x >= GetWidth() - size.y) 
			{ 
				x = 15; 
				y -= size.y + 5; 
			}
		}
		for (int i = 0; i < assets.Size(); i++)
		{
			if (!IsPathDirectSubpathOf(m_CurrentPath, assets[i]->m_Path)) continue;

			if (assets[i] && !assets[i]->IsFlagSet(AssetFlags::Missing))
			{
				DrawAsset(assets[i], x, y, index, size, rightClickPossible);
				index++;

				x += size.x + 5;
				if (x >= GetWidth() - size.y)
				{
					x = 15;
					y -= size.y + 5;
				}
			}
			
		}

		if (NativeInput::GetMouseButtonDown(Mouse::ButtonRight) && IsInputValid() && rightClickPossible && EditorUI::_GetOverlays().Size() == 0)
		{
			float width = GetMajorTab()->GetEditorWindow()->GetWindow()->GetWidth();
			float height = GetMajorTab()->GetEditorWindow()->GetWindow()->GetHeight();
			std::string currentPath = m_CurrentPath;
			
			EditorUI::CreateContextMenu({ EditorUI::ContextMenuElement{
												{
													EditorUI::ContextMenuElement{{}, [width, height, currentPath]() { EditorUI::CreateOverlay<CreateClassOverlay>(width / 2 - 425.0f, height / 2 - 275.0f, 850.0f, 550.0f, currentPath); }, "Blueprint Class", nullptr },
													EditorUI::ContextMenuElement{{}, [width, height, currentPath]() { EditorUI::CreateOverlay<CreateLevelOverlay>(width / 2 - 425.0f, height / 2 - 275.0f, 850.0f, 550.0f, currentPath); }, "Level", nullptr },
													EditorUI::ContextMenuElement{{}, [width, height, currentPath]() { EditorUI::CreateOverlay<CreateSimpleAssetOverlay>(width / 2 - 150.0f, height / 2 - 75.0f, 300.0f, 150.0f, currentPath, "Create a new ShaderGraph", "MyShaderGraph", ShaderGraph::StaticClass(), [](Asset* shader) { shader->As<ShaderGraph>()->m_BaseShader = "DeferredLit.glsl"; }); }, "ShaderGraph", nullptr},
													EditorUI::ContextMenuElement{{}, [width, height, currentPath]() { EditorUI::CreateOverlay<CreateSimpleAssetOverlay>(width / 2 - 150.0f, height / 2 - 75.0f, 300.0f, 150.0f, currentPath, "Create a new Material", "MyMaterial", Material::StaticClass(), [](Asset* material) {}); }, "Material", nullptr},
												}, []() {}, "Create Asset", nullptr },
										  EditorUI::ContextMenuElement{{}, [width, height, currentPath]() { EditorUI::CreateOverlay<CreateNewFolderOverlay>(width / 2 - 150.0f, height / 2 - 75.0f, 300.0f, 150.0f, currentPath); }, "Create Empty Folder", nullptr },
										  EditorUI::ContextMenuElement{{}, [&]() { Platform::ShowInExplorer(m_CurrentPath); }, "Show in Explorer", nullptr} });
		}

		// Header
		EditorUI::DrawRect(0, GetHeight() - 40, GetWidth(), 40, 0, Color(Vec3(EditorPreferences::Get()->UiColor) * 0.9f, 1.0f));

		DrawContentPaths();

		// Draw the Asset Importer Button
		if (EditorUI::Button("Asset Importer", GetWidth() - 175.0f, GetHeight() - 25.0f, 150.0f, 20.0f, EditorUI::ButtonParams::Highlight()))
		{
			EditorUI::CreateOverlay<AssetImporter>(GetMajorTab()->GetEditorWindow()->GetWindow()->GetWidth() / 6.0f, GetMajorTab()->GetEditorWindow()->GetWindow()->GetHeight() / 6.0f, GetMajorTab()->GetEditorWindow()->GetWindow()->GetWidth() * 2.0f / 3.0f, GetMajorTab()->GetEditorWindow()->GetWindow()->GetHeight() * 2.0f / 3.0f, AssetManager::HotReload());
		}

		const float scrollDown = y - m_ScrollY;
		EditorUI::ScrollbarVertical(GetWidth() - 10, 0, 10, GetHeight(), 0, 0, GetWidth(), GetHeight(), 0, scrollDown > 0 ? 0 : Math::Abs(scrollDown), &m_ScrollY);
	}

	void ContentBrowser::DrawFolder(Folder& folder, float& x, float& y, int& index, const Vec2& size, bool& rightClickPossible)
	{
		static bool Hover = false;
		static EditorUI::ButtonParams Params;
		Params.ButtonOutlineColor = Color(0);
		Params.ButtonColor = Color(0);
		Params.OutHover = &Hover;

		if (EditorUI::_GetOverlays().Size() == 0)
		{
			Params.TooltipText = folder.m_TargetPath;
		}
		else
		{
			Params.TooltipText = "";
		}

		if (EditorUI::Button("", x, y, size.x, size.y, Params) && ProcessElementClick(index, nullptr))
		{
			m_CurrentPath = folder.m_TargetPath;
			m_SelectedElement = -1;
			//return; 
		}
		if (Hover)
		{
			EditorUI::DrawTexturedRect(m_ShadowTexture, x + 5, y - 5, size.x, size.y, 0, Color(1.0f));
			EditorUI::Button("", x, y, size.x, size.y);
		}

		if (index == m_SelectedElement)
		{
			EditorUI::DrawRect(x, y, size.x, size.y, 4, EditorPreferences::Get()->UiHighlightColor);
		}

		const float rectX = x + ITEM_CORNER / 2.0f, rectY = y + size.y - size.x, rectWidth = size.x - ITEM_CORNER, rectHeight = size.x - ITEM_CORNER / 2.0f;

		EditorUI::DrawTexturedRect(m_FolderIcon, rectX, rectY, rectWidth, rectHeight, 0, Color(1));
		EditorUI::Text(folder.m_FolderName, Font::Instance, x, y, size.x, size.y - size.x, 22, Vec2(0, 0.5f), (index == m_SelectedElement) ? EditorPreferences::Get()->UiBackgroundColor : EditorPreferences::Get()->UiTextColor);

	}

	void ContentBrowser::DrawAsset(Asset* asset, float& x, float& y, int& index, const Vec2& size, bool& rightClickPossible)
	{
		EditorUI::ButtonParams Params;
		Params.ButtonColor = EditorPreferences::Get()->UiBackgroundColor;

		if (EditorUI::_GetOverlays().Size() == 0)
		{
			Params.TooltipText = asset->GetAssetName() + " (" + asset->GetClass().GetNativeClassName() + ")" + "\n" + "Asset-UUID: " + asset->m_UUID.GetString() + "\n" + asset->m_Path.string() + "\n" + "Filesize: " + Util::FloatToString((float)(asset->GetAssetFileSize()) / 1024.f) + " kB";
		}
		else
		{
			Params.TooltipText = "";
		}

		EditorUI::DrawTexturedRect(m_ShadowTexture, x + 5, y - 5, size.x, size.y, 0, Color(1.0f));

		// Right click
		EditorUI::ButtonParams rightClickParams = EditorUI::ButtonParams::Invisible();
		rightClickParams.OverrideActivationEvent = true;
		rightClickParams.OverrittenActivationEvent = []() { return NativeInput::GetMouseButtonDown(Mouse::ButtonRight); };
		if (rightClickPossible && EditorUI::Button("", x, y, size.x, size.y, rightClickParams))
		{
			float width = GetMajorTab()->GetEditorWindow()->GetWindow()->GetWidth();
			float height = GetMajorTab()->GetEditorWindow()->GetWindow()->GetHeight();
			rightClickPossible = false;
			m_SelectedElement = index;

			EditorUI::CreateContextMenu({ EditorUI::ContextMenuElement{{}, [asset, this]() { s_LastSelectedAsset = nullptr; GetMajorTab()->GetEditorWindow()->CloseAsset(asset); AssetManager::RemoveAsset(asset); }, "Delete Asset", AssetManager::GetAsset<Texture2D>(SuoraID("fe478bcb-d2be-4a9c-9dd3-ab79e5f3b0e4"))},
										  EditorUI::ContextMenuElement{{}, [width, height, asset]() { EditorUI::CreateOverlay<RenameAssetOverlay>(width / 2 - 150.0f, height / 2 - 75.0f, 300.0f, 150.0f, asset, asset->GetAssetName()); }, "Rename Asset", AssetManager::GetAsset<Texture2D>(SuoraID("76a3d617-de80-49b1-bc53-694e033387c9"))} });
		}

		if (EditorUI::Button("", x, y, size.x, size.y, Params) && ProcessElementClick(index, asset)) 
		{ 
			OpenAsset(asset); 
		}
		else if (NativeInput::GetMouseButton(Mouse::ButtonLeft))
		{
			if (EditorUI::Button("", x, y, size.x, size.y, Params)) 
			{ 
				m_BeginMousePos = EditorUI::GetInput(); 
				SetInputMode(EditorInputEvent::ContentBrowser_AssetDrag); 
			}
			if (glm::distance(m_BeginMousePos, EditorUI::GetInput()) >= 50.0f * EditorPreferences::Get()->UiScale && (index == m_SelectedElement) && !m_CancelAssetDragNextFrame)
			{
				s_DraggedAsset = asset;
			}
		}

		if (index == m_SelectedElement)
		{
			EditorUI::DrawRect(x, y, size.x, size.y, 4, EditorPreferences::Get()->UiHighlightColor);
		}

		const float rectX = x + ITEM_CORNER / 2.0f, rectY = y + size.y - size.x, rectWidth = size.x - ITEM_CORNER, rectHeight = size.x - ITEM_CORNER / 2.0f;

		EditorUI::DrawTexturedRect(m_Checkerboard, rectX, rectY, rectWidth, rectHeight, 0, Color(0.1f, 0.1f, 0.1f, 1));
		EditorUI::Text(asset->GetAssetName(), Font::Instance, x, y, size.x, size.y - size.x, 22, Vec2(-0.75f, 0.75f), (index == m_SelectedElement) ? EditorPreferences::Get()->UiBackgroundColor : EditorPreferences::Get()->UiTextColor);

		// Asset Type
		EditorUI::Text(Util::SmartToUpperCase(asset->GetNativeClass().GetNativeClassName(), true), Font::Instance, x, y, size.x, size.y - size.x, 18.0f, Vec2(0.92f, -0.92f), (index == m_SelectedElement) ? EditorPreferences::Get()->UiForgroundColor : EditorPreferences::Get()->UiHighlightColor);

		/// Asset Preview
		EditorUI::DrawAssetPreview(asset, asset->GetNativeClass(), rectX, rectY, rectWidth, rectHeight);
		EditorUI::DrawRect(rectX, rectY - 2.0f, rectWidth, 2.0f, 0.0f, EditorPreferences::GetAssetClassColor(asset->GetNativeClass().GetNativeClassID()));

		if (s_DraggedAsset && !NativeInput::GetMouseButton(Mouse::ButtonLeft))
		{
			m_CancelAssetDragNextFrame = true;
		}
		if (m_CancelAssetDragNextFrame || (!s_DraggedAsset && !NativeInput::GetMouseButton(Mouse::ButtonLeft) && IsInputMode(EditorInputEvent::ContentBrowser_AssetDrag)))
		{
			m_CancelAssetDragNextFrame = false;
			s_DraggedAsset = nullptr;
			SetInputMode(EditorInputEvent::None);
		}
	}

	void ContentBrowser::DrawContentPaths()
	{
		// Reset, if needed...
		if (m_CurrentMode == PathMode::ProjectPath && m_CurrentPath == AssetManager::GetAssetRootPath())
		{
			m_CurrentPath = GetRootPath();
		}

		// Draw all the Paths...
		EditorUI::ButtonParams _HdrParams;
		_HdrParams.ButtonRoundness = 15.0f;

		/*** >> ***/
		if (EditorUI::Button("", 15, GetHeight() - 35, 30, 30, _HdrParams))
		{
			m_CurrentPath = GetRootPath();
		}
		EditorUI::DrawTexturedRect(m_ArrowRight, 20 - 3, GetHeight() - 30, 20, 20, 0, Color(1.0f));
		EditorUI::DrawTexturedRect(m_ArrowRight, 20 + 3, GetHeight() - 30, 20, 20, 0, Color(1.0f));

		std::vector<std::filesystem::path> paths;
		paths.push_back(m_CurrentPath);
		int x_ = 15 + 35;
		if (EditorUI::Button(m_CurrentMode == PathMode::ProjectPath ? "ProjectPath" : "EnginePath", x_, GetHeight() - 35, 145.0f, 30, _HdrParams))
		{
			EditorUI::CreateContextMenu({ EditorUI::ContextMenuElement{{}, [&]() { m_CurrentMode = PathMode::EnginePath; m_CurrentPath = GetRootPath(); }, "EnginePath", nullptr},
										  EditorUI::ContextMenuElement{{}, [&]() { m_CurrentMode = PathMode::ProjectPath; m_CurrentPath = GetRootPath(); }, "ProjectPath", nullptr} }, x_, GetHeight() - 35);
		}
		EditorUI::DrawTexturedRect(AssetManager::GetAsset<Texture2D>(SuoraID("8742cec8-9ee5-4645-b036-577146904b41"))->GetTexture(), x_ + 125.0f, GetHeight() - 27.5f, 15.0f, 15.0f, 0.0f, Color(0.8f));
		x_ += 150.0f;
		if (EditorUI::Button("", x_, GetHeight() - 35, 30, 30, _HdrParams));
		EditorUI::DrawTexturedRect(m_ArrowRight, x_ + 5, GetHeight() - 30, 20, 20, 0, Color(1.0f));
		x_ += 35.0f;

		// Now, render the rest
		while (paths[0] != GetRootPath())
		{
			paths.insert(paths.begin(), paths[0].parent_path());
		}
		bool skipFirst = false;
		for (std::filesystem::path& path : paths)
		{
			if (!skipFirst)
			{
				skipFirst = true;
				continue;
			}

			const std::string label = path.stem().string();
			const float strWidth = Font::Instance->GetStringWidth(label, 32.0f) * 0.8f;

			if (EditorUI::Button(label, x_, GetHeight() - 35, strWidth, 30, _HdrParams))
			{
				m_CurrentPath = path.string();
			}
			x_ += strWidth + 5;
			if (EditorUI::Button("", x_, GetHeight() - 35, 30, 30, _HdrParams));
			EditorUI::DrawTexturedRect(m_ArrowRight, x_ + 5, GetHeight() - 30, 20, 20, 0, Color(1.0f));
			x_ += 35;
		}
	}

	void ContentBrowser::OpenAsset(Asset* asset)
	{
		GetMajorTab()->GetEditorWindow()->OpenAsset(asset);
		return;

		// Might change that
		if (asset->IsA<Level>())
		{
			EditorWindow::GetCurrent()->AddMajorTab<NodeClassEditor>((Level*)asset);
		}
		else if (asset->IsA<ShaderGraph>())
		{
			EditorWindow::GetCurrent()->AddMajorTab<ShaderGraphEditorPanel>((ShaderGraph*)asset);
		}
		else if (asset->IsA<Material>())
		{
			EditorWindow::GetCurrent()->AddMajorTab<MaterialEditorPanel>((Material*)asset);
		}
		else if (asset->IsA<Texture2D>())
		{
			EditorWindow::GetCurrent()->AddMajorTab<Texture2DEditorPanel>((Texture2D*)asset);
		}
		else if (asset->IsA<Mesh>())
		{
			EditorWindow::GetCurrent()->AddMajorTab<MeshEditorPanel>((Mesh*)asset);
		}
		else if (asset->IsA<Blueprint>())
		{
			EditorWindow::GetCurrent()->AddMajorTab<NodeClassEditor>((Blueprint*)asset);
		}
		else if (asset->IsA<ProjectSettings>())
		{
			EditorWindow::GetCurrent()->AddMajorTab<ProjectSettingsMajorTab>((ProjectSettings*)asset);
		}
		else if (asset->IsA<EditorPreferences>())
		{
			EditorWindow::GetCurrent()->AddMajorTab<EditorPreferencesMajorTab>((EditorPreferences*)asset);
		}
	}

	bool ContentBrowser::ProcessElementClick(int i, Asset* selectAsset)
	{
		s_LastSelectedAsset = selectAsset;

		if (i == m_SelectedElement)
		{
			const float time = Platform::GetTime();
			if (time - m_LastElementClick <= 0.5f)
			{
				return true;
			}
			else
			{
				m_LastElementClick = Platform::GetTime();
				return false;
			}
		}
		else
		{
			m_SelectedElement = i;
			m_LastElementClick = Platform::GetTime();
			return false;
		}

		return false;
	}

	std::string ContentBrowser::GetRootPath()
	{
		return m_CurrentMode == PathMode::ProjectPath ? AssetManager::GetProjectAssetPath() : AssetManager::GetAssetRootPath();
	}

}