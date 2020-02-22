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
      LOG_INFO(DOM) << "Player " << name << " joined the game. Player ID " << (int) id;
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

   /** Returns number of walls left for the current player */
   uint8_t Player::getWallsLeft() const
   {
      return getBoardState()->getPlayers(mId).at(mId).wallsLeft;
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
      std::string error;
      PlayerAction action;
      action.actionType = ActionType::Move;
      action.playerId = mId;
      action.playerPosition = position;

      return mGame->processPlayerAction(action, error);
   }

   /**
    * Places a wall at the specified coordinates (starting from the top right corner)
    * @return true if the move is allowed
    */
   bool Player::placeWall(uint8_t x, uint8_t y, Orientation orientation)
   {
      return placeWall(WallState{ { x, y }, orientation });
   }

   bool Player::placeWall(const WallState& wall)
   {
      std::string error;
      PlayerAction action;
      action.actionType = ActionType::Wall;
      action.playerId = mId;
      action.wallState = wall;

      return mGame->processPlayerAction(action, error);
   }

} // namespace qcore
