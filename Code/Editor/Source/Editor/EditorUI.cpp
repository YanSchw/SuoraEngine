#include "EditorUI.h"
#include "EditorWindow.h"
#include "Util/EditorPreferences.h"
#include "Suora/Core/Base.h"
#include "Suora/Common/Common.h"
#include "Suora/Assets/Asset.h"
#include "Suora/Assets/AssetManager.h"
#include "Overlays/DragableOverlay.h"
#include "Suora/Renderer/Renderer3D.h"
#include "Suora/Renderer/Shader.h"
#include "Suora/Renderer/Framebuffer.h"
#include "Suora/Renderer/RenderPipeline.h"
#include "Suora/Renderer/RenderCommand.h"
#include "Panels/Minor/ContentBrowser.h"
#include "Suora/GameFramework/World.h"
#include "Suora/GameFramework/Nodes/MeshNode.h"
#include "Suora/GameFramework/Nodes/DecalNode.h"
#include "Suora/GameFramework/Nodes/ShapeNodes.h"
#include "Suora/GameFramework/Nodes/PostProcess/PostProcessNode.h"
#include "Suora/GameFramework/Nodes/Light/SkyLightNode.h"
#include "Suora/GameFramework/Nodes/Light/PointLightNode.h"
#include "Suora/GameFramework/Nodes/Light/DirectionalLightNode.h"
#include "Suora/Assets/Texture2D.h"

static Texture2D* CheckboxTickTexture = nullptr;
static Texture2D* CheckerboardTexture = nullptr;
static Texture2D* DragFloatTexture = nullptr;
static Texture2D* ArrowDown = nullptr;
static Texture2D* ArrowRight = nullptr;
static Suora::Ref<Shader> ColorCircleShader;

namespace Suora
{

	struct AssetPreview
	{
		Ref<Framebuffer> m_Preview;
		Ref<Framebuffer> m_GBuffer;
		uint32_t m_LastDraw = -10000;
		bool m_Init = false;
		bool m_Done = false;
		inline static RenderPipeline* s_Pipeline = nullptr;
		Ref<World> m_World = nullptr;
		
		AssetPreview()
		{
			{
				FramebufferSpecification spec;
				spec.Width = 128;
				spec.Height = 128;
				for (int32_t i = 0; i < (int32_t)GBuffer::GBufferSlotCount; i++)
				{
					spec.Attachments.Attachments.push_back(RenderPipeline::GBufferSlotToBufferParams((GBuffer)i));
				}
				spec.Attachments.Attachments.push_back(FramebufferTextureFormat::Depth);
				m_GBuffer = Framebuffer::Create(spec);
			}
			{
				FramebufferSpecification spec;
				spec.Width = 128;
				spec.Height = 128;
				spec.Attachments.Attachments.push_back(FramebufferTextureFormat::RGBA8);
				spec.Attachments.Attachments.push_back(FramebufferTextureFormat::Depth);
				m_Preview = Framebuffer::Create(spec);
			}
		}
		~AssetPreview()
		{
			m_Preview = nullptr;
			m_World = nullptr;
		}

		void Init(Asset* asset)
		{
			static bool pipelineInit = false;
			if (!pipelineInit)
			{
				s_Pipeline = Engine::Get()->GetRenderPipeline();
				pipelineInit = true;
			}
			m_World = CreateRef<World>();
			if (asset->IsA<Material>())
			{
				CameraNode* camera = m_World->Spawn<CameraNode>();
				m_World->Spawn<SkyLightNode>();
				m_World->Spawn<MeshNode>()->mesh = AssetManager::GetAsset<Mesh>(SuoraID("7d609382-dd13-4f97-b325-7d7a04309f47"));
				m_World->Spawn<PointLightNode>()->SetPosition(Vec3(-1.5f, 1.5f, -1.5f));
				DirectionalLightNode* dirLight = m_World->Spawn<DirectionalLightNode>();
				dirLight->m_ShadowMap = false;
				dirLight->SetEulerRotation(Vec3(45, 45, 0));
				m_World->SetMainCamera(camera);
				camera->SetPerspectiveVerticalFOV(25.0f);
				camera->SetPosition(Vec3(0, 0, -5));
				camera->SetViewportSize(256, 256);
				MeshNode* mesh = m_World->Spawn<MeshNode>();
				mesh->mesh = AssetManager::GetAsset<Mesh>(SuoraID("5c43e991-86be-48a4-8b14-39d275818ec1"));
				mesh->materials = asset->As<Material>();
				mesh->materials.OverwritteMaterials = true;
			}
			if (asset->IsA<Mesh>())
			{
				CameraNode* camera = m_World->Spawn<CameraNode>();
				m_World->Spawn<SkyLightNode>();
				m_World->Spawn<MeshNode>()->mesh = AssetManager::GetAsset<Mesh>(SuoraID("7d609382-dd13-4f97-b325-7d7a04309f47"));
				DirectionalLightNode* dirLight = m_World->Spawn<DirectionalLightNode>();
				dirLight->m_ShadowMap = true;
				dirLight->m_Intensity = 0.75f;
				dirLight->SetEulerRotation(Vec3(45, 45, 0));
				m_World->Spawn<PointLightNode>()->SetPosition(Vec3(-3, 3, -3));
				m_World->SetMainCamera(camera);
				camera->SetPerspectiveVerticalFOV(75.0f);
				camera->SetPosition(Vec3(asset->As<Mesh>()->m_BoundingSphereRadius * -1.0f, asset->As<Mesh>()->m_BoundingSphereRadius, asset->As<Mesh>()->m_BoundingSphereRadius * -1.20f));
				camera->SetEulerRotation(Vec3(45, 45, 0));
				camera->SetViewportSize(256, 256);
				MeshNode* mesh = m_World->Spawn<MeshNode>();
				mesh->mesh = asset->As<Mesh>();
			}
			if (asset->IsA<Blueprint>())
			{
				CameraNode* camera = m_World->Spawn<CameraNode>();
				m_World->Spawn<SkyLightNode>();
				m_World->Spawn<MeshNode>()->mesh = AssetManager::GetAsset<Mesh>(SuoraID("7d609382-dd13-4f97-b325-7d7a04309f47"));
				DirectionalLightNode* dirLight = m_World->Spawn<DirectionalLightNode>();
				dirLight->m_ShadowMap = false;
				dirLight->SetEulerRotation(Vec3(45, 45, 0));
				m_World->Spawn<PointLightNode>()->SetPosition(Vec3(-3, 3, -3));
				m_World->SetMainCamera(camera);
				camera->SetPerspectiveVerticalFOV(75.0f);
				camera->SetPosition(Vec3(5 * -1.0f, 5, 5 * -1.0f));
				camera->SetEulerRotation(Vec3(45, 45, 0));
				camera->SetViewportSize(256, 256);
				m_World->Spawn(Class(asset->As<Blueprint>()));
			}
		}
		void Render(Asset* asset)
		{
			//if (AssetManager::s_AssetStreamPool.Size() != 0) return;

			if (m_Done) return;
			if (asset->IsMissing()) return;
			if (asset->IsA<Mesh>() && !asset->As<Mesh>()->GetVertexArray()) return;
			if (asset->IsA<Material>() && !AssetManager::GetAsset<Mesh>(SuoraID("5c43e991-86be-48a4-8b14-39d275818ec1"))->GetVertexArray()) return;

			if (!m_Init)
			{
				m_Init = true;
				Init(asset);
				
			}
			Framebuffer* current = Framebuffer::GetCurrent();
			if (!m_Preview) return;
			m_Preview->Bind();

			RenderingParams RParams;
			if (asset->IsA<Material>())
			{
				s_Pipeline->Render(*m_Preview, *m_World, *m_World->GetMainCamera(), *m_GBuffer, RParams);
				///Engine::Get()->GetRenderPipeline()->Render(*m_Preview, *m_World, *m_World->GetMainCamera());
			}
			if (asset->IsA<Mesh>())
			{
				s_Pipeline->Render(*m_Preview, *m_World, *m_World->GetMainCamera(), *m_GBuffer, RParams);
				///Engine::Get()->GetRenderPipeline()->Render(*m_Preview, *m_World, *m_World->GetMainCamera());
			}
			if (asset->IsA<Blueprint>())
			{
				s_Pipeline->Render(*m_Preview, *m_World, *m_World->GetMainCamera(), *m_GBuffer, RParams);
				///Engine::Get()->GetRenderPipeline()->Render(*m_Preview, *m_World, *m_World->GetMainCamera());
			}
			m_Done = true;
			m_Init = false;
			m_GBuffer = nullptr;
			m_World = nullptr;

			m_Preview->Unbind();
			if (current) current->Bind();
		}
	};

