#include "Precompiled.h"
#include "ContentBrowser.h"
#include "Suora/Editor/Util/EditorPreferences.h"
#include "Suora/Editor/Util/EditorCamera.h"
#include "Suora/Editor/Overlays/AssetActionsOverlays.h"
#include "Suora/Editor/Panels/Major/MaterialEditorPanel.h"
#include "Suora/Editor/Panels/Major/ShaderGraphEditorPanel.h"
#include "Suora/Editor/Panels/Major/Texture2DEditorPanel.h"
#include "Suora/Editor/Panels/Major/MeshEditorPanel.h"
#include "Suora/Editor/Panels/Major/NodeClassEditor.h"
#include "Suora/Editor/Panels/Major/SettingsTabs.h"
#include "Suora/Editor/Panels/Major/InputMappingPanel.h"
#include "Suora/Assets/ShaderGraph.h"
#include "Suora/Assets/Blueprint.h"
#include "Suora/GameFramework/InputModule.h"

#define ITEM_CORNER 4.0f

namespace Suora
{

	static bool IsPathDirectSubpathOf(const String& directory, const Path& file)
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
			String currentPath = m_CurrentPath;

			std::vector<EditorUI::ContextMenuElement> Texture2DImports;
			std::vector<EditorUI::ContextMenuElement> MeshImports;

			for (auto file : std::filesystem::directory_iterator(currentPath))
			{
				if (!file.is_directory())
				{
					Array<String> textureExtensions = Texture2D::GetSupportedSourceAssetExtensions();
					Array<String> meshExtensions = Mesh::GetSupportedSourceAssetExtensions();
					for (auto ext : textureExtensions)
					{
						if (FileUtils::GetFileExtension(file) == ext)
						{
							Texture2DImports.push_back(EditorUI::ContextMenuElement{ {}, [width, height, file, currentPath]() { EditorUI::CreateOverlay<ImportTexture2DOverlay>(width / 2 - 425.0f, height / 2 - 275.0f, 850.0f, 550.0f, file.path(), currentPath); }, "Import " + file.path().filename().string(), nullptr});
						}
					}
					for (auto ext : meshExtensions)
					{
						if (FileUtils::GetFileExtension(file) == ext)
						{
							MeshImports.push_back(EditorUI::ContextMenuElement{ {}, [width, height, file, currentPath]() { EditorUI::CreateOverlay<ImportMeshOverlay>(width / 2 - 425.0f, height / 2 - 275.0f, 850.0f, 550.0f, file.path(), currentPath); }, "Import " + file.path().filename().string(), nullptr });
						}
					}
				}
			}
			
