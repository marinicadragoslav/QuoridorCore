#ifndef Header_qcore_PlayerAction
#define Header_qcore_PlayerAction

#include <stdint.h>

namespace qcore
{
   typedef uint8_t PlayerId;

   const uint8_t BOARD_SIZE = 9;

   enum class Direction
   {
      Down,
      Right,
      Up,
      Left
   };

   /** Rotates all coordinates counterclockwise for a number of steps */
   Direction rotate(const Direction d, const uint8_t rotations);

   enum class Orientation
   {
      Vertical,
      Horizontal
   };

   enum class ActionType
   {
      Move,
      Wall
   };

   struct Position
   {
      uint8_t x;
      uint8_t y;

      Position() : x(0), y(0) {}
      Position(const Position& p) : x(p.x), y(p.y) {}
      Position(uint8_t x, uint8_t y) : x(x), y(y) {}

      bool operator==(const Position& p) const;

      /** Computes the distance between 2 positions */
      uint8_t dist(const Position& p) const;

      /** Multiplies each coordinate with the given factor */
      Position operator*(uint8_t m) const;
      Position operator/(uint8_t m) const;

      /** Moves the coordinates in the specified direction */
      Position& operator+=(const Direction direction);
      Position operator+(const Direction direction) const;

      Position operator+(const Position& p) const;
      Position operator-(const Position& p) const;

      /** Rotates all coordinates counterclockwise for a number of steps */
      Position rotate(const uint8_t rotations) const;
   };

   struct PlayerAction
   {
      PlayerId playerId;
      ActionType actionType;
      Position position;
      Orientation wallOrientation;
   };

   namespace literals
   {
      Position operator "" _x(unsigned long long int x);
      Position operator "" _y(unsigned long long int y);
   }
}

#endif // Header_qcore_PlayerAction
