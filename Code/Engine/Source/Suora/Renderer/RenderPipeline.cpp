#include "Precompiled.h"
#include "RenderPipeline.h"

#include "Suora/GameFramework/World.h"
#include "Suora/Renderer/Framebuffer.h"
#include "Suora/Renderer/Renderer3D.h"
#include "Suora/Renderer/RenderCommand.h"
#include "Suora/Renderer/VertexArray.h"
#include "Suora/Renderer/Shader.h"
#include "Suora/Renderer/Ilum.h"
#include "Suora/Renderer/Decima.h"
#include "Suora/Assets/AssetManager.h"
#include "Suora/Assets/ShaderGraph.h"

#include "Suora/GameFramework/Nodes/MeshNode.h"
#include "Suora/GameFramework/Nodes/DecalNode.h"
#include "Suora/GameFramework/Nodes/Light/DirectionalLightNode.h"
#include "Suora/GameFramework/Nodes/Light/PointLightNode.h"
#include "Suora/GameFramework/Nodes/Light/SkyLightNode.h"
#include "Suora/GameFramework/Nodes/PostProcess/PostProcessNode.h"

namespace Suora
{

	inline static glm::ivec4 BufferToRect(Framebuffer& buffer)
	{
		return glm::ivec4(0, 0, buffer.GetSpecification().Width, buffer.GetSpecification().Height);
	}

	FramebufferTextureFormat RenderPipeline::GBufferSlotToBufferFormat(GBuffer slot)
	{
		switch (slot)
		{
		case GBuffer::BaseColor: return FramebufferTextureFormat::RGB32F;
		case GBuffer::Metallic: return FramebufferTextureFormat::R8;
		case GBuffer::Roughness: return FramebufferTextureFormat::R8;
		case GBuffer::WorldPosition: return FramebufferTextureFormat::RGB32F;
		case GBuffer::WorldNormal: return FramebufferTextureFormat::RGB32F;
		case GBuffer::Emissive: return FramebufferTextureFormat::RGB32F;
		case GBuffer::MeshID: return FramebufferTextureFormat::R32I;
		case GBuffer::ClusterID: return FramebufferTextureFormat::R32I;
		case GBuffer::GBufferSlotCount:
		default:
			break;
		}
		SuoraError("RenderPipeline: Cannot convert GBufferSlot to FramebufferTextureFormat!");
		return FramebufferTextureFormat::None;
	}

	RenderPipeline::RenderPipeline()
	{
	}
	RenderPipeline::~RenderPipeline()
	{
	}
	void RenderPipeline::Initialize()
	{
		s_Instance = this;
		m_FullscreenPassShader = Shader::Create(AssetManager::GetAssetRootPath() + "/EngineContent/Shaders/FullscreenPass.glsl");
		m_AddShader = Shader::Create(AssetManager::GetAssetRootPath() + "/EngineContent/Shaders/Add.glsl");
		m_DepthBlitShader = Shader::Create(AssetManager::GetAssetRootPath() + "/EngineContent/Shaders/DepthBlit.glsl");
		m_ToneMapping = Shader::Create(AssetManager::GetAssetRootPath() + "/EngineContent/Shaders/PostProccess/ToneMapping.glsl");
		m_FXAA = Shader::Create(AssetManager::GetAssetRootPath() + "/EngineContent/Shaders/PostProccess/FXAA.glsl");
		m_DeferredDecalPreparation = Shader::Create(AssetManager::GetAssetRootPath() + "/EngineContent/Shaders/Deferred/Deferred_DecalPreparation.glsl");
		m_DeferredDirectionalLightShader = Shader::Create(AssetManager::GetAssetRootPath() + "/EngineContent/Shaders/Deferred/Deferred_DirectionalLight.glsl");
		m_DeferredPointLightShader = Shader::Create(AssetManager::GetAssetRootPath() + "/EngineContent/Shaders/Deferred/Deferred_PointLight.glsl");
		m_DeferredPointLightMatrixBuffer = ShaderStorageBuffer::Create();
		m_DeferredSkyShader = Shader::Create(AssetManager::GetAssetRootPath() + "/EngineContent/Shaders/Deferred/Deferred_Sky.glsl");
		m_DeferredSkyLightShader = Shader::Create(AssetManager::GetAssetRootPath() + "/EngineContent/Shaders/Deferred/Deferred_SkyLight.glsl");
		m_DeferredComposite = Shader::Create(AssetManager::GetAssetRootPath() + "/EngineContent/Shaders/Deferred/Deferred_Composite.glsl");
		{
			FramebufferSpecification spec;
			spec.Width = m_InternalResolution.x;
			spec.Height = m_InternalResolution.y;
			spec.Attachments.Attachments.push_back(FramebufferTextureFormat::RGB32F);
			m_TemporaryAddBuffer = Framebuffer::Create(spec);
		}
		{
			FramebufferSpecification spec;
			spec.Width = m_InternalResolution.x;
			spec.Height = m_InternalResolution.y;
			for (int32_t i = 0; i < (int32_t) GBuffer::GBufferSlotCount; i++)
			{
				spec.Attachments.Attachments.push_back(GBufferSlotToBufferFormat((GBuffer)i));
			}
			spec.Attachments.Attachments.push_back(FramebufferTextureFormat::Depth);
			m_GBuffer = Framebuffer::Create(spec);
		}
		m_DecimaInstance = CreateRef<Decima>();
	}

