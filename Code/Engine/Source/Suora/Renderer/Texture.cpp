#include "Precompiled.h"
#include "Suora/Renderer/Texture.h"
#include "Suora/Assets/AssetManager.h"

#include "Suora/Renderer/RendererAPI.h"
#include "Platform/OpenGL/OpenGLTexture.h"

#include <stb_image.h>

namespace Suora 
{

	TextureBuffer_stbi::TextureBuffer_stbi(const std::string& path)
	{
		stbi_set_flip_vertically_on_load(1);
		{
			m_Data = stbi_load(path.c_str(), &m_Width, &m_Height, &m_Channels, 0);
		}
		SUORA_ASSERT(m_Data, "Failed to load image!");
	}
	TextureBuffer_stbi::~TextureBuffer_stbi()
	{
		stbi_image_free(m_Data);
	}
//===============================================================

	Ref<Texture> Texture::Create(uint32_t width, uint32_t height)
	{
		switch (RendererAPI::GetAPI())
		{
			case RendererAPI::API::None:    SUORA_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case RendererAPI::API::OpenGL:  return CreateRef<OpenGLTexture2D>(width, height);
		}

		SUORA_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	Ref<Texture> Texture::Create(const std::string& path)
	{
		switch (RendererAPI::GetAPI())
		{
			case RendererAPI::API::None:    SUORA_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case RendererAPI::API::OpenGL:  return CreateRef<OpenGLTexture2D>(path);
		}

		SUORA_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	Texture* Texture::CreatePtr(uint32_t width, uint32_t height)
	{
		switch (RendererAPI::GetAPI())
		{
			case RendererAPI::API::None:    SUORA_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case RendererAPI::API::OpenGL:  return new OpenGLTexture2D(width, height);
		}

		SUORA_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	Texture* Texture::CreatePtr(const std::string& path)
	{
		switch (RendererAPI::GetAPI())
		{
			case RendererAPI::API::None:    SUORA_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case RendererAPI::API::OpenGL:  return new OpenGLTexture2D(path);
		}

		SUORA_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	Texture* Texture::CreatePtr(TextureBuffer_stbi& buffer)
	{
		switch (RendererAPI::GetAPI())
		{
			case RendererAPI::API::None:    SUORA_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case RendererAPI::API::OpenGL:  return new OpenGLTexture2D(buffer);
		}

		SUORA_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	static Texture* s_DefaultTexture = nullptr;

	Texture* Texture::GetOrCreateDefaultTexture()
	{
		if (!s_DefaultTexture) s_DefaultTexture = Texture::CreatePtr(AssetManager::GetAssetRootPath() + "/EngineContent/Textures/Checkerboard.png");
		return s_DefaultTexture;
	}

	

}