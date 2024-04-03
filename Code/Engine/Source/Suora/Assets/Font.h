#pragma once
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <string>
#include "StreamableAsset.h"
#include "Suora/Renderer/Texture.h"
#include "Suora/Common/Filesystem.h"
#include "Font.generated.h"

namespace Suora
{

	struct FontMeta
	{
		float x, y;
		float width, height;
		float xOffset, yOffset;
		float xAdvance;
	};

	class Font : public StreamableAsset
	{
		SUORA_CLASS(4736804738);
		ASSET_EXTENSION(".font");

	public:
		inline static Font* Instance = nullptr;
	public:
		Ref<Texture> m_FontAtlas;
		String m_FontPath;
		std::unordered_map<char, FontMeta> m_FontInfo;
		float m_LineHeight = 64;
		float m_FontSize = 12;
		bool m_IsAtlasLoaded = false;

		Font();
		Font(const String& ttf);

		Font(Path path, Path texturePath);

		void LoadFontMetaFromFile(Path path, Path texturePath);

		const FontMeta& GetMeta(char character)
		{
			return (m_FontInfo.find(character) == m_FontInfo.end()) ? m_FontInfo['?'] : m_FontInfo[character];
		}

		float GetStringWidth(const String& str, float fontSize)
		{
			float const scale = fontSize / m_FontSize;
			float width = 0;

			for (char c : str)
			{
				width += GetMeta(c).xAdvance * scale;
			}

			return width;
		}
		float GetCharWidth(const char c, float fontSize)
		{
			float const scale = fontSize / m_FontSize;
			return GetMeta(c).xAdvance * scale;
		}

		void LoadAtlas();

		void PreInitializeAsset(Yaml::Node& root) override
		{
			Super::PreInitializeAsset(root);
		}
		void InitializeAsset(Yaml::Node& root) override;

	};

}