	Ref<Shader> RenderPipeline::GetFullscreenPassShaderStatic()
	{
		RenderPipeline* pipeline = Engine::Get()->GetRenderPipeline();
		return pipeline ? pipeline->GetFullscreenPassShader() : nullptr;
	}

	void RenderPipeline::Render(Framebuffer& buffer, World& world, CameraNode& camera, Framebuffer& gbuffer, RenderingParams& params)
	{
		SUORA_ASSERT(buffer.GetSpecification().Attachments.Attachments[0].TextureFormat == FramebufferTextureFormat::RGB32F);

		if (!Ilum::IsInIlumPass())
		{
			s_LastCameraPos = camera.GetPosition();
		}

		if (!Ilum::IsInIlumPass())
		{
			ShadowPass(world, camera);
		}

		if (!Ilum::IsInIlumPass())
		{
			for (int i = 0; i < 1; i++)
			world.GetIlumContext()->Tick(Engine::Get()->GetDeltaTime(), gbuffer, camera, world);
		}
		SetFullscreenViewport(gbuffer);

		DeferredPass(world, camera, gbuffer, params);

		RenderCommand::SetAlphaBlending(AlphaBlendMode::Blend);

		ForwardPass(world, camera, gbuffer, params);

		if (!Ilum::IsInIlumPass())
		{
			PostProcessPass(world, camera, gbuffer, params);
		}

		// Output Final Buffer
		RenderFramebufferIntoFramebuffer(*GetFinalFramebuffer(gbuffer.GetSize()), buffer, *m_FullscreenPassShader, BufferToRect(buffer), "u_Texture", 0, true);

		if (!Ilum::IsInIlumPass())
		{
			m_DecimaInstance->Run(&world, &camera);
		}
	}


	void RenderPipeline::RenderFramebufferIntoFramebuffer(Framebuffer& src, Framebuffer& target, Shader& shader, const glm::ivec4& rect, const std::string& uniformName, int attachmentIndex, bool shouldClear)
	{
		target.Bind();
		if (shouldClear) RenderCommand::Clear();
		RenderCommand::SetViewport(rect.x, rect.y, rect.z, rect.w);
		shader.Bind();
		shader.SetInt(uniformName, 0);
		src.BindColorAttachmentByIndex(attachmentIndex, 0);
		RenderCommand::SetDepthTest(false);
		GetFullscreenQuad()->Bind();
		RenderCommand::DrawIndexed(GetFullscreenQuad());
	}

	void RenderPipeline::AddFramebufferToFramebuffer(Framebuffer& src, Framebuffer& target, int attachmentIndex)
	{
		s_Instance->m_FullscreenPassShader->Bind();
		RenderFramebufferIntoFramebuffer(target, *s_Instance->m_TemporaryAddBuffer, *s_Instance->m_FullscreenPassShader, glm::ivec4(0, 0, target.GetSpecification().Width, target.GetSpecification().Height), "u_Texture", 0);

		s_Instance->m_AddShader->Bind();
		s_Instance->m_TemporaryAddBuffer->BindColorAttachmentByIndex(0, 1); s_Instance->m_AddShader->SetInt("u_TempSampler", 1);
		RenderFramebufferIntoFramebuffer(src, target, *s_Instance->m_AddShader, glm::ivec4(0, 0, target.GetSpecification().Width, target.GetSpecification().Height), "u_Texture", attachmentIndex);
	}

	void RenderPipeline::BlitDepthBuffer(Framebuffer& src, Framebuffer& target, Shader& shader, const std::string& uniformName)
	{
		target.Bind();
		RenderCommand::Clear();
		shader.Bind();
		shader.SetInt(uniformName, 0);
		src.BindDepthAttachmentToSlot(0);
		RenderCommand::SetDepthTest(true);
		RenderCommand::SetAlphaBlending(false);
		RenderCommand::SetViewport(0, 0, target.GetSpecification().Width, target.GetSpecification().Height);
		GetFullscreenQuad()->Bind();
		RenderCommand::DrawIndexed(GetFullscreenQuad());
	}

