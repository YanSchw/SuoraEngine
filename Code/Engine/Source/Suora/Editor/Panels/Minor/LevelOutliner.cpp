#include "Precompiled.h"
#include "LevelOutliner.h"
#include "Suora/Editor/Util/EditorCamera.h"
#include "Suora/Editor/Util/EditorPreferences.h"
#include "Suora/Editor/Panels/Major/NodeClassEditor.h"
#include "Suora/NodeScript/NodeScriptObject.h"
#include "Suora/GameFramework/Node.h"
#include "Suora/GameFramework/Nodes/Light/SkyLightNode.h"
#include "Suora/GameFramework/Nodes/Light/PointLightNode.h"
#include "Suora/GameFramework/Nodes/Light/DirectionalLightNode.h"
#include "Suora/GameFramework/Nodes/PostProcess/PostProcessNode.h"
#include "Suora/GameFramework/Nodes/DecalNode.h"
#include "Suora/GameFramework/Nodes/OrganizationNodes.h"
#include "Suora/Editor/Overlays/SelectClassOverlay.h"

namespace Suora
{
	static float scrollY = 0;
	static float BaseEntryHeight = 20.0f;
	static float EntryHeight = BaseEntryHeight;

	std::vector<EditorUI::ContextMenuElement> LevelOutliner::CreateNodeMenu(World* world, Node* node)
	{
		std::vector<EditorUI::ContextMenuElement> out;

		out.push_back(EditorUI::ContextMenuElement{ {}, [world, node]()
		{
			EditorUI::CreateOverlay<SelectAnyClassOverlay>(EditorUI::CurrentWindow->GetWindow()->GetWidth() / 2 - 300, EditorUI::CurrentWindow->GetWindow()->GetHeight() / 2 - 400, 600, 800, "Select a Class", Node::StaticClass(), [world, node](const Class& cls)
			{
				Node* n = world ? world->Spawn(cls) : node->CreateChild(cls);
				n->m_IsActorLayer = true;
			});

		}, "Custom Class", nullptr });
		out.push_back(EditorUI::ContextMenuElement{ {}, [world, node]()
		{
			FolderNode* folder = world ? world->Spawn<FolderNode>() : node->CreateChild<FolderNode>();
			folder->SetName("New Folder");
			folder->m_IsActorLayer = true;
		}, "Folder", nullptr });
		out.push_back(EditorUI::ContextMenuElement{ {}, [world, node]()
		{
			Node* n = world ? world->Spawn<Node>() : node->CreateChild<Node>();
			n->SetName("New Node");
			n->m_IsActorLayer = true;
		}, "Empty Node", nullptr });


		std::vector<EditorUI::ContextMenuElement> _3D;
		{
			_3D.push_back(EditorUI::ContextMenuElement{ {}, [world, node]()
			{
				Node3D* node3D = world ? world->Spawn<Node3D>() : node->CreateChild<Node3D>();
				node3D->SetName("Node3D");
				node3D->m_IsActorLayer = true;
			}, "Node3D", nullptr });
			_3D.push_back(EditorUI::ContextMenuElement{ {}, [world, node]()
			{
				MeshNode* cube = world ? world->Spawn<MeshNode>() : node->CreateChild<MeshNode>();
				cube->SetName("Cube");
				cube->m_IsActorLayer = true;
				cube->m_OverwrittenProperties.Add("m_Mesh");
				cube->SetMesh(AssetManager::GetAsset<Mesh>(SuoraID("33b79a6d-2f4a-40fc-93e5-3f01794c33b8")));
			}, "Cube", nullptr });
			_3D.push_back(EditorUI::ContextMenuElement{ {}, [world, node]()
			{
				MeshNode* sphere = world ? world->Spawn<MeshNode>() : node->CreateChild<MeshNode>();
				sphere->SetName("Sphere");
				sphere->m_IsActorLayer = true;
				sphere->m_OverwrittenProperties.Add("m_Mesh");
				sphere->SetMesh(AssetManager::GetAsset<Mesh>(SuoraID("5c43e991-86be-48a4-8b14-39d275818ec1")));
			}, "Sphere", nullptr });
		}
		out.push_back(EditorUI::ContextMenuElement{ {_3D}, [world, node]() {}, "3D", nullptr });

		std::vector<EditorUI::ContextMenuElement> _Lights;
		{
			_Lights.push_back(EditorUI::ContextMenuElement{ {}, [world, node]()
			{
				SkyLightNode* sky = world ? world->Spawn<SkyLightNode>() : node->CreateChild<SkyLightNode>();
				sky->SetName("SkyLightNode");
				sky->m_IsActorLayer = true;
			}, "SkyLightNode", nullptr });
			_Lights.push_back(EditorUI::ContextMenuElement{ {}, [world, node]()
			{
				DirectionalLightNode* directionalLightNode = world ? world->Spawn<DirectionalLightNode>() : node->CreateChild<DirectionalLightNode>();
				directionalLightNode->SetName("DirectionalLightNode");
				directionalLightNode->SetEulerRotation(Vec3(45.0f, 45.0f, 0.0f));
				directionalLightNode->m_IsActorLayer = true;
			}, "DirectionalLightNode", nullptr });
			_Lights.push_back(EditorUI::ContextMenuElement{ {}, [world, node]()
			{
				PointLightNode* pointLight = world ? world->Spawn<PointLightNode>() : node->CreateChild<PointLightNode>();
				pointLight->SetName("PointLightNode");
				pointLight->m_IsActorLayer = true;
			}, "PointLightNode", nullptr });
		}
		out.push_back(EditorUI::ContextMenuElement{ {_Lights}, [world, node]() {}, "Lights", nullptr });

		std::vector<EditorUI::ContextMenuElement> _UI;
		{
			_UI.push_back(EditorUI::ContextMenuElement{ {}, [world, node]()
			{
				UINode* nodeUI = world ? world->Spawn<UINode>() : node->CreateChild<UINode>();
				nodeUI->SetName("UINode");
				nodeUI->m_IsActorLayer = true;
			}, "UINode", nullptr });
			_UI.push_back(EditorUI::ContextMenuElement{ {}, [world, node]()
			{
				UINode* nodeUI = world ? world->Spawn<UIImage>() : node->CreateChild<UIImage>();
				nodeUI->SetName("UIImage");
				nodeUI->m_IsActorLayer = true;
			}, "UIImage", nullptr });
		}
		out.push_back(EditorUI::ContextMenuElement{ {_UI}, [world, node]() {}, "UI", nullptr });

		return out;
	}

