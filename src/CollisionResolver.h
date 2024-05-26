#pragma once

#include "Entities.h"
#include "Player.h"


class CollisionResolver
{

public:
    CollisionData m_collision_data;

    CollisionResolver(CollisionData collision_data)
        : m_collision_data(collision_data)
    {
    }

    //! Dirty RTTI :(, I will make visitor later (maybe), or maybe i can steal some code from Loki
    void resolveCollision(GameObject &obj1, GameObject &obj2)
    {
        switch (obj1.getType())
        {
        case ObjectType::Meteor:
            resolveMeteorCollision(static_cast<Meteor &>(obj1), obj2);
            break;
        case ObjectType::Enemy:
            // auto& e1 = static_cast<Meteor&>(obj1);
            // resolveEnemyCollision(e1, obj2);
            break;
        case ObjectType::Bullet:
            resolveBulletCollision(static_cast<Bullet2 &>(obj1), obj2);
            break;
        case ObjectType::Player:
            resolvePlayerCollision(static_cast<PlayerEntity &>(obj1), obj2);
            break;
        default:
            break;
        }

        if (obj2.getType() == ObjectType::Player)
        {
            resolvePlayerCollision(static_cast<PlayerEntity &>(obj2), obj1);
        }

        if (obj1.getType() == ObjectType::Enemy && obj2.getType() == ObjectType::Explosion)
        {
            resolveExplosionEnemy(static_cast<Enemy &>(obj1), static_cast<Explosion &>(obj2));
        }
        if (obj2.getType() == ObjectType::Enemy && obj1.getType() == ObjectType::Explosion)
        {
            resolveExplosionEnemy(static_cast<Enemy &>(obj2), static_cast<Explosion &>(obj1));
        }
    }

    void resolveMeteorCollision(Meteor &m1, GameObject &obj2)
    {
        switch (obj2.getType())
        {
        case ObjectType::Enemy:
            resolveEnemyMeteorCollision(static_cast<Enemy &>(obj2), m1);
            break;
        case ObjectType::Bullet:
            resolveBulletMeteorCollision(static_cast<Bullet2 &>(obj2), m1);
            break;
        case ObjectType::Player:
            resolvePlayerMeteorCollision(static_cast<PlayerEntity &>(obj2), m1);
            break;
        }
    }

    void resolvePlayerCollision(PlayerEntity &p, GameObject &obj)
    {
        switch (obj.getType())
        {
        case ObjectType::Meteor:
        {
            auto mvt = m_collision_data.separation_axis;
            if (dot(mvt, p.m_vel) < 0.f)
            {
                p.m_vel -= 2.f * dot(mvt, p.m_vel) * mvt;
            }
            break;
        }
        case ObjectType::Bullet:
        {
            obj.kill();
            p.health--;
            break;
        }
        }
    }

    void resolvePlayerMeteorCollision(PlayerEntity &p, Meteor &m)
    {
        auto mvt = m_collision_data.separation_axis;
        if (dot(mvt, p.m_vel) < 0.f)
        {
            p.m_vel -= 2.f * dot(mvt, p.m_vel) * mvt;
            p.m_angle = dir2angle(p.m_vel);
            p.health -= 1;
        }
    }

    void resolveEnemyCollision(Enemy &e1, GameObject &obj2)
    {
        switch (obj2.getType())
        {
        case ObjectType::Meteor:
            resolveEnemyMeteorCollision(e1, static_cast<Meteor &>(obj2));
            break;
        case ObjectType::Bullet:
            resolveBulletEnemyCollision(static_cast<Bullet2 &>(obj2), e1);
            break;
        }
    }
    void resolveBulletCollision(Bullet2 &b1, GameObject &obj2)
    {
        switch (obj2.getType())
        {
        case ObjectType::Meteor:
            resolveBulletMeteorCollision(b1, static_cast<Meteor &>(obj2));
            break;
        case ObjectType::Enemy:
            resolveBulletEnemyCollision(b1, static_cast<Enemy &>(obj2));
            break;
        }
    }
    void resolveBulletMeteorCollision(Bullet2 &b2, Meteor &m1)
    {
        b2.kill();
    }

    void resolveEnemyMeteorCollision(Enemy &e1, Meteor &m2)
    {
        e1.m_health--;
        auto mvt = m_collision_data.separation_axis;
        if (dot(mvt, e1.m_vel) < 0.f)
        {
            e1.m_vel -= 2.f * dot(mvt, e1.m_vel) * mvt;
        }
        else
        {
            // e1.m_vel -= 2.f * dot(mvt, -e1.m_vel) * mvt;
        }
    }

    void resolveBulletEnemyCollision(Bullet2 &b1, Enemy &e1)
    {
    }

    void resolveExplosionEnemy(Enemy &e1, Explosion &b1)
    {
        auto dr_to_center = e1.getPosition() - b1.getPosition();
        auto dist_to_center = norm(dr_to_center);
        auto impulse_dir = dr_to_center / dist_to_center;

        auto alpha = 1 - dist_to_center / b1.m_collision_shape->getScale().x;
        if (alpha > 0)
        {
            e1.m_impulse += impulse_dir * (alpha * 300.f);
        }
    }

    void bounce(GameObject &obj1, GameObject &obj2)
    {
        auto inertia1 = obj1.m_rigid_body->inertia;
        auto inertia2 = obj2.m_rigid_body->inertia;
        auto &angle_vel1 = obj1.m_rigid_body->angle_vel;
        auto &angle_vel2 = obj2.m_rigid_body->angle_vel;
        auto &mass1 = obj1.m_rigid_body->mass;
        auto &mass2 = obj2.m_rigid_body->mass;

        auto c_data = m_collision_data;
        auto n = c_data.separation_axis;
        if (obj1.m_rigid_body->mass < obj2.m_rigid_body->mass)
        {
            obj1.move(-c_data.separation_axis * c_data.minimum_translation);
        }
        else
        {
            obj2.move(c_data.separation_axis * c_data.minimum_translation);
        }

        auto cont_point = c_data.contact_point;

        auto v_rel = obj1.m_vel - obj2.m_vel;
        auto v_reln = dot(v_rel, n);

        float e = 1;
        float u_ab = 1. / mass1 + 1. / mass2;

        auto r_cont_coma = cont_point - obj1.getPosition();
        auto r_cont_comb = cont_point - obj2.getPosition();

        sf::Vector2f r_cont_coma_perp = {r_cont_coma.y, -r_cont_coma.x};
        sf::Vector2f r_cont_comb_perp = {r_cont_comb.y, -r_cont_comb.x};

        float ran = dot(r_cont_coma_perp, n);
        float rbn = dot(r_cont_comb_perp, n);

        float u_ab_rot = ran * ran / inertia1 + rbn * rbn / inertia2;

        float j_factor = -(1 + e) * v_reln / (u_ab + u_ab_rot);

        angle_vel1 += ran * j_factor / inertia1;
        angle_vel2 -= rbn * j_factor / inertia2;
        obj1.m_vel += j_factor / mass1 * n;
        obj2.m_vel -= j_factor / mass2 * n;
    }
};
