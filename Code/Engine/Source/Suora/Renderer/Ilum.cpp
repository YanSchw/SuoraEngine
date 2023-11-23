#include "Precompiled.h"
#include "Ilum.h"
#include "Suora/GameFramework/World.h"
#include "Suora/GameFramework/Nodes/MeshNode.h"
#include "Suora/Renderer/Framebuffer.h"
#include "Suora/Renderer/RenderPipeline.h"
#include "Suora/Renderer/RenderCommand.h"
#include "Suora/Renderer/Renderer3D.h"
#include "Suora/Renderer/Shader.h"
#include "Suora/Assets/AssetManager.h"
#include "Suora/Assets/Texture2D.h"
#include "Suora/Assets/ShaderGraph.h"
#include "Suora/Core/Engine.h"
#include "Platform/Platform.h"
#include "Suora/Core/NativeInput.h"
#include "Suora/Common/Math.h"

static bool ENABLE = false;

namespace Suora
{
	struct PosReadbackBufferStruct
	{
		int SurfelIndex = -1;
		Vec3 BaseColor = Vec3();
		float Metallic = 0.0f;
		float Roughness = 1.0f;
		Vec3 WorldPosition = Vec3();
		Vec3 WorldNormal = Vec3();
		Vec3 Emissive = Vec3();
	};


	Ilum::Ilum(World* world)
	{
		m_World = world;
	}
	Ilum::~Ilum()
	{
	}
	void Ilum::Init()
	{
		m_Initialized = true;
		{
			FramebufferSpecification specs;
			specs.Width = 64;
			specs.Height = 64;
			for (int32_t i = 0; i < (int32_t)GBuffer::GBufferSlotCount; i++)
			{
				specs.Attachments.Attachments.push_back(FramebufferTextureParams(RenderPipeline::GBufferSlotToBufferParams((GBuffer)i).TextureFormat, FramebufferTextureFilter::Nearest));
			}
			specs.Attachments.Attachments.push_back(FramebufferTextureFormat::Depth);
			m_SurfelCache = Framebuffer::Create(specs);
			m_SecondaryView = Framebuffer::Create(specs);
		}
		{
			FramebufferSpecification specs;
			specs.Width = 64;
			specs.Height = 64;
			specs.Attachments.Attachments.push_back(FramebufferTextureParams(FramebufferTextureFormat::RGB32F, FramebufferTextureFilter::Nearest));
			specs.Attachments.Attachments.push_back(FramebufferTextureFormat::Depth);
			m_SurfelDirectLightCache = Framebuffer::Create(specs);
		}
		{
			FramebufferSpecification specs;
			specs.Width = 2048;
			specs.Height = 2048;
			specs.Attachments.Attachments.push_back(FramebufferTextureParams(FramebufferTextureFormat::RGB16F, FramebufferTextureFilter::Linear));
			m_IluminationCache = Framebuffer::Create(specs);
			m_IluminationCacheAcc = Framebuffer::Create(specs);
		}
		m_LightPropagationShader = Shader::Create(AssetManager::GetEngineAssetPath() + "/EngineContent/Shaders/Ilum/Ilum_LightPropagation.glsl");

		{
			FramebufferSpecification specs;
			specs.Width = 64;
			specs.Height = 64;
			specs.Attachments.Attachments.push_back(FramebufferTextureParams(FramebufferTextureFormat::RGB16F, FramebufferTextureFilter::Nearest));
			m_SurfelIlumCache = Framebuffer::Create(specs);
			m_SurfelIlumCacheAcc = Framebuffer::Create(specs);
		}
		m_SurfelPropagationShader = Shader::Create(AssetManager::GetEngineAssetPath() + "/EngineContent/Shaders/Ilum/Ilum_SurfelPropagation.glsl");
		m_ApplySurfels = Shader::Create(AssetManager::GetEngineAssetPath() + "/EngineContent/Shaders/Ilum/Ilum_ApplySurfels.glsl");
		m_Denoiser = Shader::Create(AssetManager::GetEngineAssetPath() + "/EngineContent/Shaders/Ilum/Ilum_Denoiser.glsl");
		{
			FramebufferSpecification specs;
			specs.Width = 64;
			specs.Height = 64;
			specs.Attachments.Attachments.push_back(FramebufferTextureFormat::R8);
			m_EmptyViewport = Framebuffer::Create(specs);
			m_EmptyViewport2 = Framebuffer::Create(specs);
		}
		m_PosReadback = Shader::Create(AssetManager::GetEngineAssetPath() + "/EngineContent/Shaders/Ilum/Ilum_PositionReadback.glsl");
		m_PosReadbackSSBO = ShaderStorageBuffer::Create();
		m_WriteToSurfelCache = Shader::Create(AssetManager::GetEngineAssetPath() + "/EngineContent/Shaders/Ilum/Ilum_WriteToSurfelCache.glsl");
	}