	LevelOutliner::LevelOutliner(MajorTab* majorTab)
		: MinorTab(majorTab)
	{
		Name = "Node Outliner";
		TexActorIcon = AssetManager::GetAsset<Texture2D>(SuoraID("ad168979-55cd-408e-afd2-a24cabf26922")); //Texture::Create("../Content/EditorContent/Icons/Actor.png");
		TexVisible0 = AssetManager::GetAsset<Texture2D>(SuoraID("8dd405ad-59d9-4a13-8a2b-b610310bf600")); //Texture::Create("../Content/EditorContent/Icons/Visible0.png");
		TexVisible1 = AssetManager::GetAsset<Texture2D>(SuoraID("b63772a8-57cf-47ef-8c6f-1e3f6dbd61e6")); //Texture::Create("../Content/EditorContent/Icons/Visible1.png");
		TexArrowRight = AssetManager::GetAsset<Texture2D>(SuoraID("970c3d0e-c5b0-4a2e-a548-661d9b00d977")); //Texture::Create("../Content/EditorContent/Icons/ArrowRight.png");
		TexArrowDown = AssetManager::GetAsset<Texture2D>(SuoraID("8742cec8-9ee5-4645-b036-577146904b41")); //Texture::Create("../Content/EditorContent/Icons/ArrowDown.png");
		m_IconTexture = AssetManager::GetAsset<Texture2D>(SuoraID("269931d5-7e60-4934-a89a-26b7993ae0f3"));
	}

	LevelOutliner::~LevelOutliner()
	{

	}

