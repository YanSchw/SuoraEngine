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

namespace Suora
{

	static Material* GizmoMaterial(bool useIdShader, int* i, const Vec3& color, const SuoraID& uuid = SuoraID("72b2a0e4-6541-4907-9527-47aa742ede45"))
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
	void ViewportPanel::DrawDebugShapes(World* world, CameraNode* camera, int* pickingID, std::unordered_map<int, Node*>* pickingMap)
	{
		const bool HandlingMousePick = pickingID;
		if (NativeInput::GetKeyDown(Key::G)) m_DrawDebugGizmos = !m_DrawDebugGizmos;
		if (!m_DrawDebugGizmos) return;
		if (!HandlingMousePick) m_GizmoBuffer->Bind();
		RenderCommand::SetWireframeMode(true);
		RenderCommand::SetWireframeThickness(3.0f);

		Array<BoxShapeNode*> boxes = world->FindNodesByClass<BoxShapeNode>();
		for (BoxShapeNode* box : boxes)
		{
			Node3D tr;
			tr.SetPosition(box->GetPosition());
			tr.SetRotation(box->GetRotation());
			tr.SetScale(2.0f * box->GetBoxExtends());
			Material* mat = GizmoMaterial(HandlingMousePick, pickingID, box->IsTrigger ? Vec3(0.3f, 1.0f, 0.4f) : Vec3(0.0f, 1.0f, 1.0f));
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
			Material* mat = GizmoMaterial(HandlingMousePick, pickingID, sphere->IsTrigger ? Vec3(0.3f, 1.0f, 0.4f) : Vec3(0.0f, 1.0f, 1.0f));
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
			Renderer3D::DrawMesh(camera, tr.GetTransformMatrix(), *AssetManager::GetAsset<Mesh>(SuoraID("77d60b2a-790a-42d6-9760-4b3cb3b853d5")), GizmoMaterial(HandlingMousePick, pickingID, capsule->IsTrigger ? Vec3(0.3f, 1.0f, 0.4f) : Vec3(1.0f)), HandlingMousePick ? MaterialType::ObjectID : MaterialType::Material);
			if (HandlingMousePick)
			{
				(*pickingMap)[*pickingID] = capsule;
				(*pickingID)++;
			}
		}*/
		RenderCommand::SetWireframeMode(false);
		RenderCommand::SetWireframeThickness(1.0f);

		Array<CameraNode*> cameras = world->FindNodesByClass<CameraNode>();
		for (CameraNode* cam : cameras)
		{
			Node3D tr;
			tr.SetPosition(cam->GetPosition());
			tr.SetRotation(cam->GetRotation());
			tr.SetScale(Vec3(0.1f));
			Renderer3D::DrawMesh(camera, cam->GetTransformMatrix(), *AssetManager::GetAsset<Mesh>(SuoraID("c37609b9-9067-4e3a-ac04-c493ed2d8009")), GizmoMaterial(HandlingMousePick, pickingID, Vec3(1.0f), SuoraID("317cf1ef-ac75-46d8-a62f-184891456960")), HandlingMousePick ? MaterialType::ObjectID : MaterialType::Material);

			Vec3 corners[4];
			{
				Vec4 pos = glm::inverse(cam->GetViewProjectionMatrix()) * Vec4(-1.0f, 1.0f, 1.0f, 1.0f);
				pos.x /= pos.w;
				pos.y /= pos.w;
				pos.z /= pos.w;
				Vec3 end = glm::normalize(((Vec3)pos) - cam->GetPosition()) * 15.0f + cam->GetPosition();
				corners[0] = end;
				Renderer3D::DrawLine3D(camera, cam->GetPosition(), end, Color(0.66f, 1.0f, 0.66f, 0.5f));
			}
			{
				Vec4 pos = glm::inverse(cam->GetViewProjectionMatrix()) * Vec4(1.0f, 1.0f, 1.0f, 1.0f);
				pos.x /= pos.w;
				pos.y /= pos.w;
				pos.z /= pos.w;
				Vec3 end = glm::normalize(((Vec3)pos) - cam->GetPosition()) * 15.0f + cam->GetPosition();
				corners[1] = end;
				Renderer3D::DrawLine3D(camera, cam->GetPosition(), end, Color(0.66f, 1.0f, 0.66f, 0.5f));
			}
			{
				Vec4 pos = glm::inverse(cam->GetViewProjectionMatrix()) * Vec4(-1.0f, -1.0f, 1.0f, 1.0f);
				pos.x /= pos.w;
				pos.y /= pos.w;
				pos.z /= pos.w;
				Vec3 end = glm::normalize(((Vec3)pos) - cam->GetPosition()) * 15.0f + cam->GetPosition();
				corners[2] = end;
				Renderer3D::DrawLine3D(camera, cam->GetPosition(), end, Color(0.66f, 1.0f, 0.66f, 0.5f));
			}
			{
				Vec4 pos = glm::inverse(cam->GetViewProjectionMatrix()) * Vec4(1.0f, -1.0f, 1.0f, 1.0f);
				pos.x /= pos.w;
				pos.y /= pos.w;
				pos.z /= pos.w;
				Vec3 end = glm::normalize(((Vec3)pos) - cam->GetPosition()) * 15.0f + cam->GetPosition();
				corners[3] = end;
				Renderer3D::DrawLine3D(camera, cam->GetPosition(), end, Color(0.66f, 1.0f, 0.66f, 0.5f));
			}
			Renderer3D::DrawLine3D(camera, corners[0], corners[1], Color(0.66f, 1.0f, 0.66f, 0.5f));
			Renderer3D::DrawLine3D(camera, corners[1], corners[3], Color(0.66f, 1.0f, 0.66f, 0.5f));
			Renderer3D::DrawLine3D(camera, corners[2], corners[0], Color(0.66f, 1.0f, 0.66f, 0.5f));
			Renderer3D::DrawLine3D(camera, corners[3], corners[2], Color(0.66f, 1.0f, 0.66f, 0.5f));
			
			if (HandlingMousePick)
			{
				(*pickingMap)[*pickingID] = cam;
				(*pickingID)++;
			}
		}
		Array<DirectionalLightNode*> dlights = world->FindNodesByClass<DirectionalLightNode>();
		for (DirectionalLightNode* light : dlights)
		{
			Node3D tr;
			tr.SetPosition(light->GetPosition());
			tr.SetRotation(camera->GetRotation());
			Material* gizmoMat = GizmoMaterial(HandlingMousePick, pickingID, Vec3(1.0f));
			const UniformSlot uniform = gizmoMat->m_UniformSlots[0];
			gizmoMat->m_UniformSlots[0].m_Texture2D = AssetManager::GetAsset<Texture2D>(SuoraID("64738d74-08a9-4383-8659-620808d5269a"));
			Renderer3D::DrawMesh(camera, tr.GetTransformMatrix(), *AssetManager::GetAsset<Mesh>(SuoraID("75f466f7-baec-4c5a-a23b-a5e3dc3d22bc")), gizmoMat, HandlingMousePick ? MaterialType::ObjectID : MaterialType::Material);
			gizmoMat->m_UniformSlots[0] = uniform;
			Node3D tr2;
			tr2.SetPosition(light->GetPosition());
			tr2.SetRotation(light->GetRotation());
			tr2.SetScale(Vec3(0.25f));
			Renderer3D::DrawMesh(camera, tr2.GetTransformMatrix(), *AssetManager::GetAsset<Mesh>(SuoraID("0eb4c94e-e88e-436e-a803-12739b755f0c")), GizmoMaterial(HandlingMousePick, pickingID, Vec3(1.0f)), HandlingMousePick ? MaterialType::ObjectID : MaterialType::Material);

			if (HandlingMousePick)
			{
				(*pickingMap)[*pickingID] = light;
				(*pickingID)++;
			}
		}
		Array<PointLightNode*> plights = world->FindNodesByClass<PointLightNode>();
		for (PointLightNode* light : plights)
		{
			Node3D tr;
			tr.SetPosition(light->GetPosition());
			tr.SetRotation(camera->GetRotation());
			Material* gizmoMat = GizmoMaterial(HandlingMousePick, pickingID, Vec3(light->m_Color));
			const UniformSlot uniform = gizmoMat->m_UniformSlots[0];
			gizmoMat->m_UniformSlots[0].m_Texture2D = AssetManager::GetAsset<Texture2D>(SuoraID("f789d2bf-dcda-4e30-b2d9-3db979b7c6da"));
			Renderer3D::DrawMesh(camera, tr.GetTransformMatrix(), *AssetManager::GetAsset<Mesh>(SuoraID("75f466f7-baec-4c5a-a23b-a5e3dc3d22bc")), gizmoMat, HandlingMousePick ? MaterialType::ObjectID : MaterialType::Material);
			gizmoMat->m_UniformSlots[0] = uniform;

			if (HandlingMousePick)
			{
				(*pickingMap)[*pickingID] = light;
				(*pickingID)++;
			}
		}
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