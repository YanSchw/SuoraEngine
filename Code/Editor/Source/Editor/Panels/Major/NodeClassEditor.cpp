#include "NodeClassEditor.h"
#include "Suora/Assets/AssetManager.h"
#include "Suora/Serialization/Yaml.h"
#include "Suora/NodeScript/ScriptTypes.h"
#include "Suora/NodeScript/Scripting/ScriptVM.h"
#include "../Minor/ViewportPanel.h"
#include "../../Util/EditorCamera.h"
#include "../Minor/NodeGraphEditor.h"
#include "../Minor/LevelOutliner.h"
#include "../Minor/DebugPanels.h"
#include "../Minor/EditorConsolePanel.h"
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
			Name = m_BlueprintClass->m_YamlNode_EditorOnly["Node"]["Graphs"][std::to_string(m_GraphIndex)]["Label"].As<std::string>();
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
				if (m_PinFloat.find(&pin) == m_PinFloat.end()) m_PinFloat[&pin] = Util::StringToFloat(pin.m_AdditionalData);
				const float temp = Util::StringToFloat(pin.m_AdditionalData);
				EditorUI::DragFloat(&m_PinFloat[&pin], node.m_Position.x * m_Zoom - node.m_Size.x / 2.0f * m_Zoom - m_CameraPos.x * m_Zoom + GetWidth() / 2 + 5.0f + (GetNodeGraphFont()->GetStringWidth(pin.Label, 26.0f * m_Zoom) / 1.9f + (38.0f * m_Zoom)), y + 2.0f, 100.0f * m_Zoom, pin.PinHeight * m_Zoom - 4.0f, [&](std::string str) { pin.m_AdditionalData = str; });
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

				EditorUI::DragFloat(&vec.x, node.m_Position.x * m_Zoom - node.m_Size.x / 2.0f * m_Zoom - m_CameraPos.x * m_Zoom + GetWidth() / 2 + 5.0f, y + 2.0f - pin.PinHeight * m_Zoom, 50.0f * m_Zoom, pin.PinHeight * m_Zoom - 4.0f, [&](std::string str) { pin.m_AdditionalData = str; });
				EditorUI::DragFloat(&vec.y, node.m_Position.x * m_Zoom - node.m_Size.x / 2.0f * m_Zoom - m_CameraPos.x * m_Zoom + GetWidth() / 2 + 5.0f + 50 * m_Zoom, y + 2.0f - pin.PinHeight * m_Zoom, 50.0f * m_Zoom, pin.PinHeight * m_Zoom - 4.0f, [&](std::string str) { pin.m_AdditionalData = str; });
				EditorUI::DragFloat(&vec.z, node.m_Position.x * m_Zoom - node.m_Size.x / 2.0f * m_Zoom - m_CameraPos.x * m_Zoom + GetWidth() / 2 + 5.0f + 100 * m_Zoom, y + 2.0f - pin.PinHeight * m_Zoom, 50.0f * m_Zoom, pin.PinHeight * m_Zoom - 4.0f, [&](std::string str) { pin.m_AdditionalData = str; });

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
		m_Actor = m_BlueprintClass->CreateInstance(true, true, true)->As<Node>();
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
			if (IObjectCompositionData* obj = m_SelectedObject->As<Node>()->GetInterface<IObjectCompositionData>())
			{
				if (obj->m_IsActorLayer)
				{
					if (m_SelectedObject == m_DetailsPanel->m_Data) m_DetailsPanel->m_Data = nullptr;
					m_SelectedObject->As<Node>()->Destroy();
					m_SelectedObject = nullptr;
				}
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
		std::string out;
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
		GetEditorWindow()->m_InputEvent = EditorInputEvent::None;
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

		if (EditorUI::Button("Compile", x + 15, y + height * 0.1f, height * 0.8f + 70.0f, height * 0.8f, Params))
		{
			SerializeActor();
			SerializeAllNodeGraphs();
			
			BlueprintCompiler::Compile(*m_BlueprintClass);
		}
		EditorUI::DrawTexturedRect(AssetManager::GetAsset<Texture2D>(SuoraID("3e254a4e-cc83-4254-a462-73739fce6d61"))->GetTexture(), x + 15, y + height * 0.1f, height * 0.8f, height * 0.8f, 4, Color(1));
		x += 15 + height * 0.8f + 70.0f;

		if (EditorUI::Button(m_CurrentPlayState == PlayState::Editor ? "Play" : "Stop", x + 15, y + height * 0.1f, height * 0.8f + 50.0f, height * 0.8f, Params) || (m_CurrentPlayState != PlayState::Editor && NativeInput::GetKeyDown(Key::Escape) && !NativeInput::GetKey(Key::F1)) || NativeInput::GetKeyDown(Key::F5))
		{
			if (m_CurrentPlayState == PlayState::Editor)
			{
				GetEditorWindow()->m_InputEvent = EditorInputEvent::Viewport_PlayMode;
				PlayInEditor();
			}
			else
			{
				GetEditorWindow()->m_InputEvent = EditorInputEvent::None;
				StopPlayInEditor();
			}
			m_SelectedObject = nullptr;
			m_DetailsPanel->m_Data = nullptr;
		}
		EditorUI::DrawTexturedRect(m_CurrentPlayState == PlayState::Editor ? AssetManager::GetAsset<Texture2D>(SuoraID("c503c57f-100f-4e0d-8f27-38259b174ba8"))->GetTexture() : AssetManager::GetAsset<Texture2D>(SuoraID("bf120f8b-0aca-4e28-993e-ba1e9ce4e693"))->GetTexture(), x + 15, y + height * 0.1f, height * 0.8f, height * 0.8f, 4, Color(1));
		x += 15 + height * 0.8f + 50.0f;

		if (m_CurrentPlayState == PlayState::Editor)
		{
			if (EditorUI::Button("Simulate", x + 15, y + height * 0.1f, height * 0.8f + 90.0f, height * 0.8f, Params) || NativeInput::GetKeyDown(Key::F6))
			{
				PlayInEditor();
				m_CurrentPlayState = PlayState::Simulating;
				m_SelectedObject = nullptr;
				m_DetailsPanel->m_Data = nullptr;
			}
			EditorUI::DrawTexturedRect(AssetManager::GetAsset<Texture2D>(SuoraID("24294e57-bae7-4ff7-a0f2-73f9741069da"))->GetTexture(), x + 15, y + height * 0.1f, height * 0.8f, height * 0.8f, 4, Color(1));
		}
		else
		{
			EditorUI::DrawRect(x + 15, y + height * 0.1f, height * 0.8f + 90.0f, height * 0.8f, 4.0f, EditorPreferences::Get()->UiBackgroundColor);
		}
		x += 15 + height * 0.8f + 90.0f;

		if ((m_SelectedObject == m_BlueprintClass))
		{
			Params = EditorUI::ButtonParams::Outlined();
			Params.TextOrientation = Vec2(0.8f, 0.0f);
		}
		if (EditorUI::Button("Blueprint", x + 15, y + height * 0.1f, height * 0.8f + 90.0f, height * 0.8f, Params))
		{
			m_SelectedObject = m_BlueprintClass;
		}
		EditorUI::DrawTexturedRect(AssetManager::GetAsset<Texture2D>(SuoraID("3e254a4e-cc83-4254-a462-73739fce6d61"))->GetTexture(), x + 15, y + height * 0.1f, height * 0.8f, height * 0.8f, 4, (m_SelectedObject == m_BlueprintClass) ? EditorPreferences::Get()->UiHighlightColor : Color(1));
		x += 15 + height * 0.8f + 90.0f;
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