	Texture* LevelOutliner::GetNodeIconTexture(const Class& cls)
	{
		SuoraVerify(false, "Depricated, use the EditorPreferences Version instead!");
		NativeClassID native = cls.GetNativeClassID();

		if (native == MeshNode::StaticClass().GetNativeClassID()) return AssetManager::GetAsset<Texture2D>(SuoraID("b14b065c-a2c0-4dc9-9272-ab0415ada141"))->GetTexture();
		if (native == DecalNode::StaticClass().GetNativeClassID()) return AssetManager::GetAsset<Texture2D>(SuoraID("9d81a066-2336-42f5-bf35-7bb1c4c65d66"))->GetTexture();
		if (native == PointLightNode::StaticClass().GetNativeClassID()) return AssetManager::GetAsset<Texture2D>(SuoraID("f789d2bf-dcda-4e30-b2d9-3db979b7c6da"))->GetTexture();
		if (native == DirectionalLightNode::StaticClass().GetNativeClassID()) return AssetManager::GetAsset<Texture2D>(SuoraID("64738d74-08a9-4383-8659-620808d5269a"))->GetTexture();
		if (native == CameraNode::StaticClass().GetNativeClassID()) return AssetManager::GetAsset<Texture2D>(SuoraID("8952ef88-cbd0-41ab-9d3c-d4c4b39a30f9"))->GetTexture();
		if (cls.Inherits(PostProcessEffect::StaticClass())) return AssetManager::GetAsset<Texture2D>(SuoraID("9bdeac52-f671-4e0a-9167-aeaa30c47711"))->GetTexture();
		if (cls.Inherits(LevelNode::StaticClass()) || native == LevelNode::StaticClass().GetNativeClassID()) return AssetManager::GetAsset<Texture2D>(SuoraID("3578494c-3c74-4aa5-8d34-4d28959a21f5"))->GetTexture();
		if (cls.Inherits(Component::StaticClass()) || native == Component::StaticClass().GetNativeClassID()) return AssetManager::GetAsset<Texture2D>(SuoraID("3e254a4e-cc83-4254-a462-73739fce6d61"))->GetTexture();
		if (native == FolderNode::StaticClass().GetNativeClassID() || native == FolderNode3D::StaticClass().GetNativeClassID()) return AssetManager::GetAsset<Texture2D>(SuoraID("99898caa-a2b2-4fc4-9db7-5baacaed03e5"))->GetTexture();
		
		return TexActorIcon->GetTexture();
	}

	Node* LevelOutliner::GetSelectedObject()
	{
		if (NodeClassEditor* editor = GetMajorTab()->As<NodeClassEditor>())
		{
			return editor->m_SelectedObject->As<Node>();
		}
		return nullptr;
	}

	void LevelOutliner::SetSelectedObject(Node* node)
	{
		if (NodeClassEditor* editor = GetMajorTab()->As<NodeClassEditor>())
		{
			editor->m_SelectedObject = node;
		}
	}

	void LevelOutliner::Render(float deltaTime)
	{
		EditorUI::DrawRect(0, 0, GetWidth(), GetHeight(), 0, Math::Lerp(EditorPreferences::Get()->UiBackgroundColor, EditorPreferences::Get()->UiColor, 0.55f));

		// Reset
		m_TempYValuesOfParentNodes = std::unordered_map<Node*, float>();

		// Get Level
		World* level = GetEditorWorld();
		if (!level) return;

		if (IsInputValid() && IsInputMode(EditorInputEvent::None) && NativeInput::GetMouseButtonDown(Mouse::ButtonLeft))
		{
			SetSelectedObject(nullptr);
		}

		Array<Node*> allNodes = level->GetAllNodes();

		EntryHeight = BaseEntryHeight * EditorPreferences::Get()->UiScale * 1.277f;
		float y = GetHeight() - 35.0f - EntryHeight + scrollY;
		_EntryCount = 0;
		for (int i = 0; i < allNodes.Size(); i++)
		{
			if (!allNodes[i]->GetParent())
			{
				if (GetMajorTab()->IsA<NodeClassEditor>() && GetMajorTab()->As<NodeClassEditor>()->m_CurrentPlayState == PlayState::Editor && m_RootNode && allNodes[i] != m_RootNode)
				{
					allNodes[i]->SetParent(m_RootNode);
					continue;
				}
				float x = 0;
				DrawNode(x, y, allNodes[i]);
			}
		}

		// Header
		{
			EditorUI::DrawRect(0.0f, GetHeight() - 35.0f, GetWidth(), 35.0f, 0.0f, EditorPreferences::Get()->UiForgroundColor);
			EditorUI::Text("Name", Font::Instance, 0.0f, GetHeight() - 35.0f, GetWidth() * m_HeaderSeperator1, 35.0f, 28.0f, Vec2(-0.95f, 0.0f), Color(1.0f));
			EditorUI::Text("Class", Font::Instance, GetWidth() * m_HeaderSeperator1, GetHeight() - 35.0f, GetWidth() * (m_HeaderSeperator2 - m_HeaderSeperator1), 35.0f, 28.0f, Vec2(-0.95f, 0.0f), Color(1.0f));
			EditorUI::Text("*", Font::Instance, GetWidth() * m_HeaderSeperator2, GetHeight() - 35.0f, GetWidth() - GetWidth() * m_HeaderSeperator2, 35.0f, 28.0f, Vec2(-0.95f, 0.0f), Color(1.0f));
		}
		// Footer
		{
			EditorUI::DrawRect(0.0f, 0.0f, GetWidth(), 35.0f, 0.0f, EditorPreferences::Get()->UiForgroundColor);
			EditorUI::Text(std::to_string(allNodes.Size()) + " Nodes", Font::Instance, 10.0f, 0.0f, 200.0f, 35.0f, 28.0f, Vec2(-1.0f, 0.0f), Color(0.8f));
		}

		if (NativeInput::GetMouseButtonUp(Mouse::ButtonLeft))
			m_DragNode = nullptr;

		if (IsInputValid() && IsInputMode(EditorInputEvent::None) && NativeInput::GetMouseButtonDown(Mouse::ButtonRight) && glm::distance(NativeInput::GetMouseDelta(), Vec2(0)) <= 10.0f)
		{
			EditorUI::CreateContextMenu({ EditorUI::ContextMenuElement{ CreateNodeMenu(GetEditorWorld(), nullptr), []() {},"Create Node", nullptr } });
		}


		float scrollDown = GetHeight() - 35.0f - 35.0f - 5.0f - EntryHeight * _EntryCount - EntryHeight;
		EditorUI::ScrollbarVertical(GetWidth()-10, 35, 10, GetHeight()-70, 0, 35, GetWidth(), GetHeight()-70, 0, scrollDown > 0 ? 0 : Math::Abs(scrollDown), &scrollY);
	}

