#include "ViewportPanel.h"
#include "../MajorTab.h"
#include "../../Util/EditorCamera.h"
#include "../../Util/EditorPreferences.h"
#include "Suora/Renderer/RenderCommand.h"
#include "Suora/Renderer/RenderPipeline.h"
#include "Suora/Renderer/Shader.h"
#include "Suora/Renderer/Renderer3D.h"
#include "Suora/Renderer/Framebuffer.h"
#include "Suora/Core/Engine.h"
#include "Suora/GameFramework/GameInstance.h"
#include "Suora/Assets/ShaderGraph.h"
#include "Suora/Assets/AssetManager.h"
#include "Suora/GameFramework/Nodes/ShapeNodes.h"
#include "Suora/GameFramework/Nodes/Light/PointLightNode.h"
#include "Suora/GameFramework/Nodes/Light/DirectionalLightNode.h"
#include "Suora/GameFramework/InputModule.h"
#include "../Major/NodeClassEditor.h"

namespace Suora
{

	static const char* DebugViewString(DebugView view)
	{
		switch (view)
		{
		case DebugView::Final: return "Final";
		case DebugView::GBuffer_BaseColor: return "BaseColor";
		case DebugView::GBuffer_WorldPosition: return "WorldPosition";
		case DebugView::GBuffer_WorldNormal: return "WorldNormal";
		case DebugView::GBuffer_Emissive: return "Emissive";
		case DebugView::GBuffer_MeshID: return "MeshID";
		case DebugView::GBuffer_Cluster: return "Cluster";
		case DebugView::DeferredLightPass: return "DeferredLightPass";
		case DebugView::ForwardReadyBuffer: return "ForwardReadyBuffer";
		default: return "DebugView::MissingName";
		}
	}

	ViewportPanel::ViewportPanel(MajorTab* majorTab, World* world)
		: MinorTab(majorTab), m_World(world)
	{
		Name = "Viewport";

		{
			FramebufferSpecification spec;
			spec.Width = 1920;
			spec.Height = 1080;
			spec.Attachments.Attachments.push_back(FramebufferTextureFormat::RGBA8);
			spec.Attachments.Attachments.push_back(FramebufferTextureFormat::Depth);
			m_ViewportFramebuffer = Framebuffer::Create(spec);

			m_GizmoBuffer = Framebuffer::Create(spec);
			m_GizmoBufferSmooth = Framebuffer::Create(spec);
		} 
		{
			FramebufferSpecification spec;
			spec.Width = 1920;
			spec.Height = 1080;
			spec.Attachments.Attachments.push_back(FramebufferTextureFormat::R32I);
			spec.Attachments.Attachments.push_back(FramebufferTextureFormat::Depth);
			m_PickingBuffer = Framebuffer::Create(spec);
		}
		{
			FramebufferSpecification spec;
			spec.Width = 192;
			spec.Height = 108;
			spec.Attachments.Attachments.push_back(FramebufferTextureFormat::RGB16F);
			spec.Attachments.Attachments.push_back(FramebufferTextureFormat::Depth);
			m_TranformGizmoPickingBuffer = Framebuffer::Create(spec);
		}
		{
			FramebufferSpecification spec;
			spec.Width = 1920;
			spec.Height = 1080;
			spec.Attachments.Attachments.push_back(FramebufferTextureFormat::RGBA8);
			m_SelectionOutlineFramebuffer = Framebuffer::Create(spec);
			m_SelectionOutlineShader = Shader::Create(AssetManager::GetEngineAssetPath() + "/EditorContent/Shaders/SelectionOutline.glsl");
		}
		{
			FramebufferSpecification spec;
			spec.Width = 256;
			spec.Height = 144;
			spec.Attachments.Attachments.push_back(FramebufferTextureFormat::RGBA8);
			m_CameraPreviewBuffer = Framebuffer::Create(spec);
		}
	}

	ViewportPanel::~ViewportPanel()
	{
	}


