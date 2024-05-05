#pragma once

#include "core.h"
#include "Player.h"


class BoidAI
{


protected:
    Player *player;
    // Game *game;
    int entity_ind;
public:
    EntityData *data;

    BoidAI(int entity_ind, Player *player, EntityData *data)
        : entity_ind(entity_ind), player(player), data(data) {}

    virtual ~BoidAI() = default;

    virtual void update() =0;
};
