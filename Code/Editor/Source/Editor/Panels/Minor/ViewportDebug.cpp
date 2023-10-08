#include "ViewportPanel.h"
#include "Suora/Renderer/RenderCommand.h"
#include "Suora/Renderer/RenderPipeline.h"
#include "Suora/Renderer/Renderer3D.h"
#include "Suora/Renderer/Shader.h"
#include "Suora/Renderer/Ilum.h"
#include "Suora/Assets/ShaderGraph.h"
#include "Suora/Assets/AssetManager.h"
#include "Suora/GameFramework/Nodes/ShapeNodes.h"
#include "Suora/GameFramework/Nodes/Light/DirectionalLightNode.h"
#include "Suora/GameFramework/Nodes/Light/PointLightNode.h"
#include "../../Util/EditorCamera.h"

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
		Array<SphereShapeNode*> spheres = world->FindNodesByClass<SphereShapeNode>();
		for (SphereShapeNode* sphere : spheres)
		{
			Node3D tr;
			tr.SetPosition(sphere->GetPosition());
			tr.SetRotation(sphere->GetRotation());
			tr.SetScale(Vec3(sphere->GetSphereRadius()));
			Renderer3D::DrawMesh(camera, tr.GetTransformMatrix(), *Mesh::Sphere, GizmoMaterial(HandlingMousePick, pickingID, sphere->IsTrigger ? Vec3(0.3f, 1.0f, 0.4f) : Vec3(1.0f)), HandlingMousePick ? MaterialType::ObjectID : MaterialType::Material);
			if (HandlingMousePick)
			{
				(*pickingMap)[*pickingID] = sphere;
				(*pickingID)++;
			}
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
		RenderingParams RParams;
		RParams.DrawWireframe = m_DrawWireframe;
		Engine::Get()->GetRenderPipeline()->Render(buffer, world, camera, RParams);

#define _GBUFFER_RENDER(GBufferSlot, _shader) RenderPipeline::RenderFramebufferIntoFramebuffer(	*Engine::Get()->GetRenderPipeline()->GetGBuffer(), \
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
			static Ref<Shader> shader = Shader::Create(AssetManager::GetAssetRootPath() + "/EngineContent/Shaders/Debug/ClusterDebug.glsl");
			_GBUFFER_RENDER(GBuffer::MeshID, shader);
		} return;
		case DebugView::GBuffer_Cluster:
		{
			static Ref<Shader> shader = Shader::Create(AssetManager::GetAssetRootPath() + "/EngineContent/Shaders/Debug/ClusterDebug.glsl");
			_GBUFFER_RENDER(GBuffer::ClusterID, shader);
		} return;
		case DebugView::DeferredLightPass:
		{
			auto GBufferSize = Engine::Get()->GetRenderPipeline()->GetGBuffer()->GetSize();
			RenderPipeline::RenderFramebufferIntoFramebuffer(*Engine::Get()->GetRenderPipeline()->GetDeferredLitBuffer(GBufferSize),
				buffer, *RenderPipeline::GetFullscreenPassShaderStatic(), glm::ivec4(0, 0, GetWidth(), GetHeight()), "u_Texture", 0, false);
		} return;
		case DebugView::ForwardReadyBuffer:
		{
			auto GBufferSize = Engine::Get()->GetRenderPipeline()->GetGBuffer()->GetSize();
			RenderPipeline::RenderFramebufferIntoFramebuffer(*Engine::Get()->GetRenderPipeline()->GetForwardReadyBuffer(GBufferSize),
				buffer, *RenderPipeline::GetFullscreenPassShaderStatic(), glm::ivec4(0, 0, GetWidth(), GetHeight()), "u_Texture", 0, false);
		} return;
		case DebugView::Ilum_Surfels:
		{
			struct IlumDebugShaderGraph : public ShaderGraph
			{
				IlumDebugShaderGraph()
				{
					m_ShaderSource = Platform::ReadFromFile(AssetManager::GetAssetRootPath() + "/EngineContent/Shaders/Debug/IlumDebug.glsl");
				}
			};
			static IlumDebugShaderGraph ProbeShader;
			ProbeShader.GetShader()->Bind();
			m_World->GetIlumContext()->m_SurfelCache->BindColorAttachmentByIndex((int)GBuffer::WorldPosition, (int)GBuffer::WorldPosition); ProbeShader.GetShader()->SetInt("u_WorldPosition", (int)GBuffer::WorldPosition);
			m_World->GetIlumContext()->m_SurfelCache->BindColorAttachmentByIndex((int)GBuffer::WorldNormal, (int)GBuffer::WorldNormal); ProbeShader.GetShader()->SetInt("u_WorldNormal", (int)GBuffer::WorldNormal);
			m_World->GetIlumContext()->m_SurfelIlumCache->BindColorAttachmentByIndex((int)GBuffer::BaseColor, (int)GBuffer::BaseColor); ProbeShader.GetShader()->SetInt("u_BaseColor", (int)GBuffer::BaseColor);

			{
				VertexArray* vao = AssetManager::GetAsset<Mesh>(SuoraID("180284a6-7c63-408c-8078-6ce3b1b50d77"))->GetVertexArray();
				if (!vao) return;

				glm::mat4 viewProj = GetEditorCamera()->GetProjectionMatrix() /*Projection*/ * glm::inverse(GetEditorCamera()->GetTransformMatrix());
				ProbeShader.GetShader()->SetMat4("u_ViewProjection", viewProj);

				vao->Bind();
				RenderCommand::SetDepthTest(true);
				RenderCommand::DrawInstanced(vao, 16384);
				RenderCommand::SetDepthTest(false);
			}
		} return;

		case DebugView::Ilum_Probes:
		{
			struct IlumDebugShaderGraph : public ShaderGraph
			{
				IlumDebugShaderGraph()
				{
					m_ShaderSource = Platform::ReadFromFile(AssetManager::GetAssetRootPath() + "/EngineContent/Shaders/Debug/IlumDebug_Probes.glsl");
				}
			};
			static IlumDebugShaderGraph ProbeShader;
			ProbeShader.GetShader()->Bind();
			m_World->GetIlumContext()->m_IluminationCache->BindColorAttachmentByIndex(0, 1); ProbeShader.GetShader()->SetInt("u_IluminationCache", 1);

			{
				VertexArray* vao = AssetManager::GetAsset<Mesh>(SuoraID("4c2516be-3b7d-4684-b292-096ec621a83c"))->GetVertexArray();
				if (!vao) return;

				glm::mat4 viewProj = GetEditorCamera()->GetProjectionMatrix() /*Projection*/ * glm::inverse(GetEditorCamera()->GetTransformMatrix());
				ProbeShader.GetShader()->SetMat4("u_ViewProjection", viewProj);
				ProbeShader.GetShader()->SetFloat3("u_LightGridPos", m_World->GetIlumContext()->m_LightProbeGridPos);
				ProbeShader.GetShader()->SetFloat3("u_LightGridStep", m_World->GetIlumContext()->m_LightProbeGridOffset);

				vao->Bind();
				RenderCommand::SetDepthTest(true);
				RenderCommand::DrawInstanced(vao, 4096);
				RenderCommand::SetDepthTest(false);
			}
		} return;
		case DebugView::Ilum_Debug:
		{
			RenderPipeline::RenderFramebufferIntoFramebuffer(*m_World->GetIlumContext()->m_SurfelCache, *m_Framebuffer, *RenderPipeline::GetFullscreenPassShaderStatic(), glm::ivec4(5, 5, 256, 256), "u_Texture", (int)GBuffer::BaseColor, false);
			RenderPipeline::RenderFramebufferIntoFramebuffer(*m_World->GetIlumContext()->m_SurfelDirectLightCache, *m_Framebuffer, *RenderPipeline::GetFullscreenPassShaderStatic(), glm::ivec4(5 + 256 + 20, 5, 256, 256), "u_Texture", 0, false);
			RenderPipeline::RenderFramebufferIntoFramebuffer(*m_World->GetIlumContext()->m_SurfelIlumCache, *m_Framebuffer, *RenderPipeline::GetFullscreenPassShaderStatic(), glm::ivec4(5 + 512 + 30, 5, 256, 256), "u_Texture", 0, false);
			//RenderPipeline::RenderFramebufferIntoFramebuffer(*m_World->GetIlumContext()->m_IluminationCache, *m_Framebuffer, *RenderPipeline::GetFullscreenPassShaderStatic(), glm::ivec4(5 + 512 + 20, 5, 1024, 1024), "u_Texture", 0, false);
			RenderPipeline::RenderFramebufferIntoFramebuffer(*m_World->GetIlumContext()->m_SecondaryView, *m_Framebuffer, *RenderPipeline::GetFullscreenPassShaderStatic(), glm::ivec4(5, 5 + 256, 256, 256), "u_Texture", (int)GBuffer::BaseColor, false);
		} return;
		default: SuoraError("ViewportPanel::DrawDebugView(): DebugViewMode is not implemented!"); return;
		}
	}

}