	glm::ivec2 RenderPipeline::GetInternalResolution()
	{
		return Engine::Get()->GetRenderPipeline()->m_InternalResolution;
	}
	void RenderPipeline::SetInternalResolution(const glm::ivec2& resolution)
	{
		Engine::Get()->GetRenderPipeline()->m_InternalResolution = resolution;
	}

	void RenderPipeline::SetFullscreenViewport(Framebuffer& buffer)
	{
		RenderCommand::SetViewport(0, 0, buffer.GetSpecification().Width, buffer.GetSpecification().Height);
	}

	void RenderPipeline::ClearDepth(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
		struct RenderPipeline_ClearDepth
		{
			Ref<Shader> ClearShader;
			RenderPipeline_ClearDepth()
			{
				ClearShader = Shader::Create(AssetManager::GetAssetRootPath() + "/EngineContent/Shaders/ClearDepth.glsl");
			}
		};
		static RenderPipeline_ClearDepth Cmd;
		

		RenderCommand::SetViewport(x, y, width, height);
		Cmd.ClearShader->Bind();
		RenderCommand::SetDepthTest(true);
		RenderCommand::SetDepthMask(true);
		GetFullscreenQuad()->Bind();
		RenderCommand::DrawIndexed(GetFullscreenQuad());
		RenderCommand::SetDepthMask(false);

	}

	void RenderPipeline::ShadowPass(World& world, CameraNode& camera)
	{
		Array<LightNode*> lights = world.FindNodesByClass<LightNode>();
		for (LightNode* light : lights)
		{
			if (light->m_ShadowMap)
			{
				light->ShadowMap(world, camera);
			}
		}
	}

	void RenderPipeline::DeferredPass(World& world, CameraNode& camera, Framebuffer& gbuffer, RenderingParams& params)
	{
		RenderCommand::SetClearColor(Color(0,0,0,1));
		RenderGBuffer(world, camera, gbuffer, params);

		DecalPass(world, camera, gbuffer);

		DeferredSkyPass(world, camera, gbuffer);

		DeferredLightPass(GetDeferredLitBuffer(gbuffer.GetSize()), gbuffer, world, &camera, Ilum::IsInIlumPass());

		DeferredCompositePass(world, camera, gbuffer);

	}

	void RenderPipeline::RenderGBuffer(World& world, CameraNode& camera, Framebuffer& gbuffer, RenderingParams& params)
	{
		gbuffer.Bind();
		RenderCommand::Clear();
		RenderCommand::SetAlphaBlending(false);
		SetFullscreenViewport(gbuffer);
		RenderCommand::SetWireframeMode(params.DrawWireframe);

		Array<MeshNode*> meshes = world.FindNodesByClass<MeshNode>();
		int32_t meshID = 0;
		for (MeshNode* meshNode : meshes)
		{
			if (meshNode->mesh && meshNode->GetMaterials().Materials.Size() > 0 && meshNode->GetMaterials().Materials[0] && meshNode->GetMaterials().Materials[0]->IsDeferred())
			{
				Renderer3D::DrawMeshNode(&camera, meshNode, MaterialType::Material, ++meshID);
				
			}
		}
		RenderCommand::SetWireframeMode(false);
	}

	void RenderPipeline::DecalPass(World& world, CameraNode& camera, Framebuffer& gbuffer)
	{
		Array<DecalNode*> decals = world.FindNodesByClass<DecalNode>();
		if (decals.Size() == 0) return;
		SetFullscreenViewport(gbuffer);
		RenderFramebufferIntoFramebuffer(gbuffer, *GetDeferredDecalBuffer(gbuffer.GetSize()), *m_DeferredDecalPreparation, glm::ivec4(0, 0, gbuffer.GetSize().x, gbuffer.GetSize().y), "u_WorldPos", (int)GBuffer::WorldPosition);
		gbuffer.Bind();

		for (DecalNode* node : decals)
		{
			if (!node->IsEnabled() || !node->material || !node->material->GetShaderGraph() || node->material->GetShaderGraph()->m_BaseShader != "DeferredDecal.glsl") continue;
			node->WorldUpdate(0.0f);
			node->material->GetShaderGraph()->GetShader()->Bind();
			node->material->ApplyUniforms(MaterialType::Material);
			node->material->GetShaderGraph()->GetShader()->SetInt("u_DecalCount", 1);
			node->material->GetShaderGraph()->GetShader()->SetMat4("u_DecalViewMatrix[0]", node->m_Projection->GetProjectionMatrix() * glm::inverse(node->GetTransformMatrix()));
			node->material->GetShaderGraph()->GetShader()->SetInt("u_OG_WorldPos", 3);
			GetDeferredDecalBuffer(gbuffer.GetSize())->BindColorAttachmentByIndex(0, 3);

			RenderCommand::SetDepthTest(false);
			GetFullscreenQuad()->Bind();
			RenderCommand::DrawIndexed(GetFullscreenQuad());

		}
	}

