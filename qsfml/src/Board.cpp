#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Text.hpp>

#include <spdlog/spdlog.h>

#include "Board.h"
#include "TextFactory.h"
#include "TileMap.h"
#include "Resource.h"



Board::Board(const qcore::BoardMap& boardState) : _boardState(boardState) 
{
}

void Board::drawGridOverlay(sf::RenderTarget& target) const
{
    const int boardBackground[(qcore::BOARD_SIZE * 2 + 1)][(qcore::BOARD_SIZE * 2 + 1)] = {
        14, 15, 14, 16, 14, 17, 14, 18, 14, 19, 14, 20, 14, 21, 14, 22, 14, 23, 14,
        15, 0,  3,  0,  3,  0,  3,  0,  3,  0,  3,  0,  3,  0,  3,  0,  3,  0,  14,
        14, 2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  14,
        16, 0,  3,  0,  3,  0,  3,  0,  3,  0,  3,  0,  3,  0,  3,  0,  3,  0,  14,
        14, 2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  14,
        17, 0,  3,  0,  3,  0,  3,  0,  3,  0,  3,  0,  3,  0,  3,  0,  3,  0,  14,
        14, 2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  14,
        18, 0,  3,  0,  3,  0,  3,  0,  3,  0,  3,  0,  3,  0,  3,  0,  3,  0,  14,
        14, 2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  14,
        19, 0,  3,  0,  3,  0,  3,  0,  3,  0,  3,  0,  3,  0,  3,  0,  3,  0,  14,
        14, 2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  14,
        20, 0,  3,  0,  3,  0,  3,  0,  3,  0,  3,  0,  3,  0,  3,  0,  3,  0,  14,
        14, 2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  14,
        21, 0,  3,  0,  3,  0,  3,  0,  3,  0,  3,  0,  3,  0,  3,  0,  3,  0,  14,
        14, 2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  14,
        22, 0,  3,  0,  3,  0,  3,  0,  3,  0,  3,  0,  3,  0,  3,  0,  3,  0,  14,
        14, 2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  14,
        23, 0,  3,  0,  3,  0,  3,  0,  3,  0,  3,  0,  3,  0,  3,  0,  3,  0,  14,
        14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
    };


    int tiles[(qcore::BOARD_SIZE * 2 + 1)][(qcore::BOARD_SIZE * 2 + 1)] = { 0 };

    std::memcpy(tiles, boardBackground, sizeof(tiles));

    // This numbers represent the tile
    // number from tileset
    const int TILE_VERTICAL_WALL = 9;
    const int TILE_HORIZONTAL_WALL = 8;
    const int TILE_MID_WALL = 7;
    const int TILE_PAWN0 = 10;
    const int TILE_PAWN1 = 11;
    const int TILE_PAWN2 = 12;
    const int TILE_PAWN3 = 13;
    const int TILE_ERROR = 14;

	for (int i = 0; i < qcore::BOARD_MAP_SIZE; ++i)
	{
		for (int j = 0; j < qcore::BOARD_MAP_SIZE; ++j)
		{
			switch (_boardState(i, j))
			{
			case 0:
				break;
			case qcore::BoardMap::VertivalWall:
				tiles[i + 1][j + 1] = TILE_VERTICAL_WALL;
				break;
			case qcore::BoardMap::HorizontalWall:
				tiles[i + 1][j + 1] = TILE_HORIZONTAL_WALL;
				break;
			case qcore::BoardMap::Pawn0:
				tiles[i + 1][j + 1] = TILE_PAWN0;
				break;
			case qcore::BoardMap::Pawn1:
				tiles[i + 1][j + 1] = TILE_PAWN1;
				break;
			case qcore::BoardMap::Pawn2:
				tiles[i + 1][j + 1] = TILE_PAWN2;
				break;
			case qcore::BoardMap::Pawn3:
				tiles[i + 1][j + 1] = TILE_PAWN3;
				break;
			case qcore::BoardMap::MidWall:
                // boardBackground has all the MidWalls
				break;
			default:
				tiles[i + 1][j + 1] = TILE_ERROR;
				break;
			}
		}
	}

    // create the tilemap from the level definition
    TileMap tileMap;
    tileMap.load(Resource::GRAPHICS_PATH + "tileset.png", TILE_SIZE, (const int*)tiles, BOARD_GRID_DIMENSION, BOARD_GRID_DIMENSION);
    tileMap.setPosition(getPosition());


    target.draw(tileMap);

    //for (int i = 0; i < qcore::BOARD_MAP_SIZE; ++i)
    //{
    //    for (int j = 0; j < qcore::BOARD_MAP_SIZE; ++j)
    //    {
    //        switch (_boardState(i, j))
    //        {
    //        case 0:
    //            //_tiles[i+1][j] = _bgTiles[i][j];
    //            break;
    //        case qcore::BoardMap::VertivalWall:
    //        case qcore::BoardMap::HorizontalWall:
    //            colorizeTile(target, boardToGraphicsCoords(sf::Vector2u(i+1, j+1)), sf::Color(128, 0, 0, 64));
    //            break;
    //        case qcore::BoardMap::Pawn0:
    //        case qcore::BoardMap::Pawn1:
    //        case qcore::BoardMap::Pawn2:
    //        case qcore::BoardMap::Pawn3:
    //            colorizeTile(target, boardToGraphicsCoords(sf::Vector2u(i + 1, j + 1)), sf::Color(0, 0, 128, 64));
    //            break;
    //        case qcore::BoardMap::MidWall:
    //            break;
    //        default:
    //            break;
    //        }
    //    }
    //}

    for (const auto& selectedTile : _selectedTiles)
    {
        colorizeTile(target, std::get<0>(selectedTile), std::get<1>(selectedTile));
    }
}

