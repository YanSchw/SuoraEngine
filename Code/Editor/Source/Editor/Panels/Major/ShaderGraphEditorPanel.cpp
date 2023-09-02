#include "ShaderGraphEditorPanel.h"
#include "Suora/Assets/ShaderGraph.h"
#include "Suora/Assets/AssetManager.h"
#include "Suora/Serialization/Yaml.h"

#include "Suora/GameFramework/Nodes/MeshNode.h"
#include "Suora/GameFramework/Nodes/Light/DirectionalLightNode.h"
#include "Suora/GameFramework/Nodes/Light/SkyLightNode.h"
#include "Suora/Renderer/RenderPipeline.h"
#include "../../Util/EditorCamera.h"
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
		m_NodeEditor->LoadBaseShaderInputs(m_ShaderGraph->GetBaseShaderPath());
		//m_NodeEditor->m_PreviewMaterial.SetShader(shader->GetShader());

		Yaml::Node root;
		Yaml::Parse(root, Platform::ReadFromFile(m_ShaderGraph->m_Path.string()));
		m_NodeEditor->m_Graph->DeserializeNodeGraph(root);

		Ref<DockingSpace> ds1 = CreateRef<DockingSpace>(0.0f, 0.5f, 0.25f, 1.0f, this);					m_DockspacePanel.m_DockingSpaces.Add(ds1); ds1->m_MinorTabs.Add(m_ViewportPanel);
		Ref<DockingSpace> ds2 = CreateRef<DockingSpace>(0.0f, 0.0f, 0.25f, 0.5f, this);					m_DockspacePanel.m_DockingSpaces.Add(ds2); ds2->m_MinorTabs.Add(t2);
		Ref<DockingSpace> ds3 = CreateRef<DockingSpace>(0.25f, 0.0f, 1.0f, 1.0f, this);					m_DockspacePanel.m_DockingSpaces.Add(ds3); ds3->m_MinorTabs.Add(m_NodeEditor);

		MeshNode* PlaneMesh = m_World.Spawn<MeshNode>();
		PlaneMesh->mesh = AssetManager::GetAssetByName<Mesh>("Plane.mesh");
		PlaneMesh->materials = AssetManager::GetAsset<Material>(SuoraID("b546f092-3f80-4dd3-a73a-b4c13d28f7f8"));

		MeshNode* PreviewMesh = m_World.Spawn<MeshNode>();
		PreviewMesh->SetPosition(Vec::Up * 1.0f);
		PreviewMesh->mesh = AssetManager::GetAssetByName<Mesh>("Sphere.mesh");
		PreviewMesh->materials = m_ShaderGraph.Get();
		PreviewMesh->materials.OverwritteMaterials = true;

		DirectionalLightNode* Light = m_World.Spawn<DirectionalLightNode>();
		SkyLightNode* Sky = m_World.Spawn<SkyLightNode>();
		Light->SetRotation(Vec3(40.0f, 0.0f, 0.0f));
	}

	ShaderGraphEditorPanel::~ShaderGraphEditorPanel()
	{

	}

	void ShaderGraphEditorPanel::GenerateShaderGraphSource()
	{
		bool error = false;
		m_ShaderGraph->m_Shader = nullptr;
		VisualNode* master = nullptr;
		for (Ref<VisualNode> node : m_NodeEditor->m_Graph->m_Nodes)
		{
			if (node->m_Title == "Master" && node->m_NodeID == 1)
			{
				master = node.get();
			}
		}
		SUORA_ASSERT(master, "no Master node found!");

		std::string src = Platform::ReadFromFile(m_ShaderGraph->GetBaseShaderPath());
		Util::RemoveCommentsFromString(src);

		// Uniforms
		Array<UniformSlot> oldSlots = m_ShaderGraph->m_UniformSlots;
		m_ShaderGraph->m_UniformSlots.Clear();
		std::string uniforms;
		for (Ref<VisualNode> node : m_NodeEditor->m_Graph->m_Nodes)
		{
			if (node->m_NodeID == 2)
			{
				const ShaderGraphDataType type = (ShaderGraphDataType)std::stoi(node->m_InputPins[1].m_AdditionalData);
				uniforms += "uniform " + ShaderNodeGraph::ShaderGraphDataTypeToString(type) + " " + node->m_InputPins[0].m_AdditionalData + ";\n";

				switch (type)
				{
				case ShaderGraphDataType::Float:
					m_ShaderGraph->m_UniformSlots.Add(UniformSlot(type, node->m_InputPins[0].m_AdditionalData, 0.0f));
					break;
				case ShaderGraphDataType::Vec2:
					m_ShaderGraph->m_UniformSlots.Add(UniformSlot(type, node->m_InputPins[0].m_AdditionalData, 0.0f));
					break;
				case ShaderGraphDataType::Vec3:
					m_ShaderGraph->m_UniformSlots.Add(UniformSlot(type, node->m_InputPins[0].m_AdditionalData, 0.0f));
					break;
				case ShaderGraphDataType::Vec4:
					m_ShaderGraph->m_UniformSlots.Add(UniformSlot(type, node->m_InputPins[0].m_AdditionalData, 0.0f));
					break;
				case ShaderGraphDataType::Texture2D:
					m_ShaderGraph->m_UniformSlots.Add(UniformSlot(type, node->m_InputPins[0].m_AdditionalData, nullptr));
					break;
				case ShaderGraphDataType::None:
				default:
					SuoraError("Unkown Type!");
				}
			}
		}
		for (UniformSlot& old : oldSlots)
		{
			for (UniformSlot& newer : m_ShaderGraph->m_UniformSlots)
			{
				if (old.m_Label == newer.m_Label && old.m_Type == newer.m_Type)
				{
					newer = old;
				}
			}
		}

		// Proccess BaseShader
		while (src.find("$VERT_INPUTS") != std::string::npos) Util::ReplaceSequence(src, "$VERT_INPUTS", "" + uniforms);
		while (src.find("$FRAG_INPUTS") != std::string::npos) Util::ReplaceSequence(src, "$FRAG_INPUTS", "" + uniforms);

		int64_t begin = 0;
		while (true)
		{
			begin = src.find("$VERT_INPUT", begin);
			if (begin == std::string::npos) break;
			m_NodeEditor->GenerateShaderInput(src, begin, master, true, error);
		}
		begin = 0;
		while (true)
		{
			begin = src.find("$FRAG_INPUT", begin);
			if (begin == std::string::npos) break;
			m_NodeEditor->GenerateShaderInput(src, begin, master, false, error);
		}

		if (!error)
		{
			m_ShaderGraph->m_ShaderSource = src;
			m_ShaderGraph->m_SourceFileTime = std::chrono::time_point<std::chrono::steady_clock>(std::filesystem::last_write_time(m_ShaderGraph->GetBaseShaderPath()).time_since_epoch());
		}
		else
		{
			SuoraError("ShaderGraph compilation failed!");
		}

		// Preview Material
		//m_NodeEditor->m_PreviewMaterial.SetShader(m_ShaderGraph->GetShader());
	}

	void ShaderGraphEditorPanel::Update(float deltaTime)
	{
		Super::Update(deltaTime);

		if (m_LastBaseShader != m_ShaderGraph->m_BaseShader)
		{
			m_LastBaseShader = m_ShaderGraph->m_BaseShader;
			m_NodeEditor->LoadBaseShaderInputs(m_ShaderGraph->GetBaseShaderPath());
		}

		if (!m_InitResetCamera && m_ViewportPanel->GetEditorCamera())
		{
			m_InitResetCamera = true;
			m_ViewportPanel->GetEditorCamera()->SetPosition(Vec3(0.0f, 1.0f, -5.0f));
		}


	}

	Texture* ShaderGraphEditorPanel::GetIconTexture()
	{
		return AssetManager::GetAsset<Texture2D>(SuoraID("10db4794-ec22-46fc-8be9-6cd8e5245349"))->GetTexture();
	}

	void ShaderGraphEditorPanel::SaveAsset()
	{
		Super::SaveAsset();

		for (Ref<VisualNode> node : m_NodeEditor->m_Graph->m_Nodes)
		{
			// TODO: Remove.
			//SuoraWarn(node->m_NodeID);
		}
		GenerateShaderGraphSource();
		Yaml::Node root;
		m_NodeEditor->m_Graph->SerializeNodeGraph(root);
		m_ShaderGraph->Serialize(root);
		std::string out;
		Yaml::Serialize(root, out);
		Platform::WriteToFile(m_ShaderGraph->m_Path.string(), out);
	}

	/*** ShaderGraphNodeEditor ***/

	void ShaderGraphNodeEditor::LoadBaseShaderInput(BaseShaderInput& input, int64_t& begin, int64_t& end, const std::string& str)
	{
		while (str[begin++] != '"');
		end = begin + 1;
		while (str[end++] != '"');
		input.m_Label = str.substr(begin, end - begin - 1);

		begin = end;
		while (str[begin++] != ',');
		end = begin;
		while (str[end++] != ',');
		input.m_Type = ShaderNodeGraph::StringToShaderGraphDataType(str.substr(begin, end - begin - 1));

		begin = end;
		while (str[begin++] != '{');
		end = begin;
		while (str[end++] != '}');
		input.m_DefaultSource = str.substr(begin, end - begin - 2);
	}

	void ShaderGraphNodeEditor::LoadBaseShaderInputs(const std::string& path)
	{
		std::string str = Platform::ReadFromFile(path);
		Util::RemoveCommentsFromString(str);
		int64_t begin = 0, end = 0;
		m_BaseShaderInputs.Clear();
		while (true)
		{
			begin = str.find("$VERT_INPUT", begin);
			if (begin == std::string::npos) break;
			BaseShaderInput input;
			input.m_InVertexShader = true;
			LoadBaseShaderInput(input, begin, end, str);
			m_BaseShaderInputs.Add(input);
		}
		begin = 0;
		while (true)
		{
			begin = str.find("$FRAG_INPUT", begin);
			if (begin == std::string::npos) break;
			BaseShaderInput input;
			input.m_InVertexShader = false;
			LoadBaseShaderInput(input, begin, end, str);
			m_BaseShaderInputs.Add(input);
		}
	}

	void ShaderGraphNodeEditor::GenerateShaderInput(std::string& str, int64_t begin, VisualNode* master, bool vertex, bool& error)
	{
		int64_t labelBegin = begin;
		int64_t end = begin;
		while (str[labelBegin++] != '"');
		end = labelBegin + 1;
		while (str[end++] != '"');
		std::string label = str.substr(labelBegin, end - labelBegin - 1);
		int brackets = 1;
		while (brackets > 0)
		{
			if (str[end] == '(') brackets++;
			if (str[end] == ')') brackets--;
			end++;
		}
		str.erase(begin, end - begin);

		for (VisualNodePin& pin : master->m_InputPins)
		{
			if (pin.Label == label)
			{
				for (BaseShaderInput& input : m_BaseShaderInputs)
				{
					if (pin.PinID == (int64_t)input.m_Type && pin.Label == input.m_Label && input.m_InVertexShader == vertex)
					{
						if (pin.Target)
						{
							std::string src = ShaderGraphCompiler::CompileShaderNode(*(pin.Target->GetNode()), *(pin.Target), vertex, error);
							str.insert(begin, src);
							return;
						}
						else
						{
							str.insert(begin, input.m_DefaultSource);
							return;
						}
					}
				}
				return;
			}
		}
	}

	void ShaderGraphNodeEditor::OnNodeGraphRender(float deltaTime)
	{
		PinIndex = 0;
	}

	void ShaderGraphNodeEditor::DrawVisualNode(VisualNode& node)
	{
		DefaultDrawVisualNode(node);
		// MasterNode
		if (node.m_NodeID == 1)
		{
			for (int i = node.m_InputPins.Size() - 1; i >= 0; i--)
			{
				bool hasInput = false;
				for (BaseShaderInput& input : m_BaseShaderInputs)
				{
					if (node.m_InputPins[i].PinID == (int64_t)input.m_Type && node.m_InputPins[i].Label == input.m_Label) hasInput = true;
				}
				if (!hasInput) node.m_InputPins.RemoveAt(i);
			}
			for (BaseShaderInput& input : m_BaseShaderInputs)
			{
				bool hasPin = false;
				for (VisualNodePin& pin : node.m_InputPins)
				{
					if (pin.PinID == (int64_t)input.m_Type && pin.Label == input.m_Label) hasPin = true;
				}
				if (!hasPin) node.AddInputPin(input.m_Label, ShaderNodeGraph::GetShaderDataTypeColor(input.m_Type), (int64_t)input.m_Type, true);
			}
		}
		// Uniform
		if (node.m_NodeID == 2)
		{
			node.m_Color = ShaderNodeGraph::GetShaderDataTypeColor((ShaderGraphDataType)(int64_t)std::stoi(node.m_InputPins[1].m_AdditionalData));
		}
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
			const float temp = Util::StringToFloat(pin.m_AdditionalData);
			m_TempDragFloatFields[PinIndex] = temp;
			EditorUI::DragFloat(&m_TempDragFloatFields[PinIndex], node.m_Position.x * m_Zoom - node.m_Size.x / 2.0f * m_Zoom - m_CameraPos.x * m_Zoom + GetWidth() / 2 + 5.0f + (GetNodeGraphFont()->GetStringWidth(pin.Label, 26.0f * m_Zoom) / 1.9f + (38.0f * m_Zoom)), y + 2.0f, 100.0f * m_Zoom, pin.PinHeight * m_Zoom - 4.0f, [&](std::string str) { pin.m_AdditionalData = str; });
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