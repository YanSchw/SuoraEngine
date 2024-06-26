#include "Precompiled.h"
#include "MeshEditorPanel.h"
#include "Suora/Assets/AssetManager.h"
#include "Suora/Serialization/Yaml.h"
#include "Suora/Assets/Mesh.h"
#include "Suora/GameFramework/Nodes/Light/DirectionalLightNode.h"
#include "Suora/GameFramework/Nodes/Light/SkyLightNode.h"
#include "Suora/Editor/Panels/MinorTab.h"
#include "Suora/Editor/Panels/Minor/ViewportPanel/ViewportPanel.h"
#include "Suora/Editor/Util/EditorCamera.h"

namespace Suora
{

	class MeshViewportPanel : public ViewportPanel
	{
	public:
		MeshViewportPanel(MeshEditorPanel* editor, MajorTab* majorTab, World* world)
			: ViewportPanel(majorTab, world)
		{
			m_MeshEditor = editor;
		}
		virtual void OnRenderOutlines() override
		{
			if (m_MeshEditor->m_MaterialSlotOutline != -1)
			{
				if (!m_MeshEditor->m_MeshAsset->IsMasterMesh())
				{
					if (m_MeshEditor->m_MaterialSlotOutline == 0)
					{
						MeshNode node;
						node.SetPosition(Vec3(0.0f, 0.0f - m_MeshEditor->m_MeshAsset->m_NegativeY_Bounds + 0.1f, 0.0f));
						node.SetMesh(m_MeshEditor->m_MeshAsset);
						node.m_Materials.OverwritteMaterials = true;
						node.m_Materials.Materials.Add(m_MeshEditor->m_MeshAsset->m_Materials.Materials[0]);
						DrawSelectionOutline(&node, EditorPreferences::Get()->UiHighlightColor);
					}
				}
				else
				{
					MeshNode node;
					node.SetPosition(Vec3(0.0f, 0.0f - m_MeshEditor->m_MeshAsset->m_NegativeY_Bounds + 0.1f, 0.0f));
					node.SetMesh(m_MeshEditor->m_MeshAsset->m_Submeshes[m_MeshEditor->m_MaterialSlotOutline].get());
					node.m_Materials.OverwritteMaterials = true;
					node.m_Materials.Materials.Add(m_MeshEditor->m_MeshAsset->m_Materials.Materials[m_MeshEditor->m_MaterialSlotOutline]);
					DrawSelectionOutline(&node, EditorPreferences::Get()->UiHighlightColor);
				}
			}
			m_MeshEditor->m_MaterialSlotOutline = -1;
		}
		WeakPtr<MeshEditorPanel> m_MeshEditor;
	};


	static void CalculateMeshData(Mesh* mesh)
	{
		mesh->m_BoundingSphereRadius = 0.0f;
		mesh->m_NegativeY_Bounds = 9999999.0f;

		if (mesh->IsMasterMesh())
		{
			for (auto submesh : mesh->m_Submeshes)
			{
				if (submesh)
				{
					while (!submesh->GetVertexArray());
				}
			}
		}
		else while (!mesh->GetVertexArray());

		if (mesh->IsMasterMesh())
		{
			for (auto submesh : mesh->m_Submeshes)
			{
				for (int i = 0; i < submesh->m_MeshBuffer.Vertices.size(); i++)
				{
					const Vec3& pos = submesh->m_MeshBuffer.Vertices[i].Position;

					if (pos.y < submesh->m_NegativeY_Bounds) submesh->m_NegativeY_Bounds = pos.y;
					if (glm::length(pos) > submesh->m_BoundingSphereRadius) submesh->m_BoundingSphereRadius = glm::length(pos);
				}
			}
			for (auto submesh : mesh->m_Submeshes)
			{
				if (submesh->m_NegativeY_Bounds < mesh->m_NegativeY_Bounds)
				{
					mesh->m_NegativeY_Bounds = submesh->m_NegativeY_Bounds;
				}
				if (submesh->m_BoundingSphereRadius > mesh->m_BoundingSphereRadius)
				{
					mesh->m_BoundingSphereRadius = submesh->m_BoundingSphereRadius;
				}
			}
		}
		else
		{
			for (int i = 0; i < mesh->m_MeshBuffer.Vertices.size(); i++)
			{
				const Vec3& pos = mesh->m_MeshBuffer.Vertices[i].Position;

				if (pos.y < mesh->m_NegativeY_Bounds) mesh->m_NegativeY_Bounds = pos.y;
				if (glm::length(pos) > mesh->m_BoundingSphereRadius) mesh->m_BoundingSphereRadius = glm::length(pos);
			}
		}


	}

