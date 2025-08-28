#include "Player.h"

#include "../DrawLayer.h"
#include "../Utils/RandomTools.h"
// #include "../ResourceHolder.h"
#include "Entities.h"
#include "Attacks.h"
#include "../GameWorld.h"

PlayerEntity::PlayerEntity(GameWorld *world, TextureHolder &textures, PlayerEntity *player)
    : GameObject(world, textures, ObjectType::Player, player)
{
    meteor_detector.points = {{0., 0.}, {1., -0.7}, {1., 0.7}};
}

void PlayerEntity::update(float dt)
{
    meteor_detector.setPosition(m_pos);
    meteor_detector.setScale(m_size.x * 2., m_size.y / 2.);
    meteor_detector.setRotation(m_angle);

    auto detected_meteor_ids = m_world->m_collision_system.findIntersections(ObjectType::Meteor, meteor_detector);

    fixAngle();
    boost(dt);
    if (m_is_turning_left)
    {
        //! slower turning when shooting laser
        auto angle_vel = m_angle_vel - m_angle_vel * 0.4 * m_is_shooting_laser;
        setAngle(m_angle + angle_vel * dt);
    }
    if (m_is_turning_right)
    {
        auto angle_vel = m_angle_vel - m_angle_vel * 0.4 * m_is_shooting_laser;
        setAngle(m_angle - angle_vel * dt);
    }
    if (m_is_shooting_laser)
    {
        m_laser_timer -= dt;
        if (m_laser_timer <= 0.)
        {
            m_laser_timer = 0.;
            m_is_shooting_laser = false;
        }
    }

    bool is_boosting = booster == BoosterState::Boosting;
    auto acc = acceleration + 2 * acceleration * is_boosting;
    speed += acc * dt;
    m_vel = (speed)*utils::angle2dir(m_angle);
    // utilss::truncate(m_vel, (!is_boosting) * max_speed + is_boosting * boost_max_speed);
    if (m_deactivated_time > 0)
    {
        m_deactivated_time -= dt;
        m_vel /= 2.f;
        booster = BoosterState::Ready;
    }
    m_pos += m_vel * dt;
    //! speed fallout
    if (booster != BoosterState::Boosting && speed > max_speed)
    {
        speed -= speed * 1.1 * slowing_factor * dt;
    }
    if (booster == BoosterState::Boosting && speed > boost_max_speed)
    {
        speed -= speed * 0.5 * slowing_factor * dt;
    }
    else
    {
        // speed -=  10. * slowing_factor * dt;
        // speed = std::max(0.f, speed);
    }

    m_particles_left->setSpawnPos(m_pos - m_size.x / 2. * utils::angle2dir(m_angle + 40));
    m_particles_left->update(dt);
    m_particles_right->setSpawnPos(m_pos - m_size.x / 2. * utils::angle2dir(m_angle - 40));
    m_particles_right->update(dt);
}

void PlayerEntity::onCollisionWith(GameObject &obj, CollisionData &c_data)
{
    switch (obj.getType())
    {
    case ObjectType::Meteor:
    {
        auto mvt = c_data.separation_axis;
        if (dot(mvt, m_vel) > 0.f)
        {
            m_vel -= 2.f * dot(mvt, m_vel) * mvt;
            m_angle = utils::dir2angle(m_vel);
            health -= 1;
        }
        break;
    }
    case ObjectType::Explosion:
    {
        health -= 0.5f;
        break;
    }
    case ObjectType::Bullet:
    {
        health--;
        break;
    }
    case ObjectType::Heart:
    {
        health += 3;
        break;
    }
    }
}

void PlayerEntity::draw(LayersHolder &layers)
{

    auto &target = layers.getCanvas("Unit");
    auto &shiny_target = layers.getCanvas("Bloom");

    // shiny_target.drawSprite(m_player_shape, "boostBar");

    m_player_shape.setPosition(m_pos);
    m_player_shape.setScale(m_size / 2.f);
    m_player_shape.setRotation(glm::radians(m_angle));
    m_player_shape.setTexture(*m_textures->get("PlayerShip"));
    target.drawSprite(m_player_shape);

    if (booster == BoosterState::Boosting)
    {
        m_particles_left->setInitColor({50., 1, 0, 1.0});
        m_particles_right->setInitColor({50., 1, 0, 1.0});
    }
    else
    {
        m_particles_left->setInitColor({400., 0., 0, 0.2});
        m_particles_right->setInitColor({400., 0., 0, 0.2});
    }

    // m_particles_left->draw(shiny_target);
    // m_particles_right->draw(shiny_target);

    if (m_shield_id != -1)
    {
        auto &anim = m_world->m_systems.get<AnimationComponent>(getId());
        Sprite shield;
        shield.setTexture(anim.texture_id, 0);
        shield.m_tex_rect = anim.tex_rect;
        shield.m_tex_size = anim.texture_size;
        shield.setScale(m_size * 1.4f);
        shield.setRotation(glm::radians(m_angle - 90));
        shield.setPosition(m_pos);
        target.drawSprite(shield);
    }

    drawShape(target, meteor_detector);
}

