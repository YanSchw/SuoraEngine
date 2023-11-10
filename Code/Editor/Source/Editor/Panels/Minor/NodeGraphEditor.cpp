#include "NodeGraphEditor.h"
#include "../MajorTab.h"
#include "../Major/NodeClassEditor.h"
#include "../../EditorWindow.h"
#include "../../Util/EditorPreferences.h"
#include "Suora/Renderer/RenderCommand.h"
#include "Suora/Renderer/RenderPipeline.h"
#include "Suora/Renderer/Framebuffer.h"
#include "Suora/Renderer/Shader.h"
#include "Suora/Renderer/Renderer3D.h"

#define NODEGRAPH_GRID_SNAP 10.0f

namespace Suora
{
	static Font* font = nullptr;
	static int NodeDraggingIndex = 0;
	static Vec2 NodeDraggingGridSnap = Vec2(0.0f);
	static glm::ivec2 buttonDownMousePos;
	static Texture2D* NodeLabelTexture = nullptr;

	struct VisualNodeSearchOverlay : public EditorUI::SelectionOverlay
	{
		NodeGraphEditor& m_Editor;

		VisualNodeSearchOverlay(NodeGraphEditor& editor) : m_Editor(editor) 
		{ 
			s_SearchLabel = "";
			PullNodeEntries(); 
		}

		void PullNodeEntries()
		{
			m_Entries.Clear();
			for (VisualNodeEntry& node : m_Editor.m_Graph->m_SupportedNodes)
			{
				m_Entries.Add(EditorUI::SelectionOverlay::SelectionOverlayEntry(node.m_Node->m_Title, node.m_Tags, [&]() { SelectNodeEntry(&node, m_Editor); }));
			}
			RefreshEntries();
		}
		static void SelectNodeEntry(VisualNodeEntry* entry, NodeGraphEditor& editor)
		{
			Ref<VisualNode> node = std::make_shared<VisualNode>(*entry->m_Node.get());
			editor.m_Graph->m_Nodes.Add(node);
			for (VisualNodePin& pin : node->m_InputPins) { pin.m_Node = node.get(); }
			for (VisualNodePin& pin : node->m_OutputPins) { pin.m_Node = node.get(); }
			node->m_Position = editor.m_MouseCursorPosInGraph;
			std::vector<std::string> strs = StringUtil::SplitString(node->m_Title, '/');
			node->m_Title = strs[strs.size() - 1];
		}
	};

	bool VisualNodePin::HasOtherPin(const Array<Ref<VisualNode>>& nodes)
	{
		if (IsReceivingPin)
		{
			return Target;
		}
		else
		{
			for (Ref<VisualNode> node : nodes)
			{
				for (VisualNodePin& it : node->m_InputPins)
				{
					if (it.IsReceivingPin && it.Target == this) return it.Target;
				}
				for (VisualNodePin& it : node->m_OutputPins)
				{
					if (it.IsReceivingPin && it.Target == this) return it.Target;
				}
			}
		}
		return false;
	}


	NodeGraphEditor::NodeGraphEditor(MajorTab* majorTab, const Ref<VisualNodeGraph>& graph)
		: MinorTab(majorTab)
	{
		m_Graph = graph;

		FramebufferSpecification spec;
		spec.Attachments.Attachments.push_back(FramebufferTextureFormat::RGBA8);
		spec.Attachments.Attachments.push_back(FramebufferTextureFormat::Depth);
		spec.Width = 16;
		spec.Height = 16;
		m_NodeBuffer = Framebuffer::Create(spec);

		Name = "Node Graph";
		m_Checkerboard = Texture::Create(AssetManager::GetEngineAssetPath() + "/EngineContent/Textures/Checkerboard.png");
		m_PinConnectionTexture = Texture::Create(AssetManager::GetEngineAssetPath() + "/EditorContent/Icons/VisualNodePin.png");
		m_PinConnectionTexture2 = Texture::Create(AssetManager::GetEngineAssetPath() + "/EditorContent/Icons/VisualNodePin2.png");
		m_PinConnectionExecTexture = Texture::Create(AssetManager::GetEngineAssetPath() + "/EditorContent/Icons/VisualNodeExecPin.png");
		m_PinConnectionExecTexture2 = Texture::Create(AssetManager::GetEngineAssetPath() + "/EditorContent/Icons/VisualNodeExecPin2.png");
		m_LineShader = Shader::Create(AssetManager::GetEngineAssetPath() + "/EditorContent/Shaders/Line.glsl");
		NodeLabelTexture = AssetManager::GetAssetByName<Texture2D>("NodeLabel.texture");

		font = AssetManager::GetAssetByName<Font>("InterSDF.font");
	}