void Board::colorizeTile(sf::RenderTarget& target, sf::Vector2u tile, sf::Color color) const
{
    auto rect = sf::RectangleShape(sf::Vector2f(TILE_SIZE));
    rect.setFillColor(color);

    auto position = getPosition();;
    position.x += tile.x * TILE_SIZE.x;
    position.y += tile.y * TILE_SIZE.y;
    rect.setPosition(position);

    target.draw(rect);
}

void Board::setSelectedTiles(std::list<sf::Vector2u>& tiles)
{
    for (const auto& selectedTile : tiles)
    {
        _selectedTiles.push_back(std::tuple(selectedTile, COLOR_HOVER_TILE));
        if (selectedTile.y != 0 and selectedTile.y != (BOARD_GRID_DIMENSION-1))
        {
            for (auto x = 0; x < 19; x++)
            {
                auto tile = sf::Vector2u(x, selectedTile.y);
                _selectedTiles.push_back(std::tuple(tile, COLOR_GRID_TILE));
            }
        }
        if (selectedTile.x != 0 and selectedTile.x != (BOARD_GRID_DIMENSION - 1))
        {
            for (auto y = 0; y < 19; y++)
            {
                auto tile = sf::Vector2u(selectedTile.x, y);
                _selectedTiles.push_back(std::tuple(tile, COLOR_GRID_TILE));
            }
        }

    }
}

void Board::drawWindow(sf::RenderTarget& window) const
{
    drawGridOverlay(window);
}

void Board::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    drawWindow(target);
}

bool Board::isPawn(sf::Vector2u tileCoord) const
{ 
    return not isBorder(tileCoord) and _boardState.isPawn(tileToBoardCoords(tileCoord));
};

bool Board::isWall(sf::Vector2u tileCoord) const
{
    return not isBorder(tileCoord) and _boardState.isWall(tileToBoardCoords(tileCoord));
};

bool Board::isEmptyWall(sf::Vector2u tileCoord) const
{ 
    return not isBorder(tileCoord) and not isPawn(tileCoord) and not isWall(tileCoord) and not _boardState.isPawnSpace(tileToBoardCoords(tileCoord));
};

bool Board::isPawnSpace(sf::Vector2u tileCoord) const
{
    return not isBorder(tileCoord) and _boardState.isPawnSpace(tileToBoardCoords(tileCoord));
};

bool Board::isBorder(sf::Vector2u tileCoord) const
{
    return tileCoord.x == 0 or tileCoord.y == 0 or tileCoord.x == BOARD_GRID_DIMENSION-1 or tileCoord.y == BOARD_GRID_DIMENSION-1;
}
