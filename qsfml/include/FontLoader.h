#pragma once
#include <map>

#include <SFML/Graphics/Font.hpp>

class FontLoader
{
public:
	enum class FONT {
		DEFAULT,
	};

	static void init();
	static void deinit();
	static const sf::Font& getFont(const FONT font = FontLoader::FONT::DEFAULT);

private:
	static std::map<FONT, sf::Font> *_fontCache;
};

