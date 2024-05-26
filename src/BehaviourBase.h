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


class Enemy;
class GameWorld;

class BoidAI2
{
protected:
    PlayerEntity *p_player;
    Enemy* p_owner;
    GameWorld* p_world;
public:

    BoidAI2(PlayerEntity *player, Enemy *owner, GameWorld* world)
        :  p_player(player), p_owner(owner), p_world(world) {}

    virtual ~BoidAI2() = default;

    virtual void update() =0;
};


class FollowAndShootAI2 : public BoidAI2
{

    int cool_down = 200;
    int frames_since_shot = 5000;
    float vision_radius = 60.f;
    bool player_spotted = false;

public:
    FollowAndShootAI2(PlayerEntity *player, Enemy* owner, GameWorld* world);

    virtual ~FollowAndShootAI2() override;

    virtual void update() override;
};

class FollowAndShootLasersAI : public BoidAI2
{

    int cool_down = 100;
    int frames_since_shot_laser = 0;
    int frames_since_shot_bullet = 0;
    float vision_radius = 60.f;
    bool player_spotted = false;
    bool following_player = true;
    bool shooting_laser = true;
    int laser_timer = 0;
    float orig_max_vel;

public:
    FollowAndShootLasersAI(PlayerEntity *player, Enemy* owner, GameWorld* world);

    virtual ~FollowAndShootLasersAI() = default;

    virtual void update();
};


