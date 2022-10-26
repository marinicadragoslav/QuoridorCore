#ifndef Header_qcore_GameState
#define Header_qcore_GameState

#include "Qcore_API.h"
#include "PlayerAction.h"

#include <list>
#include <vector>
#include <memory>
#include <mutex>
#include <functional>
#include <cstring>

namespace qcore
{
   const uint8_t BOARD_MAP_SIZE = BOARD_SIZE * 2 - 1;

   class QCODE_API BoardMap
   {
      // Type definitions
   public:
      enum ItemType : uint8_t
      {
         Invalid = 0xFF,
         VertivalWall = 0xFE,
         HorizontalWall = 0xFD,
         Pawn0 = 0xFC,
         Pawn1 = 0xFB,
         Pawn2 = 0xFA,
         Pawn3 = 0xF9,
         MidWall = 0x2D // "-" ASCII
      };

      // Encapsulated data members
   private:

      uint8_t map[BOARD_MAP_SIZE][BOARD_MAP_SIZE];
      uint8_t invalidPos;

      // Methods
   public:
      BoardMap() : map{}, invalidPos(Invalid) {}
      BoardMap(const BoardMap&);

      uint8_t& operator() (int8_t x, int8_t y);
      uint8_t operator() (int8_t x, int8_t y) const;

      uint8_t& operator() (const Position& p) { return operator()(p.x, p.y); }
      uint8_t operator() (const Position& p) const { return operator()(p.x, p.y); }

      BoardMap& operator=(const BoardMap& from) { std::memcpy(map, from.map, sizeof(map)); invalidPos = from.invalidPos; return *this; };

      bool isPawn(const Position& p) const;
      bool isWall(const Position& p) const;
      bool isPawnSpace(const Position& p) const;
   };

   class QCODE_API BoardState
   {
      // Type definitions
   public:

      typedef std::function<void()> StateChangeCb;

      // Encapsulated data members
   private:

      /** List of walls placed on the board */
      std::list<WallState> mWalls;

      /** List of players and their position */
      std::vector<PlayerState> mPlayers;

      /** Flags if the game has finished */
      bool mFinished;

      /** The player who won */
      PlayerId mWinner;

      /** Last action made */
      PlayerAction mLastAction;

      /** List of state change callbacks */
      mutable std::list<StateChangeCb> mStateChangeCb;

      /** Protection against concurrent access */
      mutable std::mutex mMutex;

      // Methods
   public:

      /** Construction */
      BoardState(uint8_t players, uint8_t walls = 0);

      BoardState(const BoardState& bs) :
          mWalls(bs.mWalls),
          mPlayers(bs.mPlayers),
          mFinished(bs.mFinished),
          mWinner(bs.mWinner),
          mLastAction(bs.mLastAction),
          mStateChangeCb(bs.mStateChangeCb)
      {};

      /** Registers callback for state change notification */
      void registerStateChange(StateChangeCb cb) const;

      //
      // Getters over different board information
      // All information are from the perspective of the player set as parameter.
      //

      /** Get wall states */
      std::list<WallState> getWalls(const PlayerId id) const;

      /** Get player states */
      std::vector<PlayerState> getPlayers(const PlayerId id) const;

      /** Check if the specified space is occupied by a pawn */
      bool isSpaceEmpty(const Position& position, const PlayerId id) const;

      /** Flags if the game has finished */
      bool isFinished() const;

      /** Returns the ID of the player who won the game. Valid only when the game has finished. */
      PlayerId getWinner() const;

      /** Returns the last action made */
      PlayerAction getLastAction() const;

      /**
       * Creates a matrix representing the elements on the board. Between 'pawn' rows / columns are
       * inserted 'wall' rows / columns, therefore the map size will be BOARD_SIZE * 2 - 1.
       */
      void createBoardMap(BoardMap& map, const PlayerId id) const;

      //
      // Board updates (called only from inside the Game).
      // Players will not be able to alter the board state directly.
      //

      /** Sets the specified action on the board, after it has been validated */
      void applyAction(const PlayerAction& action);

   private:

      /** Notifies all listeners that the board state has changed */
      void notifyStateChange();
   };

   typedef std::shared_ptr<const BoardState> BoardStatePtr;
}

#endif // Header_qcore_GameState