	void Ilum::Tick(float deltaTime, Framebuffer& gBuffer, CameraNode& camera, World& world)
	{
		if (!m_Initialized) Init();

		if (NativeInput::GetKeyDown(Key::F10)) ENABLE = !ENABLE;
		if (!ENABLE) return;

		s_InIlumPass = true;

		SurfelPass(gBuffer, camera, world);
#if 0
		Engine::Get()->GetRenderPipeline()->As<RenderPipeline>()->DeferredLightPass(m_SurfelDirectLightCache, *m_SurfelCache, *m_World, &camera, true);
#endif
		LightPropagationPass(gBuffer);

		
		
		s_InIlumPass = false;
	}


	void Ilum::SurfelPass(Framebuffer& gBuffer, CameraNode& camera, World& world)
	{
		std::random_device rd;
		static std::mt19937 mt(rd());
		std::uniform_int_distribution<int> dt(-123456, 123456);

		static PosReadbackBufferStruct SSBO[160] = {};
		memset(SSBO, 0, sizeof(SSBO));
		m_PosReadbackSSBO->Write(sizeof(SSBO), SSBO);
		{
			if (m_EmptyViewport->GetSize() != gBuffer.GetSize()) m_EmptyViewport->Resize(gBuffer.GetSize());
			m_EmptyViewport->Bind();
			RenderCommand::SetViewport(0, 0, gBuffer.GetSize().x, gBuffer.GetSize().y);
			m_PosReadback->Bind();
			m_PosReadbackSSBO->Bind(); 
			for (int i = 0; i < 128; i++) m_PosReadback->SetFloat2("u_Hotspot["+std::to_string(i) + "]", Vec2(Math::RandomFloat(), Math::RandomFloat()));
			m_PosReadback->SetInt("u_Seed", dt(mt));
			m_PosReadback->SetBool("u_ScreenPass", true);
			
			gBuffer.BindColorAttachmentByIndex((int)GBuffer::BaseColor, (int)GBuffer::BaseColor); m_PosReadback->SetInt("u_BaseColor", (int)GBuffer::BaseColor);
			gBuffer.BindColorAttachmentByIndex((int)GBuffer::Metallic, (int)GBuffer::Metallic); m_PosReadback->SetInt("u_Metallic", (int)GBuffer::Metallic);
			gBuffer.BindColorAttachmentByIndex((int)GBuffer::Roughness, (int)GBuffer::Roughness); m_PosReadback->SetInt("u_Roughness", (int)GBuffer::Roughness);
			gBuffer.BindColorAttachmentByIndex((int)GBuffer::WorldPosition, (int)GBuffer::WorldPosition); m_PosReadback->SetInt("u_WorldPosition", (int)GBuffer::WorldPosition);
			gBuffer.BindColorAttachmentByIndex((int)GBuffer::WorldNormal, (int)GBuffer::WorldNormal); m_PosReadback->SetInt("u_WorldNormal", (int)GBuffer::WorldNormal);
			gBuffer.BindColorAttachmentByIndex((int)GBuffer::Emissive, (int)GBuffer::Emissive); m_PosReadback->SetInt("u_Emissive", (int)GBuffer::Emissive);

			glm::mat4 viewProj = camera.GetProjectionMatrix() * glm::inverse(camera.GetTransformMatrix());
			m_PosReadback->SetMat4("u_ViewProjection", viewProj);
			m_SurfelCache->BindColorAttachmentByIndex((int)GBuffer::WorldPosition, (int)GBuffer::GBufferSlotCount); m_PosReadback->SetInt("u_SurfelPositions", (int)GBuffer::GBufferSlotCount);

			RenderCommand::SetDepthTest(false);
			RenderPipeline::GetFullscreenQuad()->Bind();
			RenderCommand::DrawIndexed(RenderPipeline::GetFullscreenQuad());
		}
		{
			PosReadbackBufferStruct readBack; m_PosReadbackSSBO->Read(sizeof(PosReadbackBufferStruct), &readBack);
			CameraNode View;
			RenderingParams RParams;
			View.SetPosition(readBack.WorldPosition);
			View.SetEulerRotation(Vec3(Math::RandomFloat() * 360.0f, Math::RandomFloat() * 360.0f, Math::RandomFloat() * 360.0f));
			View.SetPerspectiveVerticalFOV(Math::Remap(Math::RandomFloat(), 0.0f, 1.0f, 90.0f, 145.0f));
#if 0
			Engine::Get()->GetRenderPipeline()->As<RenderPipeline>()->RenderGBuffer(world, View, *m_SecondaryView, RParams);
#endif
			if (m_EmptyViewport2->GetSize() != m_SecondaryView->GetSize()) m_EmptyViewport2->Resize(m_SecondaryView->GetSize());
			m_EmptyViewport2->Bind();
			RenderCommand::SetViewport(0, 0, m_SecondaryView->GetSize().x, m_SecondaryView->GetSize().y);
			m_PosReadback->Bind();
			m_PosReadbackSSBO->Bind();
			for (int i = 128; i < 160; i++) m_PosReadback->SetFloat2("u_Hotspot[" + std::to_string(i) + "]", Vec2(Math::RandomFloat(), Math::RandomFloat()));
			m_PosReadback->SetInt("u_Seed", dt(mt));
			m_PosReadback->SetBool("u_ScreenPass", false);

			m_SecondaryView->BindColorAttachmentByIndex((int)GBuffer::BaseColor, (int)GBuffer::BaseColor); m_PosReadback->SetInt("u_BaseColor", (int)GBuffer::BaseColor);
			m_SecondaryView->BindColorAttachmentByIndex((int)GBuffer::Metallic, (int)GBuffer::Metallic); m_PosReadback->SetInt("u_Metallic", (int)GBuffer::Metallic);
			m_SecondaryView->BindColorAttachmentByIndex((int)GBuffer::Roughness, (int)GBuffer::Roughness); m_PosReadback->SetInt("u_Roughness", (int)GBuffer::Roughness);
			m_SecondaryView->BindColorAttachmentByIndex((int)GBuffer::WorldPosition, (int)GBuffer::WorldPosition); m_PosReadback->SetInt("u_WorldPosition", (int)GBuffer::WorldPosition);
			m_SecondaryView->BindColorAttachmentByIndex((int)GBuffer::WorldNormal, (int)GBuffer::WorldNormal); m_PosReadback->SetInt("u_WorldNormal", (int)GBuffer::WorldNormal);
			m_SecondaryView->BindColorAttachmentByIndex((int)GBuffer::Emissive, (int)GBuffer::Emissive); m_PosReadback->SetInt("u_Emissive", (int)GBuffer::Emissive);

			m_SurfelCache->BindColorAttachmentByIndex((int)GBuffer::WorldPosition, (int)GBuffer::GBufferSlotCount); m_PosReadback->SetInt("u_SurfelPositions", (int)GBuffer::GBufferSlotCount);

			RenderCommand::SetDepthTest(false);
			RenderPipeline::GetFullscreenQuad()->Bind();
			RenderCommand::DrawIndexed(RenderPipeline::GetFullscreenQuad());
		}
		{
			
			m_SurfelCache->Bind();
			RenderCommand::SetViewport(0, 0, 64, 64);
			m_WriteToSurfelCache->Bind();
			m_PosReadbackSSBO->Bind();
			m_WriteToSurfelCache->SetInt("u_Seed", dt(mt));
			
			RenderCommand::SetDepthTest(false);
			RenderPipeline::GetFullscreenQuad()->Bind();
			RenderCommand::DrawIndexed(RenderPipeline::GetFullscreenQuad());
		}
	}

