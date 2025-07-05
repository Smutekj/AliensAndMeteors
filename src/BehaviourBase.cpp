#include "BehaviourBase.h"

#include "GameWorld.h"
#include "Utils/RandomTools.h"
#include "Entities/Entities.h"
#include "Entities/Player.h"
#include "Entities/Enemy.h"
#include "Entities/Attacks.h"

FollowAndShootAI2::FollowAndShootAI2(PlayerEntity *player, Enemy *owner, GameWorld *world)
    : BoidAI2(player, owner, world)
{
}

FollowAndShootAI2::~FollowAndShootAI2() = default;

void FollowAndShootAI2::update(float dt = 0.033)
{
    auto dr_player = p_player->getPosition() - p_owner->getPosition();
    auto dist_to_player = utils::norm(dr_player);
    
    player_spotted = true;
    p_owner->m_target_pos = p_player->getPosition();
    
    if (player_spotted)
    {
        time_since_shot += dt;
        if (time_since_shot > cool_down)
        {
            time_since_shot = 0;
            auto &bullet = p_world->addObject2<Bullet>();
            bullet.setPosition(p_owner->getPosition());

            if (rand() % 2 == 0)
            {
                bullet.setTarget(p_player);
                bullet.m_max_vel = 100.;
                bullet.m_max_acc = 50.;
                bullet.m_vel = bullet.m_max_vel * dr_player / dist_to_player;
            }
            else
            { //! fire bullets don't follow player
                bullet.setBulletType(BulletType::Fire);
                bullet.m_vel = 120. * dr_player / dist_to_player;
                bullet.m_max_vel = 120.;
            }
        }
    }
}

FollowAndShootLasersAI::FollowAndShootLasersAI(PlayerEntity *player, Enemy *owner, GameWorld *world)
    : BoidAI2(player, owner, world), orig_max_vel(owner->max_vel), orig_max_acc(owner->max_acc) {}

void FollowAndShootLasersAI::update(float dt = 0.033)
{
    if (dist(p_player->getPosition(), p_owner->getPosition()) > vision_radius)
    {
        p_owner->m_target_pos = p_player->getPosition() + utils::angle2dir(randf(0, 360.f)) * randf(0, 3);
        time_from_player_away += dt;
    }
    else
    { //! player is in vision
        time_from_player_away = 0.;
        following_player = false;
    }
    if (time_from_player_away > 1.) //! if player is at least a second out of vision we start following him
    {
        following_player = true;
    }

    time_from_last_shot += dt;
    if (time_from_last_shot > cool_down && !following_player)
    {
        time_from_last_shot = 0;
        //! shoots somwhere around players future positiion
        auto predicted_pos = p_player->getPosition() + utils::angle2dir(p_player->getAngle()) * p_player->speed * randf(0., 1.5);

        Laser laser = p_world->addObject2<Laser>();
        laser.setPosition(p_owner->getPosition());
        laser.setOwner(p_owner);
        auto new_angle = utils::dir2angle(predicted_pos - p_owner->getPosition());
        p_owner->m_vel = orig_max_vel * utils::angle2dir(new_angle);
        laser.m_life_time = 2.;
        laser.m_max_length = 200.f;
        laser.m_max_dmg = 0.5;
        shooting_laser = true;
    }

    if (shooting_laser)
    {
        p_owner->max_vel = orig_max_vel / 4.f;
        p_owner->max_acc = orig_max_acc / 8.f;
        laser_timer += dt;
        if (laser_timer > 2.)
        {
            laser_timer = 0;
            shooting_laser = false;
            p_owner->max_vel = orig_max_vel;
            p_owner->max_acc = orig_max_acc;
        }
    }
}

BomberAI::BomberAI(PlayerEntity *player, Enemy *owner, GameWorld *world)
    : BoidAI2(player, owner, world)
{
    orig_max_vel = p_owner->max_vel;
}

void BomberAI::update(float dt)
{

    float dist2player = dist(p_player->getPosition(), p_owner->getPosition());
    if (dist2player > vision_radius)
    {
        p_owner->m_target_pos = p_player->getPosition() + utils::angle2dir(rand() % 180) * randf(0, 3);
        following_player = true;
        previous_state = state;
        state = State::FOLLOWING;
    }

    if (dist2player < vision_radius && state == State::FOLLOWING)
    {
        following_player = false;
        if (rand() % 2 == 0)
        {
            state = State::BOMBING;
        }
        else
        {
            state = State::SHOOTING;
        }
    }

    frames_since_shot++;
    if (frames_since_shot > cool_down && state == State::BOMBING)
    {
        frames_since_shot = 0;
        auto predicted_pos = p_player->getPosition() + utils::angle2dir(p_player->getAngle()) * p_player->speed * 0.5f;
        auto dir = (predicted_pos - p_owner->getPosition()) / utils::norm(predicted_pos - p_owner->getPosition());
        auto &bullet =p_world->addObject2<Bullet>();
        bullet.setPosition(p_owner->getPosition());
        bullet.m_vel = dir * 90.;
        bullet.m_max_vel = 120.;

        p_owner->m_target_pos = p_owner->getPosition();
        state = State::SHOOTING;
    }

    if (frames_since_shot > cool_down && state == State::SHOOTING)
    {
        auto &bomb = p_world->addObject2<Bomb>();
        bomb.setPosition(p_owner->getPosition());

        //! calculate velocity so as to hit the player in exactly bomb lifetime
        auto predicted_pos = p_player->getPosition() + utils::angle2dir(p_player->getAngle()) * p_player->speed * bomb.m_life_time;
        auto dr_to_player = predicted_pos - p_owner->getPosition();
        auto dir = (dr_to_player) / norm(dr_to_player);
        auto dist = utils::norm(dr_to_player);
        auto bomb_init_speed = 2. * dist / bomb.m_life_time;

        bomb.m_vel = dir * bomb_init_speed;
        
        bomb.m_acc = bomb_init_speed / bomb.m_life_time;

        p_owner->m_target_pos = p_player->getPosition() + utils::angle2dir(randf(0, 360.)) * randf(30, 40);
        state = State::BOMBING;
    }
}