	NodeGraphEditor::~NodeGraphEditor()
	{

	}

	Font* NodeGraphEditor::GetNodeGraphFont() const
	{
		return font;
	}

	Ref<Texture> NodeGraphEditor::GetPinIconTexture(int64_t pinID, bool hasOtherPin)
	{
		return pinID == 1 ? (hasOtherPin ? m_PinConnectionExecTexture2 : m_PinConnectionExecTexture) : (hasOtherPin ? m_PinConnectionTexture2 : m_PinConnectionTexture);
	}

	void NodeGraphEditor::DefaultDrawVisualNode(VisualNode& node)
	{
		bool Hovering = EditorUI::GetInput().x > (node.m_Position.x * m_Zoom - node.m_Size.x / 2.0f * m_Zoom - m_CameraPos.x * m_Zoom + GetWidth() / 2)
			&& EditorUI::GetInput().x < (node.m_Position.x * m_Zoom - node.m_Size.x / 2.0f * m_Zoom - m_CameraPos.x * m_Zoom + GetWidth() / 2) + node.m_Size.x * m_Zoom
			&& EditorUI::GetInput().y >(node.m_Position.y * m_Zoom + node.m_Size.y / 2.0f * m_Zoom - m_CameraPos.y * m_Zoom + GetHeight() / 2 - 25 * m_Zoom)
			&& EditorUI::GetInput().y < (node.m_Position.y * m_Zoom + node.m_Size.y / 2.0f * m_Zoom - m_CameraPos.y * m_Zoom + GetHeight() / 2 - 25 * m_Zoom) + 25 * m_Zoom + 1;

		// Hover Selection
		Color hoverColor = m_SelectedNodes.Contains(&node) ? EditorPreferences::Get()->UiHighlightColor : Color(1.0f, 1.0f, 1.0f, 0.45f);
		
		EditorUI::ButtonParams MainNodeParams = EditorUI::ButtonParams::DarkerButton();
		MainNodeParams.HoverCursor = Cursor::Default;
		if (Hovering || m_SelectedNodes.Contains(&node))
		{
			MainNodeParams.ButtonOutlineColor = hoverColor;
			MainNodeParams.ButtonOutlineHoverColor = hoverColor;
			MainNodeParams.useButtonOutlineHoverColor = true;
		}
		EditorUI::Button("", node.m_Position.x * m_Zoom - node.m_Size.x / 2.0f * m_Zoom - m_CameraPos.x * m_Zoom + GetWidth() / 2,
			node.m_Position.y * m_Zoom - node.m_Size.y / 2.0f * m_Zoom - m_CameraPos.y * m_Zoom + GetHeight() / 2,
			node.m_Size.x * m_Zoom, node.m_Size.y * m_Zoom, MainNodeParams);
		EditorUI::ButtonParams NodeLabelParams = EditorUI::ButtonParams::DarkerButton();
		NodeLabelParams.ButtonRoundness = -4;
		NodeLabelParams.ButtonColor = Math::Lerp(node.m_Color, Color(0.0f, 0.0f, 0.0f, 1.0f), 0.0f); NodeLabelParams.ButtonColor.a = 1.0f;
		NodeLabelParams.ButtonColorHover = Math::Lerp(NodeLabelParams.ButtonColor, Color(1.0f), 0.15f); NodeLabelParams.ButtonColorHover.a = 1.0f;
		if (Hovering || m_SelectedNodes.Contains(&node))
		{
			NodeLabelParams.ButtonOutlineColor = hoverColor;
			NodeLabelParams.ButtonOutlineHoverColor = hoverColor;
			NodeLabelParams.useButtonOutlineHoverColor = true;
		}
		EditorUI::Button("", node.m_Position.x * m_Zoom - node.m_Size.x / 2.0f * m_Zoom - m_CameraPos.x * m_Zoom + GetWidth() / 2,
			node.m_Position.y * m_Zoom + node.m_Size.y / 2.0f * m_Zoom - m_CameraPos.y * m_Zoom + GetHeight() / 2 - 25 * m_Zoom,
			node.m_Size.x * m_Zoom, 25 * m_Zoom + 1, NodeLabelParams);
		/*EditorUI::DrawTexturedRect(NodeLabelTexture->GetTexture(), node.m_Position.x * m_Zoom - node.m_Size.x / 2.0f * m_Zoom - m_CameraPos.x * m_Zoom + GetWidth() / 2,
			node.m_Position.y * m_Zoom + node.m_Size.y / 2.0f * m_Zoom - m_CameraPos.y * m_Zoom + GetHeight() / 2 - 25 * m_Zoom,
			node.m_Size.x * m_Zoom, 25 * m_Zoom + 1, -4, node.m_Color * Color(1.0f, 1.0f, 1.0f, 0.5f));*/
		
		EditorUI::Text(node.m_Title, font, node.m_Position.x * m_Zoom - node.m_Size.x / 2.0f * m_Zoom - m_CameraPos.x * m_Zoom + GetWidth() / 2,
			node.m_Position.y * m_Zoom + node.m_Size.y / 2.0f * m_Zoom - m_CameraPos.y * m_Zoom + GetHeight() / 2 - 25 * m_Zoom,
			node.m_Size.x * m_Zoom, 25 * m_Zoom + 1, 22 * m_Zoom, Vec2(-0.8f, 0), Color(1.0f));

		if (Hovering && (NativeInput::GetMouseButtonDown(Mouse::ButtonLeft) || NativeInput::GetMouseButtonUp(Mouse::ButtonRight)) && IsInputValid() && IsInputMode(EditorInputEvent::None))
		{
			NodeDraggingIndex = m_Graph->IndexOf(node);
			SetInputMode(EditorInputEvent::NodeGraph_DragNode);
			buttonDownMousePos = glm::ivec2(NativeInput::GetMousePosition());
			if (!NativeInput::GetKey(Key::LeftShift) && !m_SelectedNodes.Contains(m_Graph->m_Nodes[NodeDraggingIndex].get())) m_SelectedNodes.Clear();
		}

		float offset = DrawNodeInputPins(node, -25.0f * m_Zoom);

		node.m_Size.y = Math::Lerp(node.m_Size.y, 25.0f + Math::Abs(offset) / m_Zoom, 0.5f);
	}

