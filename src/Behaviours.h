#pragma once
#include <array>

#include "core.h"
#include "BoidSystem.h"

class GroupManager
{

    std::array<int, 5000> entity2group;
    std::unordered_map<int, std::vector<int>> group_ind2entities;
    int next_free_group_ind = 0;

    std::array<EntityData, 5000> &entity2boid_data;

    // std::unordered_map<int, std::unique_ptr<GroupAI>> group2ai;

public:
    GroupManager(std::array<EntityData, 5000> &entitydata) : entity2boid_data(entitydata) {}

    void destroyGroup(int group_ind)
    {
        auto &group_entities = group_ind2entities.at(group_ind);
        for (auto entity : group_entities)
        {
            entity2boid_data.at(entity).group_ind = -1;
        }
        group_ind2entities.erase(group_ind);
        if (group_ind < next_free_group_ind)
        {
            next_free_group_ind = group_ind;
        }
    }

    int createGroup(std::vector<int> entities = {})
    {
        int group_ind = next_free_group_ind;
        group_ind2entities[group_ind] = entities;
        for (auto entity : entities)
        {
            entity2boid_data.at(entity).group_ind = group_ind;
        }

        while (group_ind2entities.count(next_free_group_ind) != 0)
        {
            next_free_group_ind++;
        }
        return group_ind;
    }

    void addToGroup(int entity_ind, int group_ind)
    {
        assert(group_ind2entities.count(group_ind) != 0);        //! the group should exist
        assert(entity2boid_data.at(entity_ind).group_ind == -1); //! entity should not be in another group

        if (group_ind2entities.count(group_ind) == 0)
        {
            createGroup({group_ind});
        }
        else
        {
            group_ind2entities.at(group_ind).push_back(entity_ind);
            entity2boid_data.at(entity_ind).group_ind = group_ind;
        }
    }

    void removeFromGroup(int entity_ind)
    {
        auto &group_ind = entity2boid_data.at(entity_ind).group_ind;
        assert(group_ind2entities.count(group_ind) != 0); //! the group should exist

        auto &group_entities = group_ind2entities.at(group_ind);
        group_entities.erase(
            std::find(group_entities.begin(), group_entities.end(), entity_ind));

        if (group_entities.empty())
        {
            destroyGroup(group_ind);
        }
        group_ind = -1;
    }

    auto &getGroup(int group_ind)
    {
        return group_ind2entities.at(group_ind);
    }
};

class MoveRandomlyAndShootAI : public BoidAI
{

    bool is_moving = true;
    int cool_down = 200;
    int frames_since_shot = 200;
    BulletSystem *p_bs;
    float vision_radius = 50.f;
    bool player_spotted = false;

public:
    virtual ~MoveRandomlyAndShootAI() = default;

    MoveRandomlyAndShootAI(int entity_ind, Player *player, EntityData *data, BulletSystem *p_bs)
        : BoidAI(entity_ind, player, data), p_bs(p_bs)
    {
    }
    virtual void update()
    {

        frames_since_shot++;
        if (dist2(player->pos, data->r) < vision_radius * vision_radius)
        {
            if (frames_since_shot > cool_down)
            {
                frames_since_shot = 0;
                p_bs->spawnBullet(entity_ind, data->r, data->vel, nullptr);
            }
        }
        if (dist2(data->r, data->target_position) < 10.f)
        {
            data->target_position = randomPosInBox();
        }
    }
};

class ShootAI : public BoidAI
{

    int cool_down = 5000;
    int frames_since_shot = 5000;
    BulletSystem *p_bs;

public:
    ShootAI(int entity_ind, Player *player, EntityData *data, BulletSystem *p_bs)
        : BoidAI(entity_ind, player, data), p_bs(p_bs)
    {
    }

    virtual ~ShootAI() = default;

    virtual void update()
    {
        frames_since_shot++;
        if (frames_since_shot > cool_down)
        {
            frames_since_shot = 0;
            p_bs->spawnBullet(entity_ind, data->r, data->vel, nullptr);
        }
    }
};

class FollowAndShootAI : public BoidAI
{

    int cool_down = 500;
    int frames_since_shot = 5000;
    float vision_radius = 60.f;
    BulletSystem *p_bs;
    bool player_spotted = false;

public:
    FollowAndShootAI(int entity_ind, Player *player, EntityData *data, BulletSystem *p_bs)
        : BoidAI(entity_ind, player, data), p_bs(p_bs)
    {
    }

    virtual ~FollowAndShootAI() = default;

    virtual void update()
    {

        if (dist2(player->pos, data->r) < vision_radius * vision_radius || player_spotted)
        {
            player_spotted = true;
            data->target_position = player->pos;
        }
        if (player_spotted)
        {
            frames_since_shot++;
            if (frames_since_shot > cool_down)
            {
                frames_since_shot = 0;
                p_bs->spawnBulletNoSeek(entity_ind, data->r, player->pos);
            }
        }
    }
};