	void Ilum::LightPropagationPass(Framebuffer& gBuffer)
	{
		/*RenderPipeline::RenderFramebufferIntoFramebuffer(*m_IluminationCache, *m_IluminationCacheAcc, *RenderPipeline::GetFullscreenPassShaderStatic(), glm::vec4(0, 0, 2048, 2048));

		std::random_device rd;
		static std::mt19937 mt(rd());
		std::uniform_int_distribution<int> dt(-123456, 123456);

		m_LightPropagationShader->Bind();
		m_LightPropagationShader->SetInt("u_Seed", dt(mt));
		m_LightPropagationShader->SetFloat3("u_LightGridPos", m_LightProbeGridPos);
		m_LightPropagationShader->SetFloat3("u_LightGridStep", m_LightProbeGridOffset);
		m_SurfelDirectLightCache->BindColorAttachmentByIndex(0, 1); m_LightPropagationShader->SetInt("u_SurfelDirectLight", 1);
		m_SurfelCache->BindColorAttachmentByIndex((int)GBuffer::WorldPosition, 2); m_LightPropagationShader->SetInt("u_SurfelWorldPos", 2);
		m_SurfelCache->BindColorAttachmentByIndex((int)GBuffer::WorldNormal, 3); m_LightPropagationShader->SetInt("u_SurfelWorldNormal", 3);
		RenderPipeline::RenderFramebufferIntoFramebuffer(*m_IluminationCacheAcc, *m_IluminationCache, *m_LightPropagationShader, glm::vec4(0, 0, 2048, 2048), "u_Accumulator", 0);*/


		RenderPipeline::RenderFramebufferIntoFramebuffer(*m_SurfelIlumCache, *m_SurfelIlumCacheAcc, *RenderPipeline::GetFullscreenPassShaderStatic(), glm::vec4(0, 0, 64, 64));

		std::random_device rd;
		static std::mt19937 mt(rd());
		std::uniform_int_distribution<int> dt(-123456, 123456);

		m_SurfelPropagationShader->Bind();
		m_SurfelPropagationShader->SetInt("u_Seed", dt(mt));
		m_SurfelDirectLightCache->BindColorAttachmentByIndex(0, 1); m_SurfelPropagationShader->SetInt("u_SurfelDirectLight", 1);
		m_SurfelCache->BindColorAttachmentByIndex((int)GBuffer::WorldPosition, 2); m_SurfelPropagationShader->SetInt("u_SurfelWorldPos", 2);
		m_SurfelCache->BindColorAttachmentByIndex((int)GBuffer::WorldNormal, 3); m_SurfelPropagationShader->SetInt("u_SurfelWorldNormal", 3);
		RenderPipeline::RenderFramebufferIntoFramebuffer(*m_SurfelIlumCacheAcc, *m_SurfelIlumCache, *m_SurfelPropagationShader, glm::vec4(0, 0, 64, 64), "u_Accumulator", 0);

	}
	void Ilum::ApplyIlumination(Framebuffer& gBuffer, CameraNode& camera, World& world, Framebuffer& lightBuffer)
	{
		if (!ENABLE) return;

		if (IsInIlumPass())
		{
			return;
		}
		else
		{
			struct StaticInit
			{
				StaticInit()
				{
					FramebufferSpecification specs;
					specs.Width = 64;
					specs.Height = 64;
					specs.Attachments.Attachments.push_back(FramebufferTextureParams(FramebufferTextureFormat::RGB32F, FramebufferTextureFilter::Linear));
					specs.Attachments.Attachments.push_back(FramebufferTextureFormat::Depth);
					m_Temp = Framebuffer::Create(specs);
				}
				Ref<Framebuffer> m_Temp;
			};
			static StaticInit Buffer = StaticInit();
			if (Buffer.m_Temp->GetSize() != gBuffer.GetSize()) Buffer.m_Temp->Resize(gBuffer.GetSize());

			RenderCommand::SetAlphaBlending(AlphaBlendMode::Disable);
			RenderPipeline::BlitDepthBuffer(gBuffer, *Buffer.m_Temp, *Engine::Get()->GetRenderPipeline()->As<RenderPipeline>()->m_DepthBlitShader);
			m_ApplySurfels->Bind();
			m_ApplySurfels->SetFloat3("u_CameraPos", camera.GetPosition());
			m_SurfelCache->BindColorAttachmentByIndex((int)GBuffer::WorldPosition, 1); m_ApplySurfels->SetInt("u_SurfelWorldPosition", 1);
			m_SurfelIlumCache->BindColorAttachmentByIndex(0, 2); m_ApplySurfels->SetInt("u_LightPropagation", 2);
			gBuffer.BindColorAttachmentByIndex((int)GBuffer::WorldPosition, 3); m_ApplySurfels->SetInt("u_GBufferWorldPosition", 3);
			gBuffer.BindColorAttachmentByIndex((int)GBuffer::BaseColor, 4); m_ApplySurfels->SetInt("u_GBufferBaseColor", 4);

			{
				VertexArray* vao = AssetManager::GetAsset<Mesh>(SuoraID("180284a6-7c63-408c-8078-6ce3b1b50d77"))->GetVertexArray();
				if (!vao) return;

				glm::mat4 viewProj = camera.GetProjectionMatrix() /*Projection*/ * glm::inverse(camera.GetTransformMatrix());
				m_ApplySurfels->SetMat4("u_ViewProjection", viewProj);

				vao->Bind();
				RenderCommand::SetAlphaBlending(AlphaBlendMode::Blend);
				RenderCommand::SetDepthTest(true);
				RenderCommand::DrawInstanced(vao, 4096);
				RenderCommand::SetDepthTest(false);
			}
			m_Denoiser->Bind();
			m_Denoiser->SetFloat2("u_Resolution", lightBuffer.GetSize());
			RenderCommand::SetAlphaBlending(AlphaBlendMode::Additive);
			RenderPipeline::RenderFramebufferIntoFramebuffer(*Buffer.m_Temp, lightBuffer, *m_Denoiser, glm::vec4(0, 0, lightBuffer.GetSize().x, lightBuffer.GetSize().y), "imageData", 0, false);
			RenderCommand::SetAlphaBlending(AlphaBlendMode::Disable);
		}


	}


}

