#include "NodeClassEditor.h"
#include "Suora/Assets/AssetManager.h"
#include "Suora/Serialization/Yaml.h"
#include "Suora/NodeScript/ScriptTypes.h"
#include "Suora/NodeScript/Scripting/ScriptVM.h"
#include "Editor/Panels/Minor/ViewportPanel.h"
#include "Editor/Util/EditorCamera.h"
#include "Editor/Panels/Minor/NodeGraphEditor.h"
#include "Editor/Panels/Minor/LevelOutliner.h"
#include "Editor/Panels/Minor/EditorConsolePanel.h"
#include "Suora/NodeScript/BlueprintNodeGraph.h"

#define NODE_ID_EVENT 14
#define NODE_ID_NATIVE_FUNC 15
#define NODE_ID_NATIVE_PURE 16


namespace Suora
{

	class NodeClassNodeGraph : public NodeGraphEditor
	{
		std::unordered_map<VisualNodePin*, Vec3> m_PinVec3;
		std::unordered_map<VisualNodePin*, float> m_PinFloat;
		int m_GraphIndex;
	public:
		Blueprint* m_BlueprintClass = nullptr;

		NodeClassNodeGraph(MajorTab* majorTab, Blueprint* blueprint, int index)
			: NodeGraphEditor(majorTab, CreateRef<BlueprintNodeGraph>(blueprint)), m_BlueprintClass(blueprint), m_GraphIndex(index)
		{
			m_InvertDragPinForIDs.Add(1);
			m_Graph->DeserializeNodeGraph(m_BlueprintClass->m_YamlNode_EditorOnly["Node"]["Graphs"][std::to_string(m_GraphIndex)]);
		}
		void OnNodeGraphRender(float deltaTime) override
		{
			Name = m_BlueprintClass->m_YamlNode_EditorOnly["Node"]["Graphs"][std::to_string(m_GraphIndex)]["Label"].As<String>();
		}
		void DrawVisualNode(VisualNode& node) override
		{
			DefaultDrawVisualNode(node);
		}
		float DrawVisualNodePin(VisualNode& node, VisualNodePin& pin, bool inputPin, float y) override
		{
			DefaultDrawVisualNodePin(node, pin, inputPin, y);

			// Self Input
			if (pin.PinID == (int64_t)ScriptDataType::ObjectPtr && pin.IsReceivingPin && !pin.Target)
			{
				if (Class(m_BlueprintClass).Inherits(Class::FromString(pin.m_AdditionalData)))
				{
					EditorUI::Text("[Self]", Font::Instance, node.m_Position.x * m_Zoom - node.m_Size.x / 2.0f * m_Zoom - m_CameraPos.x * m_Zoom + GetWidth() / 2 + 5.0f + (GetNodeGraphFont()->GetStringWidth(pin.Label, 26.0f * m_Zoom) / 1.9f + (38.0f * m_Zoom)), y + 2.0f, 35.0f * m_Zoom, pin.PinHeight * m_Zoom - 4.0f, 24.0f * m_Zoom, Vec2(-1.0f, 0.0f), Color(0.5f));
				}
			}
			// Float Input
			if (pin.PinID == (int64_t)ScriptDataType::Float && pin.IsReceivingPin && !pin.Target)
			{
				if (m_PinFloat.find(&pin) == m_PinFloat.end()) m_PinFloat[&pin] = StringUtil::StringToFloat(pin.m_AdditionalData);
				const float temp = StringUtil::StringToFloat(pin.m_AdditionalData);
				EditorUI::DragFloat(&m_PinFloat[&pin], node.m_Position.x * m_Zoom - node.m_Size.x / 2.0f * m_Zoom - m_CameraPos.x * m_Zoom + GetWidth() / 2 + 5.0f + (GetNodeGraphFont()->GetStringWidth(pin.Label, 26.0f * m_Zoom) / 1.9f + (38.0f * m_Zoom)), y + 2.0f, 100.0f * m_Zoom, pin.PinHeight * m_Zoom - 4.0f, [&](String str) { pin.m_AdditionalData = str; });
				if (m_PinFloat[&pin] != temp)
				{
					pin.m_AdditionalData = std::to_string(m_PinFloat[&pin]);
				}
			}
			// Bool Input
			if (pin.PinID == (int64_t)ScriptDataType::Bool && pin.IsReceivingPin && !pin.Target)
			{
				if (pin.m_AdditionalData != "true" && pin.m_AdditionalData != "false") pin.m_AdditionalData = "true";
				bool b = pin.m_AdditionalData == "true";
				const bool temp = b;
				EditorUI::Checkbox(&b, node.m_Position.x * m_Zoom - node.m_Size.x / 2.0f * m_Zoom - m_CameraPos.x * m_Zoom + GetWidth() / 2 + 5.0f + (GetNodeGraphFont()->GetStringWidth(pin.Label, 26.0f * m_Zoom) / 1.9f + (38.0f * m_Zoom)), y + 2.0f, pin.PinHeight * m_Zoom - 4.0f, pin.PinHeight * m_Zoom - 4.0f);
				if (b != temp)
				{
					pin.m_AdditionalData = b ? "true" : "false";
				}
			}
			// Vec3 Input
			if (pin.PinID == (int64_t)ScriptDataType::Vec3 && pin.IsReceivingPin && !pin.Target)
			{
				if (m_PinVec3.find(&pin) == m_PinVec3.end()) m_PinVec3[&pin] = Vec::FromString<Vec3>(pin.m_AdditionalData);
				Vec3& vec = m_PinVec3[&pin];

				EditorUI::DragFloat(&vec.x, node.m_Position.x * m_Zoom - node.m_Size.x / 2.0f * m_Zoom - m_CameraPos.x * m_Zoom + GetWidth() / 2 + 5.0f, y + 2.0f - pin.PinHeight * m_Zoom, 50.0f * m_Zoom, pin.PinHeight * m_Zoom - 4.0f, [&](String str) { pin.m_AdditionalData = str; });
				EditorUI::DragFloat(&vec.y, node.m_Position.x * m_Zoom - node.m_Size.x / 2.0f * m_Zoom - m_CameraPos.x * m_Zoom + GetWidth() / 2 + 5.0f + 50 * m_Zoom, y + 2.0f - pin.PinHeight * m_Zoom, 50.0f * m_Zoom, pin.PinHeight * m_Zoom - 4.0f, [&](String str) { pin.m_AdditionalData = str; });
				EditorUI::DragFloat(&vec.z, node.m_Position.x * m_Zoom - node.m_Size.x / 2.0f * m_Zoom - m_CameraPos.x * m_Zoom + GetWidth() / 2 + 5.0f + 100 * m_Zoom, y + 2.0f - pin.PinHeight * m_Zoom, 50.0f * m_Zoom, pin.PinHeight * m_Zoom - 4.0f, [&](String str) { pin.m_AdditionalData = str; });

				pin.m_AdditionalData = Vec::ToString(vec);
				return pin.PinHeight * m_Zoom;
			}

			return 0.0f;
			//return 5.0f * m_Zoom;
		}
		void ProccessNodePinIDConversion(VisualNodePin& receivingPin, VisualNodePin& targetPin) override
		{
		}

