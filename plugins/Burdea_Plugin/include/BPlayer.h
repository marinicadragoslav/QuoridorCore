/*
 * BPlayer.h
 *
 *  Created on: Nov 16, 2022
 *      Author: uib15381
 */

#ifndef PLUGINS_BURDEA_PLUGIN_INCLUDE_BPLAYER_H_
#define PLUGINS_BURDEA_PLUGIN_INCLUDE_BPLAYER_H_

#include "Player.h"
#include "Game.h"

using namespace std;

namespace qplugin
{
struct QCODE_API PlayerActionRefactor
   {
        qcore::PlayerAction action;
        uint8_t m_mylen = 0;
        uint8_t m_oplen = 0;
   };

    class QCODE_API MyBoardState
    {
    public:
        MyBoardState(qcore::BoardState& bs, qcore::PlayerId PlayerId) :
            mWalls(bs.getWalls(PlayerId)),
            mPlayers(bs.getPlayers(PlayerId)),
            mLastAction(bs.getLastAction())
        {};

        /** Get player states from the player's perspective */
        std::vector<qcore::PlayerState> getPlayers(const qcore::PlayerId id) const;

        /** Get wall states */
        std::list<qcore::WallState> getWalls(const qcore::PlayerId id) const;

        void applyAction(const qcore::PlayerAction& action);
        void createBoardMap(qcore::BoardMap& map, const qcore::PlayerId id) const;

    private:

        /** List of walls placed on the board */
        std::list<qcore::WallState> mWalls;

        /** List of players and their position */
        std::vector<qcore::PlayerState> mPlayers;

        /** Last action made */
        qcore::PlayerAction mLastAction;

    };
    class GameState
    {
        public:
            GameState(qcore::PlayerId myId, qcore::Position myPos, qcore::Position opPos, int score, MyBoardState boardstate, uint8_t depth);

            vector <PlayerActionRefactor> getPossibleMoves();
            GameState applyMove(PlayerActionRefactor& player_decide_move);
            bool isActionValid(PlayerActionRefactor& action, std::string& reason);
            qcore::BoardMap addWalltoBoardMap(const qcore::PlayerId playerId, const PlayerActionRefactor& action);

            qcore::PlayerId m_myId;
            qcore::Position m_myPos;
            qcore::Position m_opPos;
            int m_score;
            MyBoardState m_boardstate;
            uint8_t m_depth;

   };

    class BPlayer: public qcore::Player
    {
        public:
            BPlayer(qcore::PlayerId id, const std::string& name, qcore::GamePtr game);
            void doNextMove() override;
        private:
            void decide_to_move(std::list<qcore::Position> &pos);
            int minimax(GameState& state, int alpha, int beta, bool maximizingPlayer, PlayerActionRefactor &move);
    };
}

#endif /* PLUGINS_BURDEA_PLUGIN_INCLUDE_BPLAYER_H_ */
