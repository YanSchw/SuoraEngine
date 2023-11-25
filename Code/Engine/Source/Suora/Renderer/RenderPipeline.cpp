#include "Precompiled.h"
#include "RenderPipeline.h"

#include "Suora/GameFramework/World.h"
#include "Suora/Renderer/Framebuffer.h"
#include "Suora/Renderer/Renderer3D.h"
#include "Suora/Renderer/RenderCommand.h"
#include "Suora/Renderer/VertexArray.h"
#include "Suora/Renderer/Shader.h"
#include "Suora/Renderer/Decima.h"
#include "Suora/Assets/AssetManager.h"
#include "Suora/Assets/ShaderGraph.h"
#include "Suora/Assets/SuoraProject.h"

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

	FramebufferTextureParams RenderPipeline::GBufferSlotToBufferParams(GBuffer slot)
	{
		switch (slot)
		{
		case GBuffer::BaseColor: return FramebufferTextureFormat::RGB16F;
		case GBuffer::Metallic: return FramebufferTextureFormat::R8;
		case GBuffer::Roughness: return FramebufferTextureFormat::R8;
		case GBuffer::WorldPosition: return FramebufferTextureFormat::RGB16F;
		case GBuffer::WorldNormal: return FramebufferTextureParams(FramebufferTextureFormat::RGB16F, FramebufferTextureFilter::Nearest);
		case GBuffer::Emissive: return FramebufferTextureFormat::RGB16F;
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
		m_FullscreenPassShader = Shader::Create(AssetManager::GetEngineAssetPath() + "/EngineContent/Shaders/FullscreenPass.glsl");
		m_DepthBlitShader = Shader::Create(AssetManager::GetEngineAssetPath() + "/EngineContent/Shaders/DepthBlit.glsl");
		m_ToneMapping = Shader::Create(AssetManager::GetEngineAssetPath() + "/EngineContent/Shaders/PostProccess/ToneMapping.glsl");
		m_FXAA = Shader::Create(AssetManager::GetEngineAssetPath() + "/EngineContent/Shaders/PostProccess/FXAA.glsl");
		m_DeferredDecalPreparation = Shader::Create(AssetManager::GetEngineAssetPath() + "/EngineContent/Shaders/Deferred/Deferred_DecalPreparation.glsl");
		m_DeferredDirectionalLightShader = Shader::Create(AssetManager::GetEngineAssetPath() + "/EngineContent/Shaders/Deferred/Deferred_DirectionalLight.glsl");
		m_DeferredPointLightShader = Shader::Create(AssetManager::GetEngineAssetPath() + "/EngineContent/Shaders/Deferred/Deferred_PointLight.glsl");
		m_DeferredPointLightMatrixBuffer = ShaderStorageBuffer::Create();
		m_DeferredSkyShader = Shader::Create(AssetManager::GetEngineAssetPath() + "/EngineContent/Shaders/Deferred/Deferred_Sky.glsl");
		m_DeferredSkyLightShader = Shader::Create(AssetManager::GetEngineAssetPath() + "/EngineContent/Shaders/Deferred/Deferred_SkyLight.glsl");
		m_DeferredComposite = Shader::Create(AssetManager::GetEngineAssetPath() + "/EngineContent/Shaders/Deferred/Deferred_Composite.glsl");
		
		m_DecimaInstance = CreateRef<Decima>();
	}

	Ref<Shader> RenderPipeline::GetFullscreenPassShaderStatic()
	{
		RenderPipeline* pipeline = Engine::Get()->GetRenderPipeline();
		return pipeline ? pipeline->GetFullscreenPassShader() : nullptr;
	}

	void RenderPipeline::Render(Framebuffer& buffer, World& world, CameraNode& camera, RenderingParams& params)
	{
		SUORA_ASSERT(buffer.GetSpecification().Attachments.Attachments[0].TextureFormat == FramebufferTextureFormat::RGBA8);

		params.ValidateBuffers();

		ShadowPass(world, camera);

		SetFullscreenViewport(*params.GetGBuffer());

		if (params.EnableDeferredRendering)
		{
			DeferredPass(world, camera, params);
		}
		else
		{
			params.GetForwardReadyBuffer()->Bind();
			RenderCommand::SetClearColor(camera.GetClearColor());
			RenderCommand::Clear();
		}

		RenderCommand::SetAlphaBlending(AlphaBlendMode::Blend);

		ForwardPass(world, camera, params);

		PostProcessPass(world, camera, params);

		UserInterfacePass(world, glm::orthoLH(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f), *params.GetFinalBuffer(), params);

		// Output Final Buffer
		RenderFramebufferIntoFramebuffer(*params.GetFinalBuffer(), buffer, *m_FullscreenPassShader, BufferToRect(buffer), "u_Texture", 0, true);
	}


	void RenderPipeline::RenderFramebufferIntoFramebuffer(Framebuffer& src, Framebuffer& target, Shader& shader, const glm::ivec4& rect, const String& uniformName, int attachmentIndex, bool shouldClear)
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

	void RenderPipeline::BlitDepthBuffer(Framebuffer& src, Framebuffer& target, Shader& shader, const String& uniformName)
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
				ClearShader = Shader::Create(AssetManager::GetEngineAssetPath() + "/EngineContent/Shaders/ClearDepth.glsl");
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

	void RenderPipeline::DeferredPass(World& world, CameraNode& camera, RenderingParams& params)
	{
		RenderCommand::SetClearColor(Color(0,0,0,1));
		RenderGBuffer(world, camera, params);

		DecalPass(world, camera, params);

		DeferredSkyPass(world, camera, params);

		DeferredLightPass(params.GetDeferredLitBuffer(), params, world, &camera);

		DeferredCompositePass(world, camera, params);

	}

	void RenderPipeline::RenderGBuffer(World& world, CameraNode& camera, RenderingParams& params)
	{
		params.GetGBuffer()->Bind();
		RenderCommand::Clear();
		RenderCommand::SetAlphaBlending(false);
		SetFullscreenViewport(*params.GetGBuffer());
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

	void RenderPipeline::DecalPass(World& world, CameraNode& camera, RenderingParams& params)
	{
		Array<DecalNode*> decals = world.FindNodesByClass<DecalNode>();
		if (decals.Size() == 0) return;
		SetFullscreenViewport(*params.GetGBuffer());
		RenderFramebufferIntoFramebuffer(*params.GetGBuffer(), *params.GetDeferredDecalBuffer(), *m_DeferredDecalPreparation, BufferToRect(*params.GetGBuffer()), "u_WorldPos", (int)GBuffer::WorldPosition);
		params.GetGBuffer()->Bind();

		for (DecalNode* node : decals)
		{
			if (!node->IsEnabled() || !node->material || !node->material->GetShaderGraph() || node->material->GetShaderGraph()->m_BaseShader != "DeferredDecal.glsl") continue;
			node->WorldUpdate(0.0f);
			node->material->GetShaderGraph()->GetShader()->Bind();
			node->material->ApplyUniforms(MaterialType::Material);
			node->material->GetShaderGraph()->GetShader()->SetInt("u_DecalCount", 1);
			node->material->GetShaderGraph()->GetShader()->SetMat4("u_DecalViewMatrix[0]", node->m_Projection->GetProjectionMatrix() * glm::inverse(node->GetTransformMatrix()));
			node->material->GetShaderGraph()->GetShader()->SetInt("u_OG_WorldPos", 3);
			params.GetDeferredDecalBuffer()->BindColorAttachmentByIndex(0, 3);

			RenderCommand::SetDepthTest(false);
			GetFullscreenQuad()->Bind();
			RenderCommand::DrawIndexed(GetFullscreenQuad());

		}
	}

	void RenderPipeline::DeferredSkyPass(World& world, CameraNode& camera, RenderingParams& params)
	{
		params.GetGBuffer()->Bind();
		RenderCommand::SetAlphaBlending(false);
		SetFullscreenViewport(*params.GetGBuffer());

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
				m_DeferredSkyShader->SetFloat2("u_Resolution", params.Resolution);
				RenderCommand::SetDepthTest(true);
				GetFullscreenQuad()->Bind();
				RenderCommand::DrawIndexed(GetFullscreenQuad());
			}
		}
	}

	void RenderPipeline::DeferredLightPass(Ref<Framebuffer> target, RenderingParams& params, World& world, CameraNode* camera)
	{
		RenderCommand::SetClearColor(Vec4(0.0f));
		target->Bind();
		RenderCommand::Clear();
		RenderCommand::SetAlphaBlending(AlphaBlendMode::Disable);
		
		SetFullscreenViewport(*params.GetGBuffer());

		RenderFramebufferIntoFramebuffer(*params.GetGBuffer(), *target, *m_FullscreenPassShader, BufferToRect(*params.GetGBuffer()), "u_Texture", (int)GBuffer::Emissive);
		RenderCommand::SetAlphaBlending(AlphaBlendMode::Additive);


		/* ----- Sky Light ----- */
		m_DeferredSkyLightShader->Bind();
		Array<SkyLightNode*> lights = world.FindNodesByClass<SkyLightNode>();
		if (lights.Size() > 0)
		{
			SkyLightNode* sky = lights[0];
			if (sky && sky->IsEnabled())
			{
				m_DeferredSkyLightShader->SetFloat("u_Intensity", sky->m_Intensity);
				m_DeferredSkyLightShader->SetFloat3("u_Color", sky->m_Color);
				RenderFramebufferIntoFramebuffer(*params.GetGBuffer(), *target, *m_DeferredSkyLightShader, BufferToRect(*params.GetGBuffer()), "u_BaseColor", (int)GBuffer::BaseColor, false);
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

				m_DeferredDirectionalLightShader->SetBool("u_Volumetric", false);
				m_DeferredDirectionalLightShader->SetBool("u_ShadowMapping", lights[i]->m_ShadowMap);
				m_DeferredDirectionalLightShader->SetBool("u_SoftShadows", lights[i]->m_SoftShadows);

				int index = 0;
				for (ShadowCascade& cascade : lights[i]->m_Cascades)
				{
					constexpr int DepthTextureOffset = (int)GBuffer::GBufferSlotCount + 1;
					m_DeferredDirectionalLightShader->SetMat4("u_LightProjection[" + std::to_string(index) + "]", cascade.m_Matrix * glm::inverse(lights[i]->m_LightCamera.GetTransform()->GetTransformMatrix()));
					m_DeferredDirectionalLightShader->SetInt("u_ShadowMap[" + std::to_string(index) + "]", index + DepthTextureOffset); cascade.m_ShadowMapBuffer->BindDepthAttachmentToSlot(index + DepthTextureOffset);
					index++;
				}

				m_DeferredDirectionalLightShader->SetInt("u_BaseColor", (int)GBuffer::BaseColor); params.GetGBuffer()->BindColorAttachmentByIndex((int)GBuffer::BaseColor, (int)GBuffer::BaseColor);
				m_DeferredDirectionalLightShader->SetInt("u_Roughness", (int)GBuffer::Roughness); params.GetGBuffer()->BindColorAttachmentByIndex((int)GBuffer::Roughness, (int)GBuffer::Roughness);
				m_DeferredDirectionalLightShader->SetInt("u_Metallness", (int)GBuffer::Metallic); params.GetGBuffer()->BindColorAttachmentByIndex((int)GBuffer::Metallic, (int)GBuffer::Metallic);
				m_DeferredDirectionalLightShader->SetInt("u_WorldNormal", (int)GBuffer::WorldNormal); params.GetGBuffer()->BindColorAttachmentByIndex((int)GBuffer::WorldNormal, (int)GBuffer::WorldNormal);
				m_DeferredDirectionalLightShader->SetInt("u_WorldPos", (int)GBuffer::WorldPosition); params.GetGBuffer()->BindColorAttachmentByIndex((int)GBuffer::WorldPosition, (int)GBuffer::WorldPosition);
				m_DeferredDirectionalLightShader->SetInt("u_CascadeCount", lights[i]->m_Cascades.Size()); 
				m_DeferredDirectionalLightShader->SetInt("u_CascadeBeginIndex", 0);


				RenderFramebufferIntoFramebuffer(*params.GetGBuffer(), *target, *m_DeferredDirectionalLightShader, BufferToRect(*params.GetGBuffer()), "u_BaseColor", (int)GBuffer::BaseColor, false);
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
			m_DeferredPointLightShader->SetBool("u_Volumetric", false);

			m_DeferredPointLightShader->SetFloat3("u_ViewPos", camera->GetTransform()->GetPosition());
			m_DeferredPointLightShader->SetInt("u_WorldNormal", 1); params.GetGBuffer()->BindColorAttachmentByIndex((int)GBuffer::WorldNormal, 1);
			m_DeferredPointLightShader->SetInt("u_Roughness", 2); params.GetGBuffer()->BindColorAttachmentByIndex((int)GBuffer::Roughness, 2);
			m_DeferredPointLightShader->SetInt("u_BaseColor", 3); params.GetGBuffer()->BindColorAttachmentByIndex((int)GBuffer::BaseColor, 3);
			m_DeferredPointLightShader->SetInt("u_Metallic", 4); params.GetGBuffer()->BindColorAttachmentByIndex((int)GBuffer::Metallic, 4);
			m_DeferredPointLightShader->SetInt("u_ShadowAtlas", 5); if (PointLightNode::s_ShadowAtlas) PointLightNode::s_ShadowAtlas->BindDepthAttachmentToSlot(5);

			RenderFramebufferIntoFramebuffer(*params.GetGBuffer(), *target, *m_DeferredPointLightShader, BufferToRect(*params.GetGBuffer()), "u_WorldPos", (int)GBuffer::WorldPosition, false);
		}

		RenderCommand::SetAlphaBlending(AlphaBlendMode::Disable);
	}

	void RenderPipeline::DeferredCompositePass(World& world, CameraNode& camera, RenderingParams& params)
	{
		BlitDepthBuffer(*params.GetGBuffer(), *params.GetForwardReadyBuffer(), *m_DepthBlitShader);
		
		m_DeferredComposite->Bind();
		m_DeferredComposite->SetFloat3("u_View", camera.GetForwardVector());
		m_DeferredComposite->SetFloat3("u_ViewPos", camera.GetPosition());
		m_DeferredComposite->SetFloat4("u_ForwardClearColor", camera.GetClearColor());
		// BaseColor = 0
		params.GetGBuffer()->BindColorAttachmentByIndex((int)GBuffer::Metallic, 1); m_DeferredComposite->SetInt("u_Metallic", 1);
		params.GetDeferredLitBuffer()->BindColorAttachmentByIndex(0, 2); m_DeferredComposite->SetInt("u_Radiance", 2);
		params.GetGBuffer()->BindColorAttachmentByIndex((int)GBuffer::WorldNormal, 5); m_DeferredComposite->SetInt("u_WorldNormal", 5);
		params.GetGBuffer()->BindColorAttachmentByIndex((int)GBuffer::WorldPosition, 13); m_DeferredComposite->SetInt("u_WorldPosition", 13);
		params.GetGBuffer()->BindColorAttachmentByIndex((int)GBuffer::Roughness, 6); m_DeferredComposite->SetInt("u_Roughness", 6);
		Texture2D* texture = (world.FindNodesByClass<SkyLightNode>().Size() != 0) ? world.FindNodesByClass<SkyLightNode>()[0]->m_SkyTexture : AssetManager::GetAsset<Texture2D>(SuoraID("a6d871d8-52c5-43cc-ba73-191acfe2b7e5"));
		if (texture)
		{
			texture->GetTexture()->Bind(7); m_DeferredComposite->SetInt("u_IrradianceMap", 7); m_DeferredComposite->SetInt("u_PrefilterMap", 7);
		}
		AssetManager::GetAsset<Texture2D>(SuoraID("d1fcff5c-fc7b-4470-9f5d-8167f0bf874c"))->GetTexture()->Bind(8); m_DeferredComposite->SetInt("u_BrdfLUT", 8);

		RenderFramebufferIntoFramebuffer(*params.GetGBuffer(), *params.GetForwardReadyBuffer(), *m_DeferredComposite, BufferToRect(*params.GetGBuffer()), "u_BaseColor", (int)GBuffer::BaseColor, false);
	}

	void RenderPipeline::ForwardPass(World& world, CameraNode& camera, RenderingParams& params)
	{
		SetFullscreenViewport(*params.GetGBuffer());

		RenderCommand::SetWireframeMode(params.DrawWireframe);

		Array<MeshNode*> meshes = world.FindNodesByClass<MeshNode>();
		int32_t meshID = 0;
		for (MeshNode* meshNode : meshes)
		{
			if (meshNode->mesh && meshNode->GetMaterials().Materials.Size() > 0 && meshNode->GetMaterials().Materials[0] && meshNode->GetMaterials().Materials[0]->GetShaderGraph() && !meshNode->GetMaterials().Materials[0]->IsDeferred())
			{
				Renderer3D::DrawMeshNode(&camera, meshNode, MaterialType::Material, ++meshID);
			}
		}

		RenderCommand::SetWireframeMode(false);

		RenderFramebufferIntoFramebuffer(*params.GetForwardReadyBuffer(), *params.GetFinalBuffer(), *m_FullscreenPassShader, BufferToRect(*params.GetGBuffer()));
	}

	void RenderPipeline::PostProcessPass(World& world, CameraNode& camera, RenderingParams& params)
	{
		SetFullscreenViewport(*params.GetGBuffer());

		bool resultIsInTempBuffer = false;
		Ref<Framebuffer> forwardBuffer = params.GetForwardReadyBuffer();
		Ref<Framebuffer> postProcessTempBuffer = params.GetPostProcessTempBuffer();
			
		// FXAA
		{
			if (params.AntiAliasingMode == AntiAliasing::FXAA)
			{
				m_FXAA->Bind();
				m_FXAA->SetFloat2("u_Resolution", params.Resolution);
				RenderFramebufferIntoFramebuffer(*forwardBuffer, *postProcessTempBuffer, *m_FXAA, BufferToRect(*params.GetGBuffer()));
				resultIsInTempBuffer = true;
			}
		}
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
				effect->Process((resultIsInTempBuffer ? postProcessTempBuffer : forwardBuffer), 
								(resultIsInTempBuffer ? forwardBuffer : postProcessTempBuffer), params, camera);
				resultIsInTempBuffer = !resultIsInTempBuffer;
			}
		}

		RenderFramebufferIntoFramebuffer(*(resultIsInTempBuffer ? postProcessTempBuffer : forwardBuffer), *params.GetFinalBuffer(), *GetFullscreenPassShader(), BufferToRect(*params.GetGBuffer()));
	}

	void RenderPipeline::UserInterfacePass(World& world, const Mat4& view, Framebuffer& target, RenderingParams& params)
	{
		Array<UIRenderable*> renderables = world.FindNodesByClass<UIRenderable>();
		for (UIRenderable* It : renderables)
		{
			if (It->IsEnabled())
			{
				It->RenderUI(view, target);
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

		Vertex vertices[4] = { Vertex(Vec3(1, -1, 0), Vec2(1, 0)),
							   Vertex(Vec3(-1, 1, 0), Vec2(0, 1)),
							   Vertex(Vec3(1, 1, 0),  Vec2(1, 1)),
							   Vertex(Vec3(-1, -1, 0),Vec2(0, 0)) };
		uint32_t indices[6] = { 2, 1, 0, 0, 1, 3 };

		s_FullscreenQuadVBO = VertexBuffer::Create(sizeof(Vertex) * 4);
		s_FullscreenQuadVBO->SetLayout(VertexLayout::VertexBufferLayout);

		s_FullscreenQuadIB = IndexBuffer::Create(&indices[0], sizeof(uint32_t) * 6);
		s_FullscreenQuadVAO->SetIndexBuffer(s_FullscreenQuadIB);

		s_FullscreenQuadVBO->SetData(&vertices[0], 4 * sizeof(Vertex));
		s_FullscreenQuadVAO->AddVertexBuffer(s_FullscreenQuadVBO);

		return s_FullscreenQuadVAO;
	}


	RenderingParams::RenderingParams()
	{
		if (ProjectSettings::Get())
		{
			EnableDeferredRendering = ProjectSettings::Get()->m_EnableDeferredRendering;
		}
	}

	void RenderingParams::ValidateBuffers()
	{
		if (!m_InitializedBuffers)
		{
			m_InitializedBuffers = true;

			{
				FramebufferSpecification spec;
				spec.Width = Resolution.x; spec.Height = Resolution.y;
				for (int32_t i = 0; i < (int32_t)GBuffer::GBufferSlotCount; i++)
				{
					spec.Attachments.Attachments.push_back(RenderPipeline::GBufferSlotToBufferParams((GBuffer)i));
				}
				spec.Attachments.Attachments.push_back(FramebufferTextureFormat::Depth);
				m_GBuffer = Framebuffer::Create(spec);
			}
			{
				FramebufferSpecification spec;
				spec.Width = Resolution.x; spec.Height = Resolution.y;
				spec.Attachments.Attachments.push_back(FramebufferTextureFormat::RGB16F);
				m_DeferredLitBuffer = Framebuffer::Create(spec);
			}
			{
				FramebufferSpecification spec;
				spec.Width = Resolution.x; spec.Height = Resolution.y;
				spec.Attachments.Attachments.push_back(RenderPipeline::GBufferSlotToBufferParams(GBuffer::WorldPosition));
				m_DeferredDecalBuffer = Framebuffer::Create(spec);
			}
			{
				FramebufferSpecification spec;
				spec.Width = Resolution.x; spec.Height = Resolution.y;
				spec.Attachments.Attachments.push_back(FramebufferTextureFormat::RGB16F);
				spec.Attachments.Attachments.push_back(FramebufferTextureFormat::Depth);
				m_ForwardReadyBuffer = Framebuffer::Create(spec);
			}
			{
				FramebufferSpecification spec;
				spec.Width = Resolution.x; spec.Height = Resolution.y;
				spec.Attachments.Attachments.push_back(FramebufferTextureFormat::RGB16F);
				m_PostProcessTempBuffer = Framebuffer::Create(spec);
			}
			{
				FramebufferSpecification spec;
				spec.Width = Resolution.x; spec.Height = Resolution.y;
				spec.Attachments.Attachments.push_back(FramebufferTextureFormat::RGB16F);
				spec.Attachments.Attachments.push_back(FramebufferTextureFormat::Depth);
				m_FinalBuffer = Framebuffer::Create(spec);
			}
		}

		if (Resolution != LastResolution)
		{
			LastResolution = Resolution;

			m_GBuffer->Resize(Resolution);
			m_DeferredLitBuffer->Resize(Resolution);
			m_DeferredDecalBuffer->Resize(Resolution);
			m_ForwardReadyBuffer->Resize(Resolution);
			m_PostProcessTempBuffer->Resize(Resolution);
			m_FinalBuffer->Resize(Resolution);
		}
	}

}