	static bool CanRerenderAssetPreviews = false;
	static std::unordered_map<Asset*, Ref<AssetPreview>> AssetPreviews;

	void EditorUI::RerenderAssetPreviews()
	{
		static bool InRenderPhase = false;
		if (AssetManager::s_AssetStreamPool.Size() != 0 && !InRenderPhase) return;
		InRenderPhase = true;
		//if (Application::Get().GetRuntime() < 5.f) return;

		if (!CanRerenderAssetPreviews) return;
		CanRerenderAssetPreviews = false;

		rerun:
		int64_t Index = AssetPreviews.size() ? rand() % AssetPreviews.size() + (rand() % 3) : -1;
		for (auto& it : AssetPreviews)
		{
			Index--;

			/*if (it.second.m_LastDraw++ > 100)
			{
				AssetPreviews.erase(it.first);
				goto rerun;
			}*/

			if (Index == 0 && it.second)
			{
				if (it.second->m_Init && rand() % 3 == 0) goto rerun;
				it.second->Render(it.first);
				break;
			}
		}



	}

	void EditorUI::Init()
	{
		NativeInput::s_CharInputCallback.Register(&EditorUI::TextFieldCharInput);

		UiShader = Shader::Create(AssetManager::GetEngineAssetPath() + "/EditorContent/Shaders/UI/Rect.glsl");
		TextShader = Shader::Create(AssetManager::GetEngineAssetPath() + "/EditorContent/Shaders/Text.glsl");

		CheckboxTickTexture = AssetManager::GetAssetByName<Texture2D>("CheckboxTick.texture");
		CheckerboardTexture = AssetManager::GetAssetByName<Texture2D>("Checkerboard.texture");
		DragFloatTexture = AssetManager::GetAssetByName<Texture2D>("DragFloat.texture");
		ArrowDown = AssetManager::GetAssetByName<Texture2D>("ArrowDown.texture");
		ArrowRight = AssetManager::GetAssetByName<Texture2D>("ArrowRight.texture");
		ColorCircleShader = Shader::Create(AssetManager::GetEngineAssetPath() + "/EditorContent/Shaders/ColorCircle.glsl");

		{
			FramebufferSpecification specs;
			specs.Width = 192;
			specs.Height = 108;
			specs.Attachments.Attachments.push_back(FramebufferTextureParams(FramebufferTextureFormat::RGB8, FramebufferTextureFilter::Linear));
			GlassBuffer = Framebuffer::Create(specs);
		}

		s_ClassIcons[Node::StaticClass()]					= AssetManager::GetAsset<Texture2D>(SuoraID("ad168979-55cd-408e-afd2-a24cabf26922"));
		s_ClassIcons[MeshNode::StaticClass()]				= AssetManager::GetAsset<Texture2D>(SuoraID("b14b065c-a2c0-4dc9-9272-ab0415ada141"));
		s_ClassIcons[DecalNode::StaticClass()]				= AssetManager::GetAsset<Texture2D>(SuoraID("9d81a066-2336-42f5-bf35-7bb1c4c65d66"));
		s_ClassIcons[PointLightNode::StaticClass()]			= AssetManager::GetAsset<Texture2D>(SuoraID("f789d2bf-dcda-4e30-b2d9-3db979b7c6da"));
		s_ClassIcons[DirectionalLightNode::StaticClass()]	= AssetManager::GetAsset<Texture2D>(SuoraID("64738d74-08a9-4383-8659-620808d5269a"));
		s_ClassIcons[CameraNode::StaticClass()]				= AssetManager::GetAsset<Texture2D>(SuoraID("8952ef88-cbd0-41ab-9d3c-d4c4b39a30f9"));
		s_ClassIcons[BoxShapeNode::StaticClass()]			= AssetManager::GetAsset<Texture2D>(SuoraID("269931d5-7e60-4934-a89a-26b7993ae0f3"));
		s_ClassIcons[SphereShapeNode::StaticClass()]		= AssetManager::GetAsset<Texture2D>(SuoraID("7e43f48b-3dc8-4eab-b91a-b4e2e7999190"));
		s_ClassIcons[CapsuleShapeNode::StaticClass()]		= AssetManager::GetAsset<Texture2D>(SuoraID("b7221496-4fc6-4e08-9f23-655d5edfe820"));
		s_ClassIcons[CharacterNode::StaticClass()]			= AssetManager::GetAsset<Texture2D>(SuoraID("0660326f-a8a2-4314-bc96-9b61bdbddae3"));
		s_ClassIcons[PostProcessEffect::StaticClass()]		= AssetManager::GetAsset<Texture2D>(SuoraID("9bdeac52-f671-4e0a-9167-aeaa30c47711"));
		s_ClassIcons[LevelNode::StaticClass()]				= AssetManager::GetAsset<Texture2D>(SuoraID("3578494c-3c74-4aa5-8d34-4d28959a21f5"));
		s_ClassIcons[Component::StaticClass()]				= AssetManager::GetAsset<Texture2D>(SuoraID("3e254a4e-cc83-4254-a462-73739fce6d61"));
		s_ClassIcons[FolderNode::StaticClass()]				= AssetManager::GetAsset<Texture2D>(SuoraID("fd612d87-5fb3-4585-b66e-e42f275ef262"));
		s_ClassIcons[FolderNode3D::StaticClass()]           = AssetManager::GetAsset<Texture2D>(SuoraID("fd612d87-5fb3-4585-b66e-e42f275ef262"));
		s_ClassIcons[UIImage::StaticClass()]                = AssetManager::GetAsset<Texture2D>(SuoraID("26ed62e2-e39b-4028-aca2-7b116c74541f"));
	}

	// DrawRect
	static bool DrawRectInit = false;
	static Ref<VertexArray> DrawRectVAO;
	static Ref<VertexBuffer> DrawRectVBO;
	static Ref<IndexBuffer> DrawRectIB;

	void EditorUI::Tick(float deltaTime)
	{
		if (Window::s_CurrentFocusedWindow->GetCursor() != s_CurrentCursor) Window::s_CurrentFocusedWindow->SetCursor(s_CurrentCursor);
		s_CurrentCursor = Cursor::Default;

		if (s_WasInputConsumed > 0)
		{
			s_WasInputConsumed--;
		}

		for (int i = 0; i < s_Overlays.Size(); i++)
		{
			s_Overlays[i]->BackendRender(deltaTime);
			s_Overlays[i]->m_Lifetime++;
		}
		
		CanRerenderAssetPreviews = true;
		rerun:
		for (auto& it : AssetPreviews)
		{
			if (!it.second)
			{
				it.second = CreateRef<AssetPreview>();
			}
			else if (it.second->m_LastDraw++ > 1800)
			{
				AssetPreviews.erase(it.first);
				goto rerun;
			}
		}

		static int counter = 0;
		if (counter >= 512)
			RerenderAssetPreviews();
		else
			counter++;
	}

	void EditorUI::PushInput(float x, float y, float offsetX, float offsetY)
	{
		mousePosition = Vec2(x, y);
		mouseOffset = Vec2(offsetX, offsetY);
	}
	const Vec2& EditorUI::GetInput()
	{
		return mousePosition;
	}
	const Vec2& EditorUI::GetInputOffset()
	{
		return mouseOffset;
	}

	void EditorUI::ConsumeInput()
	{
		NativeInput::ConsumeInput();
		s_WasInputConsumed = 2;
	}

	bool EditorUI::WasInputConsumed()
	{
		return s_WasInputConsumed;
	}
	
	void EditorUI::InitDrawRectVAO()
	{
		DrawRectInit = true;
		UiShader->Bind();
		DrawRectVAO = VertexArray::Create();

		Vertex vertices[4] = { Vertex(Vec3(1, -1, 0), Vec2(1, 0)),
							  Vertex(Vec3(-1, 1, 0), Vec2(0, 1)),
							  Vertex(Vec3(1, 1, 0),  Vec2(1, 1)),
							  Vertex(Vec3(-1, -1, 0),Vec2(0, 0)) };
		uint32_t indices[6] = { 2, 1, 0, 0, 1, 3 };

		DrawRectVBO = VertexBuffer::Create(sizeof(Vertex) * 4);
		DrawRectVBO->SetLayout(VertexLayout::VertexBufferLayout);

		DrawRectIB = IndexBuffer::Create(&indices[0], sizeof(uint32_t) * 6);
		DrawRectVAO->SetIndexBuffer(DrawRectIB);

		DrawRectVBO->SetData(&vertices[0], sizeof(Vertex) * 4);
		DrawRectVAO->AddVertexBuffer(DrawRectVBO);
	}