	void NodeGraphEditor::DefaultDrawVisualNodePin(VisualNode& node, VisualNodePin& pin, bool inputPin, float y)
	{
		if (pin.PinHeight <= 0.0f) return;

		EditorUI::ButtonParams Params = EditorUI::ButtonParams::DarkerButton();
		Params.TextSize = 18.0f * m_Zoom;
		Params.TextOrientation = Vec2(inputPin ? 1.0f : -1.0f, 0.0f);
		Params.Font = font;
		/*Params.ButtonColor = Color(0.0f);
		Params.ButtonOutlineColor = Color(0.0f);
		Params.ButtonColorClicked = Color(0.0f);*/
		Params.ButtonColorHover = Math::Lerp(Params.ButtonColorHover, pin.Color, 0.25f); // Color(pin.Color.r, pin.Color.g, pin.Color.b, 1.0f);
		//Params.ButtonRoundness = pin.PinHeight / 2.0f;
		Params.TooltipText = pin.PinID == (int64_t)ScriptDataType::ObjectPtr ? pin.m_AdditionalData : pin.Tooltip;
		if (m_DragPin)
		{
			Params.OverrideActivationEvent = true;
			Params.InputMode = EditorInputEvent::NodeGraph_DragWire;
			Params.OverrittenActivationEvent = []() { return !NativeInput::GetMouseButton(Mouse::ButtonLeft); };
		}
		else
		{
			Params.OverrideActivationEvent = true;
			Params.InputMode = EditorInputEvent::None;
			Params.OverrittenActivationEvent = []() { return NativeInput::GetMouseButtonDown(Mouse::ButtonLeft) || NativeInput::GetMouseButtonDown(Mouse::ButtonMiddle); };
		}
		if (inputPin)
		{
			if (EditorUI::Button("", node.m_Position.x * m_Zoom - node.m_Size.x / 2.0f * m_Zoom - m_CameraPos.x * m_Zoom + GetWidth() / 2.0f + 10.0f, y, font->GetStringWidth(pin.Label, 26.0f * m_Zoom) / 1.9f + (38.0f * m_Zoom), pin.PinHeight * m_Zoom, Params))
			{
				if (NativeInput::GetMouseButtonDown(Mouse::ButtonMiddle))
				{
					m_Graph->ClearNodePin(pin);
					return;
				}
				if (!m_DragPin)
				{
					SetInputMode(EditorInputEvent::NodeGraph_DragWire);
					m_DragPin = &pin;
				}
				else ConnectNodePins(*m_DragPin, pin);
			}
			EditorUI::ButtonParams _LabelParams = EditorUI::ButtonParams::Invisible();
			_LabelParams.TextOrientation = Vec2(-1.0f, 0.0f);
			_LabelParams.TextSize = 18.0f * m_Zoom;
			EditorUI::Button(pin.Label, node.m_Position.x * m_Zoom - node.m_Size.x / 2.0f * m_Zoom - m_CameraPos.x * m_Zoom + GetWidth() / 2.0f + 32.0f * m_Zoom, y, font->GetStringWidth(pin.Label, 26.0f * m_Zoom) / 1.9f + (25.0f * m_Zoom), pin.PinHeight * m_Zoom, _LabelParams);
			Ref<Texture> texture = GetPinIconTexture(pin.PinID, pin.HasOtherPin(m_Graph->m_Nodes));
			EditorUI::DrawTexturedRect(texture, node.m_Position.x * m_Zoom - node.m_Size.x / 2.0f * m_Zoom - m_CameraPos.x * m_Zoom + GetWidth() / 2.0f, y + (((pin.PinHeight * m_Zoom) - (25.0f * m_Zoom)) / 2.0f), 25.0f * m_Zoom, 25.0f * m_Zoom, 0.0f, pin.Color);
			pin.PinConnectionPoint = Vec2(node.m_Position.x * m_Zoom - node.m_Size.x / 2.0f * m_Zoom - m_CameraPos.x * m_Zoom + GetWidth() / 2.0f + 12.5f * m_Zoom, y + (((pin.PinHeight * m_Zoom) - (25.0f * m_Zoom)) / 2.0f) + 12.5f * m_Zoom);
		}
		else
		{
			if (EditorUI::Button("", node.m_Position.x * m_Zoom + node.m_Size.x / 2.0f * m_Zoom - m_CameraPos.x * m_Zoom - (font->GetStringWidth(pin.Label, 26.0f * m_Zoom) / 1.9f + (38.0f * m_Zoom)) + GetWidth() / 2.0f - 10.0f, y, font->GetStringWidth(pin.Label, 26.0f * m_Zoom) / 1.9f + (38.0f * m_Zoom), pin.PinHeight * m_Zoom, Params))
			{
				if (NativeInput::GetMouseButtonDown(Mouse::ButtonMiddle))
				{
					m_Graph->ClearNodePin(pin);
					return;
				}
				if (!m_DragPin)
				{
					SetInputMode(EditorInputEvent::NodeGraph_DragWire);
					m_DragPin = &pin;
				}
				else ConnectNodePins(*m_DragPin, pin);
			}
			EditorUI::ButtonParams _LabelParams = EditorUI::ButtonParams::Invisible();
			_LabelParams.TextOrientation = Vec2(1.0f, 0.0f);
			_LabelParams.TextSize = 18.0f * m_Zoom;
			EditorUI::Button(pin.Label, node.m_Position.x * m_Zoom + node.m_Size.x / 2.0f * m_Zoom - m_CameraPos.x * m_Zoom - (font->GetStringWidth(pin.Label, 26.0f * m_Zoom) / 1.9f + (25.0f * m_Zoom)) + GetWidth() / 2.0f - 32.0f * m_Zoom, y, font->GetStringWidth(pin.Label, 26.0f * m_Zoom) / 1.9f + (25.0f * m_Zoom), pin.PinHeight * m_Zoom, _LabelParams);
			Ref<Texture> texture = GetPinIconTexture(pin.PinID, pin.HasOtherPin(m_Graph->m_Nodes));
			EditorUI::DrawTexturedRect(texture, node.m_Position.x * m_Zoom + node.m_Size.x / 2.0f * m_Zoom - m_CameraPos.x * m_Zoom - 25.0f * m_Zoom + GetWidth() / 2.0f, y + (((pin.PinHeight * m_Zoom) - (25.0f * m_Zoom)) / 2.0f), 25.0f * m_Zoom, 25.0f * m_Zoom, 0.0f, pin.Color);
			pin.PinConnectionPoint = Vec2(node.m_Position.x * m_Zoom + node.m_Size.x / 2.0f * m_Zoom - m_CameraPos.x * m_Zoom - 25.0f * m_Zoom + GetWidth() / 2.0f + 12.5f * m_Zoom, y + (((pin.PinHeight * m_Zoom) - (25.0f * m_Zoom)) / 2.0f) + 12.5f * m_Zoom);
		}
	}

