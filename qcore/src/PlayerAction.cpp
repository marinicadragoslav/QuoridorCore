#include "PlayerAction.h"
#include "QcoreUtil.h"

#include <cmath>

namespace qcore
{
   /** Rotates all coordinates counterclockwise for a number of steps */
   Direction rotate(const Direction d, const uint8_t rotations)
   {
      return static_cast<Direction>((4 - (rotations & 3) + static_cast<int>(d)) & 3);
   }

   bool Position::operator==(const Position& p) const
   {
      return x == p.x and y == p.y;
   }

   /** Computes the distance between 2 positions */
   uint8_t Position::dist(const Position& p) const
   {
      return std::abs(x - p.x) + std::abs(y - p.y);
   }

   /** Multiplies each coordinate with the given factor */
   Position Position::operator*(uint8_t m) const
   {
      return Position(uint8_t(x * m), uint8_t(y * m));
   }

   Position Position::operator/(uint8_t m) const
   {
      return Position(uint8_t(x / m), uint8_t(y / m));
   }

   /** Moves the coordinates in the specified direction */
   Position& Position::operator+=(const Direction direction)
   {
      switch (direction) {
         case Direction::Up:
            --x;
            break;
         case Direction::Down:
            ++x;
            break;
         case Direction::Left:
            --y;
            break;
         case Direction::Right:
            ++y;
            break;
         default:
            break;
      }

      return *this;
   }

   /** Moves the coordinates in the specified direction */
   Position Position::operator+(const Direction direction) const
   {
      Position p = *this;
      p += direction;
      return p;
   }

   Position Position::operator+(const Position& p) const
   {
      return Position(x + p.x, y + p.y);
   }

   Position Position::operator-(const Position& p) const
   {
      return Position(x - p.x, y - p.y);
   }

   namespace literals
   {
      Position operator "" _x(unsigned long long int x)
      {
         return Position(x, 0);
      }

      Position operator "" _y(unsigned long long int y)
      {
         return Position(0, y);
      }
   }

   /** Rotates all coordinates counterclockwise for a number of steps */
   Position Position::rotate(const uint8_t rotations) const
   {
      Position p;

      switch (rotations & 3)
      {
         case 0:
            p = *this;
            break;
         case 1:
            p = Position(y, BOARD_SIZE - x - 1);
            break;
         case 2:
            p = Position(BOARD_SIZE - x - 1, BOARD_SIZE - y - 1);
            break;
         case 3:
            p = Position(BOARD_SIZE - y - 1, x);
            break;
         default:
            break;
      }

      return p;
   }

   /** Rotates all coordinates counterclockwise for a number of steps */
   WallState WallState::rotate(const uint8_t rotations) const
   {
      WallState w;

      if (orientation == Orientation::Vertical)
      {
         switch (rotations & 3)
         {
            case 0:
               w = *this;
               break;
            case 1:
               w.orientation = Orientation::Horizontal;
               w.position = Position(position.y, BOARD_SIZE - position.x - 2);
               break;
            case 2:
               w.orientation = Orientation::Vertical;
               w.position = Position(BOARD_SIZE - position.x - 2, BOARD_SIZE - position.y);
               break;
            case 3:
               w.orientation = Orientation::Horizontal;
               w.position = Position(BOARD_SIZE - position.y, position.x);
               break;
            default:
               break;
         }
      }
      else
      {
         switch (rotations & 3)
         {
            case 0:
               w = *this;
               break;
            case 1:
               w.orientation = Orientation::Vertical;
               w.position = Position(position.y, BOARD_SIZE - position.x);
               break;
            case 2:
               w.orientation = Orientation::Horizontal;
               w.position = Position(BOARD_SIZE - position.x, BOARD_SIZE - position.y - 2);
               break;
            case 3:
               w.orientation = Orientation::Vertical;
               w.position = Position(BOARD_SIZE - position.y - 2, position.x);
               break;
            default:
               break;
         }
      }

      return w;
   }

   /** Rotates all coordinates counterclockwise for a number of steps */
   PlayerState PlayerState::rotate(const uint8_t rotations) const
   {
      return { qcore::rotate(initialState, rotations), position.rotate(rotations), wallsLeft };
   }

   /** Rotates all coordinates counterclockwise for a number of steps */
   PlayerAction PlayerAction::rotate(const uint8_t rotations) const
   {
      return { playerId, actionType, wallState.rotate(rotations), playerPosition.rotate(rotations) };
   }

   std::string PlayerAction::serialize() const
   {
      return std::string {
         (char) playerId,
         (char) actionType,
         (char) playerPosition.x,
         (char) playerPosition.y,
         (char) wallState.position.x,
         (char) wallState.position.y,
         (char) wallState.orientation };
   }

   void PlayerAction::deserialize(const std::string& s)
   {
      if (s.size() != 7)
      {
         throw util::Exception("PlayerAction deserialize failed");
      }

      // TODO Validate fields

      playerId = s.at(0);
      actionType = (ActionType) s.at(1);
      playerPosition.x = s.at(2);
      playerPosition.y = s.at(3);
      wallState.position.x = s.at(4);
      wallState.position.y = s.at(5);
      wallState.orientation = (Orientation) s.at(6);
   }
} // namespace qcore
