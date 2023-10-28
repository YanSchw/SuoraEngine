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
	void PostProcessEffect::Process(const Ref<Framebuffer>& Buffer, Framebuffer& InGBuffer, CameraNode& Camera)
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
			spec.Width = RenderPipeline::GetInternalResolution().x;
			spec.Height = RenderPipeline::GetInternalResolution().y;
			spec.Attachments.Attachments.push_back(FramebufferTextureFormat::RGB32F);
			m_AccumulatedBuffer = Framebuffer::Create(spec);
		}
	}
	void MotionBlur::Process(const Ref<Framebuffer>& Buffer, Framebuffer& InGBuffer, CameraNode& Camera)
	{
		RenderPipeline::SetFullscreenViewport(*RenderPipeline::GetGBuffer());
		if (m_Accumulated)
		{
			m_Shader->Bind();
			m_Shader->SetFloat("u_Intensity", m_Intensity);
			RenderPipeline::RenderFramebufferIntoFramebuffer(*m_AccumulatedBuffer, *Buffer, *m_Shader, glm::ivec4(0, 0, Buffer->GetSize().x, Buffer->GetSize().y), "u_Texture", 0, false);
		}

		RenderPipeline::RenderFramebufferIntoFramebuffer(*Buffer, *m_AccumulatedBuffer, *RenderPipeline::GetFullscreenPassShaderStatic(), glm::ivec4(0, 0, Buffer->GetSize().x, Buffer->GetSize().y));
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
		{
			FramebufferSpecification spec;
			spec.Width = RenderPipeline::GetInternalResolution().x;
			spec.Height = RenderPipeline::GetInternalResolution().y;
			spec.Attachments.Attachments.push_back(FramebufferTextureFormat::RGB32F);
			m_Buffer = Framebuffer::Create(spec);
		}
	}

	void ChromaticAberration::Process(const Ref<Framebuffer>& Buffer, Framebuffer& InGBuffer, CameraNode& Camera)
	{
		m_Shader->Bind();
		m_Shader->SetFloat("u_Intensity", m_Intensity);
		m_Shader->SetFloat2("u_Resolution", Vec2(Buffer->GetSpecification().Width, Buffer->GetSpecification().Height));

		RenderPipeline::RenderFramebufferIntoFramebuffer(*Buffer, *m_Buffer, *m_Shader, glm::ivec4(0, 0, Buffer->GetSize().x, Buffer->GetSize().y));
		RenderPipeline::RenderFramebufferIntoFramebuffer(*m_Buffer, *Buffer, *RenderPipeline::GetFullscreenPassShaderStatic(), glm::ivec4(0, 0, Buffer->GetSize().x, Buffer->GetSize().y));
	}

	void FilmGrain::Init()
	{
		m_Shader = Shader::Create(AssetManager::GetEngineAssetPath() + "/EngineContent/Shaders/PostProccess/FilmGrain.glsl");
		{
			FramebufferSpecification spec;
			spec.Width = RenderPipeline::GetInternalResolution().x;
			spec.Height = RenderPipeline::GetInternalResolution().y;
			spec.Attachments.Attachments.push_back(FramebufferTextureFormat::RGB32F);
			m_Buffer = Framebuffer::Create(spec);
		}
	}
	void FilmGrain::Process(const Ref<Framebuffer>& Buffer, Framebuffer& InGBuffer, CameraNode& Camera)
	{
		m_Shader->Bind();
		m_Shader->SetFloat("u_Intensity", m_Intensity);
		m_Shader->SetFloat("u_Seed", Engine::Get()->GetDeltaTime());
		m_Shader->SetFloat("u_Jitter", m_Jitter);
		m_Shader->SetFloat2("u_Resolution", Vec2(Buffer->GetSpecification().Width, Buffer->GetSpecification().Height));

		RenderPipeline::RenderFramebufferIntoFramebuffer(*Buffer, *m_Buffer, *m_Shader, glm::ivec4(0, 0, Buffer->GetSize().x, Buffer->GetSize().y));
		RenderPipeline::RenderFramebufferIntoFramebuffer(*m_Buffer, *Buffer, *RenderPipeline::GetFullscreenPassShaderStatic(), glm::ivec4(0, 0, Buffer->GetSize().x, Buffer->GetSize().y));
	}

	void FXAA::Init()
	{
		m_Shader = Shader::Create(AssetManager::GetEngineAssetPath() + "/EngineContent/Shaders/PostProccess/FXAA.glsl");
		{
			FramebufferSpecification spec;
			spec.Width = RenderPipeline::GetInternalResolution().x;
			spec.Height = RenderPipeline::GetInternalResolution().y;
			spec.Attachments.Attachments.push_back(FramebufferTextureFormat::RGB32F);
			m_Buffer = Framebuffer::Create(spec);
		}
	}
	void FXAA::Process(const Ref<Framebuffer>& Buffer, Framebuffer& InGBuffer, CameraNode& Camera)
	{
		m_Shader->Bind();

		m_Shader->Bind();
		m_Shader->SetFloat2("u_Resolution", RenderPipeline::GetInternalResolution());
		for (int i = 0; i < m_Samples; i++)
		{
			RenderPipeline::RenderFramebufferIntoFramebuffer(*Buffer, *m_Buffer, *m_Shader, glm::ivec4(0, 0, Buffer->GetSize().x, Buffer->GetSize().y));
			RenderPipeline::RenderFramebufferIntoFramebuffer(*m_Buffer, *Buffer, *RenderPipeline::GetFullscreenPassShaderStatic(), glm::ivec4(0, 0, Buffer->GetSize().x, Buffer->GetSize().y));
		}

	}

	void ToneMapping::Init()
	{
		m_Shader = Shader::Create(AssetManager::GetEngineAssetPath() + "/EngineContent/Shaders/PostProccess/ToneMapping.glsl");
		{
			FramebufferSpecification spec;
			spec.Width = RenderPipeline::GetInternalResolution().x;
			spec.Height = RenderPipeline::GetInternalResolution().y;
			spec.Attachments.Attachments.push_back(FramebufferTextureFormat::RGB32F);
			m_Buffer = Framebuffer::Create(spec);
		}
	}
	void ToneMapping::Process(const Ref<Framebuffer>& Buffer, Framebuffer& InGBuffer, CameraNode& Camera)
	{
		m_Shader->Bind();
		m_Shader->SetInt("u_TonemapFunction", m_TonemapFunction);

		RenderPipeline::RenderFramebufferIntoFramebuffer(*Buffer, *m_Buffer, *m_Shader, glm::ivec4(0, 0, Buffer->GetSize().x, Buffer->GetSize().y));
		RenderPipeline::RenderFramebufferIntoFramebuffer(*m_Buffer, *Buffer, *RenderPipeline::GetFullscreenPassShaderStatic(), glm::ivec4(0, 0, Buffer->GetSize().x, Buffer->GetSize().y));
	}

}