		friend class NodeClassEditor;
	};

	NodeClassEditor::NodeClassEditor()
	{
		m_AssetClass = Blueprint::StaticClass();
	}
	NodeClassEditor::NodeClassEditor(Blueprint* blueprint)
	{
		m_AssetClass = Blueprint::StaticClass();
		Init();

	}

	void NodeClassEditor::ResetEnvironment()
	{
		m_World = CreateRef<World>();
		m_Actor = m_BlueprintClass->CreateInstance(true)->As<Node>();
		m_Actor->InitializeNode(*m_World);
		m_Actor->m_Name = m_Name;

		m_SelectedObject = nullptr;
		if (m_ViewportPanel)
		{
			m_ViewportPanel->m_World = m_World.get();
		}
		if (m_DetailsPanel)
		{
			m_DetailsPanel->m_Data = nullptr;
		}
		if (m_NodeOutliner)
		{
			m_NodeOutliner->m_RootNode = nullptr;
		}
	}

	void NodeClassEditor::Init()
	{
		Super::Init();

		SuoraVerify(m_Asset);
		m_Name = m_Asset->As<Blueprint>()->GetAssetName();
		m_BlueprintClass = m_Asset->As<Blueprint>();

		ResetEnvironment();
		m_SelectedObject = m_BlueprintClass;

		m_ViewportPanel = CreateRef<ViewportPanel>(this, m_World.get()); m_ViewportPanel->m_DrawDebugGizmos = true;
		m_DetailsPanel = CreateRef<DetailsPanel>(this);
		m_DetailsPanel->m_Data = m_BlueprintClass;
		m_NodeOutliner = CreateRef<LevelOutliner>(this);

		Ref<DockingSpace> ds1 = CreateRef<DockingSpace>(0, 0.0f, 0.2f, 0.5f, this);			m_DockspacePanel.m_DockingSpaces.Add(ds1); ds1->m_MinorTabs.Add(m_DetailsPanel);
		Ref<DockingSpace> ds2 = CreateRef<DockingSpace>(0, 0.5f, 0.2f, 1.0f, this);			m_DockspacePanel.m_DockingSpaces.Add(ds2); ds2->m_MinorTabs.Add(m_NodeOutliner);
		m_MainDockingSpace = CreateRef<DockingSpace>(0.2f, 0.0f, 1.0f, 1.0f, this);			m_DockspacePanel.m_DockingSpaces.Add(m_MainDockingSpace); m_MainDockingSpace->m_MinorTabs.Add(m_ViewportPanel);

	}