	void ViewportPanel::DrawSelectionOutline(Node3D* node, const Color& color)
	{
		if (m_SelectionOutlineFramebuffer->GetSize() != m_RParams.Resolution)
			m_SelectionOutlineFramebuffer->Resize(m_RParams.Resolution);
		m_SelectionOutlineFramebuffer->Bind();
		RenderCommand::SetViewport(0, 0, m_SelectionOutlineFramebuffer->GetSize().x, m_SelectionOutlineFramebuffer->GetSize().y);
		RenderCommand::SetClearColor(Color(0.0f));
		RenderCommand::Clear();

		if (MeshNode* meshNode = node->As<MeshNode>())
		{
			if (meshNode->GetMesh())
			{
				Renderer3D::DrawMesh(GetEditorCamera(), meshNode->GetTransformMatrix(), *meshNode->GetMesh(), meshNode->GetMaterials(), MaterialType::FlatWhite);
			}
		}
		RenderCommand::SetDepthTest(false);
		RenderCommand::SetAlphaBlending(true);
		m_Framebuffer->Bind();
		m_SelectionOutlineShader->Bind();
		m_SelectionOutlineShader->SetFloat4("u_Color", color);
		m_SelectionOutlineShader->SetInt("u_Thickness", 4);
		RenderPipeline::RenderFramebufferIntoFramebuffer(*m_SelectionOutlineFramebuffer, *m_Framebuffer, *m_SelectionOutlineShader, glm::ivec4(0, 0, m_Framebuffer->GetSize().x, m_Framebuffer->GetSize().y), "u_Texture", 0, false);
	}

	void ViewportPanel::HandleMousePick(const glm::ivec2& pos)
	{
		m_PickingBuffer->Bind();
		RenderCommand::SetClearColor(Color(0.0f));
		RenderCommand::SetViewport(0, 0, GetWidth(), GetHeight());
		RenderCommand::Clear();

		Array<MeshNode*> nodes;
		if (GetMajorTab()->IsA<NodeClassEditor>()) nodes = GetMajorTab()->As<NodeClassEditor>()->GetEditorWorld()->FindNodesByClass<MeshNode>();
		else
		{
			SuoraError("Cannot handle >World< in ViewportPanel::HandleMousePick()");
		}

		std::unordered_map<int, Node*> IDs;
		int i = 1;
		for (MeshNode* meshNode : nodes)
		{
			if (meshNode->IsEnabled() && meshNode->GetMesh())
			{
				IDs[i] = meshNode;
				for (Material* submaterial : meshNode->GetMaterials().Materials) if (submaterial && submaterial->GetShaderGraph()) submaterial->GetShaderGraph()->GetIDShader()->SetInt("u_ID", i);
				Renderer3D::DrawMesh(GetEditorCamera(), meshNode->GetTransformMatrix(), *meshNode->GetMesh(), meshNode->GetMaterials(), MaterialType::ObjectID);
			}
			i++;
		}
		i++;

		DrawDebugShapes(GetMajorTab()->IsA<NodeClassEditor>() ? GetMajorTab()->As<NodeClassEditor>()->GetEditorWorld() : nullptr, GetEditorCamera(), &i, &IDs);

		int32_t id = m_PickingBuffer->ReadPixel_R32I(pos);
		SuoraWarn("MousePick MeshID: {0}", std::to_string(id));

		Node* selection = IDs.find(id) != IDs.end() ? IDs[id] : nullptr;
		Node* actor = selection ? selection->GetActorNode() : nullptr;

		Node* currentSelection = GetMajorTab()->IsA<NodeClassEditor>() ? GetMajorTab()->As<NodeClassEditor>()->m_SelectedObject->As<Node>() : nullptr;

		if (!currentSelection || !(selection && selection->IsChildOf(currentSelection))) selection = actor;

		if (GetMajorTab()->IsA<NodeClassEditor>()) GetMajorTab()->As<NodeClassEditor>()->m_SelectedObject = selection;
		else
		{
			SuoraError("Cannot handle 'selection' in ViewportPanel::HandleMousePick()");
		}

		m_Framebuffer->Bind();
	}


