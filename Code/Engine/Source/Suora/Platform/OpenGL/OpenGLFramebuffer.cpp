#include "Precompiled.h"
#include "Suora/Platform/OpenGL/OpenGLFramebuffer.h"

#include <glad/glad.h>

namespace Suora 
{

	static const uint32_t s_MaxFramebufferSize = 8192;

	namespace Utils
	{

		static GLenum TextureTarget(bool multisampled)
		{
			return multisampled ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
		}

		static void CreateTextures(bool multisampled, uint32_t* outID, uint32_t count)
		{
			glCreateTextures(TextureTarget(multisampled), count, outID);
		}

		static void BindTexture(bool multisampled, uint32_t id)
		{
			glBindTexture(TextureTarget(multisampled), id);
		}

		static void AttachColorTexture(uint32_t id, int samples, GLenum format, FramebufferTextureFilter filter, uint32_t width, uint32_t height, int index)
		{
			SUORA_ASSERT(filter != FramebufferTextureFilter::None);

			bool multisampled = samples > 1;
			if (multisampled)
			{
				glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, format, width, height, GL_FALSE);
			}
			else
			{
				// TODO: GL_Format and GL_Type have to fit to the InternalFormat (!)
				glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, (format == GL_R32I) ? GL_RED_INTEGER : GL_RGBA, (format == GL_R32I) ? GL_INT : GL_UNSIGNED_BYTE, nullptr);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter == FramebufferTextureFilter::Nearest ? GL_NEAREST : GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter == FramebufferTextureFilter::Nearest ? GL_NEAREST : GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			}

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, TextureTarget(multisampled), id, 0);
		}

		static void AttachDepthTexture(uint32_t id, int samples, GLenum format, FramebufferTextureFilter filter, GLenum attachmentType, uint32_t width, uint32_t height)
		{
			SUORA_ASSERT(filter != FramebufferTextureFilter::None);

			bool multisampled = samples > 1;
			if (multisampled)
			{
				glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, format, width, height, GL_FALSE);
			}
			else
			{
				glTexStorage2D(GL_TEXTURE_2D, 1, format, width, height);
				
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter == FramebufferTextureFilter::Nearest ? GL_NEAREST : GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter == FramebufferTextureFilter::Nearest ? GL_NEAREST : GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			}

			glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentType, TextureTarget(multisampled), id, 0);
		}

		static bool IsDepthFormat(FramebufferTextureFormat format)
		{
			switch (format)
			{
			case FramebufferTextureFormat::DEPTH24STENCIL8:  return true;
			case FramebufferTextureFormat::DEPTH32F_STENCIL8:  return true;
			}

			return false;
		}

	}

	OpenGLFramebuffer::OpenGLFramebuffer(const FramebufferSpecification& spec)
		: m_Specification(spec)
	{
		for (auto spec : m_Specification.Attachments.Attachments)
		{
			if (!Utils::IsDepthFormat(spec.TextureFormat))
				m_ColorAttachmentSpecifications.emplace_back(spec);
			else
				m_DepthAttachmentSpecification = spec;
		}

		Invalidate();
	}

	OpenGLFramebuffer::~OpenGLFramebuffer()
	{
		glDeleteFramebuffers(1, &m_RendererID);
		glDeleteTextures(m_ColorAttachments.size(), m_ColorAttachments.data());
		glDeleteTextures(1, &m_DepthAttachment);
	}

	void OpenGLFramebuffer::Invalidate()
	{
		if (m_RendererID)
		{
			glDeleteFramebuffers(1, &m_RendererID);
			glDeleteTextures(m_ColorAttachments.size(), m_ColorAttachments.data());
			glDeleteTextures(1, &m_DepthAttachment);

			m_ColorAttachments.clear();
			m_DepthAttachment = 0;
		}

		glCreateFramebuffers(1, &m_RendererID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);

		bool multisample = m_Specification.Samples > 1;

		// Attachments
		if (m_ColorAttachmentSpecifications.size())
		{
			m_ColorAttachments.resize(m_ColorAttachmentSpecifications.size());
			Utils::CreateTextures(multisample, m_ColorAttachments.data(), m_ColorAttachments.size());

			for (size_t i = 0; i < m_ColorAttachments.size(); i++)
			{
				Utils::BindTexture(multisample, m_ColorAttachments[i]);
				switch (m_ColorAttachmentSpecifications[i].TextureFormat)
				{
				case FramebufferTextureFormat::RGBA8:
					Utils::AttachColorTexture(m_ColorAttachments[i], m_Specification.Samples, GL_RGBA8, m_ColorAttachmentSpecifications[i].TextureFilter, m_Specification.Width, m_Specification.Height, i);
					break;
				case FramebufferTextureFormat::RGB32F:
					Utils::AttachColorTexture(m_ColorAttachments[i], m_Specification.Samples, GL_RGB32F, m_ColorAttachmentSpecifications[i].TextureFilter, m_Specification.Width, m_Specification.Height, i);
					break;
				case FramebufferTextureFormat::RGB16F:
					Utils::AttachColorTexture(m_ColorAttachments[i], m_Specification.Samples, GL_RGB16F, m_ColorAttachmentSpecifications[i].TextureFilter, m_Specification.Width, m_Specification.Height, i);
					break;
				case FramebufferTextureFormat::RGB16:
					Utils::AttachColorTexture(m_ColorAttachments[i], m_Specification.Samples, GL_RGB16, m_ColorAttachmentSpecifications[i].TextureFilter, m_Specification.Width, m_Specification.Height, i);
					break;
				case FramebufferTextureFormat::RGB8:
					Utils::AttachColorTexture(m_ColorAttachments[i], m_Specification.Samples, GL_RGB8, m_ColorAttachmentSpecifications[i].TextureFilter, m_Specification.Width, m_Specification.Height, i);
					break;
				case FramebufferTextureFormat::R8:
					Utils::AttachColorTexture(m_ColorAttachments[i], m_Specification.Samples, GL_R8, m_ColorAttachmentSpecifications[i].TextureFilter, m_Specification.Width, m_Specification.Height, i);
					break;
				case FramebufferTextureFormat::R16F:
					Utils::AttachColorTexture(m_ColorAttachments[i], m_Specification.Samples, GL_R16F, m_ColorAttachmentSpecifications[i].TextureFilter, m_Specification.Width, m_Specification.Height, i);
					break;
				case FramebufferTextureFormat::R32F:
					Utils::AttachColorTexture(m_ColorAttachments[i], m_Specification.Samples, GL_R32F, m_ColorAttachmentSpecifications[i].TextureFilter, m_Specification.Width, m_Specification.Height, i);
					break;
				case FramebufferTextureFormat::R32I:
					Utils::AttachColorTexture(m_ColorAttachments[i], m_Specification.Samples, GL_R32I, m_ColorAttachmentSpecifications[i].TextureFilter, m_Specification.Width, m_Specification.Height, i);
					break;
				default:
					SUORA_ERROR(LogCategory::Rendering, "FramebufferTextureFormat implementation missing!");
					break;
				}
			}
		}

		if (m_DepthAttachmentSpecification.TextureFormat != FramebufferTextureFormat::None)
		{
			Utils::CreateTextures(multisample, &m_DepthAttachment, 1);
			Utils::BindTexture(multisample, m_DepthAttachment);
			switch (m_DepthAttachmentSpecification.TextureFormat)
			{
			case FramebufferTextureFormat::DEPTH24STENCIL8:
				Utils::AttachDepthTexture(m_DepthAttachment, m_Specification.Samples, GL_DEPTH24_STENCIL8, m_DepthAttachmentSpecification.TextureFilter, GL_DEPTH_STENCIL_ATTACHMENT, m_Specification.Width, m_Specification.Height);
				break;
			case FramebufferTextureFormat::DEPTH32F_STENCIL8:
				Utils::AttachDepthTexture(m_DepthAttachment, m_Specification.Samples, GL_DEPTH32F_STENCIL8, m_DepthAttachmentSpecification.TextureFilter, GL_DEPTH_STENCIL_ATTACHMENT, m_Specification.Width, m_Specification.Height);
				break;
			default:
				SUORA_ERROR(LogCategory::Rendering, "FramebufferTextureFormat implementation missing!");
				break;
			}
		}

		if (m_ColorAttachments.size() > 1)
		{
			SUORA_ASSERT(m_ColorAttachments.size() <= 8);
			GLenum buffers[8] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5, GL_COLOR_ATTACHMENT6, GL_COLOR_ATTACHMENT7 };
			glDrawBuffers(m_ColorAttachments.size(), buffers);
		}
		else if (m_ColorAttachments.empty())
		{
			// Only depth-pass
			glDrawBuffer(GL_NONE);
		}

		SUORA_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is incomplete!");

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void OpenGLFramebuffer::Bind()
	{
		Framebuffer::Bind();
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
		glViewport(0, 0, m_Specification.Width, m_Specification.Height);
	}

	void OpenGLFramebuffer::Unbind()
	{
		Framebuffer::Unbind();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void OpenGLFramebuffer::Resize(const Vec2& size)
	{
		Resize(size.x, size.y);
	}
	void OpenGLFramebuffer::Resize(uint32_t width, uint32_t height)
	{
		if (width == m_Specification.Width && height == m_Specification.Height) return;
		if (width == 0 || height == 0 || width > s_MaxFramebufferSize || height > s_MaxFramebufferSize)
		{
			SUORA_WARN(LogCategory::Rendering, "Attempted to rezize framebuffer to {0}, {1}", width, height);
			return;
		}
		m_Specification.Width = width;
		m_Specification.Height = height;
		
		Invalidate();
	}

	glm::ivec2 OpenGLFramebuffer::GetSize() const
	{
		return glm::ivec2(m_Specification.Width, m_Specification.Height);
	}

	void OpenGLFramebuffer::DrawToScreen(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
		glBindFramebuffer(GL_READ_FRAMEBUFFER, m_RendererID);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glBlitFramebuffer(0, 0, m_Specification.Width, m_Specification.Height, x, y, x + width, y + height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
	}

	void OpenGLFramebuffer::BindColorAttachmentByIndex(uint32_t index, uint32_t slot)
	{
		glBindTextureUnit(slot, GetColorAttachmentRendererID(index));
	}

	void OpenGLFramebuffer::BindDepthAttachmentToSlot(uint32_t slot)
	{
		glBindTextureUnit(slot, GetDepthAttachmentRendererID());
	}

	int32_t OpenGLFramebuffer::ReadPixel_R32I(const glm::ivec2& pos, uint32_t index)
	{
		int32_t out = -1;
		glReadBuffer(GL_COLOR_ATTACHMENT0 + index);
		glReadPixels(pos.x, pos.y, 1, 1, GL_RED_INTEGER, GL_INT, &out);
		return out;
	}

	Vec3 OpenGLFramebuffer::ReadPixel_RGB8(const glm::ivec2& pos, uint32_t index)
	{
		Vec3 out = Vec3();
		glReadBuffer(GL_COLOR_ATTACHMENT0 + index);
		glReadPixels(pos.x, pos.y, 1, 1, GL_RGB, GL_FLOAT, &out);
		return out;
	}
	Vec3 OpenGLFramebuffer::ReadPixel_RGB32F(const glm::ivec2& pos, uint32_t index)
	{
		Vec3 out = Vec3();
		BindColorAttachmentByIndex(index, index);
		glReadBuffer(GL_COLOR_ATTACHMENT0 + index);
		glReadPixels(pos.x, pos.y, 1, 1, GL_RGB, GL_FLOAT, &out);
		return out;
	}

}
