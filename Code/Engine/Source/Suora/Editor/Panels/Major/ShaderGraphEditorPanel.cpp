#include "Precompiled.h"
#include "ShaderGraphEditorPanel.h"
#include "Suora/Assets/ShaderGraph.h"
#include "Suora/Assets/AssetManager.h"
#include "Suora/Serialization/Yaml.h"

#include "Suora/GameFramework/Nodes/MeshNode.h"
#include "Suora/GameFramework/Nodes/Light/DirectionalLightNode.h"
#include "Suora/GameFramework/Nodes/Light/SkyLightNode.h"
#include "Suora/Renderer/RenderPipeline.h"
#include "Suora/Editor/Util/EditorCamera.h"
#include "Suora/NodeScript/ShaderNodeGraph.h"

namespace Suora
{

	ShaderGraphEditorPanel::ShaderGraphEditorPanel()
	{
		m_AssetClass = ShaderGraph::StaticClass();
	}
	ShaderGraphEditorPanel::ShaderGraphEditorPanel(ShaderGraph* shader)
	{
		m_AssetClass = ShaderGraph::StaticClass();
		Init();
	}
	void ShaderGraphEditorPanel::Init()
	{
		Super::Init();

		m_Name = m_Asset->As<ShaderGraph>()->GetAssetName();
		m_ShaderGraph = m_Asset->As<ShaderGraph>();

		m_ViewportPanel = CreateRef<ViewportPanel>(this, &m_World);
		Ref<DetailsPanel> t2 = CreateRef<DetailsPanel>(this); t2->m_Data = m_ShaderGraph;
		m_NodeEditor = CreateRef<ShaderGraphNodeEditor>(this);

		m_LastBaseShader = m_ShaderGraph->m_BaseShader;
		m_ShaderGraph->LoadBaseShaderInputs(m_ShaderGraph->GetBaseShaderPath());

		Yaml::Node root;
		Yaml::Parse(root, Platform::ReadFromFile(m_ShaderGraph->m_Path.string()));
		m_NodeEditor->m_Graph->DeserializeNodeGraph(root);

		Ref<DockingSpace> ds1 = CreateRef<DockingSpace>(0.0f, 0.5f, 0.25f, 1.0f, this);					m_DockspacePanel.m_DockingSpaces.Add(ds1); ds1->m_MinorTabs.Add(m_ViewportPanel);
		Ref<DockingSpace> ds2 = CreateRef<DockingSpace>(0.0f, 0.0f, 0.25f, 0.5f, this);					m_DockspacePanel.m_DockingSpaces.Add(ds2); ds2->m_MinorTabs.Add(t2);
		Ref<DockingSpace> ds3 = CreateRef<DockingSpace>(0.25f, 0.0f, 1.0f, 1.0f, this);					m_DockspacePanel.m_DockingSpaces.Add(ds3); ds3->m_MinorTabs.Add(m_NodeEditor);

		MeshNode* PlaneMesh = m_World.Spawn<MeshNode>();
		PlaneMesh->SetMesh(AssetManager::GetAssetByName<Mesh>("Plane.mesh"));
		PlaneMesh->m_Materials = AssetManager::GetAsset<Material>(SuoraID("b546f092-3f80-4dd3-a73a-b4c13d28f7f8"));

		MeshNode* PreviewMesh = m_World.Spawn<MeshNode>();
		PreviewMesh->SetPosition(Vec::Up * 1.0f);
		PreviewMesh->SetScale(Vec3(2.0f));
		PreviewMesh->SetMesh(AssetManager::GetAssetByName<Mesh>("Sphere.mesh"));
		PreviewMesh->m_Materials = m_ShaderGraph.Get();
		PreviewMesh->m_Materials.OverwritteMaterials = true;

		DirectionalLightNode* Light = m_World.Spawn<DirectionalLightNode>();
		SkyLightNode* Sky = m_World.Spawn<SkyLightNode>();
		Light->SetRotation(Vec3(40.0f, 0.0f, 0.0f));
	}

	ShaderGraphEditorPanel::~ShaderGraphEditorPanel()
	{

	}

	void ShaderGraphEditorPanel::Update(float deltaTime)
	{
		Super::Update(deltaTime);

		if (m_LastBaseShader != m_ShaderGraph->m_BaseShader)
		{
			m_LastBaseShader = m_ShaderGraph->m_BaseShader;
			m_ShaderGraph->LoadBaseShaderInputs(m_ShaderGraph->GetBaseShaderPath());
		}

		if (!m_InitResetCamera && m_ViewportPanel->GetEditorCamera())
		{
			m_InitResetCamera = true;
			m_ViewportPanel->GetEditorCamera()->SetPosition(Vec3(0.0f, 1.0f, -5.0f));
		}

		m_NodeEditor->GetShaderNodeGraph()->TickAllVisualNodesInShaderGraphContext(m_ShaderGraph);
	}