#if 0

namespace Suora
{

	Ilum::Ilum(World* world)
	{
		m_World = world;
		for (int i = 0; i < 64; i++)
		{
			m_LightProbes[i] = LightProbe();
		}
	}
	Ilum::~Ilum()
	{
	}
	void Ilum::Init()
	{
		m_Initialized = true;
		m_PositionReadbackShader = Shader::Create(AssetManager::GetEngineAssetPath() + "/EngineContent/Shaders/Ilum/Ilum_PositionReadback.glsl");
		m_PositionSSBO = ShaderStorageBuffer::Create();
		m_GatherIluminationShader = Shader::Create(AssetManager::GetEngineAssetPath() + "/EngineContent/Shaders/Ilum/Ilum_Gather.glsl");
		{
			FramebufferSpecification specs;
			specs.Width = m_ViewResolution;
			specs.Height = m_ViewResolution;
			for (int32_t i = 0; i < (int32_t)GBuffer::GBufferSlotCount; i++)
			{
				specs.Attachments.Attachments.push_back(RenderPipeline::GBufferSlotToBufferFormat((GBuffer)i));
			}
			specs.Attachments.Attachments.push_back(FramebufferTextureFormat::Depth);
			m_ViewGBuffers = Framebuffer::Create(specs);
		}
		{
			FramebufferSpecification specs;
			specs.Width = m_ViewResolution;
			specs.Height = m_ViewResolution;
			specs.Attachments.Attachments.push_back(FramebufferTextureFormat::RGB32F);
			specs.Attachments.Attachments.push_back(FramebufferTextureFormat::Depth);
			m_ViewTop = Framebuffer::Create(specs);
			m_ViewBottom = Framebuffer::Create(specs);
			m_ViewLeft = Framebuffer::Create(specs);
			m_ViewRight = Framebuffer::Create(specs);
			m_ViewForward = Framebuffer::Create(specs);
			m_ViewBackward = Framebuffer::Create(specs);
		}
		{
			FramebufferSpecification specs;
			specs.Width = m_ProbeSlotResolution * 64;
			specs.Height = m_ProbeSlotResolution * 6;
			specs.Attachments.Attachments.push_back(FramebufferTextureFormat::RGB32F);
			m_IluminationBuffer = Framebuffer::Create(specs);
		}
	}