	void ViewportPanel::HandleAssetDragDrop()
	{
		if (IsInputMode(EditorInputEvent::ContentBrowser_AssetDrag) && ContentBrowser::s_DraggedAsset)
		{
			//if (ContentBrowser::s_DraggedAsset->GetClass() != Mesh::StaticClass()) return;
			if (IsInputValid())
			{
				if (!AssetDragDropNode)
				{ 
					if (ContentBrowser::s_DraggedAsset->GetClass() == Mesh::StaticClass())
					{
						AssetDragDropNode = m_World->Spawn<MeshNode>();
						AssetDragDropNode->SetName(ContentBrowser::s_DraggedAsset->GetAssetName());
						AssetDragDropNode->As<MeshNode>()->SetMesh(ContentBrowser::s_DraggedAsset->As<Mesh>());
						AssetDragDropNode->As<MeshNode>()->m_Materials = AssetManager::GetAsset<Material>(SuoraID("d83842b6-ee90-4329-a4cc-7c01977107b6"));
						AssetDragDropNode->m_IsActorLayer = true;
						AssetDragDropNode->m_OverwrittenProperties.Add("mesh");
						AssetDragDropNode->m_OverwrittenProperties.Add("material");
					}
					else if (ContentBrowser::s_DraggedAsset->IsA<Blueprint>())
					{
						AssetDragDropNode = m_World->Spawn(ContentBrowser::s_DraggedAsset->As<Blueprint>());
					}
				}
				// Update Position each Frame
				m_RParams.GetGBuffer()->Bind();
				
				const Vec2 ScreenPos = Vec2(EditorUI::GetInput().x / GetWidth() * (m_RParams.Resolution.x), EditorUI::GetInput().y / GetHeight() * (m_RParams.Resolution.y));
				const Vec3 Position = m_RParams.GetGBuffer()->ReadPixel_RGB32F(ScreenPos, (int)GBuffer::WorldPosition);
				if (AssetDragDropNode && AssetDragDropNode->IsA<Node3D>()) AssetDragDropNode->As<Node3D>()->SetPosition(Position);

				if (NativeInput::GetMouseButtonUp(Mouse::ButtonLeft) && AssetDragDropNode)
				{
					AssetDragDropNode->m_IsActorLayer = true;
					AssetDragDropValidation = true;

					if (GetMajorTab()->IsA<NodeClassEditor>()) GetMajorTab()->As<NodeClassEditor>()->m_SelectedObject = AssetDragDropNode;
					else SuoraError("Cannot handle 'AssetDragDropNode' in ViewportPanel::HandleAssetDragDrop()");
				}
			}
			else
			{
				if (AssetDragDropNode)
				{
					AssetDragDropNode->Destroy();
					AssetDragDropNode = nullptr;
				}
			}
		}

		if (!IsInputMode(EditorInputEvent::ContentBrowser_AssetDrag) && AssetDragDropNode)
		{
			if (!AssetDragDropValidation) AssetDragDropNode->Destroy();
			AssetDragDropNode = nullptr;
		}

		m_Framebuffer->Bind();
	}

