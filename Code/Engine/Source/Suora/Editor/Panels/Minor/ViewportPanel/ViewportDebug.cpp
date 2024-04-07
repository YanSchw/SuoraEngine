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