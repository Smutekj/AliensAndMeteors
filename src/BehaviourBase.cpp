#include "BehaviourBase.h"

#include "GameWorld.h"
#include "Entities/Entities.h"
#include "Entities/Player.h"
#include "Entities/Enemy.h"
#include "Entities/Attacks.h"


FollowAndShootAI2::FollowAndShootAI2(PlayerEntity *player, Enemy *owner, GameWorld *world)
    : BoidAI2(player, owner, world)
{
}

FollowAndShootAI2::~FollowAndShootAI2() = default;

void FollowAndShootAI2::update()
{
    auto dr_player = p_player->getPosition() - p_owner->getPosition();
    // if (norm2(dr_player) < vision_radius * vision_radius || player_spotted)
    // {
        player_spotted = true;
        p_owner->m_target_pos = p_player->getPosition();
    // }
    if (player_spotted)
    {
        frames_since_shot++;
        if (frames_since_shot > cool_down)
        {
            frames_since_shot = 0;
            auto &bullet = static_cast<Bullet &>(p_world->addObject(ObjectType::Bullet));
            bullet.setPosition(p_owner->getPosition());
            float bullet_vel = 5000.f;
            bullet.m_vel = bullet_vel * dr_player / norm2(dr_player);
        }
    }
}

FollowAndShootLasersAI::FollowAndShootLasersAI(PlayerEntity *player, Enemy *owner, GameWorld *world)
    : BoidAI2(player, owner, world) {}

void FollowAndShootLasersAI::update()
{
    if (dist(p_player->getPosition(), p_owner->getPosition()) > vision_radius)
    {
        p_owner->m_target_pos = p_player->getPosition() + angle2dir(rand() % 180) * randf(0, 3);
        following_player = true;
    }
    else
    {
        following_player = false;
    }

    frames_since_shot_laser++;
    if (frames_since_shot_laser > cool_down && !following_player)
    {
        frames_since_shot_laser = 0;
        // p_bs->spawnBulletNoSeek(entity_ind, p_owner->getPosition(), player->pos);
        auto predicted_pos = p_player->getPosition() + angle2dir(p_player->getAngle()) * p_player->speed * 0.5f;
        auto& laser = static_cast<Laser&>(p_world->addObject(ObjectType::Laser));
        laser.setPosition(p_owner->getPosition());
        laser.setOwner(p_owner);
        laser.setAngle(dir2angle(predicted_pos - p_owner->getPosition()));       
        shooting_laser = true;
    }
    frames_since_shot_bullet++;
    if (frames_since_shot_bullet > cool_down && !following_player && !shooting_laser)
    {
        frames_since_shot_bullet = 0;
        
    }
    if (shooting_laser)
    {
        p_owner->max_vel = 0.f;
        if (laser_timer++ > 120)
        {
            laser_timer = 0;
            shooting_laser = false;
            p_owner->max_vel = orig_max_vel;
        }
    }
}


    BomberAI::BomberAI(PlayerEntity *player, Enemy *owner, GameWorld *world)
    : BoidAI2(player, owner, world)
    {
        orig_max_vel = p_owner->max_vel;
    }

     void BomberAI::update()
    {

        float dist2player = dist(p_player->getPosition(), p_owner->getPosition());
        if ( dist2player > vision_radius)
        {
            p_owner->m_target_pos = p_player->getPosition() + angle2dir(rand() % 180) * randf(0, 3);
            following_player = true;
            previous_state = state;
            state = State::FOLLOWING;
        }
        
        if(dist2player < vision_radius && state == State::FOLLOWING)
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
            auto predicted_pos = p_player->getPosition() + angle2dir(p_player->getAngle()) * p_player->speed * 0.5f;
            auto dir = (predicted_pos - p_owner->getPosition()) / norm(predicted_pos - p_owner->getPosition());
            auto& bullet = p_world->addObject(ObjectType::Bullet);
            bullet.setPosition(p_owner->getPosition());
            bullet.m_vel = dir * 50.f;
            
            p_owner->m_target_pos = p_owner->getPosition();
            state = State::SHOOTING;
        }

        if (frames_since_shot > cool_down && state == State::SHOOTING)
        {
            auto dr_to_player = p_player->getPosition() - p_owner->getPosition();
            auto dir = (dr_to_player) / norm(dr_to_player);
            auto& bomb = static_cast<Bomb&>(p_world->addObject(ObjectType::Bomb));
            bomb.setPosition(p_owner->getPosition());
            bomb.m_vel = dir * 3.f*dist2player /(1 - std::exp(-0.95f*bomb.m_life_time));

            p_owner->m_target_pos = p_player->getPosition() + angle2dir(rand() % 360) * randf(10, 20);
            state = State::BOMBING;
        }
    }