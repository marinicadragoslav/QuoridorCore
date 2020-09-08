#include "ImGUIBoardWidget.h"

#include <SFML/Graphics/Sprite.hpp>
#include <imgui.h>
#include <imgui-SFML.h>


template<typename T>
bool isPointInsideRectangle(
	T x1, T y1,
	T x2, T y2,
	T x, T y)
{
	return (x > x1 and x < x2 and y > y1 and y < y2);
}

bool FindPoint(ImVec2 topLeft, ImVec2 bottomRight, ImVec2 p)
{
	return isPointInsideRectangle<float>(topLeft.x, topLeft.y, bottomRight.x, bottomRight.y, p.x, p.y);
}

ImGUIBoardWidget::ImGUIBoardWidget() : _isManualEnabled(false) 
{
	if (not _texture.create(Board::BOARD_DIMENSION, Board::BOARD_DIMENSION))
	{
		throw "RenderTexture cannot be created";
	}
};

void ImGUIBoardWidget::setBoard(const qcore::BoardMap& board)
{
	_board = board;
}

void ImGUIBoardWidget::ImGuiDrawBoard()
{
	auto board = Board(_board);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::Begin("Board", NULL, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoResize);


	if (_isManualEnabled)
	{
		auto mouse = ImGui::GetMousePos();
		auto contentTopLeft = ImGui::GetCursorScreenPos();

		auto windowPosition = ImGui::GetWindowPos();
		auto windowHeaderHeight = contentTopLeft.y - windowPosition.y;

		auto windowSize = ImGui::GetWindowSize();
		auto contentBottomRight = ImVec2{ windowSize.x + contentTopLeft.x, windowSize.y + contentTopLeft.y - windowHeaderHeight };

		if (FindPoint(contentTopLeft, contentBottomRight, mouse))
		{
			auto relativeMousePosition = ImVec2{ mouse.x - contentTopLeft.x , mouse.y - contentTopLeft.y };
			sf::Vector2u currentlyHoveredTile = board.pixelToTile(sf::Vector2u(relativeMousePosition.x, relativeMousePosition.y));

			ImGui::PopStyleVar();
			if (not ImGuiContextMenuHandle(board, _tileToHover))
			{
				_tileToHover = currentlyHoveredTile;
			}
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

			std::list<sf::Vector2u> list = { _tileToHover };
			board.setSelectedTiles(list);
		}
	}

	_texture.clear();
	_texture.draw(board);
	_texture.display();
	auto tex = sf::Sprite(_texture.getTexture());

	//For some reason ImGUI shows the texture flipped on Y. We flip it here so it shows normally on UI
	tex.setTextureRect(sf::IntRect(0, tex.getTextureRect().width, tex.getTextureRect().width, -tex.getTextureRect().height));
	ImGui::Image(tex);

	ImGui::End();
	ImGui::PopStyleVar();
}


bool ImGUIBoardWidget::ImGuiContextMenuHandle(Board& board, const sf::Vector2u tile)
{
	if (ImGui::BeginPopupContextWindow(NULL, 0) or ImGui::BeginPopupContextWindow(NULL, 1))
	{
		auto boardPosition = board.graphicsToBoardCoords(tile);
		boardPosition.x /= 2;
		boardPosition.y /= 2;

		if (board.isPawn(tile))
		{
			ImGui::Text(std::string("Pawn(" + std::to_string(boardPosition.x) + ", " + std::to_string(boardPosition.y) + ")").c_str());
			ImGui::Separator();
			if (ImGui::MenuItem("Move pawn top"))
			{
				if (_callback)
				{
					_callback(ACTION_ID::MOVE_UP, boardPosition);
				}
			}
			if (ImGui::MenuItem("Move pawn down"))
			{
				if (_callback)
				{
					_callback(ACTION_ID::MOVE_DOWN, boardPosition);
				}
			}
			if (ImGui::MenuItem("Move pawn left"))
			{
				if (_callback)
				{
					_callback(ACTION_ID::MOVE_LEFT, boardPosition);
				}
			}
			if (ImGui::MenuItem("Move pawn right"))
			{
				if (_callback)
				{
					_callback(ACTION_ID::MOVE_RIGHT, boardPosition);
				}
			}
		}
		else if (board.isWall(tile))
		{
			ImGui::Text(std::string("Wall(" + std::to_string(boardPosition.x) + ", " + std::to_string(boardPosition.y) + ")").c_str());
			ImGui::Separator();
			ImGui::Text("No action available");
		}
		else if (board.isEmptyWall(tile))
		{
			ImGui::Text(std::string("Empty(" + std::to_string(boardPosition.x) + ", " + std::to_string(boardPosition.y) + ")").c_str());
			ImGui::Separator();
			if (ImGui::MenuItem("Add Vertical Wall"))
			{
				if (_callback)
				{
					_callback(ACTION_ID::ADD_VERTICAL_WALL, boardPosition);
				}
			}
			if (ImGui::MenuItem("Add Horizontal Wall"))
			{
				if (_callback)
				{
					_callback(ACTION_ID::ADD_HORIZONTAL_WALL, boardPosition);
				}
			}
		}
		else if(board.isPawnSpace(tile))
		{
			ImGui::Text(std::string("Pawn space(" + std::to_string(boardPosition.x) + ", " + std::to_string(boardPosition.y) + ")").c_str());
			ImGui::Separator();
			ImGui::Text("No action available");
		}
		else
		{
			ImGui::Text(std::string("UNKNOWN").c_str());
			ImGui::Separator();
			ImGui::Text("No action available");
		}
		ImGui::EndPopup();
		return true;
	}

	return false;
}
