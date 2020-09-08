#include "TextFactory.h"

#include "FontLoader.h"

sf::Text TextFactory::BigText(const std::string text) {
	sf::Text sfText(text, FontLoader::getFont(), TextFactory::BIG_TEXT_SIZE);

	sfText.setFillColor(sf::Color::White);

	return sfText;
}

sf::Text TextFactory::Text(const std::string text) {
	sf::Text sfText(text, FontLoader::getFont(), TextFactory::NORMAL_TEXT_SIZE);

	sfText.setFillColor(sf::Color::White);

	return sfText;
}