	void RenderPipeline::DeferredSkyPass(World& world, CameraNode& camera, Framebuffer& gbuffer)
	{
		gbuffer.Bind();
		RenderCommand::SetAlphaBlending(false);
		SetFullscreenViewport(gbuffer);

		Array<SkyLightNode*> skies = world.FindNodesByClass<SkyLightNode>();
		Array<DirectionalLightNode*> lights = world.FindNodesByClass<DirectionalLightNode>();
		if (skies.Size() > 0)
		{
			SkyLightNode* sky = skies[0];
			if (sky && sky->IsEnabled())
			{
				m_DeferredSkyShader->Bind();
				m_DeferredSkyShader->SetFloat("u_FarPlane", camera.GetPerspectiveFarClip());
				m_DeferredSkyShader->SetFloat3("u_ViewDirection", camera.GetTransform()->GetForwardVector());
				m_DeferredSkyShader->SetFloat3("u_ViewPos", camera.GetTransform()->GetPosition());
				m_DeferredSkyShader->SetMat4("u_ViewProjection", camera.GetProjectionMatrix() * camera.GetTransform()->GetTransformMatrix());
				for (int i = 0; i < lights.Size(); i++)
				{
					m_DeferredSkyShader->SetFloat3("u_LightDirection", lights[i]->GetTransform()->GetForwardVector());
				}
				m_DeferredSkyShader->SetFloat2("u_Resolution", m_InternalResolution);
				RenderCommand::SetDepthTest(true);
				GetFullscreenQuad()->Bind();
				RenderCommand::DrawIndexed(GetFullscreenQuad());
			}
		}
	}

