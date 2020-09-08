#pragma once

#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/System/Vector2.hpp>

#include "Board.h"

class ImGUIBoardWidget
{
    public:
        ImGUIBoardWidget();
        void ImGuiDrawBoard();
        void setBoard(const qcore::BoardMap& board);

        void setManual(bool manual) { _isManualEnabled = manual; };

        enum class ACTION_ID
        {
            MOVE_UP,
            MOVE_DOWN,
            MOVE_LEFT,
            MOVE_RIGHT,
            ADD_HORIZONTAL_WALL,
            ADD_VERTICAL_WALL,
        };

        void setCallback(std::function<void(ACTION_ID, sf::Vector2u)> cb) { _callback = cb; };

    private:
        qcore::BoardMap _board;
        bool _isManualEnabled;

        sf::RenderTexture _texture;
        sf::Vector2u _tileToHover;


        bool ImGuiContextMenuHandle(Board& board, const sf::Vector2u tile);

        std::function<void(ACTION_ID, sf::Vector2u)> _callback;
};
