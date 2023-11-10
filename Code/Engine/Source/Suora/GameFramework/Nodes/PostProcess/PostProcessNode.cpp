#include "Precompiled.h"
#include "PostProcessNode.h"
#include "Suora/Renderer/Framebuffer.h"
#include "Suora/Renderer/Shader.h"
#include "Suora/Renderer/RenderCommand.h"
#include "Suora/Renderer/RenderPipeline.h"
#include "Suora/Assets/AssetManager.h"
#include "Suora/GameFramework/Nodes/CameraNode.h"
#include "Suora/Core/Application.h"
#include "Suora/Core/Engine.h"

namespace Suora
{

	PostProcessVolume::PostProcessVolume()
	{
	}
	PostProcessVolume::~PostProcessVolume()
	{
	}
	void PostProcessVolume::Begin()
	{
	}


	PostProcessEffect::PostProcessEffect()
	{
	}
	PostProcessEffect::~PostProcessEffect()
	{
	}
	void PostProcessEffect::Init()
	{
	}
	void PostProcessEffect::Process(const Ref<Framebuffer>& SrcBuffer, const Ref<Framebuffer>& DstBuffer, RenderingParams& Params, CameraNode& Camera)
	{
	}


	MotionBlur::MotionBlur()
	{
	}
	MotionBlur::~MotionBlur()
	{
	}
	void MotionBlur::Init()
	{
		m_Shader = Shader::Create(AssetManager::GetEngineAssetPath() + "/EngineContent/Shaders/PostProccess/MotionBlur.glsl");
		{
			FramebufferSpecification spec;
			spec.Width = 32;
			spec.Height = 32;
			spec.Attachments.Attachments.push_back(FramebufferTextureFormat::RGB32F);
			m_AccumulatedBuffer = Framebuffer::Create(spec);
		}
	}
	void MotionBlur::Process(const Ref<Framebuffer>& SrcBuffer, const Ref<Framebuffer>& DstBuffer, RenderingParams& Params, CameraNode& Camera)
	{
		if (m_AccumulatedBuffer->GetSize() != Params.Resolution)
		{
			m_AccumulatedBuffer->Resize(Params.Resolution);
		}

		RenderPipeline::SetFullscreenViewport(*Params.GetGBuffer());
		if (m_Accumulated)
		{
			m_Shader->Bind();
			m_Shader->SetFloat("u_Intensity", m_Intensity);
			RenderPipeline::RenderFramebufferIntoFramebuffer(*m_AccumulatedBuffer, *DstBuffer, *m_Shader, glm::ivec4(0, 0, DstBuffer->GetSize().x, DstBuffer->GetSize().y), "u_Texture", 0, false);
		}

		RenderPipeline::RenderFramebufferIntoFramebuffer(*SrcBuffer, *m_AccumulatedBuffer, *RenderPipeline::GetFullscreenPassShaderStatic(), glm::ivec4(0, 0, m_AccumulatedBuffer->GetSize().x, m_AccumulatedBuffer->GetSize().y));
		m_Accumulated = true;
	}

	ChromaticAberration::ChromaticAberration()
	{
	}

	ChromaticAberration::~ChromaticAberration()
	{
	}

	void ChromaticAberration::Init()
	{
		m_Shader = Shader::Create(AssetManager::GetEngineAssetPath() + "/EngineContent/Shaders/PostProccess/ChromaticAberration.glsl");
	}

	void ChromaticAberration::Process(const Ref<Framebuffer>& SrcBuffer, const Ref<Framebuffer>& DstBuffer, RenderingParams& Params, CameraNode& Camera)
	{
		m_Shader->Bind();
		m_Shader->SetFloat("u_Intensity", m_Intensity);
		m_Shader->SetFloat2("u_Resolution", Vec2(SrcBuffer->GetSpecification().Width, SrcBuffer->GetSpecification().Height));

		RenderPipeline::RenderFramebufferIntoFramebuffer(*SrcBuffer, *DstBuffer, *m_Shader, glm::ivec4(0, 0, DstBuffer->GetSize().x, DstBuffer->GetSize().y));
	}

