#include "Precompiled.h"
#include "ViewportPanel.h"
#include "Suora/Renderer/RenderCommand.h"
#include "Suora/Renderer/RenderPipeline.h"
#include "Suora/Renderer/Renderer3D.h"
#include "Suora/Renderer/Framebuffer.h"
#include "Suora/Renderer/Shader.h"
#include "Suora/Assets/ShaderGraph.h"
#include "Suora/Assets/AssetManager.h"
#include "Suora/GameFramework/Nodes/ShapeNodes.h"
#include "Suora/GameFramework/Nodes/Light/DirectionalLightNode.h"
#include "Suora/GameFramework/Nodes/Light/PointLightNode.h"
#include "Suora/Editor/Util/EditorCamera.h"

#include "ViewportModules.h"

namespace Suora
{

	Material* ViewportDebugGizmo::GizmoMaterial(bool useIdShader, int* i, const Vec3& color, const SuoraID& uuid)
	{
		Material* mat = AssetManager::GetAsset<Material>(uuid);
		if (UniformSlot* uniform = mat->GetUniformSlot("Color"))
		{
			uniform->m_Vec3 = color;
		}
		if (useIdShader)
		{
			if (mat && mat->GetShaderGraph()) mat->GetShaderGraph()->GetIDShader()->SetInt("u_ID", *i);
		}
		return mat;
	}

	void ViewportPanel::DrawDebugShapes(World* world, CameraNode* camera, int* pickingID, Map<int, Node*>* pickingMap)
	{
		const bool HandlingMousePick = pickingID;
		if (NativeInput::GetKeyDown(Key::G)) m_DrawDebugGizmos = !m_DrawDebugGizmos;
		if (!m_DrawDebugGizmos) return;
		if (!HandlingMousePick) m_GizmoBuffer->Bind();

		for (Ref<ViewportDebugGizmo> It : m_ViewportDebugGizmos)
		{
			if (It)
			{
				It->DrawDebugGizmos(world, camera, pickingID, pickingMap, HandlingMousePick);
			}
		}

		RenderCommand::SetWireframeMode(true);
		RenderCommand::SetWireframeThickness(3.0f);

		Array<BoxShapeNode*> boxes = world->FindNodesByClass<BoxShapeNode>();
		for (BoxShapeNode* box : boxes)
		{
			Node3D tr;
			tr.SetPosition(box->GetPosition());
			tr.SetRotation(box->GetRotation());
			tr.SetScale(2.0f * box->GetBoxExtends());
			Material* mat = ViewportDebugGizmo::GizmoMaterial(HandlingMousePick, pickingID, box->IsTrigger ? Vec3(0.3f, 1.0f, 0.4f) : Vec3(0.0f, 1.0f, 1.0f));
			Texture2D* old = mat->m_UniformSlots[0].m_Texture2D;
			mat->m_UniformSlots[0].m_Texture2D = AssetManager::GetAsset<Texture2D>(SuoraID("55c17fb8-b445-431c-a59f-9ad9242d7c5c"));
			Renderer3D::DrawMesh(camera, tr.GetTransformMatrix(), *AssetManager::GetAsset<Mesh>(SuoraID("33b79a6d-2f4a-40fc-93e5-3f01794c33b8")), mat, HandlingMousePick ? MaterialType::ObjectID : MaterialType::Material);
			if (HandlingMousePick)
			{
				(*pickingMap)[*pickingID] = box;
				(*pickingID)++;
			}
			mat->m_UniformSlots[0].m_Texture2D = old;
		}
		RenderCommand::SetWireframeMode(false);
		Array<SphereShapeNode*> spheres = world->FindNodesByClass<SphereShapeNode>();
		for (SphereShapeNode* sphere : spheres)
		{
			Node3D tr;
			tr.SetPosition(sphere->GetPosition());
			tr.SetRotation(sphere->GetRotation());
			tr.SetScale(Vec3(sphere->GetSphereRadius()));
			Material* mat = ViewportDebugGizmo::GizmoMaterial(HandlingMousePick, pickingID, sphere->IsTrigger ? Vec3(0.3f, 1.0f, 0.4f) : Vec3(0.0f, 1.0f, 1.0f));
			Texture2D* old = mat->m_UniformSlots[0].m_Texture2D;
			mat->m_UniformSlots[0].m_Texture2D = AssetManager::GetAsset<Texture2D>(SuoraID("37b37d49-abe8-4609-929f-fec01e7bbcab"));
			Renderer3D::DrawMesh(camera, tr.GetTransformMatrix(), *AssetManager::GetAsset<Mesh>(SuoraID("005ab997-d566-4cb0-bcea-dc3ab5563cf5")), mat, HandlingMousePick ? MaterialType::ObjectID : MaterialType::Material);
			if (HandlingMousePick)
			{
				(*pickingMap)[*pickingID] = sphere;
				(*pickingID)++;
			}
			mat->m_UniformSlots[0].m_Texture2D = old;
		}
		/*Array<CapsuleCollisionNode*> capsules = world->FindNodesByClass<CapsuleCollisionNode>();
		for (CapsuleCollisionNode* capsule : capsules)
		{
			Node3D tr;
			tr.SetPosition(capsule->GetPosition());
			tr.SetScale(Vec3(capsule->m_Radius * 2.0f, capsule->m_Height / 2.0f, capsule->m_Radius * 2.0f));
			Renderer3D::DrawMesh(camera, tr.GetTransformMatrix(), *AssetManager::GetAsset<Mesh>(SuoraID("77d60b2a-790a-42d6-9760-4b3cb3b853d5")), ViewportDebugGizmo::GizmoMaterial(HandlingMousePick, pickingID, capsule->IsTrigger ? Vec3(0.3f, 1.0f, 0.4f) : Vec3(1.0f)), HandlingMousePick ? MaterialType::ObjectID : MaterialType::Material);
			if (HandlingMousePick)
			{
				(*pickingMap)[*pickingID] = capsule;
				(*pickingID)++;
			}
		}*/
		RenderCommand::SetWireframeMode(false);
		RenderCommand::SetWireframeThickness(1.0f);
		
	}



