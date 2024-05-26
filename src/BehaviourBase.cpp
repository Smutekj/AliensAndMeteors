#include "BehaviourBase.h"
#include "GameWorld.h"
#include "Entities.h"

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
            auto &bullet = static_cast<Bullet2 &>(p_world->addObject(ObjectType::Bullet));
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
        // p_bs->spawnBulletNoSeek(entity_ind, data->r, player->pos);
        auto predicted_pos = p_player->getPosition() + angle2dir(p_player->getAngle()) * p_player->speed * 0.5f;
        auto& laser = p_world->addObject(ObjectType::Laser);
        laser.setPosition(p_owner->getPosition());
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