			EditorUI::CreateContextMenu({ EditorUI::ContextMenuElement{
												{
													EditorUI::ContextMenuElement{{}, [width, height, currentPath]() { EditorUI::CreateOverlay<CreateClassOverlay>(width / 2 - 425.0f, height / 2 - 275.0f, 850.0f, 550.0f, currentPath); }, "Blueprint Class", nullptr },
													EditorUI::ContextMenuElement{{}, [width, height, currentPath]() { EditorUI::CreateOverlay<CreateLevelOverlay>(width / 2 - 425.0f, height / 2 - 275.0f, 850.0f, 550.0f, currentPath); }, "Level", nullptr },
													EditorUI::ContextMenuElement{{ Texture2DImports }, []() {}, "Texture2D", nullptr },
													EditorUI::ContextMenuElement{{ MeshImports }, []() {}, "Mesh", nullptr },
													EditorUI::ContextMenuElement{{}, [width, height, currentPath]() { EditorUI::CreateOverlay<CreateSimpleAssetOverlay>(width / 2 - 150.0f, height / 2 - 75.0f, 300.0f, 150.0f, currentPath, "Create a new ShaderGraph", "MyShaderGraph", ShaderGraph::StaticClass(), [](Asset* shader) { shader->As<ShaderGraph>()->m_BaseShader = "DeferredLit.glsl"; }); }, "ShaderGraph", nullptr},
													EditorUI::ContextMenuElement{{}, [width, height, currentPath]() { EditorUI::CreateOverlay<CreateSimpleAssetOverlay>(width / 2 - 150.0f, height / 2 - 75.0f, 300.0f, 150.0f, currentPath, "Create a new Material", "MyMaterial", Material::StaticClass(), [](Asset* material) {}); }, "Material", nullptr},
													EditorUI::ContextMenuElement{{}, [width, height, currentPath]() { EditorUI::CreateOverlay<CreateSimpleAssetOverlay>(width / 2 - 150.0f, height / 2 - 75.0f, 300.0f, 150.0f, currentPath, "Create a new InputMapping", "MyInputMapping", InputMapping::StaticClass(), [](Asset* input) {}); }, "InputMapping", nullptr}
												}, []() {}, "Create Asset", nullptr },
										  EditorUI::ContextMenuElement{{}, [width, height, currentPath]() { EditorUI::CreateOverlay<CreateNewFolderOverlay>(width / 2 - 150.0f, height / 2 - 75.0f, 300.0f, 150.0f, currentPath); }, "Create Empty Folder", nullptr },
										  EditorUI::ContextMenuElement{{}, [&]() { Platform::ShowInExplorer(m_CurrentPath); }, "Show in Explorer", nullptr} });
		}

		// Header
		EditorUI::DrawRect(0, GetHeight() - 40, GetWidth(), 40, 0, Color(Vec3(EditorPreferences::Get()->UiColor) * 0.9f, 1.0f));

		DrawContentPaths();

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
			Params.TooltipText = asset->GetAssetName() + " (" + asset->GetClass().GetNativeClassName() + ")" + "\n" + "Asset-UUID: " + asset->m_UUID.GetString() + "\n" + asset->m_Path.string() + "\n" + "Filesize: " + StringUtil::FloatToString((float)(asset->GetAssetFileSize()) / 1024.f) + " kB";
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
		EditorUI::Text(StringUtil::SmartToUpperCase(asset->GetNativeClass().GetNativeClassName(), true), Font::Instance, x, y, size.x, size.y - size.x, 18.0f, Vec2(0.92f, -0.92f), (index == m_SelectedElement) ? EditorPreferences::Get()->UiForgroundColor : EditorPreferences::Get()->UiHighlightColor);

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
		if (m_CurrentMode == PathMode::ProjectPath && m_CurrentPath == AssetManager::GetEngineAssetPath())
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
		if (EditorUI::Button(m_CurrentMode == PathMode::ProjectPath ? "ProjectPath" : "EnginePath", x_, GetHeight() - 35, 165.0f, 30, _HdrParams))
		{
			EditorUI::CreateContextMenu({ EditorUI::ContextMenuElement{{}, [&]() { m_CurrentMode = PathMode::EnginePath; m_CurrentPath = GetRootPath(); }, "EnginePath", nullptr},
										  EditorUI::ContextMenuElement{{}, [&]() { m_CurrentMode = PathMode::ProjectPath; m_CurrentPath = GetRootPath(); }, "ProjectPath", nullptr} }, x_, GetHeight() - 35);
		}
		EditorUI::DrawTexturedRect(AssetManager::GetAsset<Texture2D>(SuoraID("8742cec8-9ee5-4645-b036-577146904b41"))->GetTexture(), x_ + 140.0f, GetHeight() - 27.5f, 15.0f, 15.0f, 0.0f, Color(0.9f));
		x_ += 170.0f;
		if (EditorUI::Button("", x_, GetHeight() - 35, 30, 30, _HdrParams));
		EditorUI::DrawTexturedRect(m_ArrowRight, x_ + 5, GetHeight() - 30, 20, 20, 0, Color(1.0f));
		x_ += 35.0f;

		// Now, render the rest
		while (paths[0] != AssetManager::GetEngineAssetPath() && paths[0] != AssetManager::GetProjectAssetPath())
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

			const String label = path.stem().string();
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
		else if (asset->IsA<InputMapping>())
		{
			EditorWindow::GetCurrent()->AddMajorTab<InputMappingPanel>((InputMapping*)asset);
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

	String ContentBrowser::GetRootPath()
	{
		return m_CurrentMode == PathMode::ProjectPath ? AssetManager::GetProjectAssetPath() : AssetManager::GetEngineAssetPath();
	}

	String ContentBrowser::GetCurrentDirectory() const
	{
		return m_CurrentPath;
	}

}