	void RenderPipeline::DeferredLightPass(Ref<Framebuffer> target, Framebuffer& gBuffer, World& world, CameraNode* camera, bool lowQuality, int quadTick, bool volumetric)
	{
		RenderCommand::SetClearColor(glm::vec4(0.0f));
		target->Bind();
		RenderCommand::Clear();
		RenderCommand::SetAlphaBlending(AlphaBlendMode::Disable);
		
		if (lowQuality)
		{
			const glm::ivec4 rect = quadTick == 0 ? (glm::ivec4(0, 0, target->GetSpecification().Width / 2, target->GetSpecification().Height / 2)) :
									quadTick == 1 ? (glm::ivec4(target->GetSpecification().Width / 2, 0, target->GetSpecification().Width / 2, target->GetSpecification().Height / 2)) :
									quadTick == 2 ? (glm::ivec4(0, target->GetSpecification().Height / 2, target->GetSpecification().Width / 2, target->GetSpecification().Height / 2)) :
													(glm::ivec4(target->GetSpecification().Width / 2, target->GetSpecification().Height / 2, target->GetSpecification().Width / 2, target->GetSpecification().Height / 2));
			RenderCommand::SetViewport(rect.x, rect.y, rect.z, rect.w);
		}
		else SetFullscreenViewport(gBuffer);

		RenderFramebufferIntoFramebuffer(gBuffer, *target, *m_FullscreenPassShader, BufferToRect(gBuffer), "u_Texture", (int)GBuffer::Emissive);
		RenderCommand::SetAlphaBlending(AlphaBlendMode::Additive);


		/* ----- Sky Light ----- */
		if (!volumetric)
		{
			m_DeferredSkyLightShader->Bind();
			Array<SkyLightNode*> lights = world.FindNodesByClass<SkyLightNode>();
			if(lights.Size() > 0)
			{
				SkyLightNode* sky = lights[0];
				if (sky && sky->IsEnabled())
				{
					m_DeferredSkyLightShader->SetFloat3("u_ViewPos", camera->GetTransform()->GetPosition());
					m_DeferredSkyLightShader->SetInt("u_Emissive", (int)GBuffer::Emissive); gBuffer.BindColorAttachmentByIndex((int)GBuffer::Emissive, (int)GBuffer::Emissive);
					m_DeferredSkyLightShader->SetInt("u_WorldNormal", (int)GBuffer::WorldNormal); gBuffer.BindColorAttachmentByIndex((int)GBuffer::WorldNormal, (int)GBuffer::WorldNormal);
					RenderFramebufferIntoFramebuffer(gBuffer, *target, *m_DeferredSkyLightShader, BufferToRect(gBuffer), "u_WorldPos", (int)GBuffer::WorldPosition, false);
				}
			}

		}

		/* ----- Directional Lights ----- */
		{
			m_DeferredDirectionalLightShader->Bind();
			Array<DirectionalLightNode*> lights = world.FindNodesByClass<DirectionalLightNode>();
			for (int i = 0; i < lights.Size(); i++)
			{
				if (!lights[i]->IsEnabled()) continue;
				m_DeferredDirectionalLightShader->SetFloat3("u_LightColor", lights[i]->m_Color);
				m_DeferredDirectionalLightShader->SetFloat("u_LightIntensity", lights[i]->m_Intensity);
				m_DeferredDirectionalLightShader->SetFloat3("u_LightDirection", lights[i]->GetTransform()->GetForwardVector());
				m_DeferredDirectionalLightShader->SetFloat3("u_ViewPos", camera->GetTransform()->GetPosition());

				m_DeferredDirectionalLightShader->SetBool("u_Volumetric", volumetric);
				m_DeferredDirectionalLightShader->SetBool("u_ShadowMapping", lights[i]->m_ShadowMap);
				m_DeferredDirectionalLightShader->SetBool("u_SoftShadows", lowQuality ? false : lights[i]->m_SoftShadows);

				int index = 0;
				for (ShadowCascade& cascade : lights[i]->m_Cascades)
				{
					if (lowQuality && index != lights[i]->m_Cascades.Last())
					{
						index++;
						continue;
					}
					constexpr int DepthTextureOffset = (int)GBuffer::GBufferSlotCount + 1;
					m_DeferredDirectionalLightShader->SetMat4("u_LightProjection[" + std::to_string(index) + "]", cascade.m_Matrix * glm::inverse(lights[i]->m_LightCamera.GetTransform()->GetTransformMatrix()));
					m_DeferredDirectionalLightShader->SetInt("u_ShadowMap[" + std::to_string(index) + "]", index + DepthTextureOffset); cascade.m_ShadowMapBuffer->BindDepthAttachmentToSlot(index + DepthTextureOffset);
					index++;
				}

				m_DeferredDirectionalLightShader->SetInt("u_BaseColor", (int)GBuffer::BaseColor); gBuffer.BindColorAttachmentByIndex((int)GBuffer::BaseColor, (int)GBuffer::BaseColor);
				m_DeferredDirectionalLightShader->SetInt("u_Roughness", (int)GBuffer::Roughness); gBuffer.BindColorAttachmentByIndex((int)GBuffer::Roughness, (int)GBuffer::Roughness);
				m_DeferredDirectionalLightShader->SetInt("u_Metallness", (int)GBuffer::Metallic); gBuffer.BindColorAttachmentByIndex((int)GBuffer::Metallic, (int)GBuffer::Metallic);
				m_DeferredDirectionalLightShader->SetInt("u_WorldNormal", (int)GBuffer::WorldNormal); gBuffer.BindColorAttachmentByIndex((int)GBuffer::WorldNormal, (int)GBuffer::WorldNormal);
				m_DeferredDirectionalLightShader->SetInt("u_WorldPos", (int)GBuffer::WorldPosition); gBuffer.BindColorAttachmentByIndex((int)GBuffer::WorldPosition, (int)GBuffer::WorldPosition);
				m_DeferredDirectionalLightShader->SetInt("u_CascadeCount", lights[i]->m_Cascades.Size()); 
				m_DeferredDirectionalLightShader->SetInt("u_CascadeBeginIndex", lowQuality ? lights[i]->m_Cascades.Last() : 0);


				RenderFramebufferIntoFramebuffer(gBuffer, *target, *m_DeferredDirectionalLightShader, BufferToRect(gBuffer), "u_BaseColor", (int)GBuffer::BaseColor, false);
			}
		}

		/* ----- Point Lights ----- */
		{
			m_DeferredPointLightShader->Bind();
			Array<PointLightNode*> lights = world.FindNodesByClass<PointLightNode>();
			std::vector<PointLightMatrixStruct> SSBO;
			int32_t PointLightIter = 0;
			for (int i = 0; i < lights.Size(); i++)
			{
				if (!lights[i]->IsEnabled()) continue;
				float distance = Vec::Distance(camera->GetPosition(), lights[i]->GetPosition());
				if (distance >= lights[i]->m_LightCullRange + lights[i]->m_LightCullFalloff) continue;
				float falloff = distance < lights[i]->m_LightCullRange ? 1.0f : Math::Remap(distance, lights[i]->m_LightCullRange, lights[i]->m_LightCullRange + lights[i]->m_LightCullFalloff, 1.0f, 0.0f);

				//SSBO.push_back(lights[i]->m_ViewMatrix);

				m_DeferredPointLightShader->SetMat4("u_ViewTop[" + std::to_string(PointLightIter) + "]", lights[i]->m_ViewMatrix.ViewTop);
				m_DeferredPointLightShader->SetMat4("u_ViewBottom[" + std::to_string(PointLightIter) + "]", lights[i]->m_ViewMatrix.ViewBottom);
				m_DeferredPointLightShader->SetMat4("u_ViewLeft[" + std::to_string(PointLightIter) + "]", lights[i]->m_ViewMatrix.ViewLeft);
				m_DeferredPointLightShader->SetMat4("u_ViewRight[" + std::to_string(PointLightIter) + "]", lights[i]->m_ViewMatrix.ViewRight);
				m_DeferredPointLightShader->SetMat4("u_ViewForward[" + std::to_string(PointLightIter) + "]", lights[i]->m_ViewMatrix.ViewForward);
				m_DeferredPointLightShader->SetMat4("u_ViewBackward[" + std::to_string(PointLightIter) + "]", lights[i]->m_ViewMatrix.ViewBackward);

				m_DeferredPointLightShader->SetFloat3("u_PointLightPos[" + std::to_string(PointLightIter) + "]", lights[i]->GetTransform()->GetPosition());
				m_DeferredPointLightShader->SetFloat3("u_PointLightColor[" + std::to_string(PointLightIter) + "]", lights[i]->m_Color);
				m_DeferredPointLightShader->SetFloat("u_PointLightRadius[" + std::to_string(PointLightIter) + "]", lights[i]->m_Radius);
				m_DeferredPointLightShader->SetFloat("u_PointLightIntensity[" + std::to_string(PointLightIter) + "]", lights[i]->m_Intensity * falloff);
				m_DeferredPointLightShader->SetInt("u_PointLightShadowIndex[" + std::to_string(PointLightIter) + "]", lights[i]->m_ShadowMap ? PointLightNode::s_ShadowAtlasContent.IndexOf(lights[i]) : -1);
				PointLightIter++;
			}
			m_DeferredPointLightShader->SetInt("u_PointLights", PointLightIter);
			m_DeferredPointLightShader->SetInt("u_ShadowMapCount", PointLightNode::s_ShadowAtlasContent.Size());
			m_DeferredPointLightShader->SetBool("u_Volumetric", volumetric);

			m_DeferredPointLightShader->SetFloat3("u_ViewPos", camera->GetTransform()->GetPosition());
			m_DeferredPointLightShader->SetInt("u_WorldNormal", 1); gBuffer.BindColorAttachmentByIndex((int)GBuffer::WorldNormal, 1);
			m_DeferredPointLightShader->SetInt("u_Roughness", 2); gBuffer.BindColorAttachmentByIndex((int)GBuffer::Roughness, 2);
			m_DeferredPointLightShader->SetInt("u_BaseColor", 3); gBuffer.BindColorAttachmentByIndex((int)GBuffer::BaseColor, 3);
			m_DeferredPointLightShader->SetInt("u_Metallic", 4); gBuffer.BindColorAttachmentByIndex((int)GBuffer::Metallic, 4);
			m_DeferredPointLightShader->SetInt("u_ShadowAtlas", 5); if (PointLightNode::s_ShadowAtlas) PointLightNode::s_ShadowAtlas->BindDepthAttachmentToSlot(5);

			RenderFramebufferIntoFramebuffer(gBuffer, *target, *m_DeferredPointLightShader, BufferToRect(gBuffer), "u_WorldPos", (int)GBuffer::WorldPosition, false);
		}


		/* ----- Indirect Ilumination ----- */
		if (!lowQuality)
		{
			world.GetIlumContext()->ApplyIlumination(gBuffer, *camera, world, *target);
		}
		RenderCommand::SetAlphaBlending(AlphaBlendMode::Disable);
	}