	void Ilum::Tick(float deltaTime, Framebuffer& gBuffer)
	{
		if (!m_Initialized) Init();

		return;

		s_InIlumPass = true;

		//
		Vec3 position = ReadScreenPosition();
		if (position != Vec::Zero) m_LightProbes[m_lightProbeIndex].TargetPosition = position;

		for (int i = 0; i < 64; i++)
		{
			m_LightProbes[i].Position = Math::Lerp(m_LightProbes[i].Position, m_LightProbes[i].TargetPosition, 0.25f);
		}

		// 1. Spawn new Surfels (in ScreenSpace, etc.)
		Capture(m_LightProbes[m_lightProbeIndex].TargetPosition);

		// 3. GatherFinalIlumination
		GatherFinalIlumination();
		s_InIlumPass = false;
	}

	void Ilum::ApplyIlumination(Ref<Framebuffer> target, Ref<Framebuffer> gBuffer, float intensity)
	{
		/*
		m_ApplyIluminationShader->Bind();

		// Surfel Data
		m_ApplyIluminationShader->SetInt("u_SurfelCount", m_Surfels.size());
		for (int i = 0; i < m_Surfels.size(); i++)
		{
			m_ApplyIluminationShader->SetFloat3("u_SurfelPos[" + std::to_string(i) + "]", m_Surfels[i].View.GetPosition());
			m_ApplyIluminationShader->SetFloat3("u_SurfelNormal[" + std::to_string(i) + "]", m_Surfels[i].View.GetForwardVector());
			m_ApplyIluminationShader->SetInt("u_SurfelSlot[" + std::to_string(i) + "]", m_Surfels[i].CacheSlot);
		}

		m_ApplyIluminationShader->SetInt("u_Iluminance", 2); m_IluminationBuffer->BindColorAttachmentByIndex(0, 2);
		m_ApplyIluminationShader->SetInt("u_WorldNormal", 1); gBuffer->BindColorAttachmentByIndex((int)GBuffer::WorldNormal, 1);
		RenderPipeline::RenderFramebufferIntoFramebuffer(*gBuffer, *target, *m_ApplyIluminationShader, "u_WorldPos", (int)GBuffer::WorldPosition);
		*/
	}


