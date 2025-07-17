#pragma once


class Enemy;
class GameWorld;
struct PlayerEntity;

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

    virtual void update(float dt) =0;
};


class FollowAndShootAI2 : public BoidAI2
{

    float cool_down = 1.5;
    float time_since_shot = 0.75;
    float vision_radius = 80.f;
    bool player_spotted = false;

public:
    FollowAndShootAI2(PlayerEntity *player, Enemy* owner, GameWorld* world);

    virtual ~FollowAndShootAI2() override;

    virtual void update(float dt) override;
};

class FollowAndShootLasersAI : public BoidAI2
{

    float cool_down = 4.;
    float time_from_last_shot = 0;
    float time_from_player_away = 0;
    float vision_radius = 80.f;
    bool player_spotted = false;
    bool following_player = true;
    bool shooting_laser = true;
    float laser_timer = 0;
    float orig_max_vel;
    float orig_max_acc;

public:
    FollowAndShootLasersAI(PlayerEntity *player, Enemy* owner, GameWorld* world);

    virtual ~FollowAndShootLasersAI() = default;

    virtual void update(float dt);
};


class BomberAI : public BoidAI2
{

    int cool_down = 100;
    int frames_since_shot = 0;
    float vision_radius = 130.f;
    bool player_spotted = false;
    bool following_player = true;
    bool shooting_laser = true;
    int laser_timer = 0;
    float orig_max_vel;

    enum class State
    {
        FOLLOWING,
        SHOOTING,
        BOMBING,
    };

    State state;
    State previous_state;

public:
    BomberAI(PlayerEntity *player, Enemy *owner, GameWorld *world);

    virtual ~BomberAI() = default;

    virtual void update(float dt);
};


enum class AIType
{
    SHOOTER,
    BOMBER,
    LASER,
};