	void ViewportPanel::Render(float deltaTime)
	{
		if (!m_EditorCamera)
		{
			m_EditorCamera = Ref<EditorCamera>(new EditorCamera(GetMajorTab()->GetEditorWindow(), GetMajorTab()));
		}
		
		// Update the UI Viewport
		UINode::s_UIViewportWidth  = GetWidth();
		UINode::s_UIViewportHeight = GetHeight();


		if (m_GizmoBuffer->GetSize() != m_Framebuffer->GetSize() * 2) m_GizmoBuffer->Resize(m_Framebuffer->GetSize() * 2);
		if (m_GizmoBufferSmooth->GetSize() != m_Framebuffer->GetSize() * 2) m_GizmoBufferSmooth->Resize(m_Framebuffer->GetSize() * 2);
		m_GizmoBuffer->Bind();
		RenderCommand::SetViewport(0, 0, GetWidth(), GetHeight());
		RenderCommand::SetClearColor(Color(0.0f));
		RenderCommand::Clear();
		m_Framebuffer->Bind();

		EditorUI::DrawRect(0, 0, GetWidth(), GetHeight(), 0, Color(.25f, .25f, .25f, 1));

		RenderCommand::SetClearColor(Color(0.1f, 0.1f, 0.1f, 1));
		RenderCommand::Clear();

		m_EditorCamera->GetCameraComponent()->SetAspectRatio((float)GetWidth() / (float)GetHeight());

		// Render
		bool allowEditorCameraUpdate = true;

		if (!GetMajorTab()->IsA<NodeClassEditor>() || (GetMajorTab()->IsA<NodeClassEditor>() && GetMajorTab()->As<NodeClassEditor>()->m_CurrentPlayState == PlayState::Editor))
		{
			DrawDebugView(*m_Framebuffer.get(), *m_World, *m_EditorCamera);
			DrawDebugShapes(m_World, GetEditorCamera());
		}
		else if (GetMajorTab()->IsA<NodeClassEditor>() && GetMajorTab()->As<NodeClassEditor>()->m_CurrentPlayState == PlayState::Simulating)
		{
			DrawDebugView(*m_Framebuffer.get(), *Engine::Get()->GetGameInstance()->GetCurrentWorld(), *m_EditorCamera);
			DrawDebugShapes(Engine::Get()->GetGameInstance()->GetCurrentWorld(), GetEditorCamera());
		}
		else if (GetMajorTab()->IsA<NodeClassEditor>() && GetMajorTab()->As<NodeClassEditor>()->m_CurrentPlayState != PlayState::Editor)
		{
			if (Engine::Get()->GetGameInstance()->GetCurrentWorld()->GetMainCamera())
			{
				Engine::Get()->GetGameInstance()->GetCurrentWorld()->GetMainCamera()->SetAspectRatio((float)GetWidth() / (float)GetHeight());
				Engine::Get()->GetGameInstance()->GetCurrentWorld()->GetMainCamera()->SetPerspectiveFarClip(Engine::Get()->GetGameInstance()->GetCurrentWorld()->GetMainCamera()->GetPerspectiveFarClip());
				DrawDebugView(*m_Framebuffer.get(), *Engine::Get()->GetGameInstance()->GetCurrentWorld(), *Engine::Get()->GetGameInstance()->GetCurrentWorld()->GetMainCamera());
				if (m_DrawDebugGizmosDuringPlay)
				{
					DrawDebugShapes(Engine::Get()->GetGameInstance()->GetCurrentWorld(), Engine::Get()->GetGameInstance()->GetCurrentWorld()->GetMainCamera());
				}
			}

			allowEditorCameraUpdate = false;
			if (GetMajorTab()->IsA<NodeClassEditor>() && GetMajorTab()->As<NodeClassEditor>()->m_CurrentPlayState == PlayState::Playing)
			{
				GetMajorTab()->GetEditorWindow()->GetWindow()->SetCursorLocked(PlayerInputNode::m_LockInputCursor);
			}
		}

		// Selection Outline
		if (GetMajorTab()->IsA<NodeClassEditor>() && GetMajorTab()->As<NodeClassEditor>()->m_SelectedObject && GetMajorTab()->As<NodeClassEditor>()->m_CurrentPlayState != PlayState::Playing)
		{
			Node* node = GetMajorTab()->IsA<NodeClassEditor>() ? GetMajorTab()->As<NodeClassEditor>()->m_SelectedObject->As<Node>() : nullptr;

			if (Node3D* node3D = node->As<Node3D>())
				DrawSelectionOutline(node3D, EditorPreferences::Get()->UiHighlightColor);

			if (CameraNode* camera = node->As<CameraNode>())
			{
				m_CameraPreviewRParams.Resolution = iVec2(GetWidth()/10, GetHeight()/10);
				m_CameraPreviewRParams.DrawWireframe = m_DrawWireframe;
				m_CameraPreviewBuffer->Resize(m_CameraPreviewRParams.Resolution);
				Engine::Get()->GetRenderPipeline()->Render(*m_CameraPreviewBuffer, *m_World, *camera, m_CameraPreviewRParams);

				m_Framebuffer->Bind();
				const float PreviewWidth = m_CameraPreviewRParams.Resolution.x;
				const float PreviewHeight = m_CameraPreviewRParams.Resolution.y;
				EditorUI::DrawRect(GetWidth() - 20.0f - (float)PreviewWidth - 2.0f, 20.0f - 2.0f, (float)PreviewWidth + 4.0f, (float)PreviewHeight + 4.0f, 4.0f, Math::Lerp<Color>(EditorPreferences::Get()->UiHighlightColor, Color(0, 0, 0, 1), 0.25f));
				RenderPipeline::RenderFramebufferIntoFramebuffer(*m_CameraPreviewBuffer, *m_Framebuffer, *RenderPipeline::GetFullscreenPassShaderStatic(), glm::ivec4(GetWidth() - 20.0f - PreviewWidth, 20.0f, PreviewWidth, PreviewHeight), "u_Texture", 0, false);
			}
		}
		OnRenderOutlines();

		// Only allow mouse Input if the Users does not interact with any Overlays
		bool mousePickReady = EditorUI::IsNotHoveringOverlays() && !EditorUI::WasInputConsumed();
		if (IsInputValid() && EditorUI::IsNotHoveringOverlays())
		{
			if (GetMajorTab()->IsA<NodeClassEditor>() && GetMajorTab()->As<NodeClassEditor>()->m_CurrentPlayState != PlayState::Playing)
			{
				EditorUI::SetCursor(Cursor::Crosshair);
			}
		}
		/** Viewport Tools */
		{
			m_SelectionOutlineFramebuffer->Bind();
			RenderCommand::Clear();
			RenderPipeline::RenderFramebufferIntoFramebuffer(*m_SelectionOutlineFramebuffer, *m_Framebuffer, *m_SelectionOutlineShader, glm::ivec4(0, 0, m_RParams.Resolution.x, m_RParams.Resolution.y), "u_Texture", 0, false);
			m_Framebuffer->Bind();
			EditorUI::ButtonParams Params;
			Params.ButtonColor = EditorPreferences::Get()->UiColor;
			Params.ButtonOutlineColor = EditorPreferences::Get()->UiBackgroundColor;
			Params.ButtonColorHover = EditorPreferences::Get()->UiForgroundColor;
			Params.ButtonColorClicked = EditorPreferences::Get()->UiForgroundColor;
			bool HoverOverTools = false;
			Params.OutHover = &HoverOverTools;
			if (EditorUI::Button(DebugViewString(m_DebugView), 10.0f, GetHeight() - 35.0f, 125.0f, 25.0f, Params))
			{
				mousePickReady = false;
				EditorUI::CreateContextMenu({ EditorUI::ContextMenuElement({}, [&]() { m_DebugView = DebugView::Final; }, "Final", nullptr), 
											  EditorUI::ContextMenuElement(
												  {
													  EditorUI::ContextMenuElement({}, [&]() { m_DebugView = DebugView::GBuffer_BaseColor; }, "BaseColor", nullptr),
													  EditorUI::ContextMenuElement({}, [&]() { m_DebugView = DebugView::GBuffer_WorldPosition; }, "WorldPosition", nullptr),
													  EditorUI::ContextMenuElement({}, [&]() { m_DebugView = DebugView::GBuffer_WorldNormal; }, "WorldNormal", nullptr),
													  EditorUI::ContextMenuElement({}, [&]() { m_DebugView = DebugView::GBuffer_Emissive; }, "Emissive", nullptr),
													  EditorUI::ContextMenuElement({}, [&]() { m_DebugView = DebugView::GBuffer_MeshID; }, "MeshID", nullptr),
													  EditorUI::ContextMenuElement({}, [&]() { m_DebugView = DebugView::GBuffer_Cluster; }, "Cluster", nullptr)
												  }, []() { }, "GBuffer", nullptr),
											  EditorUI::ContextMenuElement({}, [&]() { m_DebugView = DebugView::DeferredLightPass; }, "DeferredLightPass", nullptr),
											  EditorUI::ContextMenuElement({}, [&]() { m_DebugView = DebugView::ForwardReadyBuffer; }, "ForwardReadyBuffer", nullptr) }, 10.0f, GetHeight() - 35.0f);
			}
			if (HoverOverTools) EditorUI::SetCursor(Cursor::Hand);
			if (EditorUI::Button("View", 140.0f, GetHeight() - 35.0f, 100.0f, 25.0f, Params))
			{
				mousePickReady = false;
				EditorUI::CreateContextMenu({ EditorUI::ContextMenuElement({}, [&]() { m_DrawDebugGizmos = !m_DrawDebugGizmos; }, "Show Gizmos", m_DrawDebugGizmos ? AssetManager::GetAsset<Texture2D>(SuoraID("dfdb2091-17b2-41d5-bb5b-cf3a128d201b")) : nullptr),
											  EditorUI::ContextMenuElement({}, [&]() { m_DrawDebugGizmosDuringPlay = !m_DrawDebugGizmosDuringPlay; }, "Show Gizmos during Play", m_DrawDebugGizmosDuringPlay ? AssetManager::GetAsset<Texture2D>(SuoraID("dfdb2091-17b2-41d5-bb5b-cf3a128d201b")) : nullptr),
											  EditorUI::ContextMenuElement({}, [&]() { m_DrawWireframe = !m_DrawWireframe; }, "Draw Wireframe", m_DrawWireframe ? AssetManager::GetAsset<Texture2D>(SuoraID("dfdb2091-17b2-41d5-bb5b-cf3a128d201b")) : nullptr) }, 140.0f, GetHeight() - 35.0f);
			}
			if (HoverOverTools) EditorUI::SetCursor(Cursor::Hand);

			EditorUI::Button("", GetWidth() - 135.0f, GetHeight() - 35.0f, 125.0f, 25.0f, Params);
			if (HoverOverTools) EditorUI::SetCursor(Cursor::Hand);
			for (int i = 0; i < 5; i++)
			{
				Texture2D* toolIcon = nullptr;
				Color iconColor = (TransformGizmo_Tool == i) ? Color(0, 0, 0, 1) : Color(1);
				switch (i)
				{
				case 0: toolIcon = AssetManager::GetAsset<Texture2D>(SuoraID("38654b71-4b30-43ce-b2bf-c5a516257e93")); break;
				case 1: toolIcon = AssetManager::GetAsset<Texture2D>(SuoraID("544a3477-4ec1-4a9c-acdc-1201321b26d3")); break; 
				case 2: toolIcon = AssetManager::GetAsset<Texture2D>(SuoraID("883caf6f-1080-4758-854f-713a68ef2602")); break;
				case 3: toolIcon = AssetManager::GetAsset<Texture2D>(SuoraID("b9df5e5c-349b-4c41-baca-746c98d0478a")); break;
				default:break;
				}
				if (EditorUI::Button("", GetWidth() - 135.0f + 25.0f * i, GetHeight() - 35.0f, 25.0f, 25.0f, EditorUI::ButtonParams::Invisible()))
				{
					mousePickReady = false;
					TransformGizmo_Tool = i;
				}
				
				if (TransformGizmo_Tool == i)
				{
					EditorUI::DrawRect(GetWidth() - 132.5f + 25.0f * i, GetHeight() - 32.5f, 20.0f, 20.0f, 6.0f, EditorPreferences::Get()->UiHighlightColor);
				}
				if (toolIcon) EditorUI::DrawTexturedRect(toolIcon->GetTexture(), GetWidth() - 135.0f + 25.0f * i + 2.5f, GetHeight() - 35.0f + 2.5f, 20.0f, 20.0f, 0, iconColor);
			}

			Params.CenteredIcon = AssetManager::GetAsset<Texture2D>(SuoraID("24294e57-bae7-4ff7-a0f2-73f9741069da"));
			Params.TextColor = !TransformGizmo_Local ? Color(0, 0, 0, 1) : Color(1.0f);
			Params.ButtonColor = !TransformGizmo_Local ? EditorPreferences::Get()->UiHighlightColor : EditorPreferences::Get()->UiColor;
			Params.ButtonColorHover = !TransformGizmo_Local ? EditorPreferences::Get()->UiHighlightColor : EditorPreferences::Get()->UiForgroundColor;
			if (EditorUI::Button("", GetWidth() - 175.0f, GetHeight() - 35.0f, 25.0f, 25.0f, Params))
			{
				mousePickReady = false;
				TransformGizmo_Local = !TransformGizmo_Local;
			}
			if (HoverOverTools) EditorUI::SetCursor(Cursor::Hand);

			Params.CenteredIcon = AssetManager::GetAsset<Texture2D>(SuoraID("8742cec8-9ee5-4645-b036-577146904b41"));
			Params.TextColor = TransformGizmo_SnapTranslate ? Color(0, 0, 0, 1) : Color(1.0f);
			Params.ButtonColor = TransformGizmo_SnapTranslate ? EditorPreferences::Get()->UiHighlightColor : EditorPreferences::Get()->UiColor;
			Params.ButtonColorHover = TransformGizmo_SnapTranslate ? EditorPreferences::Get()->UiHighlightColor : EditorPreferences::Get()->UiForgroundColor;
			if (EditorUI::Button("", GetWidth() - 216.0f, GetHeight() - 35.0f, 26.0f, 25.0f, Params))
			{
				mousePickReady = false;
				EditorUI::CreateContextMenu({ EditorUI::ContextMenuElement({}, [&]() { TransformGizmo_SnapTranslateGrid = 00.010f; }, "0.01m", nullptr),
											  EditorUI::ContextMenuElement({}, [&]() { TransformGizmo_SnapTranslateGrid = 00.100f; }, "0.1m", nullptr),
											  EditorUI::ContextMenuElement({}, [&]() { TransformGizmo_SnapTranslateGrid = 00.250f; }, "0.25m", nullptr),
											  EditorUI::ContextMenuElement({}, [&]() { TransformGizmo_SnapTranslateGrid = 00.500f; }, "0.5m", nullptr),
											  EditorUI::ContextMenuElement({}, [&]() { TransformGizmo_SnapTranslateGrid = 01.000f; }, "1.0m", nullptr),
											  EditorUI::ContextMenuElement({}, [&]() { TransformGizmo_SnapTranslateGrid = 05.000f; }, "5.0m", nullptr),
											  EditorUI::ContextMenuElement({}, [&]() { TransformGizmo_SnapTranslateGrid = 10.000f; }, "10.0m", nullptr) },
											GetWidth() - 240.0f, GetHeight() - 35.0f);
			}
			Params.CenteredIcon = AssetManager::GetAsset<Texture2D>(SuoraID("634893f5-8fec-4ef9-88b5-14b6a0aea623"));
			Params.TextColor = TransformGizmo_SnapTranslate ? Color(0, 0, 0, 1) : Color(1.0f);
			Params.ButtonColor = TransformGizmo_SnapTranslate ? EditorPreferences::Get()->UiHighlightColor : EditorPreferences::Get()->UiColor;
			Params.ButtonColorHover = TransformGizmo_SnapTranslate ? EditorPreferences::Get()->UiHighlightColor : EditorPreferences::Get()->UiForgroundColor;
			if (EditorUI::Button("", GetWidth() - 240.0f, GetHeight() - 35.0f, 25.0f, 25.0f, Params))
			{
				mousePickReady = false;
				TransformGizmo_SnapTranslate = !TransformGizmo_SnapTranslate;
			}
			if (HoverOverTools) EditorUI::SetCursor(Cursor::Hand);
		}

		if (GetMajorTab()->IsA<NodeClassEditor>() && GetMajorTab()->As<NodeClassEditor>()->m_SelectedObject && GetMajorTab()->As<NodeClassEditor>()->m_CurrentPlayState != PlayState::Playing)
		{
			if (mousePickReady)
			{
				Node* node = GetMajorTab()->IsA<NodeClassEditor>() ? GetMajorTab()->As<NodeClassEditor>()->m_SelectedObject->As<Node>() : nullptr;
				if (Node3D* node3D = node->As<Node3D>()) mousePickReady = !DrawTransformGizmo(node3D);
			}
		}
		if (IsInputValid() && IsInputMode(EditorInputEvent::None) && mousePickReady && (GetMajorTab()->IsA<NodeClassEditor>() && GetMajorTab()->As<NodeClassEditor>()->m_CurrentPlayState != PlayState::Playing))
		{
			if (NativeInput::GetMouseButtonDown(Mouse::ButtonLeft))
			{
				HandleMousePick(EditorUI::GetInput());
			}
		}

		if (IsInputMode(EditorInputEvent::TransformGizmo) && !NativeInput::GetMouseButton(Mouse::ButtonLeft))
		{
			SetInputMode(EditorInputEvent::None);
		}

		HandleAssetDragDrop();

		if (mousePickReady && allowEditorCameraUpdate)
			m_EditorCamera->UpdateCamera(deltaTime, (IsInputValid() && IsInputMode(EditorInputEvent::None)) || IsInputMode(EditorInputEvent::Viewport_EditorCamera));

		// Gizmo Buffer
		/*m_GizmoBuffer->Unbind();
		static Ref<Shader> FXAA = Shader::Create(AssetManager::GetEngineAssetPath() + "/EngineContent/Shaders/PostProccess/FXAA.glsl");
		FXAA->Bind();
		FXAA->SetFloat2("u_Resolution", m_GizmoBufferSmooth->GetSize());
		RenderPipeline::RenderFramebufferIntoFramebuffer(*m_GizmoBuffer, *m_GizmoBufferSmooth, *FXAA, glm::ivec4(0, 0, m_GizmoBufferSmooth->GetSize()));
		FXAA->Unbind();*/

		m_Framebuffer->Bind();
		if (Engine::Get()->GetRenderPipeline()->IsA<RenderPipeline>())
			RenderPipeline::RenderFramebufferIntoFramebuffer(*m_GizmoBuffer/*Smooth*/, *m_Framebuffer, *RenderPipeline::GetFullscreenPassShaderStatic(), glm::ivec4(0, 0, m_Framebuffer->GetSize()), "u_Texture", 0, false);


		RenderCommand::SetDepthTest(false);
		RenderCommand::SetCullingMode(CullingMode::None);
	}

}