	MeshEditorPanel::MeshEditorPanel()
	{
		m_AssetClass = Mesh::StaticClass();
	}
	MeshEditorPanel::MeshEditorPanel(Mesh* mesh)
	{
		m_AssetClass = Mesh::StaticClass();
		Init();
	}
	void MeshEditorPanel::Init()
	{
		Super::Init();
		m_Name = m_Asset->As<Mesh>()->GetAssetName();
		m_MeshAsset = m_Asset->As<Mesh>();

		CalculateMeshData(m_MeshAsset);

		m_ViewportPanel = CreateRef<MeshViewportPanel>(this, this, &m_World);
		Ref<DetailsPanel> details = CreateRef<DetailsPanel>(this);
		details->m_Data = m_MeshAsset;

		Ref<DockingSpace> ds1 = CreateRef<DockingSpace>(0, 0, 0.75f, 1, this);					    m_DockspacePanel.m_DockingSpaces.Add(ds1); ds1->m_MinorTabs.Add(m_ViewportPanel);
		Ref<DockingSpace> ds2 = CreateRef<DockingSpace>(0.75f, 0.0f, 1.0f, 1.0f, this);				m_DockspacePanel.m_DockingSpaces.Add(ds2); ds2->m_MinorTabs.Add(details);

		MeshNode* PlaneMesh = m_World.Spawn<MeshNode>();
		PlaneMesh->SetMesh(AssetManager::GetAssetByName<Mesh>("Plane.mesh"));
		PlaneMesh->m_Materials = AssetManager::GetAsset<Material>(SuoraID("b546f092-3f80-4dd3-a73a-b4c13d28f7f8"));

		MeshNode* PreviewMesh = m_World.Spawn<MeshNode>();
		PreviewMesh->SetPosition(Vec3(0.0f, 0.0f - m_MeshAsset->m_NegativeY_Bounds + 0.1f, 0.0f));
		PreviewMesh->SetMesh(m_MeshAsset);
		PreviewMesh->m_Materials = AssetManager::GetAsset<Material>(SuoraID("b546f092-3f80-4dd3-a73a-b4c13d28f7f8"));

		DirectionalLightNode* Light = m_World.Spawn<DirectionalLightNode>();
		SkyLightNode* Sky = m_World.Spawn<SkyLightNode>();
		Light->SetRotation(Vec3(40.0f, 0.0f, 0.0f));
	}

	MeshEditorPanel::~MeshEditorPanel()
	{
	}

	void MeshEditorPanel::Update(float deltaTime)
	{
		Super::Update(deltaTime);

		if (!m_InitResetCamera && m_ViewportPanel->GetEditorCamera())
		{
			m_InitResetCamera = true;
			m_ViewportPanel->GetEditorCamera()->SetPosition(Vec3(0.0f, 1.0f, -5.0f));
		}

	}

	Texture* MeshEditorPanel::GetIconTexture()
	{
		return EditorUI::GetClassIcon(MeshNode::StaticClass())->GetTexture();
	}
	void MeshEditorPanel::DrawToolbar(float& x, float y, float height)
	{
		Super::DrawToolbar(x, y, height);

		if (EditorUI::Button("Rebuild Mesh", x + 15, y, 150, height, EditorUI::ButtonParams::Highlight()))
		{
			m_MeshAsset->RebuildMesh();
		}
	}

	void MeshEditorPanel::SaveAsset()
	{
		Super::SaveAsset();

		Yaml::Node root;
		m_MeshAsset->Serialize(root);
		String out;
		Yaml::Serialize(root, out);
		Platform::WriteToFile(m_MeshAsset->m_Path.string(), out);
	}

}