	void RenderPipeline::DeferredCompositePass(World& world, CameraNode& camera, Framebuffer& gbuffer)
	{
		BlitDepthBuffer(gbuffer, *GetForwardReadyBuffer(gbuffer.GetSize()), *m_DepthBlitShader);
		
		m_DeferredComposite->Bind();
		m_DeferredComposite->SetFloat3("u_View", camera.GetForwardVector());
		m_DeferredComposite->SetFloat4("u_ForwardClearColor", camera.GetClearColor());
		// BaseColor = 0
		gbuffer.BindColorAttachmentByIndex((int)GBuffer::Metallic, 1); m_DeferredComposite->SetInt("u_Metallic", 1);
		GetDeferredLitBuffer(gbuffer.GetSize())->BindColorAttachmentByIndex(0, 2); m_DeferredComposite->SetInt("u_Radiance", 2);
		gbuffer.BindColorAttachmentByIndex((int)GBuffer::WorldNormal, 5); m_DeferredComposite->SetInt("u_WorldNormal", 5);
		gbuffer.BindColorAttachmentByIndex((int)GBuffer::WorldPosition, 13); m_DeferredComposite->SetInt("u_WorldPosition", 13);
		gbuffer.BindColorAttachmentByIndex((int)GBuffer::Roughness, 6); m_DeferredComposite->SetInt("u_Roughness", 6);
		Texture2D* texture = (world.FindNodesByClass<SkyLightNode>().Size() != 0) ? world.FindNodesByClass<SkyLightNode>()[0]->m_SkyTexture : AssetManager::GetAsset<Texture2D>(SuoraID("a6d871d8-52c5-43cc-ba73-191acfe2b7e5"));
		if (texture)
		{
			texture->GetTexture()->Bind(7); m_DeferredComposite->SetInt("u_IrradianceMap", 7); m_DeferredComposite->SetInt("u_PrefilterMap", 7);
		}
		AssetManager::GetAsset<Texture2D>(SuoraID("d1fcff5c-fc7b-4470-9f5d-8167f0bf874c"))->GetTexture()->Bind(8); m_DeferredComposite->SetInt("u_BrdfLUT", 8);

		// Ilum
		world.GetIlumContext()->m_IluminationCache->BindColorAttachmentByIndex(0, 9); m_DeferredComposite->SetInt("u_IlumCache", 9);
		m_DeferredComposite->SetFloat3("u_LightGridPos", world.GetIlumContext()->m_LightProbeGridPos);
		m_DeferredComposite->SetFloat3("u_LightGridStep", world.GetIlumContext()->m_LightProbeGridOffset);
		{
			std::random_device rd;
			static std::mt19937 mt(rd());
			std::uniform_int_distribution<int> dt(0, INT_MAX);
			m_DeferredComposite->SetInt("u_IlumSeed", dt(mt));
		}

		RenderFramebufferIntoFramebuffer(gbuffer, *GetForwardReadyBuffer(gbuffer.GetSize()), *m_DeferredComposite, BufferToRect(gbuffer), "u_BaseColor", (int)GBuffer::BaseColor, false);
	}