	Ref<Framebuffer> Ilum::GetPositonBufferSrc(const glm::ivec2& size)
	{
		if (m_PositonBufferSrc.find(size) == m_PositonBufferSrc.end())
		{
			FramebufferSpecification spec;
			spec.Width = size.x;
			spec.Height = size.y;
			spec.Attachments.Attachments.push_back(FramebufferTextureFormat::R8);
			m_PositonBufferSrc[size] = Framebuffer::Create(spec);
		}
		return m_PositonBufferSrc[size];
	}

	Vec3 Ilum::ReadScreenPosition()
	{
		Vec3 data[64];
		int i = 1;
		while (i < 64)
		{
			data[i] = (i != m_lightProbeIndex) ? m_LightProbes[i].TargetPosition: m_LightProbes[0].TargetPosition;
			i++;
		}
		m_PositionSSBO->Write(sizeof(data), &data);

		Ref<Framebuffer> TARGET = m_LightProbeScreenSpace ? RenderPipeline::GetGBuffer() : m_ViewGBuffers;

		{

			m_LightProbeScreenSpace = !m_LightProbeScreenSpace;
			TARGET->Bind();
			return TARGET->ReadPixel_RGB32F(glm::ivec2(rand() % TARGET->GetSize().x, rand() % TARGET->GetSize().y), (int)GBuffer::WorldPosition);
		}

		Ref<Framebuffer> BUFFER = GetPositonBufferSrc(TARGET->GetSize());

		m_PositionReadbackShader->Bind();
		m_PositionReadbackShader->SetFloat3("u_ViewPos", m_LightProbeScreenSpace ? RenderPipeline::s_LastCameraPos : m_LightProbes[m_lightProbeIndex].TargetPosition);
		m_PositionReadbackShader->SetInt("u_Seed", rand() % 43879453879);
		RenderPipeline::RenderFramebufferIntoFramebuffer(*TARGET, *BUFFER, *m_PositionReadbackShader, glm::ivec4(0, 0, TARGET->GetSize().x, TARGET->GetSize().y), "u_Texture", (int)GBuffer::WorldPosition, false);


		m_LightProbeScreenSpace = !m_LightProbeScreenSpace;

		m_PositionSSBO->Read(sizeof(data), &data);

		return data[0];
	}

	void Ilum::SpawnSurfelsInScreenSpace(Ref<Framebuffer> gBuffer, int nSurfel)
	{
		/*if (nSurfel > 1) SpawnSurfelsInScreenSpace(gBuffer, nSurfel - 1);

		gBuffer->Bind();

		glm::ivec2 pos = glm::ivec2(rand() % gBuffer->GetSpecification().Width, rand() % gBuffer->GetSpecification().Height);
		glm::vec3 worldPos = gBuffer->ReadPixel_RGB32F(pos, (int)GBuffer::WorldPosition);
		glm::vec3 worldNormal = gBuffer->ReadPixel_RGB32F(pos, (int)GBuffer::WorldNormal);

		m_Surfels.push_back(Surfel()); 
		Surfel& surfel = m_Surfels[m_Surfels.size() - 1];
		surfel.CacheSlot = m_Surfels.size() - 1;
		surfel.View.SetPosition(worldPos);
		surfel.View.SetLookDirection(worldNormal);

		gBuffer->Unbind();*/
	}