	void EditorUI::DrawRect(float x, float y, float width, float height, float roundness, const Color& color)
	{
		RenderCommand::SetViewport(x, y, width, height);
		UiShader->Bind();
		UiShader->SetFloat2("u_Size", Vec2(width, height));
		UiShader->SetFloat4("u_Color", color);
		UiShader->SetFloat("u_Roundness", roundness);
		UiShader->SetBool("u_UseTexture", false);

		if (!DrawRectInit) InitDrawRectVAO();
		DrawRectVAO->Bind();
		RenderCommand::DrawIndexed(DrawRectVAO);
	}

	void EditorUI::DrawTexturedRect(Texture* texture, float x, float y, float width, float height, float roundness, const Color& color)
	{
		if (texture == nullptr)
		{
			return;
		}
		RenderCommand::SetViewport(x, y, width, height);
		UiShader->Bind();
		UiShader->SetFloat2("u_Size", Vec2(width, height));
		UiShader->SetFloat4("u_Color", color);
		UiShader->SetFloat("u_Roundness", roundness);
		UiShader->SetBool("u_UseTexture", true);
		texture->Bind(0);
		UiShader->SetInt("u_Texture", 0);

		if (!DrawRectInit) InitDrawRectVAO();
		DrawRectVAO->Bind();
		RenderCommand::DrawIndexed(DrawRectVAO);
	}

	void EditorUI::DrawTexturedRect(Ref<Texture> texture, float x, float y, float width, float height, float roundness, const Color& color)
	{
		if (texture.get() == nullptr)
		{
			return;
		}
		RenderCommand::SetViewport(x, y, width, height);
		UiShader->Bind();
		UiShader->SetFloat2("u_Size", Vec2(width, height));
		UiShader->SetFloat4("u_Color", color);
		UiShader->SetFloat("u_Roundness", roundness);
		UiShader->SetBool("u_UseTexture", true);
		texture->Bind(0);
		UiShader->SetInt("u_Texture", 0);

		if (!DrawRectInit) InitDrawRectVAO();
		DrawRectVAO->Bind();
		RenderCommand::DrawIndexed(DrawRectVAO);
	}

	void EditorUI::DrawRectOutline(float x, float y, float width, float height, float thickness, const Color& color)
	{
		DrawRect(x, y, width, thickness, 0, color); // BOTTOM
		DrawRect(x, y + height - thickness, width, thickness, 0, color); // TOP
		DrawRect(x, y, thickness, height, 0, color); // LEFT
		DrawRect(x + width - thickness, y, thickness, height, 0, color);
	}

	void EditorUI::Text(const std::string& text, Font* font, float x, float y, float width, float height, float size, const Vec2& orientation, const Color& color, const Array<Color>& colors)
	{
		RenderCommand::SetViewport(x, y, width, height);
		if (!font->m_IsAtlasLoaded)
		{
			font->LoadAtlas();
		}

		font->m_FontAtlas->Bind(0);

		Ref<VertexArray> vao = VertexArray::Create();

		uint32_t* iBase = new uint32_t[6 * text.length()];
		uint32_t* indices = iBase;

		Ref<VertexBuffer> vbo = VertexBuffer::Create(sizeof(Vertex) * 4 * text.length());
		vbo->SetLayout(VertexLayout::VertexBufferLayout);

		Vertex vertices[4] = { Vertex(Vec3(1, -1, 0), Vec2(1, 0)),
							  Vertex(Vec3(-1, 1, 0), Vec2(0, 1)),
							  Vertex(Vec3(1, 1, 0),  Vec2(1, 1)),
							  Vertex(Vec3(-1, -1, 0),Vec2(0, 0)) };

		Vertex* QuadVertexBufferBase = new Vertex[4 * text.length()];
		Vertex* QuadVertexBufferPtr = QuadVertexBufferBase;

		int offset = 0; float characterVirtualCursorX = 0;
		float const Scale = size / font->m_FontSize; // = DesiredSize / FontSize;
		float stringWidth = 0; for (char c : text) stringWidth += (font->GetMeta(c).xAdvance / width * Scale) - (font->GetMeta(c).xOffset / width * Scale);
		int characterIndex = 0;
		for (char character : text)
		{
			indices[0] = offset + 2; indices[1] = offset + 1; indices[2] = offset + 0;
			indices[3] = offset + 0; indices[4] = offset + 1; indices[5] = offset + 3;
			indices += 6; offset += 4;

			const FontMeta& meta = (font->m_FontInfo.find(character) == font->m_FontInfo.end()) ? font->m_FontInfo['?'] : font->m_FontInfo[character];
			float RelativeWidth = meta.width / (float)font->m_FontAtlas->GetWidth();
			float RelativeHeight = meta.height / (float)font->m_FontAtlas->GetHeight();
			float RelativeX = meta.x / (float)font->m_FontAtlas->GetWidth();
			float RelativeY = meta.y / (float)font->m_FontAtlas->GetHeight(); RelativeY = 1.0f - (RelativeY + RelativeHeight);

			// Vertex
			float const LineHeight = font->m_LineHeight / height * Scale;
			float minBase = 0, maxBase = 1 - LineHeight;
			float BaseLine = LineHeight * .5f;
			BaseLine = (orientation.y == 0.0f) ? (BaseLine) : ((orientation.y < 0.0f) ? Math::Lerp(BaseLine, -1.0f + LineHeight, orientation.y * -1)
				: Math::Lerp(BaseLine, 1.0f, orientation.y));
			float const halfWidth = 1.0f - (stringWidth / 2.0f);
			float characterHeight = meta.height / height * Scale;
			float characterWidth = meta.width / width * Scale;
			float characterY = (BaseLine)-(meta.yOffset / height * Scale) - characterHeight; // + -
			float characterX = -1.0f + halfWidth + (halfWidth * orientation.x); //characterX = -1;
			characterX += characterVirtualCursorX + (meta.xOffset / width * Scale);
			characterVirtualCursorX += (meta.xAdvance / width * Scale) - (meta.xOffset / width * Scale);

			for (size_t i = 0; i < 4; i++)
			{
				//QuadVertexBufferPtr->Position = vertices[i].Position;
				if (i == 0) QuadVertexBufferPtr->Position = Vec3(characterX + characterWidth, characterY, 0);
				if (i == 1) QuadVertexBufferPtr->Position = Vec3(characterX, characterY + characterHeight, 0);
				if (i == 2) QuadVertexBufferPtr->Position = Vec3(characterX + characterWidth, characterY + characterHeight, 0);
				if (i == 3) QuadVertexBufferPtr->Position = Vec3(characterX, characterY, 0);
				QuadVertexBufferPtr->Color = colors.Size() == 0 ? vertices[i].Color : colors[characterIndex];
				//QuadVertexBufferPtr->TexCoord = vertices[i].TexCoord;
				if (i == 0) QuadVertexBufferPtr->TexCoord = Vec2(RelativeX + RelativeWidth, RelativeY);
				if (i == 1) QuadVertexBufferPtr->TexCoord = Vec2(RelativeX, RelativeY + RelativeHeight);
				if (i == 2) QuadVertexBufferPtr->TexCoord = Vec2(RelativeX + RelativeWidth, RelativeY + RelativeHeight);
				if (i == 3) QuadVertexBufferPtr->TexCoord = Vec2(RelativeX, RelativeY);
				QuadVertexBufferPtr->TexIndex = vertices[i].TexIndex;
				QuadVertexBufferPtr->TilingFactor = vertices[i].TilingFactor;
				QuadVertexBufferPtr++;
			}
			characterIndex++;
		}
		uint32_t dataSize = (uint32_t)((uint8_t*)QuadVertexBufferPtr - (uint8_t*)QuadVertexBufferBase);
		vbo->SetData(QuadVertexBufferBase, dataSize);
		vao->AddVertexBuffer(vbo);

		Ref<IndexBuffer> ib = IndexBuffer::Create(iBase, 6 * text.length());
		vao->SetIndexBuffer(ib);

		TextShader->Bind();
		TextShader->SetFloat4("u_Color", color);
		TextShader->SetInt("u_Texture", 0);

		vao->Bind();
		RenderCommand::DrawIndexed(vao);
		delete[] QuadVertexBufferBase;
		delete[] iBase;
	}

