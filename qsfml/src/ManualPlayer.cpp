#include "ManualPlayer.h"

ManualPlayer::ManualPlayer(qcore::PlayerId id, const std::string& name, qcore::GamePtr game) :
    qcore::Player(id, name, game)
{
}

/** Defines player's behavior */
void ManualPlayer::doNextMove()
{
}