	void NodeGraphEditor::Render(float deltaTime)
	{
		if (m_Framebuffer->GetSize() != m_NodeBuffer->GetSize())
		{
			m_NodeBuffer->Resize(m_Framebuffer->GetSize());
		}
		m_NodeBuffer->Bind();
		RenderCommand::SetClearColor(Color(0.0f));
		RenderCommand::Clear();
		m_Framebuffer->Bind();

		m_Zoom = Math::LerpTowards(m_Zoom, m_ZoomTarget, deltaTime * 25.0f);
		m_CameraPos = Math::LerpTowards(m_CameraPos, m_CameraPosTarget, deltaTime * 50.0f);
		m_Graph->FixNodePins();
		m_Graph->TickAllVisualNodes();

		if (IsInputValid() && IsInputMode(EditorInputEvent::None)) NodeDraggingIndex = -1;

		OnNodeGraphRender(deltaTime);

		EditorUI::DrawRect(0, 0, GetWidth(), GetHeight(), 0, EditorPreferences::Get()->UiBackgroundColor);

		// Draw Background
		glm::ivec2 offset = m_CameraPos; offset.x %= 150; offset.y %= 150;
		const int TilesX = GetWidth() / 2 / (150 * m_Zoom) + 2;
		const int TilesY = GetHeight() / 2 / (150 * m_Zoom) + 2;
		for (int x = -TilesX; x <= TilesX; x++)
		{
			for (int y = -TilesY; y <= TilesY; y++)
			{
				const float size = 150 * m_Zoom;
				EditorUI::DrawTexturedRect(m_Checkerboard, x * size - size/2 - offset.x * m_Zoom + GetWidth() / 2, y * size - size/2 - offset.y * m_Zoom + GetHeight() / 2, size, size, 0, Color(0.28f, 0.28f, 0.3f, 1));
				EditorUI::DrawRectOutline(x * size - size/2 - offset.x * m_Zoom + GetWidth() / 2, y * size - size/2 - offset.y * m_Zoom + GetHeight() / 2, size, size, 1, Color(0.1f, 0.1f, 0.1f, 1));
			}
		}
		const Color backGrd = EditorPreferences::Get()->UiBackgroundColor;
		EditorUI::DrawRect(0, 0, GetWidth(), GetHeight(), 0, Color(backGrd.r, backGrd.g, backGrd.b, 0.25f));

		UpdateMouseCursorInGraph();
		if (m_DragPin)
		{
			bool b = m_DragPin->IsReceivingPin;
			if (m_InvertDragPinForIDs.Contains(m_DragPin->PinID)) b = !b;
			if (b)
			{
				DrawWires(EditorUI::GetInput(), m_DragPin->PinConnectionPoint, m_DragPin->Color);
			}
			else
			{
				DrawWires(m_DragPin->PinConnectionPoint, EditorUI::GetInput(), m_DragPin->Color);
			}
		}

		m_NodeBuffer->Bind();
		for (Ref<VisualNode> node : m_Graph->m_Nodes)
		{
			DrawVisualNode(*node.get());
		}
		m_Framebuffer->Bind();
		// Wires
		for (Ref<VisualNode> node : m_Graph->m_Nodes)
		{
			for (VisualNodePin& pin : node->m_InputPins)
			{
				if (pin.IsReceivingPin && pin.Target) DrawWires(*pin.Target, pin);
			}
			for (VisualNodePin& pin : node->m_OutputPins)
			{
				if (pin.IsReceivingPin && pin.Target) DrawWires(pin, *pin.Target);
			}
		}
		RenderPipeline::RenderFramebufferIntoFramebuffer(*m_NodeBuffer, *m_Framebuffer, *RenderPipeline::GetFullscreenPassShaderStatic(), Vec4(0, 0, m_Framebuffer->GetSize()), "u_Texture", 0, false);

		
		// Cancel DragPin
		if (m_DragPin && !NativeInput::GetMouseButton(Mouse::ButtonLeft))
		{
			m_DragPin = nullptr;
			SetInputMode(EditorInputEvent::None);
		}

		if (IsInputValid() && IsInputMode(EditorInputEvent::None))
		{
			m_ZoomTarget += 0.08f * NativeInput::GetMouseScrollDelta();
			m_ZoomTarget = Math::Clamp(m_ZoomTarget, 0.15f, 1.15f);
			//m_Zoom = Math::Lerp(m_Zoom, m_ZoomTarget, deltaTime * 25.0f);
			if (NativeInput::IsMouseButtonPressed(Mouse::ButtonRight))
			{
				Vec2 delta = NativeInput::GetMouseDelta() * (1.0f / m_ZoomTarget);
				delta.x *= -1;
				m_CameraPosTarget += delta;
			}
			//m_CameraPos = Math::Lerp(m_CameraPos, m_CameraPosTarget, deltaTime * 25.0f);
		}
		else if (IsInputValid() && IsInputMode(EditorInputEvent::NodeGraph_DragNode) && NativeInput::GetMouseButton(Mouse::ButtonLeft))
		{
			NodeDraggingGridSnap.x += NativeInput::GetMouseDelta().x * (1.0f / m_Zoom);
			NodeDraggingGridSnap.y -= NativeInput::GetMouseDelta().y * (1.0f / m_Zoom);

			Array<VisualNode*> draggedNodes;
			if (m_SelectedNodes.Size() == 0) draggedNodes.Add(m_Graph->m_Nodes[NodeDraggingIndex].get());
			else draggedNodes = m_SelectedNodes;

			const Vec2 TempNodeSnap = NodeDraggingGridSnap;
			for (VisualNode* It : draggedNodes)
			{
				NodeDraggingGridSnap = TempNodeSnap;
				bool bDone = false;
				while (!bDone)
				{
					bDone = true;
					if (NodeDraggingGridSnap.x > NODEGRAPH_GRID_SNAP)  { It->m_Position.x += NODEGRAPH_GRID_SNAP; NodeDraggingGridSnap.x -= NODEGRAPH_GRID_SNAP; bDone = false; }
					if (NodeDraggingGridSnap.x < -NODEGRAPH_GRID_SNAP) { It->m_Position.x -= NODEGRAPH_GRID_SNAP; NodeDraggingGridSnap.x += NODEGRAPH_GRID_SNAP; bDone = false; }
					if (NodeDraggingGridSnap.y > NODEGRAPH_GRID_SNAP)  { It->m_Position.y += NODEGRAPH_GRID_SNAP; NodeDraggingGridSnap.y -= NODEGRAPH_GRID_SNAP; bDone = false; }
					if (NodeDraggingGridSnap.y < -NODEGRAPH_GRID_SNAP) { It->m_Position.y -= NODEGRAPH_GRID_SNAP; NodeDraggingGridSnap.y += NODEGRAPH_GRID_SNAP; bDone = false; }
				}
			}
			

			/*m_Nodes[NodeDraggingIndex].m_Position.x += NativeInput::GetMouseDelta().x * (1.0f / m_Zoom);
			m_Nodes[NodeDraggingIndex].m_Position.y -= NativeInput::GetMouseDelta().y * (1.0f / m_Zoom);*/
		}
		else if (IsInputMode(EditorInputEvent::NodeGraph_DragNode) && !NativeInput::GetMouseButton(Mouse::ButtonLeft))
		{
			GetMajorTab()->GetEditorWindow()->m_InputEvent = EditorInputEvent::None;
			NodeDraggingGridSnap = Vec2(0.0f);
		}
		
		if (IsInputMode(EditorInputEvent::None) && IsInputValid())
		{
			if (NativeInput::GetMouseButtonDown(Mouse::ButtonRight) || NativeInput::GetMouseButtonDown(Mouse::ButtonLeft))
			{
				buttonDownMousePos = glm::ivec2(NativeInput::GetMousePosition());
			}
			if ((NativeInput::GetMouseButtonUp(Mouse::ButtonLeft) || NativeInput::GetMouseButtonUp(Mouse::ButtonRight)) && glm::distance((Vec2)buttonDownMousePos, NativeInput::GetMousePosition()) <= 45.0f)
			{
				// Left Click...
				if (!NativeInput::GetKey(Key::LeftShift) && (NodeDraggingIndex == -1 || !m_SelectedNodes.Contains(m_Graph->m_Nodes[NodeDraggingIndex].get())))
				{
					m_SelectedNodes.Clear();
				}
				if (NodeDraggingIndex != -1 && !m_SelectedNodes.Contains(m_Graph->m_Nodes[NodeDraggingIndex].get()))
				{
					m_SelectedNodes.Add(m_Graph->m_Nodes[NodeDraggingIndex].get());
				}

			}
			if (NativeInput::GetMouseButtonUp(Mouse::ButtonRight) && glm::distance((Vec2)buttonDownMousePos, NativeInput::GetMousePosition()) <= 45.0f)
			{
				// Right Click...
				if (m_SelectedNodes.Size() == 0)
				{
					if (GetMajorTab()->IsA<NodeClassEditor>()) GetMajorTab()->As<NodeClassEditor>()->SerializeActor();
					m_Graph->UpdateSupportedNodes();
					EditorUI::CreateOverlay<VisualNodeSearchOverlay>(NativeInput::GetMousePosition().x, GetMajorTab()->GetEditorWindow()->GetWindow()->GetHeight() - NativeInput::GetMousePosition().y - 400.0f, 400.0f, 400.0f, *this);
				}
				else
				{
					std::vector<EditorUI::ContextMenuElement> Elements;
					Elements.push_back(EditorUI::ContextMenuElement({}, [&]() 
						{ 
							Array<VisualNode*> nodes;
							if (m_SelectedNodes.Size() == 0) nodes.Add(m_Graph->m_Nodes[NodeDraggingIndex].get());
							else nodes = m_SelectedNodes;
							for (VisualNode* It : nodes) m_Graph->RemoveVisualNode(*It);
							m_SelectedNodes.Clear(); 
						}, "Delete Node", nullptr));
					EditorUI::CreateContextMenu(Elements);
				}
			}
		}

		if (NativeInput::GetKeyDown(Key::Delete))
		{
			for (VisualNode* node : m_SelectedNodes)
			{
				m_Graph->RemoveVisualNode(*node);
			}
			m_SelectedNodes.Clear();
		}

	}