	bool EditorUI::Button(const std::string& text, float x, float y, float width, float height, ButtonParams params)
	{
		bool Hovering = mousePosition.x >= x && mousePosition.x <= x + width && mousePosition.y >= y && mousePosition.y <= y + height && (GetHoveredOverlay() == s_CurrentProcessedOverlay);
		const bool ButtonClicked = Hovering && (params.OverrideActivationEvent ? params.OverrittenActivationEvent() : NativeInput::GetMouseButtonDown(Mouse::ButtonLeft)) && CurrentWindow->m_InputEvent == params.InputMode && !WasInputConsumed() && !CurrentWindow->GetWindow()->IsCursorLocked();

		if (params.OutHover) 
			*params.OutHover = Hovering;

		if (params.ButtonDropShadow)
			EditorUI::DrawRect(x + 4, y - 4, width, height, params.ButtonRoundness, Color(0, 0, 0, 0.25f));
		EditorUI::DrawRect(x, y, width, height, params.ButtonRoundness, (Hovering && params.useButtonOutlineHoverColor) ? params.ButtonOutlineHoverColor : params.ButtonOutlineColor);
		const Color color = Hovering ? params.ButtonColorHover : params.ButtonColor;
		EditorUI::DrawRect(x + 1, y + 1, width - 2, height - 2, params.ButtonRoundness, color);

		if (params.TextDropShadow)
			EditorUI::Text(text, params.Font, x + params.TextOffsetLeft + 2, y + params.TextOffsetBottom - 2, width - params.TextOffsetLeft - params.TextOffsetRight, height - params.TextOffsetBottom - params.TextOffsetTop, params.TextSize, params.TextOrientation, Color(0, 0, 0, 0.25f));
		EditorUI::Text(text, params.Font, x + params.TextOffsetLeft, y + params.TextOffsetBottom, width - params.TextOffsetLeft - params.TextOffsetRight, height - params.TextOffsetBottom - params.TextOffsetTop, params.TextSize, params.TextOrientation, params.TextColor);

		if (Hovering && params.TooltipText.length() > 0) 
			Tooltip(params.TooltipText);

		if (params.CenteredIcon)
		{
			DrawTexturedRect(params.CenteredIcon->GetTexture(), x, y, glm::min(width, height), glm::min(width, height), 0.0f, params.TextColor);
		}

		if (Hovering) 
			SetCursor(params.HoverCursor);

		return ButtonClicked;
	}

	bool EditorUI::DragSource(float x, float y, float width, float height, float offset, EditorInputEvent input)
	{
		struct DragSourceBuffer
		{
			float x = 0, y = 0, width = 0, height = 0;
		};
		static DragSourceBuffer Buffer;
		static bool inDrag = false;
		static Vec2 Pos;

		bool Hovering = mousePosition.x >= x && mousePosition.x <= x + width && mousePosition.y >= y && mousePosition.y <= y + height;
		const bool ButtonClicked = Hovering && NativeInput::GetMouseButtonDown(Mouse::ButtonLeft) && CurrentWindow->m_InputEvent == input;
		
		if (ButtonClicked)
		{
			inDrag = true;
			Buffer.x = x;
			Buffer.y = y;
			Buffer.width = width;
			Buffer.height = height;
			Pos = GetInput();
		}
		else if (NativeInput::GetMouseButtonUp(Mouse::ButtonLeft))
		{
			inDrag = false;
		}
		return inDrag && Buffer.x == x && Buffer.y == y && Buffer.width == width && Buffer.height == height && glm::distance(Pos, GetInput()) > offset;
	}

	bool EditorUI::DropDown(const std::vector<std::pair<std::string, std::function<void(void)>>>& options, int index, float x, float y, float width, float height)
	{
		if (Button(options[index].first, x, y, width, height))
		{
			std::vector<ContextMenuElement> context;
			for (auto& It : options)
			{
				ContextMenuElement element;
				element.Label = It.first;
				element.Lambda = It.second;
				context.push_back(element);
			}
			CreateContextMenu(context);
		}

		return false;
	}
	void EditorUI::TextFieldCharInput(char keyCode)
	{
		if (TextField_Str)
		{
			TextFieldCharBuffer.Add(keyCode);
		}
	}
	void EditorUI::TextField(std::string* str, float x, float y, float width, float height, ButtonParams params, const std::function<void(std::string)>& lambda)
	{
		if (EditorUI::Button(*str, x, y, width, height, params) && !TextField_Str && CurrentWindow->m_InputEvent == EditorInputEvent::None)
		{
			TextField_StrFlag = true;
			TextField_Str = str;
			TextFieldCharBuffer.Clear();
			CreateOverlay<TextFieldOverlay>(x + GetInputOffset().x, y + GetInputOffset().y, width, height, str, params.TextSize, lambda);
		}

		if (TextField_Str == str)
		{
			TextField_StrFlag = true;
		}

	}

	void EditorUI::_SetTextFieldStringPtr(std::string* str, float x, float y, float width, float height, bool needsFlag)
	{
		TextField_StrFlag = true;
		TextField_Str = str;
		TextFieldCharBuffer.Clear();
		TextFieldOverlay* overlay = CreateOverlay<TextFieldOverlay>(x + GetInputOffset().x, y + GetInputOffset().y, width, height, str, 32.0f);
		overlay->needsFlag = needsFlag;
	}

	void EditorUI::DragFloat(float* f, float x, float y, float width, float height, const std::function<void(std::string)>& lambda)
	{
		struct DragFloatTextField : public TextFieldOverlay
		{
			DragFloatTextField(std::string* str, const std::function<void(std::string)>& lambda, bool needsFlag = true) : TextFieldOverlay(str, 26.0f, lambda, needsFlag) 
			{
				CursorSelectionOffset = str->size();
			}
			void OnDispose() override
			{
				if (NativeInput::GetKeyDown(Key::Tab)) DraggedFloatTabulatePtr = DraggedFloatPtr;
				*DraggedFloatPtr = Util::StringToFloat(DraggedFloatStr);// std::stof(DraggedFloatStr);
				DraggedFloatPtr = nullptr;
			}
		};
		// Tabulating
		if (f == DraggedFloatTabulatePtr) { DraggedFloatTabulatePtr = nullptr; DraggedFloatTabulateNext = true; return; }
		if (DraggedFloatTabulateNext)
		{
			DraggedFloatTabulateNext = false;
			DraggedFloatPtr = f;
			DraggedFloatStr = Util::FloatToString(*DraggedFloatPtr);
			DragFloatTextField* overlay = CreateOverlay<DragFloatTextField>(x + GetInputOffset().x, y + GetInputOffset().y, width, height, &DraggedFloatStr, lambda, false);
			return;
		}

		ButtonParams Params;
		Params.TextOrientation = Vec2(-1, 0);
		Params.TextOffsetLeft = 5.0f;
		Params.HoverCursor = Cursor::HorizontalResize;
		if (Button(Util::FloatToString(*f), x, y, width, height, Params) && CurrentWindow->m_InputEvent == EditorInputEvent::None)
		{
			CurrentWindow->m_InputEvent = EditorInputEvent::EditorUI_DragFloat;
			DraggedFloatPtr = f;
			DraggedFloatBeginValue = *f;
			CurrentWindow->GetWindow()->SetCursorLocked(true);
		}
		else if (f == DraggedFloatPtr && CurrentWindow->m_InputEvent == EditorInputEvent::EditorUI_DragFloat && !NativeInput::GetMouseButton(Mouse::ButtonLeft)) // f == DraggedFloatPtr might lead to softlocks, if DragFloat() is not called on the float* 
		{
			CurrentWindow->m_InputEvent = EditorInputEvent::None;
			CurrentWindow->GetWindow()->SetCursorLocked(false);
			if (*DraggedFloatPtr == DraggedFloatBeginValue)
			{
				DraggedFloatStr = Util::FloatToString(*DraggedFloatPtr);
				DragFloatTextField* overlay = CreateOverlay<DragFloatTextField>(x + GetInputOffset().x, y + GetInputOffset().y, width, height, &DraggedFloatStr, lambda, false);
			}
			else
			{
				DraggedFloatPtr = nullptr;
			}
		}

		if (CurrentWindow->m_InputEvent == EditorInputEvent::EditorUI_DragFloat && f == DraggedFloatPtr)
			*f += NativeInput::GetMouseDelta().x / 50.0f;

		/*EditorUI::DrawRect(x, y, width, height, 4, Hovering ? Color(0.5f, 0.5f, 0.5f, 1.0f) : Color(0.3f, 0.3f, 0.3f, 1.0f));
		EditorUI::DrawRect(x + 1, y + 1, width - 2, height - 2, 4, EditorPreferences::Get()->UiInputColor);*/

		//EditorUI::Text(Util::FloatToString(*f), Font::Instance, x + 10, y, width * 0.75f, height, 28, Vec2(-1, 0), Color(1));
		EditorUI::DrawTexturedRect(DragFloatTexture->GetTexture(), x + width - height, y + height * 0.05f, height * 0.9f, height * 0.9f, 0, Color(1.0f, 1.0f, 1.0f, 0.5f));
	}

