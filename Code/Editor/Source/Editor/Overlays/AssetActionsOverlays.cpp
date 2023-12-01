#include "AssetActionsOverlays.h"
#include "Suora/Assets/AssetManager.h"
#include "Suora/Assets/Blueprint.h"
#include "Suora/Assets/Texture2D.h"
#include "Suora/Assets/Mesh.h"
#include "Suora/Assets/Level.h"
#include "Suora/Assets/ShaderGraph.h"
#include "Platform/Platform.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>

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

		const String assetPath = m_Directory + s_AssetName + GetAssetExtension();
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

	String CreateAssetOverlay::GetAssetExtension()
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

	String CreateSimpleAssetOverlay::GetAssetExtension()
	{
		return Asset::GetAssetExtensionByClass(m_AssetClass.GetNativeClassID());
	}

	void CreateSimpleAssetOverlay::CreateAsset()
	{
		Asset* asset = AssetManager::CreateAsset(m_AssetClass, s_AssetName, m_Directory);
		m_Lambda(asset);
		Yaml::Node root;
		asset->Serialize(root);
		String out;
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
			_Params.RootClass = Node::StaticClass();
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
		String out;
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
		String out;
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
	String ImportTexture2DOverlay::GetAssetExtension()
	{
		return ".texture";
	}
	void ImportTexture2DOverlay::CreateAsset()
	{
		Texture2D* asset = AssetManager::CreateAsset(Texture2D::StaticClass(), s_AssetName, m_Directory)->As<Texture2D>();
		asset->SetSourceAssetName(m_SourceFile.filename().string());
		Yaml::Node root;
		asset->Serialize(root);
		String out;
		Yaml::Serialize(root, out);
		Platform::WriteToFile(asset->m_Path.string(), out);
	}

	void ImportMeshOverlay::Render(float deltaTime)
	{
		CreateAssetOverlay::Render(deltaTime);

		EditorUI::Checkbox(&m_CreateMaterials, x + 15.0f, y + 15.0f, 25.0f, 25.0f);
		EditorUI::Text("Create Materials", Font::Instance, x + 45.0f, y + 15.0f, 250.0f, 25.0f, 22.0f, Vec2(-1.0f, 0.0f), Color(1.0f));
		EditorUI::AssetDropdown((Asset**)&m_Shadergraph, ShaderGraph::StaticClass(), x + 15.0f, y + 50.0f, 250.0f, 35.0f);
	}
	String ImportMeshOverlay::GetAssetExtension()
	{
		return ".mesh";
	}

	void ImportMeshOverlay::CreateAsset()
	{
		Mesh* asset = AssetManager::CreateAsset(Mesh::StaticClass(), s_AssetName, m_Directory)->As<Mesh>();
		asset->SetSourceAssetName(m_SourceFile.filename().string());
		
		if (m_CreateMaterials)
		{
			Assimp::Importer importer;
			const aiScene* scene = importer.ReadFile(m_SourceFile.string(), 0);

			std::unordered_map<aiMaterial*, Material*> materials;
			for (int32_t i = 0; i < scene->mNumMeshes; i++)
			{
				aiMaterial* mat = scene->mMaterials[scene->mMeshes[i]->mMaterialIndex];
				aiReturn ret;

				Material* material = nullptr;
				if (materials.find(mat) == materials.end())
				{
					material = AssetManager::CreateAsset(Material::StaticClass(), s_AssetName + "_" + std::to_string(i), m_Directory)->As<Material>();
				}
				else
				{
					material = materials.at(mat);
				}
				asset->m_Materials.Materials.Add(material);
				material->SetShaderGraph(m_Shadergraph);

				for (auto& It : material->m_UniformSlots)
				{
					if (It.m_Type == ShaderGraphDataType::Texture2D)
					{
						String label = StringUtil::ToLower(It.m_Label);
						if (label.find("base") != String::npos || label.find("albedo") != String::npos || label.find("diffuse") != String::npos)
						{
							aiString textureBaseColor;
							ret = mat->Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0), textureBaseColor);
							if (ret == AI_SUCCESS)
							{
								Array<Texture2D*> allTextures = AssetManager::GetAssets<Texture2D>();
								for (int j = 0; j < allTextures.Size(); j++)
								{
									if (File::IsPathSubpathOf(m_Directory, allTextures[j]->m_Path))
									{
										if (allTextures[j]->GetSourceAssetName() == String(textureBaseColor.C_Str()))
										{
											It.m_Texture2D = allTextures[j];
											break;
										}
									}
								}
							}
						}
						if (label.find("normal") != String::npos)
						{
							aiString textureNormal;
							ret = mat->Get(AI_MATKEY_TEXTURE(aiTextureType_NORMALS, 0), textureNormal);
							if (ret == AI_SUCCESS)
							{
								Array<Texture2D*> allTextures = AssetManager::GetAssets<Texture2D>();
								for (int j = 0; j < allTextures.Size(); j++)
								{
									if (File::IsPathSubpathOf(m_Directory, allTextures[j]->m_Path))
									{
										if (allTextures[j]->GetSourceAssetName() == String(textureNormal.C_Str()))
										{
											It.m_Texture2D = allTextures[j];
											break;
										}
									}
								}
							}
						}
					}
				}

				Yaml::Node root;
				material->Serialize(root);
				String out;
				Yaml::Serialize(root, out);
				Platform::WriteToFile(material->m_Path.string(), out);

				materials[mat] = material;
			}
		}
		
		Yaml::Node root;
		asset->Serialize(root);
		String out;
		Yaml::Serialize(root, out);
		Platform::WriteToFile(asset->m_Path.string(), out);

	}

}