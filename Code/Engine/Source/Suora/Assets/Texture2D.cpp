#include "Precompiled.h"
#include "Texture2D.h"
#include "Suora/Renderer/Texture.h"
#include "Suora/Core/Threading.h"
#include "Suora/Assets/AssetManager.h"
#include "Suora/Common/Common.h"

namespace Suora
{

	Texture2D::Texture2D()
	{
		m_TextureFilter = ETextureFilter::Linear;
	}
	Texture2D::~Texture2D()
	{
		if (m_AsyncTextureBuffer.get())
		{
			// TextureBuffer_stbi has to be destroyed here (!)
			// We also wait for the async load task to finish
			m_AsyncTextureBuffer->get();
		}
		if (m_Texture)
		{
			delete m_Texture;
		}
	}

	void Texture2D::PreInitializeAsset(const std::string& str)
	{
		Super::PreInitializeAsset(str);

		Yaml::Node root;
		Yaml::Parse(root, str);
		m_UUID = root["UUID"].As<std::string>();

	}

	uint32_t Texture2D::GetAssetFileSize()
	{
		uint32_t baseSize = Super::GetAssetFileSize();
		uint32_t textureSize = 0;

		if (IsSourceAssetPathValid())
		{
			textureSize = std::filesystem::file_size(GetSourceAssetPath());
		}

		return baseSize + textureSize;
	}

	void Texture2D::Serialize(Yaml::Node& root)
	{
		Super::Serialize(root);

	}

	Texture* Texture2D::GetTexture()
	{
		if (IsMissing() || !IsSourceAssetPathValid())
		{
			return Texture::GetOrCreateDefaultTexture();
		}

		if (!IsLoaded())
		{
			if (!m_AsyncTextureBuffer.get() && AssetManager::s_AssetStreamPool.Size() < ASSET_STREAM_COUNT_LIMIT)
			{
				AssetManager::s_AssetStreamPool.Add(this);
				
				m_AsyncTextureBuffer = CreateRef<std::future<Ref<TextureBuffer_stbi>>>(std::async(std::launch::async, &Texture2D::Async_LoadTexture, this, GetSourceAssetPath().string()));
			}
			else if (m_AsyncTextureBuffer.get() && IsFutureReady(*m_AsyncTextureBuffer.get()))
			{
				AssetManager::s_AssetStreamPool.Remove(this);
				Ref<TextureBuffer_stbi> buffer = m_AsyncTextureBuffer->get();
				m_AsyncTextureBuffer = nullptr;
				m_Texture = Texture::CreatePtr(*buffer.get());
				m_Texture->SetFilter(m_TextureFilter);
			}

			return Texture::GetOrCreateDefaultTexture();
		}

		return m_Texture;
	}

	Ref<TextureBuffer_stbi> Texture2D::Async_LoadTexture(const std::string& path)
	{
		return CreateRef<TextureBuffer_stbi>(path);
	}

}