void PlayerEntity::activateShield()
{

    CollisionComponent shield_comp;
    shield_comp.type = ObjectType::Shield;
    Polygon shape_left;
    Polygon shape_right;
    shape_left.points = {{1.2f, 0.f}, {-0.2f, 0.9f}, {-0.2f, 0.3f}};
    shape_right.points = {{1.2f, 0.f}, {-0.2f, -0.3f}, {-0.2f, -0.9f}};
    shield_comp.shape.convex_shapes.push_back(shape_right);
    shield_comp.shape.convex_shapes.push_back(shape_left);
    auto &shield_obj = m_world->addObject3(ObjectType::Shield);
    addChild(&shield_obj);
    shield_obj.setSize(m_size * 4.f);
    shield_obj.m_collision_resolvers[ObjectType::Bullet] = [&shield_comp](auto &bullet, auto &c_data)
    {
        bullet.kill();
    };
    shield_obj.m_collision_resolvers[ObjectType::Meteor] = [&shield_comp, &shield_obj](GameObject &meteor, CollisionData &c_data)
    {
        //! bounce the meteor away
        utils::Vector2f rel_vel = shield_obj.m_parent->m_vel - meteor.m_vel;
        if(utils::dot(rel_vel, c_data.separation_axis) > 0.f) //! if moving into meteor
        {
            static_cast<Meteor&>(meteor).m_impulse_vel = 2. * c_data.separation_axis * utils::norm(shield_obj.m_parent->m_vel);
        }
    };
    
    m_shield_id = shield_obj.getId();

    TimedEventComponent time_comp;
    time_comp.addEvent({0.1f, [this](float t, int n){
        shield_timeleft = shield_lifetime - t;
    }, (int)(shield_lifetime  / 0.1f)});
    time_comp.addEvent({shield_lifetime, [this](float t, int n){
        deactivateShield();
    },1});

    m_world->m_systems.addEntityDelayed(shield_obj.getId(), shield_comp, time_comp);
    shield_active = true;
}

void PlayerEntity::deactivateShield()
{
    if(m_shield_id != -1)
    {
        shield_active = false;
        m_world->get(m_shield_id)->kill();
        m_shield_id = -1;
    }
}

void PlayerEntity::onBoostDown()
{
    if (m_fuel > 0. && booster != BoosterState::CoolingDown && booster != BoosterState::Disabled)
    {
        booster = BoosterState::Boosting;
    }
}
void PlayerEntity::onBoostUp()
{
    if (booster != BoosterState::CoolingDown && booster != BoosterState::Disabled)
    {
        booster = BoosterState::Ready;
    }
}

void PlayerEntity::onCreation()
{
    m_size = 12.;

    CollisionComponent c_comp;
    Polygon shape = {4};
    c_comp.shape.convex_shapes.push_back(shape);
    c_comp.type = ObjectType::Player;

    HealthComponent h_comp = {100, 100, 1.};
    AnimationComponent a_comp = {.id = AnimationId::FrontShield2, .cycle_duration = 0.25f};

    m_world->m_systems.addEntity(getId(), c_comp, h_comp, a_comp);

    m_particles_left = std::make_unique<Particles>(100);
    m_particles_right = std::make_unique<Particles>(100);

    auto basic_emitter = [this](utils::Vector2f spawn_pos)
    {
        Particle new_part;
        new_part.pos = spawn_pos;
        new_part.vel = 0. * m_vel - 0.8 * utils::angle2dir(m_angle - 180 + randf(-60, 60));
        return new_part;
    };

    auto basic_updater = [](Particle &part, float dt)
    {
        part.vel *= 0.03 * dt;
        part.pos = part.pos + part.vel * dt;
        part.scale += utils::Vector2f{5.15, 5.155} * dt;
    };

    m_particles_left->setEmitter(basic_emitter);
    m_particles_right->setEmitter(basic_emitter);
    m_particles_left->setUpdater(basic_updater);
    m_particles_right->setUpdater(basic_updater);

    m_particles_left->setInitColor({2.5, 1., 0, 1.0});
    m_particles_right->setInitColor({2.5, 1., 0, 0.1});
    m_particles_left->setFinalColor({2.5, 500.3, 0, 0.0});
    m_particles_right->setFinalColor({2.5, 500.3, 0, 0.0});
}
void PlayerEntity::onDestruction()
{
    GameObject::onDestruction();
}

void PlayerEntity::fixAngle()
{
    if (m_angle < -180.f)
    {
        m_angle = 180.f;
    }
    else if (m_angle > 180.f)
    {
        m_angle = -180.f;
    }
}

void PlayerEntity::boost(float dt)
{
    if (booster == BoosterState::Boosting && m_fuel > 0)
    {
        boost_heat += 30. * dt;
        m_fuel -= 5. * dt;

        if (boost_heat > max_boost_heat)
        {
            booster = BoosterState::Disabled;
        }
    }
    if (booster != BoosterState::Boosting)
    {
        boost_heat -= 10. * dt;
    }

    if (m_fuel < 0)
    {
        m_fuel = 0.;
        booster = BoosterState::Disabled;
    }

    if (boost_heat < 0.)
    {
        boost_heat = 0.;
        booster = BoosterState::Ready;
    }
}

float PlayerEntity::getHp() const
{
    if (m_world->m_systems.has<HealthComponent>(getId()))
    {
        return m_world->m_systems.get<HealthComponent>(getId()).hp;
    }
    return 0.;
}

float PlayerEntity::getHpRatio() const
{
    if (m_world->m_systems.has<HealthComponent>(getId()))
    {
        auto &comp = m_world->m_systems.get<HealthComponent>(getId());
        return comp.hp / comp.max_hp;
    }
    return 0.;
}