	void FilmGrain::Init()
	{
		m_Shader = Shader::Create(AssetManager::GetEngineAssetPath() + "/EngineContent/Shaders/PostProccess/FilmGrain.glsl");
	}
	void FilmGrain::Process(const Ref<Framebuffer>& SrcBuffer, const Ref<Framebuffer>& DstBuffer, RenderingParams& Params, CameraNode& Camera)
	{
		m_Shader->Bind();
		m_Shader->SetFloat("u_Intensity", m_Intensity);
		m_Shader->SetFloat("u_Seed", Engine::Get()->GetDeltaTime() * rand());
		m_Shader->SetFloat("u_Jitter", m_Jitter);
		m_Shader->SetFloat2("u_Resolution", Vec2(SrcBuffer->GetSpecification().Width, SrcBuffer->GetSpecification().Height));

		RenderPipeline::RenderFramebufferIntoFramebuffer(*SrcBuffer, *DstBuffer, *m_Shader, glm::ivec4(0, 0, DstBuffer->GetSize().x, DstBuffer->GetSize().y));
	}

	void FXAA::Init()
	{
		m_Shader = Shader::Create(AssetManager::GetEngineAssetPath() + "/EngineContent/Shaders/PostProccess/FXAA.glsl");
	}
	void FXAA::Process(const Ref<Framebuffer>& SrcBuffer, const Ref<Framebuffer>& DstBuffer, RenderingParams& Params, CameraNode& Camera)
	{
		m_Shader->Bind();

		m_Shader->Bind();
		m_Shader->SetFloat2("u_Resolution", Params.Resolution);
		for (int i = 0; i < m_Samples; i++)
		{
			RenderPipeline::RenderFramebufferIntoFramebuffer(*SrcBuffer, *DstBuffer, *m_Shader, glm::ivec4(0, 0, DstBuffer->GetSize().x, DstBuffer->GetSize().y));
		}

	}

	void ToneMapping::Init()
	{
		m_Shader = Shader::Create(AssetManager::GetEngineAssetPath() + "/EngineContent/Shaders/PostProccess/ToneMapping.glsl");
	}
	void ToneMapping::Process(const Ref<Framebuffer>& SrcBuffer, const Ref<Framebuffer>& DstBuffer, RenderingParams& Params, CameraNode& Camera)
	{
		m_Shader->Bind();
		m_Shader->SetInt("u_TonemapFunction", m_TonemapFunction);

		RenderPipeline::RenderFramebufferIntoFramebuffer(*SrcBuffer, *DstBuffer, *m_Shader, glm::ivec4(0, 0, DstBuffer->GetSize().x, DstBuffer->GetSize().y));
	}


	void Vignette::Init()
	{
		m_Shader = Shader::Create(AssetManager::GetEngineAssetPath() + "/EngineContent/Shaders/PostProccess/Vignette.glsl");
	}
	void Vignette::Process(const Ref<Framebuffer>& SrcBuffer, const Ref<Framebuffer>& DstBuffer, RenderingParams& Params, CameraNode& Camera)
	{
		m_Shader->Bind();
		m_Shader->SetFloat("u_Intensity", m_Intensity);
		m_Shader->SetFloat("u_Extend", m_Extend);
		m_Shader->SetFloat("u_Alpha", m_Alpha);
		m_Shader->SetFloat4("u_VignetteColor", m_VignetteColor);

		RenderPipeline::RenderFramebufferIntoFramebuffer(*SrcBuffer, *DstBuffer, *m_Shader, glm::ivec4(0, 0, DstBuffer->GetSize().x, DstBuffer->GetSize().y));
	}

	void SSAO::Init()
	{
		m_Shader = Shader::Create(AssetManager::GetEngineAssetPath() + "/EngineContent/Shaders/PostProccess/SSAO.glsl");
	}
	void SSAO::Process(const Ref<Framebuffer>& SrcBuffer, const Ref<Framebuffer>& DstBuffer, RenderingParams& Params, CameraNode& Camera)
	{
		m_Shader->Bind();
		m_Shader->SetInt("u_Samples", m_Samples);
		m_Shader->SetFloat("u_Radius", m_Radius);
		m_Shader->SetFloat("u_Intensity", m_Intensity);
		m_Shader->SetFloat("u_Alpha", m_Alpha);
		m_Shader->SetFloat("u_NearGaussBellFactor", m_NearGaussBellFactor);
		m_Shader->SetFloat("u_Near", Camera.GetPerspectiveNearClip());
		m_Shader->SetFloat("u_Far", Camera.GetPerspectiveFarClip());
		m_Shader->SetFloat2("u_Resolution", DstBuffer->GetSize());
		m_Shader->SetFloat4("u_AOColor", m_AOColor);
		m_Shader->SetInt("u_Depth", 6);
		Params.GetGBuffer()->BindDepthAttachmentToSlot(6);

		RenderPipeline::RenderFramebufferIntoFramebuffer(*SrcBuffer, *DstBuffer, *m_Shader, glm::ivec4(0, 0, DstBuffer->GetSize().x, DstBuffer->GetSize().y));
	}

}