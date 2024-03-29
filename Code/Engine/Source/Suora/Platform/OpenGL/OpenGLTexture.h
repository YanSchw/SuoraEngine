#pragma once

#include "Suora/Renderer/Texture.h"

#include <glad/glad.h>

namespace Suora 
{

	class OpenGLTexture2D : public Texture
	{
	public:
		OpenGLTexture2D(uint32_t width, uint32_t height);
		OpenGLTexture2D(const String& path);
		OpenGLTexture2D(TextureBuffer_stbi& buffer);
		virtual ~OpenGLTexture2D();

		virtual uint32_t GetWidth() const override { return m_Width;  }
		virtual uint32_t GetHeight() const override { return m_Height; }
		virtual uint32_t GetRendererID() const override { return m_RendererID; }
		
		virtual void SetData(void* data, uint32_t size) override;
		virtual void SetFilter(ETextureFilter filter) override;

		virtual void Bind(uint32_t slot = 0) const override;

		virtual bool operator==(const Texture& other) const override
		{
			return m_RendererID == ((OpenGLTexture2D&)other).m_RendererID;
		}
	private:
		String m_Path;
		uint32_t m_Width, m_Height;
		uint32_t m_RendererID;
		GLenum m_InternalFormat, m_DataFormat;
	};

}
