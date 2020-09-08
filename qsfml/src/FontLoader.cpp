#include "FontLoader.h"
#include "Resource.h"

std::map<FontLoader::FONT, sf::Font> *FontLoader::_fontCache = nullptr;

void FontLoader::init()
{
	if (FontLoader::_fontCache == nullptr)
	{
		FontLoader::_fontCache = new std::map<FontLoader::FONT, sf::Font>;
	}
}

void FontLoader::deinit()
{
	if (FontLoader::_fontCache)
	{
		delete FontLoader::_fontCache;
	}
}

const sf::Font& FontLoader::getFont(const FontLoader::FONT font)
{
	const std::map<FontLoader::FONT, std::string> _fontPaths = {
		{FontLoader::FONT::DEFAULT, Resource::FONT_PATH + "IBMPlexMono-ExtraLight.ttf"}
	};


	if (FontLoader::_fontCache->find(font) == FontLoader::_fontCache->end())
	{
		sf::Font sfFont;

		if (_fontPaths.find(font) == _fontPaths.end())
		{
			throw "Font has no path";
		}

		if (not sfFont.loadFromFile(_fontPaths.at(font)))
		{
			throw "Font file not found";
		}
		FontLoader::_fontCache->insert(std::make_pair(font, sfFont));
	}

	return FontLoader::_fontCache->operator[](font);
}