	inline void EditorUI::SliderFloat(float* f, float min, float max, float x, float y, float width, float height)
	{
		if (max <= min) SUORA_ASSERT(false, "EditorUI Slider -> Assert: MIN has to be lower than MAX!");
		bool Hovering = mousePosition.x >= x && mousePosition.x <= x + width && mousePosition.y >= y && mousePosition.y <= y + height;
		if (Hovering) SetCursor(Cursor::HorizontalResize);

		if (Hovering && NativeInput::GetMouseButton(Mouse::ButtonLeft) && CurrentWindow->m_InputEvent == EditorInputEvent::None)
		{
			CurrentWindow->m_InputEvent = EditorInputEvent::EditorUI_SliderFloat;
			SliderFloat_F = f;
		}
		else if (CurrentWindow->m_InputEvent == EditorInputEvent::EditorUI_SliderFloat && !NativeInput::GetMouseButton(Mouse::ButtonLeft))
		{
			CurrentWindow->m_InputEvent = EditorInputEvent::None;
			SliderFloat_F = nullptr;
		}

		if (CurrentWindow->m_InputEvent == EditorInputEvent::EditorUI_SliderFloat && f == SliderFloat_F)
			*f = Math::Clamp(Math::Remap(GetInput().x, x, x + width, min, max), min, max);

		EditorUI::DrawRect(x, y + height * .33f, width, height * .33f, 4, Color(.25f, .25f, .25f, 1));
		if (*f >= min && *f <= max) EditorUI::DrawRect(x - height / 2.f + Math::Remap(*f, min, max, 0, width), y, height, height, height / 2, Color(.45f, .45f, .45f, 1));
	}

	bool EditorUI::Checkbox(bool* b, float x, float y, float width, float height)
	{
		bool HasChanged = false;
		
		if (Button("", x, y, width, height))
		{
			*b = !(*b);
			HasChanged = true;
		}

		if (*b)
		{
			EditorUI::DrawTexturedRect(CheckboxTickTexture->GetTexture(), x, y, width, height, 4, EditorPreferences::Get()->UiHighlightColor);
		}
		return HasChanged;
	}

	static Asset** AssetDropDownTarget = nullptr;
	static Asset*  AssetDropDownAsignment = nullptr;
	static bool	   AssetDropDownDone = false;
	struct AssetDropDownOverlay : public EditorUI::Overlay
	{
		inline static std::string s_SearchLabel, s_LastSearchLabel;
		Array<Asset*> m_Entries;
		Class m_AssetClass = Class::None;
		float m_ScrollY = 0.0f;
		int m_EntriesPerRow = 3;
		int m_Ticks = 0;
		AssetDropDownOverlay(const Class& assetClass) : m_AssetClass(assetClass) 
		{ 
			s_SearchLabel = s_LastSearchLabel = ""; 
			PullAssetEntries();
		}
		void PullAssetEntries()
		{
			m_Entries.Clear();
			m_Entries = AssetManager::GetAssetsByClass(m_AssetClass);
		}
		void PullAssetEntries(const std::string& label)
		{
			const std::string lowerLabel = Util::ToLower(label);
			m_Entries.Clear();
			Array<Asset*> assets = AssetManager::GetAssetsByClass(m_AssetClass);
			for (Asset* asset : assets)
			{
				if (Util::ToLower(asset->GetAssetName()).find(lowerLabel) != std::string::npos) m_Entries.Add(asset);
			}
		}
		virtual void Render(float deltaTime) override
		{
			if (m_Ticks++ < 5) return;
			EditorUI::DrawRect(x, y, width, height, 2.0f, Color(0.1f, 0.1f, 0.11f, 1.0f));

			if (s_SearchLabel != s_LastSearchLabel)
			{
				s_LastSearchLabel = s_SearchLabel;
				if (s_SearchLabel != "") PullAssetEntries(s_SearchLabel);
				else					 PullAssetEntries();
			}

			EditorUI::ButtonParams params;
			params.OverrideActivationEvent = true;
			params.OverrittenActivationEvent = [=]() { return NativeInput::GetMouseButtonDown(Mouse::ButtonLeft) && EditorUI::GetInput().y >= y + 50.0f && EditorUI::GetInput().y <= y + height - 55.0f; };
			float w = y + height - 55.0f; w -= 51.0f;
			float k = x + 5.0f;
			const float entryWidth = (width - 15.0f) / (float)m_EntriesPerRow;
			for (Asset* entry : m_Entries)
			{
				if (entry->IsFlagSet(AssetFlags::Missing)) continue;
				if (w + m_ScrollY <= y || w + m_ScrollY >= y + height - 55.0f) goto skipDraw;
				if (EditorUI::Button("", k, w + m_ScrollY, entryWidth, 50.0f, params))
				{
					AssetDropDownAsignment = entry;
					AssetDropDownDone = true;
					Dispose();
				}
				EditorUI::DrawAssetPreview(entry, m_AssetClass, k + 2.0f, w + m_ScrollY + 2.0f, 46.0f, 46.0f);
				EditorUI::Text(entry->GetAssetName(), Font::Instance, k + 55.0f, w + m_ScrollY + 25.0f, entryWidth - 55.0f, 25.0f, 22.0f, glm::ivec2(-1, 0), Color(1));
				skipDraw:
				k += entryWidth + 1.0f;
				if (k >= x + width - 150.0f)
				{
					k = x + 5.0f;
					w -= 51.0f;
				}
			}

			EditorUI::DrawRect(x, y, width, 50.0f, 2.0f, Color(0.1f, 0.1f, 0.11f, 1.0f));
			EditorUI::DrawRect(x, y + height - 55.0f, width, 55.0f, 2.0f, Color(0.1f, 0.1f, 0.11f, 1.0f));

			if (EditorUI::Button("None", x + width - (width / 3.0f) - 5.0f, y + 7.5f, width / 3.0f, 35.0f))
			{
				AssetDropDownAsignment = nullptr;
				AssetDropDownDone = true;
				Dispose();
			}
			EditorUI::Text(std::to_string(m_Entries.Size()) + " Entries", Font::Instance, x + 5.0f, y + height - 55.0f, 125.0f, 25.0f, 28.0f, Vec2(-1, 0), Color(0.8f));

			EditorUI::TextField(&s_SearchLabel, x + 5.0f, y + height - 30.0f, width - 10.0f, 25.0f);
			if (s_SearchLabel == "") EditorUI::Text("Search...", Font::Instance, x + 5.0f, y + height - 30.0f, width - 10.0f, 25.0f, 18.0f, Vec2(-0.95f, 0.0f), Color(1.0f));

			const float EntryHeight = 25.0f;
			const float scrollDown = height - EntryHeight * m_Entries.Size() - (EntryHeight + 15.0f);
			EditorUI::ScrollbarVertical(x + width - 6.0f, y + 50.0f, 5.0f, height - 80.0f, x, y, width, height, 0.0f, scrollDown > 0 ? 0 : Math::Abs(scrollDown), & m_ScrollY);

			if (WasMousePressedOutsideOfOverlay() && m_Ticks > 5) Dispose();
		}
		void OnDispose() override
		{
		}
	};