	float NodeGraphEditor::DrawNodeInputPins(VisualNode& node, float headerOffset)
	{
		float out1 = Math::Abs(headerOffset), out2 = Math::Abs(headerOffset);
		{
			float y = node.m_Position.y * m_Zoom + node.m_Size.y / 2.0f * m_Zoom - m_CameraPos.y * m_Zoom + GetHeight() / 2 + headerOffset;
			const float begin = y;
			for (VisualNodePin& pin : node.m_InputPins)
			{
				y -= pin.PinHeight * m_Zoom;
				if (pin.PinHeight > 0.0f)
				{
					y -= 5.0f * m_Zoom;
				}
				y -= DrawVisualNodePin(node, pin, true, y);
			}
			out1 += Math::Abs((y - begin));
		}
		{
			float y = node.m_Position.y * m_Zoom + node.m_Size.y / 2.0f * m_Zoom - m_CameraPos.y * m_Zoom + GetHeight() / 2 + headerOffset;
			const float begin = y;
			for (VisualNodePin& pin : node.m_OutputPins)
			{
				y -= pin.PinHeight * m_Zoom;
				if (pin.PinHeight > 0.0f)
				{
					y -= 5.0f * m_Zoom;
				}
				y -= DrawVisualNodePin(node, pin, false, y);
			}
			out2 += Math::Abs((y - begin));
		}
		return out1 > out2 ? out1 : out2;
	}