	void RenderPipeline::ForwardPass(World& world, CameraNode& camera, Framebuffer& gbuffer, RenderingParams& params)
	{
		SetFullscreenViewport(gbuffer);

		RenderCommand::SetWireframeMode(params.DrawWireframe);

		Array<MeshNode*> meshes = world.FindNodesByClass<MeshNode>();
		for (MeshNode* meshNode : meshes)
		{
			if (meshNode->mesh && meshNode->GetMaterials().Materials.Size() > 0 && meshNode->GetMaterials().Materials[0] && meshNode->GetMaterials().Materials[0]->GetShaderGraph() && meshNode->GetMaterials().Materials[0]->GetShaderGraph()->m_BaseShader != "DeferredLit.glsl")
			{
				if (camera.IsInFrustum(meshNode->GetPosition(), 1.5f * meshNode->GetBoundingSphereRadius()))
					Renderer3D::DrawMesh(&camera, meshNode->GetTransformMatrix(), *meshNode->mesh, meshNode->GetMaterials());
			}
		}

		RenderCommand::SetWireframeMode(false);

		RenderFramebufferIntoFramebuffer(*GetForwardReadyBuffer(gbuffer.GetSize()), *GetFinalFramebuffer(gbuffer.GetSize()), *m_FullscreenPassShader, BufferToRect(gbuffer));
	}