	NodeClassEditor::~NodeClassEditor()
	{

	}
	
	void NodeClassEditor::Update(float deltaTime)
	{
		Super::Update(deltaTime);
		
		// Remove possible duplicate Names...
		{
			m_NodeNameUpdateIndex %= m_World->GetAllNodes().Size();
			Node* temp = m_World->GetAllNodes()[m_NodeNameUpdateIndex++];
			temp->SetName(temp->GetName());
		}
		m_World->ResolvePendingKills();

		m_DetailsPanel->m_Data = m_SelectedObject;

		if (!m_InitResetCamera && m_ViewportPanel->GetEditorCamera())
		{
			m_InitResetCamera = true;
			m_ViewportPanel->GetEditorCamera()->SetPosition(Vec3(0.0f, 1.0f, -5.0f));
		}

		
		if (NativeInput::GetKeyDown(Key::Delete) && m_SelectedObject && m_SelectedObject->IsA<Node>())
		{
			if (m_SelectedObject->As<Node>()->m_IsActorLayer)
			{
				if (m_SelectedObject == m_DetailsPanel->m_Data) m_DetailsPanel->m_Data = nullptr;
				m_SelectedObject->As<Node>()->Destroy();
				m_SelectedObject = nullptr;
			}
		}
	}

	void NodeClassEditor::ApplyChangesOfOtherMajorTabs(MajorTab* other)
	{
		SerializeActor();
		ResetEnvironment();
	}

	Texture* NodeClassEditor::GetIconTexture()
	{
		return EditorUI::GetClassIcon(m_BlueprintClass->m_ParentClass)->GetTexture();
	}

	void NodeClassEditor::SaveAsset()
	{
		Super::SaveAsset();

		SerializeActor();

		SerializeAllNodeGraphs();

		Yaml::Node root;
		m_BlueprintClass->Serialize(root);
		String out;
		Yaml::Serialize(root, out);
		Platform::WriteToFile(m_BlueprintClass->m_Path.string(), out);

		GetEditorWindow()->DelegateChanges(this);
	}