	void NodeGraphEditor::DrawWires(VisualNodePin& a, VisualNodePin& b)
	{
		DrawWires(a.PinConnectionPoint, b.PinConnectionPoint, a.Color);
	}
	void NodeGraphEditor::DrawWires(const Vec2& a, const Vec2& b, const Color& color)
	{
		const float OutLength = 15.0f * m_Zoom;
		DrawWireLine(a.x, a.y, a.x + OutLength, a.y, color);
		Vec2 pos = Vec2(a.x + OutLength, a.y);
		Vec2 path = Vec2(b.x - OutLength, b.y) - pos;

		if (path.x > 0 && path.x >= glm::abs(path.y))
		{
			float pathToRight = (path.x - glm::abs(path.y)) / 2.0f;
			DrawWireLine(pos.x, pos.y, pos.x + pathToRight, pos.y, color); pos.x += pathToRight;
			DrawWireLine(pos.x, pos.y, pos.x + glm::abs(path.y), pos.y + path.y, color); pos.x += glm::abs(path.y); pos.y += path.y;
			DrawWireLine(pos.x, pos.y, pos.x + pathToRight, pos.y, color); pos.x += pathToRight;
		}
		else if (path.x > 0 && path.x < glm::abs(path.y))
		{
			float pathToY = glm::abs(path.y) - path.x;
			bool Up = path.y >= 0;
			DrawWireLine(pos.x, pos.y, pos.x + path.x / 2.0f, pos.y + (path.x / 2.0f) * (Up ? 1 : -1), color); pos.x += path.x / 2.0f; pos.y += (path.x / 2.0f) * (Up ? 1 : -1);
			DrawWireLine(pos.x, pos.y, pos.x, pos.y + pathToY * (Up ? 1 : -1), color); pos.y += pathToY * (Up ? 1 : -1);
			DrawWireLine(pos.x, pos.y, pos.x + path.x / 2.0f, pos.y + (path.x / 2.0f) * (Up ? 1 : -1), color); pos.x += path.x / 2.0f; pos.y += (path.x / 2.0f) * (Up ? 1 : -1);
		}
		else
		{
			float halfToY = glm::abs(path.y) / 2.0f;
			bool Up = path.y >= 0;
			DrawWireLine(pos.x, pos.y, pos.x + halfToY / 3.0f, pos.y + (halfToY / 3.0f) * (Up ? 1 : -1), color); pos.x += halfToY / 3.0f; pos.y += (halfToY / 3.0f) * (Up ? 1 : -1);
			DrawWireLine(pos.x, pos.y, pos.x, pos.y + (halfToY / 3.0f) * (Up ? 1 : -1), color); pos.y += (halfToY / 3.0f) * (Up ? 1 : -1);
			DrawWireLine(pos.x, pos.y, pos.x - halfToY / 3.0f, pos.y + (halfToY / 3.0f) * (Up ? 1 : -1), color); pos.x -= halfToY / 3.0f; pos.y += (halfToY / 3.0f) * (Up ? 1 : -1);
			
			const float RightToLeft = ((a.x + OutLength) - (b.x - OutLength));
			DrawWireLine(pos.x, pos.y, pos.x - RightToLeft, pos.y, color); pos.x -= RightToLeft;
			
			DrawWireLine(pos.x, pos.y, pos.x - halfToY / 3.0f, pos.y + (halfToY / 3.0f) * (Up ? 1 : -1), color); pos.x -= halfToY / 3.0f; pos.y += (halfToY / 3.0f) * (Up ? 1 : -1);
			DrawWireLine(pos.x, pos.y, pos.x, pos.y + (halfToY / 3.0f) * (Up ? 1 : -1), color); pos.y += (halfToY / 3.0f) * (Up ? 1 : -1);
			DrawWireLine(pos.x, pos.y, pos.x + halfToY / 3.0f, pos.y + (halfToY / 3.0f) * (Up ? 1 : -1), color); pos.x += halfToY / 3.0f; pos.y += (halfToY / 3.0f) * (Up ? 1 : -1);
		}

		DrawWireLine(b.x - OutLength, b.y, b.x, b.y, color);
		//DrawWireLine(a.PinConnectionPoint.x, a.PinConnectionPoint.y, b.PinConnectionPoint.x, b.PinConnectionPoint.y, a.Color);
	}