	Texture* ShaderGraphEditorPanel::GetIconTexture()
	{
		return AssetManager::GetAsset<Texture2D>(SuoraID("10db4794-ec22-46fc-8be9-6cd8e5245349"))->GetTexture();
	}

	void ShaderGraphEditorPanel::SaveAsset()
	{
		Super::SaveAsset();

		m_ShaderGraph->GenerateShaderGraphSource(*m_NodeEditor->GetShaderNodeGraph());
		Yaml::Node root;
		m_ShaderGraph->Serialize(root);
		m_NodeEditor->m_Graph->SerializeNodeGraph(root);
		String out;
		Yaml::Serialize(root, out);
		Platform::WriteToFile(m_ShaderGraph->m_Path.string(), out);
	}

	/*** ShaderGraphNodeEditor ***/

	Ref<Texture> ShaderGraphNodeEditor::GetPinIconTexture(int64_t pinID, bool hasOtherPin)
	{
		return (hasOtherPin ? m_PinConnectionTexture2 : m_PinConnectionTexture);
	}

	void ShaderGraphNodeEditor::OnNodeGraphRender(float deltaTime)
	{
		PinIndex = 0;
	}

	void ShaderGraphNodeEditor::DrawVisualNode(VisualNode& node)
	{
		DefaultDrawVisualNode(node);
		
	}

	float ShaderGraphNodeEditor::DrawVisualNodePin(VisualNode& node, VisualNodePin& pin, bool inputPin, float y)
	{
		DefaultDrawVisualNodePin(node, pin, inputPin, y);

		// Skip Master Node
		if (node.m_NodeID == 1)
		{
			return 0.0f;
		}

		if (pin.PinID == (int64_t)ShaderGraphDataType::Float && pin.IsReceivingPin && !pin.Target)
		{
			while (m_TempDragFloatFields.Size() <= PinIndex) m_TempDragFloatFields.Add(0.0f);
			const float temp = StringUtil::StringToFloat(pin.m_AdditionalData);
			m_TempDragFloatFields[PinIndex] = temp;
			EditorUI::DragFloat(&m_TempDragFloatFields[PinIndex], node.m_Position.x * m_Zoom - node.m_Size.x / 2.0f * m_Zoom - m_CameraPos.x * m_Zoom + GetWidth() / 2 + 5.0f + (GetNodeGraphFont()->GetStringWidth(pin.Label, 26.0f * m_Zoom) / 1.9f + (38.0f * m_Zoom)), y + 2.0f, 100.0f * m_Zoom, pin.PinHeight * m_Zoom - 4.0f, [&](String str) { pin.m_AdditionalData = str; });
			if (m_TempDragFloatFields[PinIndex] != temp)
			{
				pin.m_AdditionalData = std::to_string(m_TempDragFloatFields[PinIndex]);
			}
		}
		PinIndex++;

		// Uniform Node
		if (node.m_NodeID == 2)
		{
			if (pin.Label == "Name")
			{
				EditorUI::TextField(&pin.m_AdditionalData, node.m_Position.x * m_Zoom - node.m_Size.x / 2.0f * m_Zoom - m_CameraPos.x * m_Zoom + GetWidth() / 2 + 5.0f, y - 25.0f * m_Zoom + 2.0f, 100.0f * m_Zoom, pin.PinHeight * m_Zoom - 4.0f);
				return 30.0f * m_Zoom;
			}
			if (pin.Label == "Type")
			{
				if (EditorUI::Button(ShaderNodeGraph::ShaderGraphDataTypeToLabel((ShaderGraphDataType)(int64_t)std::stoi(pin.m_AdditionalData)), node.m_Position.x * m_Zoom - node.m_Size.x / 2.0f * m_Zoom - m_CameraPos.x * m_Zoom + GetWidth() / 2 + 5.0f, y - 25.0f * m_Zoom + 2.0f, 100.0f * m_Zoom, pin.PinHeight * m_Zoom - 4.0f))
				{
					int64_t i = (int64_t)std::stoi(pin.m_AdditionalData);
					i++;
					if (i > 5) i = 1;
					pin.m_AdditionalData = std::to_string(i);
				}
				return 30.0f * m_Zoom;
			}
			if (pin.Label == "Uniform")
			{
				pin.Color = node.m_Color;
				pin.PinID = (int64_t)std::stoi(node.m_InputPins[1].m_AdditionalData);
			}
		}

		return 0.0f;
	}

}