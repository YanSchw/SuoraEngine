#include "AssetActionsOverlays.h"

namespace Suora
{

	void CreateNewFolderOverlay::Render(float deltaTime)
	{
		EditorUI::DrawRect(0, 0, EditorUI::CurrentWindow->GetWindow()->GetWidth(), EditorUI::CurrentWindow->GetWindow()->GetHeight(), 0.0f, Color(1, 1, 1, 0.2f));
		DragableOverlay::Render(deltaTime);

		EditorUI::Text("Name", Font::Instance, x + width - 280.0f, y + 85.0f, 265.0f, 20.0f, 26.0f, Vec2(-1.0f, 0.0f), Color(1));
		EditorUI::TextField(&s_FolderName, x + width - 280.0f, y + 55.0f, 265.0f, 30.0f);

		if (EditorUI::Button("Create", x + width - 280.0f, y + 15.0f, 125.0f, 30.0f, EditorUI::ButtonParams::Highlight()))
		{
			CreateFolder();

			Dispose();
			return;
		}
	}
	void CreateNewFolderOverlay::CreateFolder()
	{
		Platform::CreateDirectory(m_Directory + "/" + s_FolderName);
	}

	void RenameAssetOverlay::Render(float deltaTime)
	{
		EditorUI::DrawRect(0, 0, EditorUI::CurrentWindow->GetWindow()->GetWidth(), EditorUI::CurrentWindow->GetWindow()->GetHeight(), 0.0f, Color(1, 1, 1, 0.2f));
		DragableOverlay::Render(deltaTime);

		EditorUI::Text("Name", Font::Instance, x + width - 280.0f, y + 85.0f, 265.0f, 20.0f, 26.0f, Vec2(-1.0f, 0.0f), Color(1));
		EditorUI::TextField(&s_Name, x + width - 280.0f, y + 55.0f, 265.0f, 30.0f);

		if (EditorUI::Button("Rename", x + width - 280.0f, y + 15.0f, 125.0f, 30.0f, EditorUI::ButtonParams::Highlight()))
		{
			AssetManager::RenameAsset(m_Asset, s_Name);

			Dispose();
			return;
		}
	}

	void CreateAssetOverlay::Init()
	{
	}
	void CreateAssetOverlay::Render(float deltaTime)
	{
		if (!m_WasInitialized)
		{
			CenterOverlay();
		}

		EditorUI::DrawRect(0, 0, EditorUI::CurrentWindow->GetWindow()->GetWidth(), EditorUI::CurrentWindow->GetWindow()->GetHeight(), 0.0f, Color(1, 1, 1, 0.2f));
		DragableOverlay::Render(deltaTime);

		if (!m_WasInitialized)
		{
			m_WasInitialized = true;
			Init();
		}

		EditorUI::ButtonParams Params;
		
		EditorUI::Text("Asset Name", Font::Instance, x + width - 280.0f, y + 85.0f, 265.0f, 20.0f, 26.0f, Vec2(-1.0f, 0.0f), Color(1));
		EditorUI::TextField(&s_AssetName, x + width - 280.0f, y + 55.0f, 265.0f, 30.0f);

		const std::string assetPath = m_Directory + s_AssetName + GetAssetExtension();
		EditorUI::Text(assetPath, Font::Instance, x, y + 1.0f, width, 15.0f, 18.0f, Vec2(0.95f, 0.0f), Color(0.8f));

		if (EditorUI::Button("Cancel", x + width - 140.0f, y + 15.0f, 125.0f, 30.0f))
		{
			Dispose();
			return;
		}
		if (EditorUI::Button("Create", x + width - 280.0f, y + 15.0f, 125.0f, 30.0f, EditorUI::ButtonParams::Highlight()))
		{
			CreateAsset();

			Dispose();
			return;
		}
	}

	std::string CreateAssetOverlay::GetAssetExtension()
	{
		return ".asset";
	}

	void CreateAssetOverlay::CreateAsset()
	{
		SuoraVerify(false, "Needs to be overriden!");
	}

	void CreateSimpleAssetOverlay::Render(float deltaTime)
	{
		CreateAssetOverlay::Render(deltaTime);
	}

	std::string CreateSimpleAssetOverlay::GetAssetExtension()
	{
		return Asset::GetAssetExtensionByClass(m_AssetClass.GetNativeClassID());
	}

	void CreateSimpleAssetOverlay::CreateAsset()
	{
		Asset* asset = AssetManager::CreateAsset(m_AssetClass, s_AssetName, m_Directory);
		m_Lambda(asset);
		Yaml::Node root;
		asset->Serialize(root);
		std::string out;
		Yaml::Serialize(root, out);
		Platform::WriteToFile(asset->m_Path.string(), out);
	}

