#ifndef BOARD_H
#define BOARD_H

#include <list>
#include <tuple>

#include <SFML/System/NonCopyable.hpp>
#include <SFML/Graphics/Transformable.hpp>
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

#include "BoardState.h"

class Board : public sf::Transformable, public sf::Drawable, public sf::NonCopyable
{
    public:
    Board(const qcore::BoardMap& boardState);

    void draw(sf::RenderTarget& target, sf::RenderStates states) const;
    void setSelectedTiles(std::list<sf::Vector2u>& tiles);

    inline static const int BOARD_DIMENSION = 608;

    inline auto switchCoords(sf::Vector2u coord) const { return sf::Vector2u(coord.y, coord.x); };
    auto graphicsToBoardCoords(sf::Vector2u graphicsCoord) const { return switchCoords(graphicsCoord); };
//    auto boardToGraphicsCoords(sf::Vector2u boardCoord) const { return switchCoords(boardCoord); };
    auto tileToBoardCoords(sf::Vector2u tileCoord) const { return qcore::Position(tileCoord.y - BORDER_WIDTH_TILES, tileCoord.x - BORDER_HEIGH_TILES); };

    bool isPawn(sf::Vector2u tileCoord) const;
    bool isWall(sf::Vector2u tileCoord) const;
    bool isEmptyWall(sf::Vector2u tileCoord) const;
    bool isPawnSpace(sf::Vector2u tileCoord) const;
    bool isBorder(sf::Vector2u tileCoord) const;

    sf::Vector2u pixelToTile(sf::Vector2u pixelCoord) { return sf::Vector2u(pixelCoord.x / TILE_SIZE.x, pixelCoord.y / TILE_SIZE.y); };
private:
    const qcore::BoardMap& _boardState;
    void drawGrid(sf::RenderTarget& window, sf::RectangleShape maxDimension) const;
    void drawWindow(sf::RenderTarget& window) const;
    void drawGridOverlay(sf::RenderTarget& target) const;
    void colorizeTile(sf::RenderTarget& target, sf::Vector2u tile, sf::Color color) const;

    std::list<std::tuple<sf::Vector2u, sf::Color>> _selectedTiles;
    inline static const auto COLOR_GRID_TILE = sf::Color(255, 255, 255, 32);
    inline static const auto COLOR_HOVER_TILE = sf::Color(255, 255, 255, 128);
    inline static const int BOARD_GRID_DIMENSION = 19;
    inline static const int BORDER_WIDTH_TILES = 1;
    inline static const int BORDER_HEIGH_TILES = 1;
    inline static const auto TILE_SIZE = sf::Vector2u(32, 32);
};

#endif //BOARD_H