class BossAI : public BoidAI
{

    int cool_down = 100;
    int frames_since_shot_laser = 0;
    int frames_since_shot_bullet = 0;
    float vision_radius = 60.f;
    BulletSystem *p_bs;
    bool player_spotted = false;
    bool following_player = true;
    bool shooting_laser = true;
    int laser_timer = 0;
    float orig_max_vel;
public:
    BossAI(int entity_ind, Player *player, EntityData *data, BulletSystem *p_bs)
        : BoidAI(entity_ind, player, data), p_bs(p_bs)
    {
        orig_max_vel = data->max_vel;
    }

    virtual ~BossAI() = default;

    virtual void update()
    {
        if(dist(player->pos, data->r) > vision_radius){
            data->target_position = player->pos + angle2dir(rand()%180)*randf(0,3);
            following_player = true;
        }

        frames_since_shot_laser++;
        if (frames_since_shot_laser > cool_down && !following_player)
        {
            frames_since_shot_laser = 0;
            // p_bs->spawnBulletNoSeek(entity_ind, data->r, player->pos);
            auto predicted_pos = player->pos + angle2dir(player->angle) * player->speed*0.5f;
            p_bs->createLaser(entity_ind, data->r, predicted_pos - data->r, 200.f  );
            shooting_laser = true;    

        }
        frames_since_shot_bullet++;
        if (frames_since_shot_bullet > cool_down && !following_player && !shooting_laser)
        {
            frames_since_shot_bullet = 0;
            p_bs->spawnBulletNoSeek(entity_ind, data->r, player->pos);
            // auto predicted_pos = player->pos + angle2dir(player->angle) * player->speed*0.5f;
        }
        if(shooting_laser){
            laser_timer++;
            data->max_vel = 0.f;
            if(laser_timer == 120){
                laser_timer = 0;
                shooting_laser = false;  
                data->max_vel = orig_max_vel;
            }
        }

        if(dist(player->pos, data->r) < vision_radius){
            following_player = false;
        }
    }
};

class FollowPlayerIfVIsible : public BoidAI
{

    float vision_radius = 40.f;

public:
    virtual ~FollowPlayerIfVIsible() = default;

    virtual void update()
    {
        auto dist2_to_player = dist2(player->pos, data->r);
        if (dist2_to_player < vision_radius * vision_radius)
        {
            data->target_position = player->pos;
        }
        else
        {
            data->target_position = data->r;
        }
        // if(dist2_to_player < vision_radius/9.f){
        //     data->max_vel = 25.f;
        // }
    }
};

class ChasePlayerThenSuicide : public BoidAI
{

    float vision_radius = 40.f;
    float follow_distance = 100.f;
    sf::Vector2f return_position;
    bool is_chasing = false;
    bool is_suiciding = false;

public:
    ChasePlayerThenSuicide(int entity_ind, Player *player, EntityData *data)
        : BoidAI(entity_ind, player, data)
    {
        return_position = data->r;
    }

    virtual ~ChasePlayerThenSuicide() = default;

    virtual void update()
    {
        auto dist2_to_player = dist2(player->pos, data->r);

        if (dist2_to_player < vision_radius * vision_radius && !is_suiciding)
        {
            data->target_position = player->pos;
            is_chasing = true;
        }
        else if (is_chasing)
        {
        }

        if (is_chasing)
        {
            data->target_position = player->pos;
            if (dist2_to_player < vision_radius * vision_radius / 9.f)
            {
                is_chasing = false;
                is_suiciding = true;
                data->target_position = data->r + 3.f * (player->pos - data->r);
                data->max_vel *= 3.f;
            }
        }
        if (is_suiciding)
        {

            if (dist2(data->target_position, data->r) < 5.f)
            {
                data->health = 0;
            }
        }
    }
};

class ChasePlayerSomeDistance : public BoidAI
{

    float vision_radius = 40.f;
    float follow_distance = 100.f;
    sf::Vector2f return_position;
    bool is_chasing = false;

public:
    ChasePlayerSomeDistance(int entity_ind, Player *player, EntityData *data)
        : BoidAI(entity_ind, player, data)
    {
        return_position = data->r;
    }

    virtual ~ChasePlayerSomeDistance() = default;

    virtual void update()
    {
        auto dist2_to_player = dist2(player->pos, data->r);

        if (dist2_to_player < vision_radius * vision_radius)
        {
            data->target_position = player->pos;
            is_chasing = true;
        }

        if (is_chasing)
        {
            data->target_position = player->pos;
        }
    }
};

class ChasePlayer : public BoidAI
{

    float vision_radius = 40.f;
    float follow_distance = 100.f;
    sf::Vector2f return_position;
    bool is_chasing = false;

public:
    ChasePlayer(int entity_ind, Player *player, EntityData *data)
        : BoidAI(entity_ind, player, data)
    {
        return_position = data->r;
    }