	void Ilum::Capture(const Vec3& position)
	{
		CameraNode View;
		View.SetPosition(position);
		View.SetPerspective(90, 0.05f, 750.0f);
		View.SetAspectRatio(1);

		int FaceIndex = rand() % 6;
		int count = 0;
	LOOP:

		if (count < 6)
		{
			count++;
			FaceIndex = FaceIndex + 1 % 6;
			if (FaceIndex == 0) goto TOP;
			if (FaceIndex == 1) goto BOTTOM;
			if (FaceIndex == 2) goto LEFT;
			if (FaceIndex == 3) goto RIGHT;
			if (FaceIndex == 4) goto FORWARD;
			if (FaceIndex == 5) goto BACKWARD;
			else goto TOP;
		}

		return;

	TOP:
		View.SetEulerRotation(Vec3(-90, 0, 0));
		Engine::Get()->GetRenderPipeline()->As<RenderPipeline>()->Render(*m_ViewTop, *m_World, View, *m_ViewGBuffers);
		goto LOOP;
	BOTTOM:
		View.SetEulerRotation(Vec3(90, 0, 0));
		Engine::Get()->GetRenderPipeline()->As<RenderPipeline>()->Render(*m_ViewBottom, *m_World, View, *m_ViewGBuffers);
		goto LOOP;
	LEFT:
		View.SetEulerRotation(Vec3(0, -90, 0));
		Engine::Get()->GetRenderPipeline()->As<RenderPipeline>()->Render(*m_ViewLeft, *m_World, View, *m_ViewGBuffers);
		goto LOOP;
	RIGHT:
		View.SetEulerRotation(Vec3(0, 90, 0));
		Engine::Get()->GetRenderPipeline()->As<RenderPipeline>()->Render(*m_ViewRight, *m_World, View, *m_ViewGBuffers);
		goto LOOP;
	FORWARD:
		View.SetEulerRotation(Vec3(0, 0, 0));
		Engine::Get()->GetRenderPipeline()->As<RenderPipeline>()->Render(*m_ViewForward, *m_World, View, *m_ViewGBuffers);
		goto LOOP;
	BACKWARD:
		View.SetEulerRotation(Vec3(0, 180, 0));
		Engine::Get()->GetRenderPipeline()->As<RenderPipeline>()->Render(*m_ViewBackward, *m_World, View, *m_ViewGBuffers);
		goto LOOP;
	}

