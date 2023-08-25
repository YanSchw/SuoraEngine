#pragma once
#include "Asset.h"
#include <vector>
#include <string>
#include <future>
#include "Texture2D.generated.h"

namespace Suora
{
	enum class ETextureFilter : uint32_t;
	struct TextureBuffer_stbi;
	class Texture;

	enum class ETexture2DFileFormat
	{
		None = 0,
		PNG,
		JPG
	};

	class Texture2D : public Asset
	{
		SUORA_CLASS(7546894832);
		ASSET_EXTENSION(".texture");

	public:
		Texture2D();
		~Texture2D();

		void PreInitializeAsset(const std::string& str) override;
		virtual uint32_t GetAssetFileSize() override;

		void Serialize(Yaml::Node& root) override;

		bool IsLoaded() const override
		{
			return m_Texture;
		}

		std::string GetTexturePath();
		Texture* GetTexture();
		Ref<TextureBuffer_stbi> Async_LoadTexture(const std::string& path);

		Ref<std::future<Ref<TextureBuffer_stbi>>> m_AsyncTextureBuffer;

		ETextureFilter m_TextureFilter;

	private:
		ETexture2DFileFormat m_Texture2DFileFormat = ETexture2DFileFormat::PNG;
		Texture* m_Texture = nullptr;
		inline static Texture2D* Default = nullptr;
	};
}