	bool EditorUI::AssetDropdown(Asset** asset, Class assetClass, float x, float y, float width, float height)
	{
		SuoraVerify(asset);
		Array<Asset*> assets = AssetManager::GetAssetsByClass(assetClass);
		std::vector<std::string> assetsPaths;
		assetsPaths.push_back("None");
		for (int j = 0; j < assets.Size(); j++) assetsPaths.push_back(assets[j]->m_Name);

		EditorUI::DrawRect(x, y, height, height, 3.0f, EditorPreferences::Get()->UiInputColor);
		EditorUI::DrawRect(x, y, height, 2.0f, 1.0f, EditorPreferences::GetAssetClassColor(assetClass.GetNativeClassID()));
		DrawAssetPreview(*asset, assetClass, x + 2, y + 2, height - 4.0f, height - 4.0f);
		if (asset && *asset && GetHoveredOverlay() == s_CurrentProcessedOverlay)
		{
			EditorUI::Tooltip((*asset)->IsFlagSet(AssetFlags::Missing) ? "Missing Asset" : (*asset)->m_Path.string(), x + 2, y + 2, height - 4.0f, height - 4.0f);
		}
		int i = assets.IndexOf((Asset*)asset);
		if (i == -1) i = 0; else i++;

		std::string assetName = (*asset) ? (*asset)->GetAssetName() : "None";
		ButtonParams DropDownParams;
		if (*asset && (*asset)->IsFlagSet(AssetFlags::Missing))
		{
			assetName = "Missing Asset";
			DropDownParams.TextColor = Color(1, 0, 0, 1);
		}
		if (EditorUI::Button(assetName, x + height, y + height * 0.5f, width - height, height * 0.5f, DropDownParams))
		{
			AssetDropDownTarget = asset;
			CreateOverlay<AssetDropDownOverlay>(x + GetInputOffset().x - 300, y + GetInputOffset().y - 300, 600, 300, assetClass);
		}
		EditorUI::DrawTexturedRect(ArrowDown->GetTexture(), x + width - height * 0.5f - 2.5f, y + height * 0.5f, height * 0.5f, height * 0.5f, 0.0f, Color(1.0f));

		ButtonParams chooseSelectedAssetParams;
		chooseSelectedAssetParams.CenteredIcon = AssetManager::GetAsset<Texture2D>(SuoraID("f11389f9-2062-497a-a24c-c27c3c7b93f9"));
		chooseSelectedAssetParams.TooltipText = ContentBrowser::s_LastSelectedAsset ? "Use selected Asset: " + ContentBrowser::s_LastSelectedAsset->m_Path.string()
																					: "Select an Asset in the ContentDrawer first!";
		if (EditorUI::Button("", x + height, y, height * 0.5f, height * 0.5f, chooseSelectedAssetParams) && ContentBrowser::s_LastSelectedAsset)
		{
			if (ContentBrowser::s_LastSelectedAsset->GetClass().Inherits(assetClass))
			{
				Asset*& ref = *asset;
				ref = ContentBrowser::s_LastSelectedAsset;
				return true;
			}
		}
		ButtonParams browseToAssetParams;
		browseToAssetParams.CenteredIcon = AssetManager::GetAsset<Texture2D>(SuoraID("fd612d87-5fb3-4585-b66e-e42f275ef262"));
		if (asset && *asset) browseToAssetParams.TooltipText = "Browse to Asset: " + (*asset)->m_Path.string();
		if (EditorUI::Button("", x + height + height * 0.5f, y, height * 0.5f, height * 0.5f, browseToAssetParams) && *asset)
		{
			ContentBrowser::s_BrowseToAsset = *asset;
			CurrentWindow->ForceOpenContentDrawer();
		}

		if (AssetDropDownTarget == asset && AssetDropDownDone)
		{
			Asset*& ref = *AssetDropDownTarget;
			ref = AssetDropDownAsignment;
			AssetDropDownTarget = nullptr;
			AssetDropDownAsignment = nullptr;
			AssetDropDownDone = false;
			return true;
		}
		if (CurrentWindow->m_InputEvent == EditorInputEvent::ContentBrowser_AssetDrag && ContentBrowser::s_DraggedAsset && (ContentBrowser::s_DraggedAsset->GetClass().Inherits(assetClass) || ContentBrowser::s_DraggedAsset->GetClass() == assetClass))
		{
			EditorUI::DrawRect(x, y, width, height, 0.0f, EditorPreferences::Get()->UiHighlightColor * 0.5f);
			EditorUI::DrawRectOutline(x, y, width, height, 2.0f, EditorPreferences::Get()->UiHighlightColor);
			ButtonParams DropParams = ButtonParams::Invisible();
			DropParams.OverrideActivationEvent = true;
			DropParams.OverrittenActivationEvent = []() { return NativeInput::GetMouseButtonUp(Mouse::ButtonLeft); };
			DropParams.InputMode = EditorInputEvent::ContentBrowser_AssetDrag;
			if (EditorUI::Button("", x, y, width, height, DropParams))
			{
				Asset*& ref = *asset;
				ref = ContentBrowser::s_DraggedAsset;
				return true;
			}
		}

		return false;
	}

	void EditorUI::DrawAssetPreview(Asset* asset, const Class& assetClass, float x, float y, float width, float height)
	{
		EditorUI::DrawTexturedRect(CheckerboardTexture->GetTexture(), x, y, width, height, 0.0f, Color(0.1f, 0.1f, 0.1f, 1));
		if (!asset) return;
		if (!Engine::Get()->GetRenderPipeline()->IsA<RenderPipeline>()) return;

		if (Texture2D* texture = Cast<Texture2D>(asset))
		{
			EditorUI::DrawTexturedRect(texture->GetTexture(), x, y, width, height, 0.0f, Color(1.0f));
		}
		else
		{
			if (AssetPreviews.find(asset) == AssetPreviews.end())
			{
				AssetPreviews[asset] = nullptr;
				//AssetPreviews[asset].Render(asset);
			}
			
			if (!AssetPreviews[asset]) return;
			if (AssetPreviews[asset]->m_LastDraw < 0) return;
			AssetPreviews[asset]->m_LastDraw = 0;
			//RerenderAssetPreviews();

			if (AssetPreviews.find(asset) != AssetPreviews.end() && AssetPreviews[asset]->m_Preview && AssetPreviews[asset]->m_Done)
			{
				RenderPipeline::GetFullscreenPassShaderStatic()->Bind();
				RenderPipeline::GetFullscreenPassShaderStatic()->SetInt("u_Texture", 0);
				AssetPreviews[asset]->m_Preview->BindColorAttachmentByIndex(0, 0);
				RenderCommand::SetViewport(x, y, width, height);
				RenderCommand::DrawIndexed(RenderPipeline::__GetFullscreenQuad());
			}
		}
	}

	void EditorUI::SubclassSelectionMenu(const Class& base, const std::function<void(Class)>& lambda)
	{
		Array<Class> subClasses = Class::GetSubclassesOf(base);

		SelectionOverlay* overlay = CreateOverlay<SelectionOverlay>(NativeInput::GetMousePosition().x,  CurrentWindow->GetWindow()->GetHeight() - NativeInput::GetMousePosition().y - 400.0f, 400.0f, 400.0f);
		for (Class& cls : subClasses)
		{
			std::string className = cls.GetClassName();
			Class level = cls;
			while (level != base)
			{
				level = level.GetParentClass();
				className.insert(0, level.GetClassName() + "/");
			}
			overlay->m_Entries.Add(SelectionOverlay::SelectionOverlayEntry(className, {}, [lambda, cls]() { lambda(cls); }));
		}
		overlay->RefreshEntries();
	}

	EditorUI::Overlay* EditorUI::GetHoveredOverlay()
	{
		for (int32_t i = s_Overlays.Last(); i >= 0; i--)
		{
			if (!s_Overlays[i]) continue;

			Vec2 mPos = NativeInput::GetMousePosition();
			mPos.y = CurrentWindow->GetWindow()->GetHeight() - mPos.y;
			bool Hovering = mPos.x >= s_Overlays[i]->x && mPos.x <= s_Overlays[i]->x + s_Overlays[i]->width && mPos.y >= s_Overlays[i]->y && mPos.y <= s_Overlays[i]->y + s_Overlays[i]->height;
			if (Hovering)
			{
				return s_Overlays[i].get();
			}
		}
		return nullptr;
	}

	bool EditorUI::IsNotHoveringOverlays()
	{
		return GetHoveredOverlay() == nullptr;
	}