	void RenderPipeline::PostProcessPass(World& world, CameraNode& camera, Framebuffer& gbuffer, RenderingParams& params)
	{
		SetFullscreenViewport(gbuffer);
		
		// PostProcessNodes
		{
			Array<PostProcessEffect*> effects = world.FindNodesByClass<PostProcessEffect>();
			for (PostProcessEffect* effect : effects)
			{
				if (!effect->IsEnabled()) continue;
				if (!effect->m_Initialized)
				{
					effect->Init();
					effect->m_Initialized = true;
				}
				effect->m_ForwardBuffer = GetForwardReadyBuffer(gbuffer.GetSize());
				effect->Process(GetFinalFramebuffer(gbuffer.GetSize()), gbuffer, camera);
			}
		}
	}


	Ref<VertexArray> RenderPipeline::GetFullscreenQuad()
	{
		static bool s_DrawRectInit = false;
		static Ref<VertexArray> s_FullscreenQuadVAO;
		static Ref<VertexBuffer> s_FullscreenQuadVBO;
		static Ref<IndexBuffer> s_FullscreenQuadIB;

		if (s_DrawRectInit)
		{
			return s_FullscreenQuadVAO;
		}

		if (AssetManager::s_AssetStreamPool.Size() != 0)
			s_DrawRectInit = true;

		s_FullscreenQuadVAO = VertexArray::Create();

		Vertex vertices[4] = { Vertex(glm::vec3(1, -1, 0), glm::vec2(1, 0)),
							  Vertex(glm::vec3(-1, 1, 0), glm::vec2(0, 1)),
							  Vertex(glm::vec3(1, 1, 0),  glm::vec2(1, 1)),
							  Vertex(glm::vec3(-1, -1, 0),glm::vec2(0, 0)) };
		uint32_t indices[6] = { 2, 1, 0, 0, 1, 3 };

		s_FullscreenQuadVBO = VertexBuffer::Create(sizeof(Vertex) * 4);
		s_FullscreenQuadVBO->SetLayout(VertexLayout::VertexBufferLayout);

		s_FullscreenQuadIB = IndexBuffer::Create(&indices[0], sizeof(uint32_t) * 6);
		s_FullscreenQuadVAO->SetIndexBuffer(s_FullscreenQuadIB);

		s_FullscreenQuadVBO->SetData(&vertices[0], 4 * sizeof(Vertex));
		s_FullscreenQuadVAO->AddVertexBuffer(s_FullscreenQuadVBO);

		return s_FullscreenQuadVAO;
	}

	Ref<Framebuffer> RenderPipeline::GetDeferredLitBuffer(const glm::ivec2& size)
	{
		if (m_DeferredLitBuffer.find(size) == m_DeferredLitBuffer.end())
		{
			FramebufferSpecification spec;
			spec.Width = size.x;
			spec.Height = size.y;
			spec.Attachments.Attachments.push_back(FramebufferTextureFormat::RGB32F);
			m_DeferredLitBuffer[size] = Framebuffer::Create(spec);
		}
		return m_DeferredLitBuffer[size];
	}
	Ref<Framebuffer> RenderPipeline::GetDeferredDecalBuffer(const glm::ivec2& size)
	{
		if (m_DeferredDecalBuffer.find(size) == m_DeferredDecalBuffer.end())
		{
			FramebufferSpecification spec;
			spec.Width = size.x;
			spec.Height = size.y;
			spec.Attachments.Attachments.push_back(GBufferSlotToBufferFormat(GBuffer::WorldPosition));
			m_DeferredDecalBuffer[size] = Framebuffer::Create(spec);
		}
		return m_DeferredDecalBuffer[size];
	}

	Ref<Framebuffer> RenderPipeline::GetForwardReadyBuffer(const glm::ivec2& size)
	{
		if (m_ForwardReadyBuffer.find(size) == m_ForwardReadyBuffer.end())
		{
			FramebufferSpecification spec;
			spec.Width = size.x;
			spec.Height = size.y;
			spec.Attachments.Attachments.push_back(FramebufferTextureFormat::RGB32F);
			spec.Attachments.Attachments.push_back(FramebufferTextureFormat::Depth);
			m_ForwardReadyBuffer[size] = Framebuffer::Create(spec);
		}
		return m_ForwardReadyBuffer[size];
	}

	Ref<Framebuffer> RenderPipeline::GetFinalFramebuffer(const glm::ivec2& size)
	{
		if (m_FinalFramebuffer.find(size) == m_FinalFramebuffer.end())
		{
			FramebufferSpecification spec;
			spec.Width = size.x;
			spec.Height = size.y;
			spec.Attachments.Attachments.push_back(FramebufferTextureFormat::RGB32F);
			spec.Attachments.Attachments.push_back(FramebufferTextureFormat::Depth);
			m_FinalFramebuffer[size] = Framebuffer::Create(spec);
		}
		return m_FinalFramebuffer[size];
	}

}