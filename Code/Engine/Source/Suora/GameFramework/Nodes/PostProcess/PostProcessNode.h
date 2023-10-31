#pragma once
#include <vector>
#include "Suora/GameFramework/Node.h"
#include "Suora/Common/Array.h"
#include "PostProcessNode.generated.h"

namespace Suora
{
	class RenderPipeline;
	class Framebuffer;
	class Shader;
	class CameraNode;

	class PostProcessVolume : public Node3D
	{
		SUORA_CLASS(47363763);

	public:
		PostProcessVolume();
		~PostProcessVolume();
		void Begin() override;
	};

	class PostProcessEffect : public Node
	{
		SUORA_CLASS(6784233);

	public:
		PostProcessEffect();
		~PostProcessEffect();

		virtual void Init();
		virtual void Process(const Ref<Framebuffer>& SrcBuffer, const Ref<Framebuffer>& DstBuffer, Framebuffer& InGBuffer, CameraNode& Camera);
	private:
		bool m_Initialized = false;
	protected:
		friend class RenderPipeline;
	};

	class MotionBlur : public PostProcessEffect
	{
		SUORA_CLASS(7643233);

	public:
		MotionBlur();
		~MotionBlur();

		MEMBER()
		float m_Intensity = 0.125f;

		virtual void Init() override;
		virtual void Process(const Ref<Framebuffer>& SrcBuffer, const Ref<Framebuffer>& DstBuffer, Framebuffer& InGBuffer, CameraNode& Camera) override;
	private:
		Ref<Shader> m_Shader;
		Ref<Framebuffer> m_AccumulatedBuffer;
		bool m_Accumulated = false;

	};

	class ChromaticAberration : public PostProcessEffect
	{
		SUORA_CLASS(78694434);

	public:
		ChromaticAberration();
		~ChromaticAberration();

		MEMBER()
			float m_Intensity = 0.04f;

		virtual void Init() override;
		virtual void Process(const Ref<Framebuffer>& SrcBuffer, const Ref<Framebuffer>& DstBuffer, Framebuffer& InGBuffer, CameraNode& Camera) override;
	private:
		Ref<Shader> m_Shader;

	};
	class FilmGrain : public PostProcessEffect
	{
		SUORA_CLASS(47368441);

	public:
		MEMBER() float m_Intensity = 0.4f;
		MEMBER() float m_Jitter = 1.65f;

		virtual void Init() override;
		virtual void Process(const Ref<Framebuffer>& SrcBuffer, const Ref<Framebuffer>& DstBuffer, Framebuffer& InGBuffer, CameraNode& Camera) override;
	private:
		Ref<Shader> m_Shader;

	};

	class FXAA : public PostProcessEffect
	{
		SUORA_CLASS(76458333);

	public:

		MEMBER() float m_Samples = 1; // TODO: should be int

		virtual void Init() override;
		virtual void Process(const Ref<Framebuffer>& SrcBuffer, const Ref<Framebuffer>& DstBuffer, Framebuffer& InGBuffer, CameraNode& Camera) override;
	private:
		Ref<Shader> m_Shader;

	};
	class ToneMapping : public PostProcessEffect
	{
		SUORA_CLASS(42387911);

	public:

		virtual void Init() override;
		virtual void Process(const Ref<Framebuffer>& SrcBuffer, const Ref<Framebuffer>& DstBuffer, Framebuffer& InGBuffer, CameraNode& Camera) override;

		MEMBER()
		float m_TonemapFunction = 3; /// TODO: Should be int

	private:
		Ref<Shader> m_Shader;

	};

	class Vignette : public PostProcessEffect
	{
		SUORA_CLASS(47863233);

	public:
		MEMBER() float m_Intensity = 40.0f;
		MEMBER() float m_Extend = 0.15f;
		MEMBER() Color m_VignetteColor = Color(0, 0, 0, 1);

		virtual void Init() override;
		virtual void Process(const Ref<Framebuffer>& SrcBuffer, const Ref<Framebuffer>& DstBuffer, Framebuffer& InGBuffer, CameraNode& Camera) override;
	private:
		Ref<Shader> m_Shader;

	};

}