	void EditorUI::RenderOverlays(float deltaTime)
	{
		Overlay::s_InputEventFlag = Overlay::s_InputEventFlag ? (CurrentWindow->m_InputEvent != EditorInputEvent::None) : (CurrentWindow->m_InputEvent == EditorInputEvent::EditorUI_Overlay);
		if (Overlay::s_InputEventFlag && (CurrentWindow->m_InputEvent == EditorInputEvent::EditorUI_Overlay)) CurrentWindow->m_InputEvent = EditorInputEvent::None;

		for (int32_t i = 0; i < s_Overlays.Size(); i++)
		{
			s_CurrentProcessedOverlay = s_Overlays[i].get();
			if (s_Overlays[i])
			{
				s_Overlays[i]->Render(deltaTime);
			}
		}
		s_CurrentProcessedOverlay = nullptr;

		for (Overlay* It : s_IssuedDisposalsOfOverlays)
		{
			for (int32_t i = s_Overlays.Last(); i >= 0; i--)
			{
				if (s_Overlays[i].get() == It)
				{
					s_Overlays.RemoveAt(i);
					break;
				}
			}
		}
		s_IssuedDisposalsOfOverlays.Clear();

		if (Overlay::s_InputEventFlag && CurrentWindow->m_InputEvent == EditorInputEvent::None && s_Overlays.Size() > 0)
		{
			CurrentWindow->m_InputEvent = EditorInputEvent::EditorUI_Overlay;
			Overlay::s_InputEventFlag = false;
		}
	}

	Texture2D* EditorUI::GetClassIcon(const Class& cls)
	{
		Class It = cls;

		while (It != Class::None)
		{
			if (s_ClassIcons.find(It) != s_ClassIcons.end())
			{
				return s_ClassIcons[It];
			}
			else
			{
				It = It.GetParentClass();
			}
		}

		return nullptr;
	}

	bool EditorUI::CategoryShutter(int64_t id, const std::string& category, float x, float& y, float width, float height, ButtonParams params)
	{
		y += 1.0f;
		if (CategoryShutterStates.find(id) == CategoryShutterStates.end()) CategoryShutterStates[id] = true;
		bool& state = CategoryShutterStates[id];
		if (Button("", x, y, width, height, params))
		{
			state = !state;
		}
		const float imageSize = 20.0f; //25.0f;
		Text(Util::SmartToUpperCase(category, false), Font::Instance, x + imageSize + 10 + 2, y - 2, width - height - 10, height, params.TextSize, Vec2(-1.0f, 0.0f), Color(0, 0, 0, 0.25f));
		Text(Util::SmartToUpperCase(category, false), Font::Instance, x + imageSize + 10, y, width - height - 10, height, params.TextSize, Vec2(-1.0f, 0.0f), Color(1.0f));
		EditorUI::DrawTexturedRect(state ? ArrowDown->GetTexture() : ArrowRight->GetTexture(), x + (imageSize / 4.0f) + 2.0f, y + (height - imageSize) / 2.0f - 2.0f, imageSize, imageSize, 0.0f, Color(0.0f, 0.0f, 0.0f, 0.2f));
		EditorUI::DrawTexturedRect(state ? ArrowDown->GetTexture() : ArrowRight->GetTexture(), x + (imageSize / 4.0f), y + (height - imageSize) / 2.0f, imageSize, imageSize, 0.0f, Color(1.0f));
		return state;
	}

	void EditorUI::ScrollbarVertical(float x, float y, float width, float height, float rectX, float rectY, float rectWidth, float rectHeight, float scrollUp, float scrollDown, float* scrollCurrent)
	{
		if (Math::Abs(scrollUp) == 0 && Math::Abs(scrollDown) == 0) { (*scrollCurrent) = 0.0f; return; }
		SUORA_ASSERT(scrollUp >= 0.0f && scrollDown >= 0.0f, "scrollUp and scrollDown must never be negative.");

		float scrollWidgetLengthScalar = height / (height + Math::Abs(scrollUp) + Math::Abs(scrollDown));
		float scrollWidgetPos = Math::Remap((*scrollCurrent), scrollUp * -1, scrollDown, height - (height * scrollWidgetLengthScalar), 0) + y;

		bool Hovering = mousePosition.x >= x && mousePosition.x <= x + width && mousePosition.y >= scrollWidgetPos && mousePosition.y <= scrollWidgetPos + (height * scrollWidgetLengthScalar) && (GetHoveredOverlay() == s_CurrentProcessedOverlay);

		EditorUI::DrawRect(x, y, width, height, width / 2, EditorPreferences::Get()->UiColor);
		EditorUI::DrawRect(x, scrollWidgetPos, width, height * scrollWidgetLengthScalar, width/2, Hovering ? EditorPreferences::Get()->UiForgroundColor * 1.2f : EditorPreferences::Get()->UiForgroundColor);

		// Scrolling
		if (mousePosition.x >= rectX && mousePosition.x <= rectX + rectWidth && mousePosition.y >= rectY && mousePosition.y <= rectY + rectHeight && (GetHoveredOverlay() == s_CurrentProcessedOverlay))
			(*scrollCurrent) -= NativeInput::GetMouseScrollDelta() * 50;

		// Dragging
		if (Hovering && NativeInput::GetMouseButtonDown(Mouse::ButtonLeft) && CurrentWindow->m_InputEvent == EditorInputEvent::None)
		{
			ScrollbarVerticalPtr = scrollCurrent;
			CurrentWindow->m_InputEvent = EditorInputEvent::EditorUI_ScrollbarVertical;
			ScrollbarVerticalY = mousePosition.y;
		}
		if (CurrentWindow->m_InputEvent == EditorInputEvent::EditorUI_ScrollbarVertical && ScrollbarVerticalPtr == scrollCurrent)
		{
			float delta = ScrollbarVerticalY - mousePosition.y;
			ScrollbarVerticalY = mousePosition.y;
			(*scrollCurrent) += delta / scrollWidgetLengthScalar;
		}
		if (!NativeInput::GetMouseButton(Mouse::ButtonLeft) && CurrentWindow->m_InputEvent == EditorInputEvent::EditorUI_ScrollbarVertical)
		{
			ScrollbarVerticalPtr = nullptr;
			CurrentWindow->m_InputEvent = EditorInputEvent::None;
		}

		// Clamp
		(*scrollCurrent) = Math::Clamp((*scrollCurrent), scrollUp * -1, scrollDown);
	}

	void EditorUI::Tooltip(const std::string& text, float x, float y, float width, float height)
	{
		bool Hovering = mousePosition.x >= x && mousePosition.x <= x + width && mousePosition.y >= y && mousePosition.y <= y + height;
		if (Hovering)
		{
			tooltipFrames = 0;
			tooltipText = text;
		}
	}

	void EditorUI::Tooltip(const std::string& text)
	{
		tooltipFrames = 0;
		tooltipText = text;
	}

	void EditorUI::ColorPicker(Color* color, float x, float y, float width, float height, ButtonParams params, const std::function<void(void)>& OnColorChange, const std::function<void(void)>& OnColorReset)
	{
		struct ColorPickerOverlay : public DragableOverlay
		{
			Color* color = nullptr;
			Color originalColor;
			std::function<void(void)> OnColorChange; 
			std::function<void(void)> OnColorReset;
			ColorPickerOverlay(Color* color, const std::function<void(void)>& onColorChange, const std::function<void(void)>& onColorReset) 
				: color(color), originalColor(*color), OnColorChange(onColorChange), OnColorReset(onColorReset), DragableOverlay("Color Picker") { }

			void Render(float deltaTime) override
			{
				DragableOverlay::Render(deltaTime);
				if (WasMousePressedOutsideOfOverlay())
				{
					OnColorChange();
					Dispose();
					return;
				}
				//EditorUI::DrawRect(x, y, width, height, 4, Color(1));
				EditorUI::DrawRect(x + 50, y + 150, width - 100, height - 200, 4, *color);

				RenderCommand::SetViewport(x + 50, y + 150, width - 100, height - 200);
				RenderPipeline::__GetFullscreenQuad()->Bind();
				ColorCircleShader->Bind();
				RenderCommand::DrawIndexed(RenderPipeline::__GetFullscreenQuad());

				Color temp = *color;

				Text("Red", Font::Instance, x + 5, y + 110, 35.0f, 20, 24.0f, Vec2(-1.0f, 0.0f), Color(1.0f)); 
					SliderFloat(&color->r, 0.0f, 1.0f, x + 45, y + 110, width - 130, 20);
					DragFloat(&color->r, x + 45 + (width - 130), y + 110, width - (width - 130) - 45 - 5, 20);
				Text("Green", Font::Instance, x + 5, y + 80, 35.0f, 20, 24.0f, Vec2(-1.0f, 0.0f), Color(1.0f));
					SliderFloat(&color->g, 0.0f, 1.0f, x + 45, y + 80, width - 130, 20);
					DragFloat(&color->g, x + 45 + (width - 130), y + 80, width - (width - 130) - 45 - 5, 20);
				Text("Blue", Font::Instance, x + 5, y + 50, 35.0f, 20, 24.0f, Vec2(-1.0f, 0.0f), Color(1.0f));
					SliderFloat(&color->b, 0.0f, 1.0f, x + 45, y + 50, width - 130, 20);
					DragFloat(&color->b, x + 45 + (width - 130), y + 50, width - (width - 130) - 45 - 5, 20);

				// Clamp values
				color->r = Math::Clamp(color->r, 0.0f, 1.0f);
				color->g = Math::Clamp(color->g, 0.0f, 1.0f);
				color->b = Math::Clamp(color->b, 0.0f, 1.0f);

				if (Button("Cancel", x + 50, y + 20, width - 100, 25))
				{
					*color = originalColor;
					OnColorReset();
					Dispose();
					return;
				}

				if (temp != *color)
				{
					OnColorChange();
				}
			}
		};

		if (Button("", x, y, width, height, params))
		{
			CreateOverlay<ColorPickerOverlay>(x + GetInputOffset().x, y + GetInputOffset().y, 250, 350, color, OnColorChange, OnColorReset);
		}
		EditorUI::DrawRect(x + 5, y + 5, width - 10, height - 10, params.ButtonRoundness, *color);
	}