	World* LevelOutliner::GetEditorWorld()
	{
		SuoraAssert(GetMajorTab()->IsA<NodeClassEditor>());
		return GetMajorTab()->As<NodeClassEditor>()->GetEditorWorld();
	}

	void LevelOutliner::DrawNode(float& x, float& y, Node* node)
	{
		m_TempYValuesOfParentNodes[node] = y;

		if (!m_RootNode)
		{
			m_RootNode = node;
			m_DropDowns[node] = true;
		}


		// Drawing the Parent/Child   v MyParentNode
		//                            |
		//                            |--> MyChildNode
		// Stuff
		if (node->GetParent())
		{
			EditorUI::DrawRect(x - 0.64f * EntryHeight, y + 0.5f * EntryHeight, EntryHeight * (node->HasChildren() ? 1.0f : 1.5f) - 10.0f, 2.0f, 0.0f, Color(0.3f, 0.3f, 0.3f, 1.0f));
			EditorUI::DrawRect(x - 0.64f * EntryHeight, y + 0.5f * EntryHeight, 2.0f, m_TempYValuesOfParentNodes[node->GetParent()] - y - 10.0f, 0.0f, Color(0.3f, 0.3f, 0.3f, 1.0f));
		}


		Vec2 mousePosition = EditorUI::GetInput();
		const bool Hovering = mousePosition.x >= x + (EntryHeight + 6) && mousePosition.x <= 0 + GetWidth() && mousePosition.y > y && mousePosition.y <= y + EntryHeight && (mousePosition.y > 35.0f && mousePosition.y < GetHeight() - 35.0f && mousePosition.x < GetWidth() - 10.0f);
		const bool Selected = (node == GetSelectedObject());
		const bool DropDown = m_DropDowns[node];

		const bool isNode = node->GetClass().IsBlueprintClass();
		Color LabelColor = isNode ? EditorPreferences::Get()->UiHighlightColor : Color(1.0f);
		
		if (!isNode && !node->m_IsActorLayer)
		{
			Node* parent = node->GetParent();
			while (parent)
			{
				if (/*parent->IsA<Node>() && */parent->As<Node>()->GetClass().IsBlueprintClass())
				{
					LabelColor = Math::Lerp(EditorPreferences::Get()->UiHighlightColor, Color(0.0f, 0.0f, 0.0f, 1.0f), 0.45f);
					break;
				}
				parent = parent->GetParent();
			}
		}

		if (Hovering && IsInputValid()) EditorUI::SetCursor(Cursor::Hand);

		if (y > -EntryHeight || y < GetHeight())
		{
			if (Hovering && IsInputValid() && IsInputMode(EditorInputEvent::None) && NativeInput::GetMouseButtonDown(Mouse::ButtonLeft))
			{
				SetSelectedObject(node);
			}
			else if (Hovering && IsInputValid() && IsInputMode(EditorInputEvent::None) && NativeInput::GetMouseButtonDown(Mouse::ButtonRight) && glm::distance(NativeInput::GetMouseDelta(), Vec2(0)) <= 10.0f)
			{
				SetSelectedObject(node);

				EditorUI::CreateContextMenu({ EditorUI::ContextMenuElement{ CreateNodeMenu(nullptr, node),[node]() {
					EditorUI::SubclassSelectionMenu(Node::StaticClass(), [node](const Class& cls)
					{
						Node* n = node->CreateChild(cls);
						n->m_IsActorLayer = true;
					}); },"Create Child Node", nullptr }, EditorUI::ContextMenuElement{ {},[node, this]() {
						if (node->m_IsActorLayer)
						{
							SetSelectedObject(nullptr);
							node->Destroy();
						}
					}, (node->m_IsActorLayer ? "Delete Node" : "Cannot Delete Inherited Node"), nullptr}, EditorUI::ContextMenuElement{{},[node, this]() {
						node->Duplicate();
					},"Duplicate Node", nullptr } });
			}

			if (Hovering) EditorUI::DrawRectOutline(x, y, GetWidth() - x, EntryHeight, 1, Color(.5f, .5f, .5f, 1));
			if (Selected)
			{
				EditorUI::DrawRect(x, y, GetWidth() * m_HeaderSeperator1 - x - 1, EntryHeight, 0, EditorPreferences::Get()->UiHighlightColor);
				EditorUI::DrawRect(GetWidth() * m_HeaderSeperator1, y, GetWidth() * (m_HeaderSeperator2 - m_HeaderSeperator1) - 1, EntryHeight, 0, EditorPreferences::Get()->UiHighlightColor);
				EditorUI::DrawRect(GetWidth() * m_HeaderSeperator2, y, GetWidth() - GetWidth() * m_HeaderSeperator2, EntryHeight, 0, EditorPreferences::Get()->UiHighlightColor);
			}

			if (EditorUI::DragSource(0, y, GetWidth(), EntryHeight, 5.0f))
			{
				if (node->m_IsActorLayer)
				{
					m_DragNode = node;
				}
			}
			EditorUI::ButtonParams dragParams = EditorUI::ButtonParams::Invisible();
			dragParams.OverrideActivationEvent = true;
			dragParams.OverrittenActivationEvent = []() { return NativeInput::GetMouseButtonUp(Mouse::ButtonLeft); };
			if (m_DragNode && m_DragNode != node && EditorUI::Button("", 0, y, GetWidth(), EntryHeight, dragParams))
			{
				if (m_DragNode->GetParent() == node)
				{
					m_DragNode->SetParent(node->GetParent());
				}
				else
				{
					m_DragNode->SetParent(node);
					m_DropDowns[node] = true;
				}
			}

			if (node->HasChildren())
			{
				EditorUI::ButtonParams Params;
				Params.ButtonColor = Color(0.0f); Params.ButtonColorHover = Color(0.0f); Params.ButtonOutlineColor = Color(0.0f); Params.ButtonColorClicked = Color(0.0f);
				if (EditorUI::Button("", x + 2, y, EntryHeight, EntryHeight, Params)) m_DropDowns[node] = !DropDown;
				if (GetSelectedObject() && GetSelectedObject()->IsChildOf(node)) m_DropDowns[node] = true;

				const bool DropDownHovering = mousePosition.x >= x && mousePosition.x <= x + EntryHeight && mousePosition.y > y && mousePosition.y <= y + EntryHeight;
				EditorUI::DrawTexturedRect(DropDown ? TexArrowDown->GetTexture() : TexArrowRight->GetTexture(), x + 5 + 2, y + 3 - 2, EntryHeight - 6, EntryHeight - 6, 0, Color(0, 0, 0, 0.25f));
				EditorUI::DrawTexturedRect(DropDown ? TexArrowDown->GetTexture() : TexArrowRight->GetTexture(), x + 5, y + 3, EntryHeight - 6, EntryHeight - 6, 0, DropDownHovering ? EditorPreferences::Get()->UiHighlightColor : Color(1));
			}

			Texture* NodeIcon = EditorUI::GetClassIcon(node->GetClass())->GetTexture();
			const Color Node3dIconColor = Math::Lerp(EditorPreferences::Get()->UiHighlightColor, Color(1.0f), 0.45f);
			const Color NodeUiIconColor = Math::Lerp(Color(0.4f, 0.64f, 0.2f, 1.0f), Color(1.0f), 0.45f);
			const Color NodeIconColor = node->IsA<Node3D>() ? Node3dIconColor : (node->IsA<UINode>() ? NodeUiIconColor : Color(1)); // Node3D - Color(0.40392f, 0.45490f, 0.6078f, 1) - Color(0.30588f, 0.149019f, 0.184313f, 1) ~ NodeUI - Color(0.40392f, 0.6078f, 0.45490f, 1)
			EditorUI::DrawTexturedRect(NodeIcon, x + (EntryHeight + 6) + 3 + 2, y + 3 - 2, EntryHeight - 6, EntryHeight - 6, 0, Color(0, 0, 0, 0.25f));
			EditorUI::DrawTexturedRect(NodeIcon, x + (EntryHeight + 6) + 3, y + 3, EntryHeight - 6, EntryHeight - 6, 0, NodeIconColor);


			const float nameX = x + (EntryHeight * 2 + 14.0f);
			EditorUI::Text(node->m_Name, Font::Instance, nameX + 1, y - 1, GetWidth() * m_HeaderSeperator1 - nameX, EntryHeight, EntryHeight, Vec2(-1, 0), Color(0, 0, 0, node->IsEnabled() ? 0.15f : 0));
			EditorUI::Text(node->m_Name, Font::Instance, nameX, y, GetWidth() * m_HeaderSeperator1 - nameX, EntryHeight, EntryHeight, Vec2(-1, 0), (Selected ? EditorPreferences::Get()->UiColor : LabelColor) * (node->IsEnabled() ? EditorPreferences::Get()->UiTextColor : EditorPreferences::Get()->UiTextColor * 0.65f));

			if (node->GetParent())
			{
				String className = ClassReflector::GetClassName(node->GetNativeClass());
				bool isNativeClass = true;
				if (INodeScriptObject* obj = node->GetInterface<INodeScriptObject>())
				{
					Blueprint* blueprint = obj->m_Class.GetBlueprintClass();
					if (blueprint)
					{
						className = blueprint->GetAssetName();
						isNativeClass = false;
						EditorUI::ButtonParams params = EditorUI::ButtonParams::Invisible();
						if (EditorUI::Button("", GetWidth() * m_HeaderSeperator1, y, GetWidth() * (m_HeaderSeperator2 - m_HeaderSeperator1), EntryHeight, params))
						{
							NativeInput::ConsumeInput();
							EditorWindow::GetCurrent()->OpenAsset(blueprint);
						}
						EditorUI::ButtonParams hoverParams = EditorUI::ButtonParams::Invisible();
						hoverParams.OverrideActivationEvent = true;
						hoverParams.OverrittenActivationEvent = []() { return true; };
						if (EditorUI::Button("", GetWidth() * m_HeaderSeperator1, y, GetWidth() * (m_HeaderSeperator2 - m_HeaderSeperator1), EntryHeight, hoverParams))
						{
							float strWidth = Font::Instance->GetStringWidth(className, EntryHeight * 0.45f) + 7.5f;
							EditorUI::DrawRect(GetWidth() * m_HeaderSeperator1 + 1.0f, y + EntryHeight * 0.1f, glm::min(GetWidth() * (m_HeaderSeperator2 - m_HeaderSeperator1), strWidth), EntryHeight * 0.075f, 0, Selected ? EditorPreferences::Get()->UiColor : EditorPreferences::Get()->UiHighlightColor);
						}
					}
				}
				EditorUI::Text(className, Font::Instance, GetWidth() * m_HeaderSeperator1, y, GetWidth() * (m_HeaderSeperator2 - m_HeaderSeperator1), EntryHeight, EntryHeight * 0.9f, Vec2(-0.95f, 0), Selected ? EditorPreferences::Get()->UiColor : (isNativeClass ? Color(0.65f) : EditorPreferences::Get()->UiHighlightColor));
			}
		}

		y -= EntryHeight;
		_EntryCount++;
		if (node->HasChildren() && DropDown)
		{
			float inlinedX = x + (EntryHeight + 6.0f);
			for (int i = 0; i < node->GetChildCount(); i++)
			{
				DrawNode(inlinedX, y, node->GetChild(i));
			}
		}

	}
}