    virtual ~ChasePlayer() = default;

    virtual void update()
    {
        auto dist2_to_player = dist2(player->pos, data->r);

        if (dist2_to_player < vision_radius * vision_radius)
        {
            data->target_position = player->pos;
            is_chasing = true;
            data->state = MoveState::MOVING;
        }

        if (is_chasing)
        {
            data->target_position = player->pos;
        }
    }
};

class PatrolAI : public BoidAI
{

    float vision_radius = 40.f;
    float follow_distance = 100.f;
    sf::Vector2f return_position;
    bool moving_to_target = true;

    GroupManager *p_groups;
    BoidSystem *p_boids;

public:
    PatrolAI(int entity_ind, Player *player, EntityData *data, GroupManager *p_groups, BoidSystem *p_boidsboids)
        : p_groups(p_groups), p_boids(p_boids), BoidAI(entity_ind, player, data)
    {
        return_position = data->r;
    }

    virtual ~PatrolAI() = default;

    virtual void update()
    {
        if (dist2(data->r, data->target_position) < 10.f)
        {
            data->target_position = randomPosInBox();
        }
        if (moving_to_target)
        {
        }
    }
};

class UnderlingAI;

class LeaderAI : public BoidAI
{

    float vision_radius = 40.f;
    float follow_distance = 100.f;
    sf::Vector2f return_position;
    bool spotted_player = false;
    bool released_underlings = false;

    GroupManager *p_groups;
    BoidSystem *p_boids;
    BulletSystem *p_bs;

    friend UnderlingAI;

public:
    LeaderAI(int entity_ind, Player *player, EntityData *data, GroupManager *p_groups, BoidSystem *p_boids, BulletSystem *p_bs);

    virtual ~LeaderAI() = default;

    virtual void update();
};

class UnderlingAI : public BoidAI
{

    float vision_radius = 40.f;
    float cluster_size = 100.f;
    sf::Vector2f return_position;
    bool spotted_player = false;
    bool released_underlings = true;

    GroupManager *p_groups;
    BoidSystem *p_boids;
    LeaderAI *p_leader;

public:
    UnderlingAI(int entity_ind, Player *player, EntityData *data, GroupManager *p_groups, BoidSystem *p_boids, LeaderAI *leader);

    virtual ~UnderlingAI() = default;

    virtual void update();
};

class FollowPlayerSomeDistance : public BoidAI
{

    float vision_radius = 40.f;
    float follow_distance = 100.f;
    sf::Vector2f return_position;
    bool is_following = false;
    bool is_returning = false;

public:
    FollowPlayerSomeDistance(int entity_ind, Player *player, EntityData *data)
        : BoidAI(entity_ind, player, data)
    {
        return_position = data->r;
    }

    virtual ~FollowPlayerSomeDistance() = default;

    virtual void update()
    {

        if (dist2(player->pos, data->r) < vision_radius * vision_radius && !is_returning)
        {
            data->target_position = player->pos;
            is_following = true;
        }
        else if (is_following)
        {
            data->target_position = player->pos;
        }
        if (dist2(return_position, data->r) > follow_distance * follow_distance)
        {
            data->target_position = return_position;
            is_following = false;
            is_returning = true;
        }
        if (is_returning && dist2(return_position, data->r) < 5.f)
        {
            is_returning = false;
        }
    }
};

class IdleAI : public BoidAI
{

    sf::Vector2f center_point;

    float omega_1 = 3.f;
    float omega_2 = 3.f;
    float x_max = 1.f;
    float y_max = 1.f;
    float phase_shift = 90.f;
    float t = 0.f;

public:
    IdleAI(int entity_ind, Player *player, EntityData *data, float radius = 0.f,
           float omega_x = 1.f, float omega_y = 2.f)
        : x_max(radius), y_max(radius), omega_1(omega_x), omega_2(omega_y), BoidAI(entity_ind, player, data)
    {
        center_point = data->r;
    }

    virtual ~IdleAI() = default;

    virtual void update() override
    {

        t += 1.f / 60.f;

        data->acc.x += x_max * omega_1 * omega_1 * std::sin(omega_1 * t);
        data->acc.y += y_max * omega_2 * omega_2 * std::sin(omega_2 * t + phase_shift / 180.f * M_PI);

        truncate(data->acc, 0.5f);
    }
};

class AvoidMeteors : public BoidAI
{

    float vision_radius = 40.f;
    float force_multiplier = 1.f;
    sf::Vector2f return_position;
    bool is_following = false;
    bool is_returning = false;

public:
    AvoidMeteors(int entity_ind, Player *player, EntityData *data)
        : BoidAI(entity_ind, player, data)
    {
    }

    virtual ~AvoidMeteors() = default;

    virtual void update()
    {
    }
};