	void Ilum::GatherFinalIlumination()
	{
		//const glm::ivec4 Rect = glm::ivec4(m_ProbeSlotResolution * (m_lightProbeIndex % 8), m_ProbeSlotResolution * (m_lightProbeIndex / 8), m_ProbeSlotResolution, m_ProbeSlotResolution);
		
		CameraNode View;
		View.SetPosition(Vec3());
		View.SetPerspective(90, 0.05f, 750.0f);
		View.SetAspectRatio(1);


		m_GatherIluminationShader->Bind();
		m_ViewBottom->BindColorAttachmentByIndex(0, 1); m_GatherIluminationShader->SetInt("u_Bottom", 1);
		m_ViewLeft->BindColorAttachmentByIndex(0, 2); m_GatherIluminationShader->SetInt("u_Left", 2);
		m_ViewRight->BindColorAttachmentByIndex(0, 3); m_GatherIluminationShader->SetInt("u_Right", 3);
		m_ViewForward->BindColorAttachmentByIndex(0, 4); m_GatherIluminationShader->SetInt("u_Forward", 4);
		m_ViewBackward->BindColorAttachmentByIndex(0, 5); m_GatherIluminationShader->SetInt("u_Backward", 5);

		/*View.SetEulerRotation(Vec3(-90, 0, 0));
		View.RecalculateProjection();
		m_GatherIluminationShader->SetMat4("u_TopMatrix", View.GetProjectionMatrix() * glm::inverse(View.GetTransformMatrix()));
		View.SetEulerRotation(Vec3(90, 0, 0));
		View.RecalculateProjection();
		m_GatherIluminationShader->SetMat4("u_BottomMatrix", View.GetProjectionMatrix() * glm::inverse(View.GetTransformMatrix()));
		View.SetEulerRotation(Vec3(0, -90, 0));
		View.RecalculateProjection();
		m_GatherIluminationShader->SetMat4("u_LeftMatrix", View.GetProjectionMatrix() * glm::inverse(View.GetTransformMatrix()));
		View.SetEulerRotation(Vec3(0, 90, 0));
		View.RecalculateProjection();
		m_GatherIluminationShader->SetMat4("u_RightMatrix", View.GetProjectionMatrix() * glm::inverse(View.GetTransformMatrix()));
		View.SetEulerRotation(Vec3(0, 0, 0));
		View.RecalculateProjection();
		m_GatherIluminationShader->SetMat4("u_ForwardMatrix", View.GetProjectionMatrix() * glm::inverse(View.GetTransformMatrix()));
		View.SetEulerRotation(Vec3(0, 180, 0));
		View.RecalculateProjection();
		m_GatherIluminationShader->SetMat4("u_BackwardMatrix", View.GetProjectionMatrix() * glm::inverse(View.GetTransformMatrix()));*/
		
		AssetManager::GetAsset<Texture2D>(SuoraID("70d7e306-6a70-4b28-8740-c3296cf41269"))->GetTexture()->Bind(6);
		m_GatherIluminationShader->SetInt("u_EquirectangularLUT", 6);

		const glm::ivec4 RectBackward = glm::ivec4(m_ProbeSlotResolution * m_lightProbeIndex, m_ProbeSlotResolution * 5, m_ProbeSlotResolution, m_ProbeSlotResolution);
		const glm::ivec4 RectForward = glm::ivec4(m_ProbeSlotResolution * m_lightProbeIndex, m_ProbeSlotResolution * 4, m_ProbeSlotResolution, m_ProbeSlotResolution);
		const glm::ivec4 RectBottom = glm::ivec4(m_ProbeSlotResolution * m_lightProbeIndex, m_ProbeSlotResolution * 3, m_ProbeSlotResolution, m_ProbeSlotResolution);
		const glm::ivec4 RectTop = glm::ivec4(m_ProbeSlotResolution * m_lightProbeIndex, m_ProbeSlotResolution * 2, m_ProbeSlotResolution, m_ProbeSlotResolution);
		const glm::ivec4 RectLeft = glm::ivec4(m_ProbeSlotResolution * m_lightProbeIndex, m_ProbeSlotResolution * 1, m_ProbeSlotResolution, m_ProbeSlotResolution);
		const glm::ivec4 RectRight = glm::ivec4(m_ProbeSlotResolution * m_lightProbeIndex, m_ProbeSlotResolution * 0, m_ProbeSlotResolution, m_ProbeSlotResolution);

		m_GatherIluminationShader->SetInt("u_Face", 0);
		RenderPipeline::RenderFramebufferIntoFramebuffer(*m_ViewTop, *m_IluminationBuffer, *m_GatherIluminationShader, RectBackward, "u_Top", 0, false);
		m_GatherIluminationShader->SetInt("u_Face", 1);
		RenderPipeline::RenderFramebufferIntoFramebuffer(*m_ViewTop, *m_IluminationBuffer, *m_GatherIluminationShader, RectForward, "u_Top", 0, false);
		m_GatherIluminationShader->SetInt("u_Face", 2);
		RenderPipeline::RenderFramebufferIntoFramebuffer(*m_ViewTop, *m_IluminationBuffer, *m_GatherIluminationShader, RectBottom, "u_Top", 0, false);
		m_GatherIluminationShader->SetInt("u_Face", 3);
		RenderPipeline::RenderFramebufferIntoFramebuffer(*m_ViewTop, *m_IluminationBuffer, *m_GatherIluminationShader, RectTop, "u_Top", 0, false);
		m_GatherIluminationShader->SetInt("u_Face", 4);
		RenderPipeline::RenderFramebufferIntoFramebuffer(*m_ViewTop, *m_IluminationBuffer, *m_GatherIluminationShader, RectLeft, "u_Top", 0, false);
		m_GatherIluminationShader->SetInt("u_Face", 5);
		RenderPipeline::RenderFramebufferIntoFramebuffer(*m_ViewTop, *m_IluminationBuffer, *m_GatherIluminationShader, RectRight, "u_Top", 0, false);

		m_lightProbeIndex++;
		m_lightProbeIndex %= 64;

		/*
		m_GatherShader->Bind();
		m_GatherShader->SetInt("u_CacheResolution", m_CacheResolution);
		m_GatherShader->SetInt("u_CacheSlotResolution", m_CacheSlotResolution);
		m_GatherShader->SetInt("u_Resolution", GetIluminationBufferResolution());
		m_GatherShader->SetInt("u_BaseColor", 1); m_SurfelCache->BindColorAttachmentByIndex((int)GBuffer::BaseColor, 1);
		// WorldPos = 3
		m_GatherShader->SetInt("u_DirectLight", 7); m_SceneLightCache->BindColorAttachmentByIndex(0, 7);

		const glm::ivec4 rect = glm::ivec4(0, 0, GetIluminationBufferResolution(), GetIluminationBufferResolution());
		RenderPipeline::RenderFramebufferIntoFramebuffer(*m_SurfelCache, *m_IluminationBuffer, *m_GatherShader, "u_WorldPos", (int)GBuffer::WorldPosition, false, rect);
		*/
	}

	

}
#endif