	void CreateClassOverlay::Render(float deltaTime)
	{
		CreateAssetOverlay::Render(deltaTime);

		EditorUI::ButtonParams Params;
		const float leftCenterX = (width / 2.0f - 100.0f) / 2.0f + 50.0f;
		if (EditorUI::Button("Common Classes", x + leftCenterX - 87.5f, y + height - 80.0f, 175.0f, 30.0f, Params))
		{
			m_ClassTab = 0;
		}
		if (EditorUI::Button("All Classes", x + leftCenterX + 87.5f, y + height - 80.0f, 175.0f, 30.0f, Params))
		{
			m_ClassTab = 1;
		}


		if (m_ClassTab == 0)
		{
			const float ButtonHeight = (height - 175.0f) / 6.0f;
			if (EditorUI::Button("Node", x + 15.0f, y + 75.0f + ButtonHeight * 5.0f, 550.0f, ButtonHeight, m_ParentClass == Node::StaticClass() ? EditorUI::ButtonParams::Outlined() : EditorUI::ButtonParams()))
			{
				m_ParentClass = Node::StaticClass();
			}
			if (EditorUI::Button("Node3D", x + 15.0f, y + 75.0f + ButtonHeight * 4.0f, 550.0f, ButtonHeight, m_ParentClass == Node3D::StaticClass() ? EditorUI::ButtonParams::Outlined() : EditorUI::ButtonParams()))
			{
				m_ParentClass = Node3D::StaticClass();
			}
			if (EditorUI::Button("UINode", x + 15.0f, y + 75.0f + ButtonHeight * 3.0f, 550.0f, ButtonHeight, m_ParentClass == UINode::StaticClass() ? EditorUI::ButtonParams::Outlined() : EditorUI::ButtonParams()))
			{
				m_ParentClass = UINode::StaticClass();
			}
			if (EditorUI::Button("Component", x + 15.0f, y + 75.0f + ButtonHeight * 2.0f, 550.0f, ButtonHeight, m_ParentClass == Component::StaticClass() ? EditorUI::ButtonParams::Outlined() : EditorUI::ButtonParams()))
			{
				m_ParentClass = Component::StaticClass();
			}
		}
		else
		{
			_Params.HideColor = EditorPreferences::Get()->UiColor;
			_Params.CurrentSelectedClass = m_ParentClass;
			if (EditorUI::DrawSubclassHierarchyBox(x + 15.0f, y + 15.0f, 550, height - 100.0f, _Params))
			{
				m_ParentClass = _Params.CurrentSelectedClass;
			}
		}

	}

	void CreateClassOverlay::CreateAsset()
	{
		Blueprint* blueprint = AssetManager::CreateAsset<Blueprint>(s_AssetName, m_Directory);
		blueprint->InitComposition(m_ParentClass);
		Yaml::Node root;
		blueprint->Serialize(root);
		std::string out;
		Yaml::Serialize(root, out);
		Platform::WriteToFile(blueprint->m_Path.string(), out);
	}

	void CreateLevelOverlay::Render(float deltaTime)
	{
		CreateAssetOverlay::Render(deltaTime);

		_Params.HideColor = EditorPreferences::Get()->UiColor;
		_Params.CurrentSelectedClass = m_ParentClass;
		_Params.RootClass = LevelNode::StaticClass();
		if (EditorUI::DrawSubclassHierarchyBox(x + 15.0f, y + 15.0f, 550, height - 100.0f, _Params))
		{
			m_ParentClass = _Params.CurrentSelectedClass;
		}
	}

	void CreateLevelOverlay::CreateAsset()
	{
		Level* level = AssetManager::CreateAsset<Level>(s_AssetName, m_Directory);
		level->InitComposition(m_ParentClass);
		Yaml::Node root;
		level->Serialize(root);
		std::string out;
		Yaml::Serialize(root, out);
		Platform::WriteToFile(level->m_Path.string(), out);
	}

	void ImportTexture2DOverlay::Render(float deltaTime)
	{
		CreateAssetOverlay::Render(deltaTime);

		if (m_PreviewTexture == nullptr)
		{
			m_PreviewTexture = Texture::Create(m_SourceFile.string());
		}

		EditorUI::DrawTexturedRect(m_PreviewTexture, x + 50.0f, y + 50.0f, 250.0f, 250.0f, 0.0f, Color(1.0f));

	}
	std::string ImportTexture2DOverlay::GetAssetExtension()
	{
		return ".texture";
	}
	void ImportTexture2DOverlay::CreateAsset()
	{
		Texture2D* asset = AssetManager::CreateAsset(Texture2D::StaticClass(), s_AssetName, m_Directory)->As<Texture2D>();
		asset->SetSourceAssetName(m_SourceFile.filename().string());
		Yaml::Node root;
		asset->Serialize(root);
		std::string out;
		Yaml::Serialize(root, out);
		Platform::WriteToFile(asset->m_Path.string(), out);
	}

	void ImportMeshOverlay::Render(float deltaTime)
	{
		CreateAssetOverlay::Render(deltaTime);
	}
	std::string ImportMeshOverlay::GetAssetExtension()
	{
		return ".mesh";
	}
	void ImportMeshOverlay::CreateAsset()
	{
		Mesh* asset = AssetManager::CreateAsset(Mesh::StaticClass(), s_AssetName, m_Directory)->As<Mesh>();
		asset->SetSourceAssetName(m_SourceFile.filename().string());
		Yaml::Node root;
		asset->Serialize(root);
		std::string out;
		Yaml::Serialize(root, out);
		Platform::WriteToFile(asset->m_Path.string(), out);
	}

}