	void EditorUI::CreateContextMenu(const std::vector<ContextMenuElement>& elements)
	{
		CreateContextMenu(elements, GetInput().x, GetInput().y);
	}
	void EditorUI::CreateContextMenu(const std::vector<ContextMenuElement>& elements, float x, float y)
	{
		struct ContextMenuOverlay : EditorUI::Overlay
		{
			std::vector<ContextMenuElement> Elements;
			ContextMenuOverlay* ParentMenu = nullptr;
			ContextMenuOverlay* ChildMenu = nullptr;
			bool InputReady = false;

			ContextMenuOverlay(const std::vector<ContextMenuElement>& elements)
				: Elements(elements)
			{
			}

			void OnDispose() override
			{
				if (ParentMenu) ParentMenu->Dispose();
			}

			void Render(float deltaTime) override
			{
				EditorUI::Overlay::Render(deltaTime);
				if (WasMousePressedOutsideOfOverlay() || NativeInput::GetMouseButtonDown(Mouse::ButtonRight))
				{
					if (!ChildMenu && InputReady)
					{
						if (ParentMenu)
						{
							ParentMenu->ChildMenu = nullptr;
							ParentMenu->InputReady = false;
						}
						Dispose();
					}
				}
				EditorUI::DrawRect(x, y, width, height, 0.0f, EditorPreferences::Get()->UiForgroundColor);

				float t = y + height;
				EditorUI::ButtonParams ContextParams = EditorUI::ButtonParams::Invisible();
				ContextParams.ButtonColorHover = EditorPreferences::Get()->UiHighlightColor;
				ContextParams.ButtonRoundness = 0.0f;
				ContextParams.TextOrientation = Vec2(-1, 0);

				for (auto& it : Elements)
				{
					t -= 25.0f;
					bool outHover = false;
					ContextParams.OutHover = &outHover;
					if (EditorUI::Button("", x, t, width, 24.0f, ContextParams) && !ChildMenu && InputReady)
					{
						if (it.SubElements.size() == 0)
						{
							it.Lambda();
							Dispose();
						}
						else
						{
							ChildMenu = CreateOverlay<ContextMenuOverlay>(x + 225, t, 225, it.SubElements.size() * 25.0f + 5.0f, it.SubElements);
							ChildMenu->ParentMenu = this;
						}
					}
					EditorUI::Text(it.Label, Font::Instance, x + 27.0f, t, width - 27.0f, 24.0f, 22.0f, Vec2(-1, 0), outHover ? Color(0, 0, 0, 1) : Color(1));
					if (it.SubElements.size() > 0)
					{
						EditorUI::DrawTexturedRect(ArrowRight->GetTexture(), x + width - 25.0f, t, 25.0f, 25.0f, 0.0f, Color(1.0f));
					}
					if (it.Image)
					{
						EditorUI::DrawTexturedRect(it.Image->GetTexture(), x, t, 25.0f, 25.0f, 0.0f, Color(1.0f));
					}
				}
				InputReady = true;
				EditorUI::DrawRectOutline(x, y, width, height, 1.0f, EditorPreferences::Get()->UiBackgroundColor);
			}
		};

		const float height = elements.size() * 25.0f + 5.0f;
		CreateOverlay<ContextMenuOverlay>(GetInputOffset().x + x, GetInputOffset().y + y - height, 225, height, elements);
	}

	static const float SubclassHierarchyEntryHeight = 28.0f;
	bool EditorUI::DrawSubclassHierarchyBox(float x, float y, float width, float height, SubclassHierarchyBoxParams& params)
	{
		float entryY = y + height - SubclassHierarchyEntryHeight * 2.0f + params.ScrollY;
		bool changed = DrawSubclassHierarchyEntry(params.RootClass, x, entryY, width - 10.0f, y, y + height, params);

		float scrollDown = entryY - params.ScrollY - y;
		EditorUI::ScrollbarVertical(x + width - 10, y + SubclassHierarchyEntryHeight, 10, height - SubclassHierarchyEntryHeight * 2.0f, x, y, width, height, 0, scrollDown > 0 ? 0 : Math::Abs(scrollDown), &params.ScrollY);

		DrawRect(x, y, width, SubclassHierarchyEntryHeight, 0.0f, params.HideColor);
		DrawRect(x, y + height - SubclassHierarchyEntryHeight, width, SubclassHierarchyEntryHeight, 0.0f, params.HideColor);

		return changed;
	}

	bool EditorUI::DrawSubclassHierarchyEntry(const Class& cls, float x, float& y, float width, float yMin, float yMax, SubclassHierarchyBoxParams& params)
	{
		if ((cls.GetParentClass() == Class::None && cls != Object::StaticClass()) || (cls.IsBlueprintClass() && cls.GetBlueprintClass()->IsMissing()))
		{
			return false;
		}
		bool hasChanged = false;

		ButtonParams BParams = (cls == params.CurrentSelectedClass) ? ButtonParams::Outlined() : ButtonParams();
		BParams.TextOrientation = Vec2(-0.95f, 0.0f);
		float CurrentLevelY = y;
		if (CurrentLevelY >= yMin && CurrentLevelY + SubclassHierarchyEntryHeight <= yMax)
		{
			if (EditorUI::Button(cls.GetClassName(), x + SubclassHierarchyEntryHeight * 2.0f, CurrentLevelY, width - SubclassHierarchyEntryHeight * 2.0f, SubclassHierarchyEntryHeight, BParams))
			{
				params.CurrentSelectedClass = cls;
				hasChanged = true;
			}
		}
		y -= SubclassHierarchyEntryHeight;

		bool hasSubclasses = false;
		Array<Class> subClasses = Class::GetSubclassesOf(cls);
		for (auto& It : subClasses)
		{
			if (It.GetParentClass() == cls)
			{
				hasSubclasses = true;
				if (!params.Dropdowns[cls])
				{
					if (DrawSubclassHierarchyEntry(It, x + SubclassHierarchyEntryHeight, y, width - SubclassHierarchyEntryHeight, yMin, yMax, params))
					{
						hasChanged = true;
					}
				}
				
			}
		}

		if (CurrentLevelY >= yMin && CurrentLevelY + SubclassHierarchyEntryHeight <= yMax)
		{
			if (hasSubclasses)
			{
				ButtonParams DropdownParams;
				DropdownParams.CenteredIcon = AssetManager::GetAsset<Texture2D>(SuoraID(params.Dropdowns[cls] ? "970c3d0e-c5b0-4a2e-a548-661d9b00d977" : "8742cec8-9ee5-4645-b036-577146904b41"));
				if (EditorUI::Button("", x, CurrentLevelY, SubclassHierarchyEntryHeight, SubclassHierarchyEntryHeight, DropdownParams))
				{
					params.Dropdowns[cls] = !params.Dropdowns[cls];
				}
			}

			ButtonParams IconParams;
			IconParams.CenteredIcon = EditorUI::GetClassIcon(cls);
			EditorUI::Button("", x + SubclassHierarchyEntryHeight, CurrentLevelY, SubclassHierarchyEntryHeight, SubclassHierarchyEntryHeight, IconParams);
		}

		return hasChanged;
	}

	EditorUI::SubclassHierarchyBoxParams::SubclassHierarchyBoxParams()
	{
		HideColor = EditorPreferences::Get()->UiBackgroundColor;
	}

}