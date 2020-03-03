#ifndef Header_qcore_ICPlayer
#define Header_qcore_ICPlayer

#include "Player.h"

namespace qplugin
{
   class BState
   {
   public:
      int level;
      qcore::BoardMap baseMap;
      qcore::Position myPos;
      qcore::Position opPos;
      qcore::WallState wall;
      qcore::WallState initWall;
      qcore::Direction direction;

      int myPathLen;
      int opPathLen;
      int myPathNo;
      int opPathNo;
      int score;

   public:
      BState(const qcore::Player& player, bool opw);
      BState(const BState& bs, const qcore::WallState& wall, const qcore::Player& player, bool opw, int level = -1);

      bool operator<(const BState& bs);

   private:
      void compute(const qcore::Player& player, bool opw);
      int compute(const qcore::BoardMap& baseMap, const qcore::Position& startPos, bool me);
   };

   struct BStateOrder
   {
      bool operator()(std::shared_ptr<BState> lhs, std::shared_ptr<BState> rhs)
      {
         return *lhs < *rhs;
      }
   };

   class ICPlayer : public qcore::Player
   {
      int mMoves;

   public:

      ICPlayer(uint8_t id, const std::string& name, qcore::GamePtr game);
      void doNextMove() override;
   };
}

#endif // Header_qcore_ICPlayer
