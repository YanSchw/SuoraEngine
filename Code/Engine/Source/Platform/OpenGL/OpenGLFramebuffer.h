#pragma once

#include "Suora/Renderer/Framebuffer.h"

namespace Suora 
{

	class OpenGLFramebuffer : public Framebuffer
	{
	public:
		OpenGLFramebuffer(const FramebufferSpecification& spec);
		virtual ~OpenGLFramebuffer();

		void Invalidate();

		virtual void Bind() override;
		virtual void Unbind() override;

		virtual void Resize(const glm::vec2& size) override;
		virtual void Resize(uint32_t width, uint32_t height) override;
		virtual glm::ivec2 GetSize() const override;

		virtual void DrawToScreen(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;

		virtual uint32_t GetColorAttachmentRendererID(uint32_t index = 0) const override { SUORA_ASSERT(index < m_ColorAttachments.size()); return m_ColorAttachments[index]; }
		virtual uint32_t GetDepthAttachmentRendererID() const override { return m_DepthAttachment; }
		virtual void BindColorAttachmentByIndex(uint32_t index = 0, uint32_t slot = 0) override;
		virtual void BindDepthAttachmentToSlot(uint32_t slot = 0) override;

		virtual int32_t ReadPixel_R32I(const glm::ivec2& pos, uint32_t index = 0) override;
		virtual glm::vec3 ReadPixel_RGB8(const glm::ivec2& pos, uint32_t index = 0) override;
		virtual glm::vec3 ReadPixel_RGB32F(const glm::ivec2& pos, uint32_t index = 0) override;

		virtual const FramebufferSpecification& GetSpecification() const override { return m_Specification; }
	private:
		uint32_t m_RendererID = 0;
		FramebufferSpecification m_Specification;

		std::vector<FramebufferTextureParams> m_ColorAttachmentSpecifications;
		FramebufferTextureParams m_DepthAttachmentSpecification = FramebufferTextureFormat::None;

		std::vector<uint32_t> m_ColorAttachments;
		uint32_t m_DepthAttachment = 0;

		friend class OpenGLFramebuffer;
	};

}