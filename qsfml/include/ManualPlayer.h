#pragma once
#include <Player.h>

class ManualPlayer : public qcore::Player
{
public:

    /** Construction */
    ManualPlayer(qcore::PlayerId id, const std::string& name, qcore::GamePtr game);

    /** Defines player's behavior */
    void doNextMove() override;
};