	void NodeClassEditor::SerializeActor()
	{
		m_BlueprintClass->m_Composition = Yaml::Node();
		m_Actor->Serialize(m_BlueprintClass->m_Composition);
	}
	void NodeClassEditor::SerializeAllNodeGraphs()
	{
		for (Ref<NodeGraphEditor> It : m_NodeGraphs)
		{
			It->m_Graph->SerializeNodeGraph(m_BlueprintClass->m_YamlNode_EditorOnly["Node"]["Graphs"][std::to_string(((NodeClassNodeGraph*)It.get())->m_GraphIndex)]);
		}
	}

	void NodeClassEditor::PlayInEditor()
	{
		Engine::Get()->CreateGameInstance();
		GameInstance* game = Engine::Get()->GetGameInstance();

		m_Actor->Serialize(m_BlueprintClass->m_Composition);

		SerializeAllNodeGraphs();

		Yaml::Node root;
		m_BlueprintClass->Serialize(root);

		World* world = game->LoadLevel(m_BlueprintClass);

		game->SwitchToWorld(world);

		m_CurrentPlayState = PlayState::Playing;
	}

	void NodeClassEditor::StopPlayInEditor()
	{
		Engine::Get()->DisposeGameInstance();

		GetEditorWindow()->GetWindow()->SetCursorLocked(false);
		m_CurrentPlayState = PlayState::Editor;
		m_SelectedObject = nullptr;
		m_DetailsPanel->m_Data = nullptr;
	}
	World* NodeClassEditor::GetEditorWorld()
	{
		return (m_CurrentPlayState == PlayState::Editor) ? m_World.get() : Engine::Get()->GetGameInstance()->GetCurrentWorld();
	}

