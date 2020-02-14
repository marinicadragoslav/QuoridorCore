#include "Player.h"
#include "Game.h"
#include "QcoreUtil.h"

namespace qcore
{
   /** Log domain */
   const char * const DOM = "qcore::PL";

   /** Construction */
   Player::Player(uint8_t id, const std::string& name, GamePtr game) :
      mId(id),
      mName(name),
      mGame(game)
   {
      LOG_INFO(DOM) << "Player " << name << " joined the game. Player ID " << (int) id << "\n";
   }

   /** Returns the GameState object */
   BoardStatePtr Player::getBoardState() const
   {
      return mGame->getBoardState();
   }

   /** Returns player's position on the board */
   Position Player::getPosition() const
   {
      return getBoardState()->getPlayers(mId).at(mId).position;
   }

   /**
    * Performs a 'move' action
    * @return true if the move is allowed
    */
   bool Player::move(Direction direction)
   {
      Position position = getPosition() + direction;

      // Skip over other pawn
      if (not mGame->getBoardState()->isSpaceEmpty(position, mId))
      {
         position += direction;
      }

      return move(position);
   }

   /**
    * Performs a 'move' action
    * @return true if the move is allowed
    */
   bool Player::move(uint8_t x, uint8_t y)
   {
      return move(Position(x, y));
   }

   /**
    * Performs a 'move' action
    * @return true if the move is allowed
    */
   bool Player::move(const Position& position)
   {
      PlayerAction action;
      action.actionType = ActionType::Move;
      action.playerId = mId;
      action.position = position;

      return mGame->processPlayerAction(action);
   }

   /**
    * Places a wall at the specified coordinates (starting from the top right corner)
    * @return true if the move is allowed
    */
   bool Player::placeWall(uint8_t x, uint8_t y, Orientation orientation)
   {
      PlayerAction action;
      action.actionType = ActionType::Wall;
      action.wallOrientation = orientation;
      action.playerId = mId;
      action.position.x = x;
      action.position.y = y;

      return mGame->processPlayerAction(action);
   }

   bool Player::placeWall(const BoardState::Wall& wall)
   {
      return placeWall(wall.position.x, wall.position.y, wall.orientation);
   }

} // namespace qcore
