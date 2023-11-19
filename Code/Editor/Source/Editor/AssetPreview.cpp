#include "AssetPreview.h"
#include "EditorUI.h"
#include "Util/Icon.h"
#include "Suora/GameFramework/InputModule.h"
#include "Suora/Renderer/RenderPipeline.h"
#include "Suora/Renderer/RenderCommand.h"
#include "Suora/Renderer/Framebuffer.h"
#include "Suora/Renderer/Texture.h"
#include "Suora/Renderer/Shader.h"

#include "Suora/GameFramework/Nodes/MeshNode.h"
#include "Suora/GameFramework/Nodes/DecalNode.h"
#include "Suora/GameFramework/Nodes/ShapeNodes.h"
#include "Suora/GameFramework/Nodes/PostProcess/PostProcessNode.h"
#include "Suora/GameFramework/Nodes/Light/SkyLightNode.h"
#include "Suora/GameFramework/Nodes/Light/PointLightNode.h"
#include "Suora/GameFramework/Nodes/Light/DirectionalLightNode.h"

namespace Suora
{
	AssetPreview::AssetPreview()
	{
		FramebufferSpecification spec;
		spec.Width = 128;
		spec.Height = 128;
		spec.Attachments.Attachments.push_back(FramebufferTextureFormat::RGBA8);
		spec.Attachments.Attachments.push_back(FramebufferTextureFormat::Depth);
		m_Preview = Framebuffer::Create(spec);
	}
	AssetPreview::~AssetPreview()
	{
		m_Preview = nullptr;
		m_World = nullptr;
	}
	void AssetPreview::Init(Asset* asset)
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
			mesh->SetScale(Vec3(2.0f));
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
	void AssetPreview::Render(Asset* asset)
	{
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

		if (asset->IsA<Material>())
		{
			s_Pipeline->Render(*m_Preview, *m_World, *m_World->GetMainCamera(), s_RParams);
		}
		if (asset->IsA<Mesh>())
		{
			s_Pipeline->Render(*m_Preview, *m_World, *m_World->GetMainCamera(), s_RParams);
		}
		if (asset->IsA<Blueprint>())
		{
			s_Pipeline->Render(*m_Preview, *m_World, *m_World->GetMainCamera(), s_RParams);
		}
		m_Done = true;
		m_Init = false;
		m_World = nullptr;

		m_Preview->Unbind();
		if (current) current->Bind();
	}
	void AssetPreview::RerenderAssetPreviews()
	{
		static bool InRenderPhase = false;
		if (AssetManager::s_AssetStreamPool.Size() != 0 && !InRenderPhase) return;
		InRenderPhase = true;

		if (!CanRerenderAssetPreviews) return;
		CanRerenderAssetPreviews = false;

	rerun:
		int64_t Index = AssetPreviews.size() ? rand() % AssetPreviews.size() + (rand() % 3) : -1;
		for (auto& it : AssetPreviews)
		{
			Index--;

			if (Index == 0 && it.second)
			{
				if (it.second->m_Init && rand() % 3 == 0) goto rerun;
				it.second->Render(it.first);
				break;
			}
		}

	}
	void AssetPreview::DrawAssetPreview(Asset* asset, const Class& assetClass, float x, float y, float width, float height)
	{
		EditorUI::DrawTexturedRect(Texture::GetOrCreateDefaultTexture(), x, y, width, height, 0.0f, Color(0.1f, 0.1f, 0.1f, 1));
		if (!asset) return;

		if (Texture2D* texture = Cast<Texture2D>(asset))
		{
			EditorUI::DrawTexturedRect(texture->GetTexture(), x, y, width, height, 0.0f, Color(1.0f));
		}
		else if (Font* font = Cast<Font>(asset))
		{
			EditorUI::DrawTexturedRect(font->m_FontAtlas, x, y, width, height, 0.0f, Color(1.0f));
		}
		else if (asset->IsA<InputMapping>())
		{
			EditorUI::DrawTexturedRect(Icon::Controller, x, y, width, height, 0.0f, Color(0.85f));
		}
		else if (asset->IsA<class Level>())
		{
			EditorUI::DrawTexturedRect(Icon::World, x, y, width, height, 0.0f, Color(0.85f));
		}
		else if (asset->IsA<ProjectSettings>() || asset->IsA<EditorPreferences>())
		{
			EditorUI::DrawTexturedRect(Icon::Cogwheel, x, y, width, height, 0.0f, Color(0.85f));
		}
		else
		{
			if (AssetPreviews.find(asset) == AssetPreviews.end())
			{
				AssetPreviews[asset] = nullptr;
				//AssetPreviews[asset].Render(asset);
			}

			if (!AssetPreviews[asset]) return;
			/*if (AssetPreviews[asset]->m_LastDraw < 0) return;
			AssetPreviews[asset]->m_LastDraw = 0;*/
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
	void AssetPreview::Tick(float deltaTime)
	{
		CanRerenderAssetPreviews = true;
	rerun:
		for (auto& it : AssetPreviews)
		{
			if (!it.second)
			{
				it.second = CreateRef<AssetPreview>();
			}
		}

		static int counter = 0;
		if (counter >= 512)
			RerenderAssetPreviews();
		else
			counter++;
	}
}