#pragma once

#include <string>

#include "Suora/Core/Base.h"

namespace Suora 
{

	enum class ETextureFilter : uint32_t
	{
		Linear = 0,
		Nearest
	};

	/** For asynchronous Texture loading */
	struct TextureBuffer_stbi
	{
		std::string m_Path;
		int m_Width, m_Height, m_Channels;

		typedef unsigned char stbi_uc;
		stbi_uc* m_Data = nullptr;

		TextureBuffer_stbi(const std::string& path);
		~TextureBuffer_stbi();

	};
//===============================================================

	class Texture
	{
	public:

		static Ref<Texture> Create(uint32_t width, uint32_t height);
		static Ref<Texture> Create(const std::string& path);
		static Texture* CreatePtr(uint32_t width, uint32_t height);
		static Texture* CreatePtr(const std::string& path);
		static Texture* CreatePtr(TextureBuffer_stbi& buffer);
		static Texture* GetOrCreateDefaultTexture();

		virtual ~Texture() = default;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual uint32_t GetRendererID() const = 0;

		virtual void SetData(void* data, uint32_t size) = 0;
		virtual void SetFilter(ETextureFilter filter) = 0;

		virtual void Bind(uint32_t slot = 0) const = 0;

		virtual bool operator==(const Texture& other) const = 0;
	};

	/*class Texture2D : public Texture
	{
	public:
		static Ref<Texture2D> Create(uint32_t width, uint32_t height);
		static Ref<Texture2D> Create(const std::string& path);
	};*/

}