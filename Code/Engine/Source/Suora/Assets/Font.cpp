#include "Precompiled.h"
#include "Font.h"


namespace Suora
{
    Font::Font()
    {
    }
    Font::Font(const String& ttf)
    {
        m_FontPath = ttf;
    }
    Font::Font(FilePath path, FilePath texturePath)
    {
		LoadFontMetaFromFile(path, texturePath);
    }
	void Font::LoadFontMetaFromFile(FilePath path, FilePath texturePath)
	{
		if (!File::GetFileExtension(DirectoryEntry(path))._Equal(".font")) static_assert("Font file needed!");
		m_FontPath = texturePath.string();
		//Instance = this;

		std::ifstream reader(path.c_str());
		if (reader.is_open())
		{
			while (!reader.eof())
			{
				String str;
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

				String id;			reader >> id;						id = id.substr(3, id.size() - 3);
				String x;			reader >> x;						x = x.substr(2, x.size() - 2);
				String y;			reader >> y;						y = y.substr(2, y.size() - 2);
				String width;		reader >> width;					width = width.substr(6, width.size() - 6);
				String height;		reader >> height;					height = height.substr(7, height.size() - 7);
				String xOffset;	reader >> xOffset;					xOffset = xOffset.substr(8, xOffset.size() - 8);
				String yOffset;	reader >> yOffset;					yOffset = yOffset.substr(8, yOffset.size() - 8);
				String xAdvance;	reader >> xAdvance;					xAdvance = xAdvance.substr(9, xAdvance.size() - 9);

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
    void Font::LoadAtlas()
    {
        m_FontAtlas = Texture::Create(m_FontPath);
        m_IsAtlasLoaded = true;

        // TODO: Using STB_FREETYPE

    }
    void Font::InitializeAsset(Yaml::Node& root)
	{
		Super::InitializeAsset(root);

		String atlasPath = GetSourceAssetPath().string();
		String texturePath = atlasPath;
		StringUtil::ReplaceSequence(texturePath, ".atlas", ".png");

		LoadFontMetaFromFile(atlasPath, texturePath);
		//m_FontPath = m_Path.string().substr(0, m_Path.string().size() - 5).append(".png");

	}

}