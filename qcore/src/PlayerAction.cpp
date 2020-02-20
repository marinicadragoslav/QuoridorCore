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

   std::string PlayerAction::serialize() const
   {
      return std::string { (char) playerId, (char) actionType, (char) position.x, (char) position.y, (char) wallOrientation };
   }

   void PlayerAction::deserialize(const std::string& s)
   {
      if (s.size() != 5)
      {
         throw util::Exception("PlayerAction deserialize failed");
      }

      // TODO Validate fields

      playerId = s.at(0);
      actionType = (ActionType) s.at(1);
      position.x = s.at(2);
      position.y = s.at(3);
      wallOrientation = (Orientation) s.at(4);
   }
} // namespace qcore
