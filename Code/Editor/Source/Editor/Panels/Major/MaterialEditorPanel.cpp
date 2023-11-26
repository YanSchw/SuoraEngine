#include "MaterialEditorPanel.h"
#include "Suora/Assets/AssetManager.h"
#include "Suora/Serialization/Yaml.h"
#include "Suora/GameFramework/Nodes/Light/DirectionalLightNode.h"
#include "Suora/GameFramework/Nodes/Light/SkyLightNode.h"
#include "../Minor/ViewportPanel.h"
#include "../../Util/EditorCamera.h"

namespace Suora
{

	MaterialEditorPanel::MaterialEditorPanel()
	{
		m_AssetClass = Material::StaticClass();
	}
	MaterialEditorPanel::MaterialEditorPanel(Material* material)
	{
		m_AssetClass = Material::StaticClass();
		Init();
	}
	void MaterialEditorPanel::Init()
	{
		Super::Init();

		m_Name = "Material Editor";
		m_Material = m_Asset->As<Material>();

		m_ViewportPanel = CreateRef<ViewportPanel>(this, &m_World);
		Ref<DetailsPanel> t2 = CreateRef<DetailsPanel>(this); t2->m_Data = m_Asset;

		Ref<DockingSpace> ds1 = CreateRef<DockingSpace>(0, 0, 0.5f, 1, this);					m_DockspacePanel.m_DockingSpaces.Add(ds1); ds1->m_MinorTabs.Add(m_ViewportPanel);
		Ref<DockingSpace> ds2 = CreateRef<DockingSpace>(0.5f, 0.0f, 1.0f, 1.0f, this);			m_DockspacePanel.m_DockingSpaces.Add(ds2); ds2->m_MinorTabs.Add(t2);


		MeshNode* PlaneMesh = m_World.Spawn<MeshNode>();
		PlaneMesh->SetMesh(AssetManager::GetAssetByName<Mesh>("Plane.mesh"));
		PlaneMesh->m_Materials = AssetManager::GetAsset<Material>(SuoraID("b546f092-3f80-4dd3-a73a-b4c13d28f7f8"));

		MeshNode* PreviewMesh = m_World.Spawn<MeshNode>();
		PreviewMesh->SetPosition(Vec::Up * 1.0f);
		PreviewMesh->SetScale(Vec3(2.0f));
		PreviewMesh->SetMesh(AssetManager::GetAssetByName<Mesh>("Sphere.mesh"));
		PreviewMesh->m_Materials = m_Material.Get();
		PreviewMesh->m_Materials.OverwritteMaterials = true;

		DirectionalLightNode* Light = m_World.Spawn<DirectionalLightNode>();
		SkyLightNode* Sky = m_World.Spawn<SkyLightNode>();
		Light->SetRotation(Vec3(40.0f, 0.0f, 0.0f));
	}

	MaterialEditorPanel::~MaterialEditorPanel()
	{

	}

	void MaterialEditorPanel::Update(float deltaTime)
	{
		Super::Update(deltaTime);

		if (!m_InitResetCamera && m_ViewportPanel->GetEditorCamera())
		{
			m_InitResetCamera = true;
			m_ViewportPanel->GetEditorCamera()->SetPosition(Vec3(0.0f, 1.0f, -5.0f));
		}

	}

	Texture* MaterialEditorPanel::GetIconTexture()
	{
		return AssetManager::GetAsset<Texture2D>(SuoraID("10db4794-ec22-46fc-8be9-6cd8e5245349"))->GetTexture();
	}

	void MaterialEditorPanel::SaveAsset()
	{
		Super::SaveAsset();

		Yaml::Node root;
		m_Material->Serialize(root);
		String out;
		Yaml::Serialize(root, out);
		Platform::WriteToFile(m_Material->m_Path.string(), out);
	}

}