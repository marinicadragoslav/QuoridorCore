#pragma once

#include <SFML/Graphics/Text.hpp>

class TextFactory {
public:
	static sf::Text BigText( const std::string text);
	static sf::Text Text(const std::string text);

	inline static const int BIG_TEXT_SIZE = 65;
	inline static const int NORMAL_TEXT_SIZE = 25;
};