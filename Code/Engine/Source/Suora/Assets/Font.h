#pragma once
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <string>
#include "Asset.h"
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

	class Font : public Asset
	{
		SUORA_CLASS(4736804738);
		ASSET_EXTENSION(".font");

	public:
		inline static Font* Instance = nullptr;
	public:
		Ref<Texture> m_FontAtlas;
		std::string m_FontPath;
		std::unordered_map<char, FontMeta> m_FontInfo;
		float m_LineHeight = 64;
		float m_FontSize = 12;
		bool m_IsAtlasLoaded = false;

		Font()
		{
			//Instance = this;
		}

		Font(const std::string& ttf)
		{
			m_FontPath = ttf;
			//Instance = this;
		}

		Font(FilePath path, const std::string& texture)
		{
			if (!File::GetFileExtension(DirectoryEntry(path))._Equal(".font")) static_assert("Font file needed!");
			m_FontPath = texture;
			//Instance = this;

			std::ifstream reader(path.c_str());
			if (reader.is_open())
			{
				while (!reader.eof())
				{
					std::string str;
					reader >> str;

					if (str.substr(0, 11)._Equal("lineHeight="))
					{
						str = str.substr(11, str.size() - 11);
						m_LineHeight = std::stof(str);
						continue;
					}
					if (str.substr(0, 5)._Equal("size="))
					{
						str = str.substr(5, str.size() - 5);
						m_FontSize = std::stof(str);
						continue;
					}

					if (!str._Equal("char")) continue;

					std::string id;			reader >> id;						id = id.substr(3, id.size() - 3);
					std::string x;			reader >> x;						x = x.substr(2, x.size() - 2);
					std::string y;			reader >> y;						y = y.substr(2, y.size() - 2);
					std::string width;		reader >> width;					width = width.substr(6, width.size() - 6);
					std::string height;		reader >> height;					height = height.substr(7, height.size() - 7);
					std::string xOffset;	reader >> xOffset;					xOffset = xOffset.substr(8, xOffset.size() - 8);
					std::string yOffset;	reader >> yOffset;					yOffset = yOffset.substr(8, yOffset.size() - 8);
					std::string xAdvance;	reader >> xAdvance;					xAdvance = xAdvance.substr(9, xAdvance.size() - 9);

					char character = (char)std::stoi(id);
					m_FontInfo[character].x = std::stof(x);
					m_FontInfo[character].y = std::stof(y);
					m_FontInfo[character].width = std::stof(width);
					m_FontInfo[character].height = std::stof(height);
					m_FontInfo[character].xOffset = std::stof(xOffset);
					m_FontInfo[character].yOffset = std::stof(yOffset);
					m_FontInfo[character].xAdvance = std::stof(xAdvance);

				}
			}
			reader.close();
		}

		void LoadFontMetaFromFile(FilePath path)
		{
			if (!File::GetFileExtension(DirectoryEntry(path))._Equal(".font")) static_assert("Font file needed!");
			//Instance = this;

			std::ifstream reader(path.c_str());
			if (reader.is_open())
			{
				while (!reader.eof())
				{
					std::string str;
					reader >> str;

					if (str.substr(0, 11)._Equal("lineHeight="))
					{
						str = str.substr(11, str.size() - 11);
						m_LineHeight = std::stof(str);
						continue;
					}
					if (str.substr(0, 5)._Equal("size="))
					{
						str = str.substr(5, str.size() - 5);
						m_FontSize = std::stof(str);
						continue;
					}

					if (!str._Equal("char")) continue;

					std::string id;			reader >> id;						id = id.substr(3, id.size() - 3);
					std::string x;			reader >> x;						x = x.substr(2, x.size() - 2);
					std::string y;			reader >> y;						y = y.substr(2, y.size() - 2);
					std::string width;		reader >> width;					width = width.substr(6, width.size() - 6);
					std::string height;		reader >> height;					height = height.substr(7, height.size() - 7);
					std::string xOffset;	reader >> xOffset;					xOffset = xOffset.substr(8, xOffset.size() - 8);
					std::string yOffset;	reader >> yOffset;					yOffset = yOffset.substr(8, yOffset.size() - 8);
					std::string xAdvance;	reader >> xAdvance;					xAdvance = xAdvance.substr(9, xAdvance.size() - 9);

					char character = (char)std::stoi(id);
					m_FontInfo[character].x = std::stof(x);
					m_FontInfo[character].y = std::stof(y);
					m_FontInfo[character].width = std::stof(width);
					m_FontInfo[character].height = std::stof(height);
					m_FontInfo[character].xOffset = std::stof(xOffset);
					m_FontInfo[character].yOffset = std::stof(yOffset);
					m_FontInfo[character].xAdvance = std::stof(xAdvance);

				}
			}
			reader.close();
		}

		const FontMeta& GetMeta(char character)
		{
			return (m_FontInfo.find(character) == m_FontInfo.end()) ? m_FontInfo['?'] : m_FontInfo[character];
		}

		float GetStringWidth(const std::string& str, float fontSize)
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

		void LoadAtlas()
		{
			m_FontAtlas = Texture::Create(m_FontPath);
			m_IsAtlasLoaded = true;

			// Using STB_FREETYPE
		}

		void PreInitializeAsset(const std::string& str) override
		{
			m_UUID = SuoraID::Generate();
		}
		void InitializeAsset(const std::string& str) override
		{
			LoadFontMetaFromFile(m_Path);
			m_FontPath = m_Path.string().substr(0, m_Path.string().size() - 5).append(".png");
		}

	};

}