	void NodeClassEditor::DrawToolbar(float& x, float y, float height)
	{
		Super::DrawToolbar(x, y, height);
		EditorUI::ButtonParams Params;
		Params.TextOrientation = Vec2(0.8f, 0.0f);
		x += 15;

		Params.TooltipText = "Compile this Blueprints VisualScript Code";
		if (EditorUI::Button("", x, y + height * 0.1f, height * 0.8f, height * 0.8f, Params))
		{
			SerializeActor();
			SerializeAllNodeGraphs();
			
			BlueprintCompiler::Compile(*m_BlueprintClass);
		}
		EditorUI::DrawTexturedRect(Icon::Cogwheel, x, y + height * 0.1f, height * 0.8f, height * 0.8f, 4, Color(1));
		EditorUI::DrawTexturedRect(Icon::Tickmark, x + height * 0.4f, y + height * 0.1f, height * 0.4f, height * 0.4f, 4, EditorPreferences::Get()->UiHighlightColor);
		x += height * 0.8f;

		x += 15;
		EditorUI::ButtonParams BackgroundParams;
		BackgroundParams.ButtonColorHover = BackgroundParams.ButtonColor;
		BackgroundParams.HoverCursor = Cursor::Default;
		EditorUI::Button("", x, y + height * 0.1f, height * 2.7f, height * 0.8f, BackgroundParams);

		x += height * 0.15f;
		Params.TooltipText = m_CurrentPlayState == PlayState::Editor ? "Play in Editor" : "Stop PIE";
		if (EditorUI::Button("", x, y + height * 0.15f, height * 0.7f, height * 0.7f, Params) || (m_CurrentPlayState != PlayState::Editor && NativeInput::GetKeyDown(Key::Escape) && !NativeInput::GetKey(Key::F1)) || NativeInput::GetKeyDown(Key::F5))
		{
			if (m_CurrentPlayState == PlayState::Editor)
			{
				PlayInEditor();
			}
			else
			{
				StopPlayInEditor();
			}
			m_SelectedObject = nullptr;
			m_DetailsPanel->m_Data = nullptr;
		}
		EditorUI::DrawTexturedRect(m_CurrentPlayState == PlayState::Editor ? Icon::Play : Icon::Stop, x, y + height * 0.15f, height * 0.7f, height * 0.7f, 4, Color(1));
		x += height * 0.7f;

		x += height * 0.15f;
		if (m_CurrentPlayState == PlayState::Editor)
		{
			Params.TooltipText = "Simulate PIE";
			if (EditorUI::Button("", x, y + height * 0.15f, height * 0.7f, height * 0.7f, Params) || NativeInput::GetKeyDown(Key::F6))
			{
				PlayInEditor();
				m_CurrentPlayState = PlayState::Simulating;
				m_SelectedObject = nullptr;
				m_DetailsPanel->m_Data = nullptr;
			}
			EditorUI::DrawTexturedRect(Icon::World, x, y + height * 0.15f, height * 0.7f, height * 0.7f, 4, Color(1));
		}
		else
		{
			if (m_CurrentPlayState == PlayState::Playing)
			{
				Params.TooltipText = "Eject";
				if (EditorUI::Button("", x, y + height * 0.15f, height * 0.7f, height * 0.7f, Params) || NativeInput::GetKeyDown(Key::F6))
				{
					m_CurrentPlayState = PlayState::Simulating;
				}
				EditorUI::DrawTexturedRect(Icon::ArrowUp, x, y + height * 0.15f, height * 0.7f, height * 0.7f, 4, Color(1));
			}
			else if (m_CurrentPlayState == PlayState::Simulating)
			{
				Params.TooltipText = "Inject";
				if (EditorUI::Button("", x, y + height * 0.15f, height * 0.7f, height * 0.7f, Params) || NativeInput::GetKeyDown(Key::F6))
				{
					m_CurrentPlayState = PlayState::Playing;
				}
				EditorUI::DrawTexturedRect(Icon::ArrowDown, x, y + height * 0.15f, height * 0.7f, height * 0.7f, 4, Color(1));
			}
		}
		x += height * 0.7f;

		x += height * 0.15f;
		Params.TooltipText = "Continue";
		if (EditorUI::Button("", x, y + height * 0.15f, height * 0.7f, height * 0.7f, Params) || NativeInput::GetKeyDown(Key::F7))
		{
			/* Placeholder */
		}
		EditorUI::DrawTexturedRect(Icon::Continue, x, y + height * 0.15f, height * 0.7f, height * 0.7f, 4, Color(0.5f));
		x += height * 0.7f;

		x += 20.0f;
		if (m_SelectedObject == m_BlueprintClass)
		{
			Params = EditorUI::ButtonParams::Outlined();
			Params.TextOrientation = Vec2(0.8f, 0.0f);
		}
		Params.TooltipText = "Edit this Blueprint Class";
		if (EditorUI::Button("Blueprint", x, y + height * 0.1f, height * 0.8f + 90.0f, height * 0.8f, Params))
		{
			m_SelectedObject = m_BlueprintClass;
		}
		EditorUI::DrawTexturedRect(Icon::Cogwheel, x, y + height * 0.1f, height * 0.8f, height * 0.8f, 4, (m_SelectedObject == m_BlueprintClass) ? EditorPreferences::Get()->UiHighlightColor : Color(1));
		x += height * 0.8f + 90.0f;
	}

	Ref<NodeGraphEditor> NodeClassEditor::CreateNodeClassGraphEditorInstance(Blueprint* blueprint, int i)
	{
		return Ref<NodeGraphEditor>(new NodeClassNodeGraph(this, blueprint, i));
	}
	void NodeClassEditor::OpenNodeGraph(Blueprint* blueprint, int i)
	{
		Ref<NodeClassNodeGraph> graph = CreateRef<NodeClassNodeGraph>(this, blueprint, i);
		m_MainDockingSpace->m_MinorTabs.Add(graph);
		m_NodeGraphs.Add(graph);
		m_MainDockingSpace->m_SelectedMinorTab = m_MainDockingSpace->m_MinorTabs.Last();
	}

}