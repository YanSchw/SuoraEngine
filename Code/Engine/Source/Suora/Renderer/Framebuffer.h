#pragma once

#include "inttypes.h"
#include "Suora/Core/Base.h"
#include <glm/glm.hpp>

namespace Suora 
{

	enum class FramebufferTextureFormat : uint32_t
	{
		None = 0,

		// Color
		RGBA8,
		RGB32F,
		RGB16F,
		RGB16,
		RGB8,
		R8,
		R16F,
		R32F,
		R32I,

		// Depth/stencil
		DEPTH24STENCIL8,
		DEPTH32F_STENCIL8,

		// Defaults
		Depth = DEPTH24STENCIL8
	};

	enum class FramebufferTextureFilter : uint32_t
	{
		None = 0,
		Nearest,
		Linear,
	};

	struct FramebufferTextureParams
	{
		FramebufferTextureParams() = default;
		FramebufferTextureParams(FramebufferTextureFormat format)
			: TextureFormat(format), TextureFilter(FramebufferTextureFilter::Linear) {}
		FramebufferTextureParams(FramebufferTextureFormat format, FramebufferTextureFilter filter)
			: TextureFormat(format), TextureFilter(filter) {}

		FramebufferTextureFormat TextureFormat = FramebufferTextureFormat::None;
		FramebufferTextureFilter TextureFilter = FramebufferTextureFilter::None;
		// TODO: filtering/wrap
	};

	struct FramebufferAttachmentParams
	{
		FramebufferAttachmentParams() = default;
		FramebufferAttachmentParams(std::initializer_list<FramebufferTextureParams> attachments)
			: Attachments(attachments) {}

		std::vector<FramebufferTextureParams> Attachments;
	};

	struct FramebufferSpecification
	{
		uint32_t Width = 0, Height = 0;
		FramebufferAttachmentParams Attachments;
		uint32_t Samples = 1;

		bool SwapChainTarget = false;
	};

	class Framebuffer
	{
		inline static Framebuffer* s_Current = nullptr;
	public:
		virtual ~Framebuffer() = default;

		virtual void Bind()
		{
			s_Current = this;
		}
		virtual void Unbind()
		{
			if (s_Current == this)
				s_Current = nullptr;
		}

		virtual void Resize(const glm::vec2& size) = 0;
		virtual void Resize(uint32_t width, uint32_t height) = 0;
		virtual glm::ivec2 GetSize() const = 0;

		virtual void DrawToScreen(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;

		virtual uint32_t GetColorAttachmentRendererID(uint32_t index = 0) const = 0;
		virtual uint32_t GetDepthAttachmentRendererID() const = 0;
		virtual void BindColorAttachmentByIndex(uint32_t index = 0, uint32_t slot = 0) = 0;
		virtual void BindDepthAttachmentToSlot(uint32_t slot = 0) = 0;

		virtual int32_t ReadPixel_R32I(const glm::ivec2& pos, uint32_t index = 0) = 0;
		virtual glm::vec3 ReadPixel_RGB8(const glm::ivec2& pos, uint32_t index = 0) = 0;
		virtual glm::vec3 ReadPixel_RGB32F(const glm::ivec2& pos, uint32_t index = 0) = 0;

		virtual const FramebufferSpecification& GetSpecification() const = 0;

		static Ref<Framebuffer> Create(const FramebufferSpecification& spec);

		static Framebuffer* GetCurrent() { return s_Current; }
	};


}