	void ViewportPanel::DrawDebugView(Framebuffer& buffer, World& world, CameraNode& camera)
	{
		// Default Render Final Scene
		m_RParams.EnableDeferredRendering = ProjectSettings::Get()->m_EnableDeferredRendering;
		m_RParams.DrawWireframe = m_DrawWireframe;
		m_RParams.Resolution = iVec2(GetWidth(), GetHeight());
		Engine::Get()->GetRenderPipeline()->Render(buffer, world, camera, m_RParams);

#define _GBUFFER_RENDER(GBufferSlot, _shader) RenderPipeline::RenderFramebufferIntoFramebuffer(	*m_RParams.GetGBuffer(), \
			buffer,\
			*_shader,\
			glm::ivec4(0, 0, GetWidth(), GetHeight()),\
			"u_Texture",\
			(int)GBufferSlot,\
			false);

		switch (m_DebugView)
		{
		case DebugView::Final: return;
		case DebugView::GBuffer_BaseColor: _GBUFFER_RENDER(GBuffer::BaseColor, RenderPipeline::GetFullscreenPassShaderStatic()); return;
		case DebugView::GBuffer_WorldPosition: _GBUFFER_RENDER(GBuffer::WorldPosition, RenderPipeline::GetFullscreenPassShaderStatic()); return;
		case DebugView::GBuffer_WorldNormal: _GBUFFER_RENDER(GBuffer::WorldNormal, RenderPipeline::GetFullscreenPassShaderStatic()); return;
		case DebugView::GBuffer_Emissive: _GBUFFER_RENDER(GBuffer::Emissive, RenderPipeline::GetFullscreenPassShaderStatic()); return;
		case DebugView::GBuffer_MeshID:
		{
			static Ref<Shader> shader = Shader::Create(AssetManager::GetEngineAssetPath() + "/EngineContent/Shaders/Debug/ClusterDebug.glsl");
			_GBUFFER_RENDER(GBuffer::MeshID, shader);
		} return;
		case DebugView::GBuffer_Cluster:
		{
			static Ref<Shader> shader = Shader::Create(AssetManager::GetEngineAssetPath() + "/EngineContent/Shaders/Debug/ClusterDebug.glsl");
			_GBUFFER_RENDER(GBuffer::ClusterID, shader);
		} return;
		case DebugView::DeferredLightPass:
		{
			auto GBufferSize = m_RParams.GetGBuffer()->GetSize();
			RenderPipeline::RenderFramebufferIntoFramebuffer(*m_RParams.GetDeferredLitBuffer(),
				buffer, *RenderPipeline::GetFullscreenPassShaderStatic(), glm::ivec4(0, 0, GetWidth(), GetHeight()), "u_Texture", 0, false);
		} return;
		case DebugView::ForwardReadyBuffer:
		{
			auto GBufferSize = m_RParams.GetGBuffer()->GetSize();
			RenderPipeline::RenderFramebufferIntoFramebuffer(*m_RParams.GetForwardReadyBuffer(),
				buffer, *RenderPipeline::GetFullscreenPassShaderStatic(), glm::ivec4(0, 0, GetWidth(), GetHeight()), "u_Texture", 0, false);
		} return;
		default: SuoraError("ViewportPanel::DrawDebugView(): DebugViewMode is not implemented!"); return;
		}
	}

}