	void NodeGraphEditor::DrawWireLine(float x1, float y1, float x2, float y2, const Color& color)
	{
		RenderCommand::SetViewport(0, 0, GetWidth(), GetHeight());
		m_LineShader->Bind();
		m_LineShader->SetFloat("u_Zoom", m_Zoom);
		m_LineShader->SetFloat2("u_ViewportSize", Vec2(GetWidth(), GetHeight()));
		m_LineShader->SetFloat2("u_PointBegin", Vec2(x1 - GetWidth() / 2.0f, y1 - GetHeight() / 2.0f));
		m_LineShader->SetFloat2("u_PointEnd", Vec2(x2 - GetWidth() / 2.0f, y2 - GetHeight() / 2.0f));
		m_LineShader->SetFloat4("u_Color", color);
		RenderCommand::SetDepthTest(false);
		RenderPipeline::__GetFullscreenQuad()->Bind();
		RenderCommand::DrawIndexed(RenderPipeline::__GetFullscreenQuad());
	}

	void NodeGraphEditor::ConnectNodePins(VisualNodePin& a, VisualNodePin& b)
	{
		// Reset Input
		m_DragPin = nullptr;
		SetInputMode(EditorInputEvent::None);

		if (a == b) return;
		if (a.m_Node == b.m_Node) return;
		if (a.IsReceivingPin == b.IsReceivingPin) return;

		// Connect
		if (a.PinID == b.PinID)
		{
			if (a.IsReceivingPin)	a.Target = &b;
			else					b.Target = &a;
		}
		else
		{
			ProccessNodePinIDConversion(a.IsReceivingPin ? a : b, a.IsReceivingPin ? b : a);
		}
	}

	void NodeGraphEditor::UpdateMouseCursorInGraph()
	{
		if (EditorUI::_GetOverlays().Size() > 0)
		{
			return;
		}
		m_MouseCursorPosInGraph = m_CameraPos;
	}

	

}