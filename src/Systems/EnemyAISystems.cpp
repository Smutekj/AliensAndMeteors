#include "EnemyAISystems.h"

#include "../GameWorld.h"

#include "../Utils/RandomTools.h"

AISystem::AISystem(GameWorld &world,
                   ContiguousColony<ShootPlayerAIComponent, int> &comps,
                   ContiguousColony<LaserAIComponent, int> &l_comps)
    : m_world(world),
      m_bullet_factory(world, world.m_textures),
      m_laser_factory(world, world.m_textures),
      m_shooters(comps),
      m_laser_shooters(l_comps)
{
    initializeShooterAI();
    initializeLaserShooterAI();
}

void AISystem::preUpdate(float dt, EntityRegistryT &entities)
{
    auto &comps = m_shooters.data;
    auto &ids = m_shooters.data_ind2id;
    for (std::size_t comp_id = 0; comp_id < comps.size(); ++comp_id)
    {
        comps[comp_id].pos = entities.at(ids.at(comp_id))->getPosition();
    }
    auto &lcomps = m_laser_shooters.data;
    auto &lids = m_laser_shooters.data_ind2id;
    for (std::size_t comp_id = 0; comp_id < lcomps.size(); ++comp_id)
    {
        lcomps[comp_id].pos = entities.at(lids.at(comp_id))->getPosition();
    }
}
void AISystem::update(float dt)
{
    auto &comps = m_shooters.data;
    auto &ids = m_shooters.data_ind2id;
    for (std::size_t comp_id = 0; comp_id < comps.size(); ++comp_id)
    {
        auto &comp = comps[comp_id];
        for (auto &timer : comp.timers)
        {
            timer.update(dt);
        }
        updateShooterAI(comp, ids.at(comp_id));
    }

    auto &lcomps = m_laser_shooters.data;
    auto &l_ids = m_laser_shooters.data_ind2id;
    for (std::size_t comp_id = 0; comp_id < lcomps.size(); ++comp_id)
    {
        auto &comp = lcomps[comp_id];
        for (auto &timer : comp.timers)
        {
            timer.update(dt);
        }
        updateLaserAI(comp, l_ids.at(comp_id));
    }
}

void AISystem::initializeLaserShooterAI()
{
    m_change_state_callbacks_laser[ShooterAIState::FollowingPlayer] = [this](LaserAIComponent &comp, int id)
    {
        comp.timers.clear();
        //! DO NOT USE REFERENCES TO COMPONENTS FOR THE LOVE OF GOD!!!! THEY GET INVALIDATED
        auto shoot_laser = [this, id](float t, int count)
        {
            auto &comp = m_world.m_systems.get<LaserAIComponent>(id);
            auto &laser = m_laser_factory.create2(comp.laser_type, comp.pos, {255, 25, 0, 255});
            auto dr_to_player = m_world.m_player->getPosition() - comp.pos;
            auto shooter_entity = m_world.get(id);

            shooter_entity->addChild(&laser);

            auto old_max_vel = shooter_entity->m_max_vel;
            auto old_max_acc = shooter_entity->m_max_acc;
            shooter_entity->m_max_vel *= 0.5;
            shooter_entity->m_max_acc *= 0.01;
            laser.setDestructionCallback([shooter_entity, id, old_max_vel, old_max_acc](int laser_id, ObjectType t)
                                         {
            shooter_entity->m_max_vel = old_max_vel;
            shooter_entity->m_max_acc = old_max_acc; });

            laser.m_life_time = comp.laser_time;
            laser.m_max_length = comp.laser_range;
            laser.setAngle(utils::dir2angle(dr_to_player));

            changeState(comp, id, ShooterAIState::Shooting);
        };
        comp.timers.push_back({0.5f, shoot_laser, 1});
    };
    m_change_state_callbacks_laser[ShooterAIState::Shooting] = [this](LaserAIComponent &comp, int id)
    {
        comp.timers.clear();
        comp.timers.push_back({comp.laser_time, [this, id](float t, int count)
                               {
                                   auto &comp = m_world.m_systems.get<LaserAIComponent>(id);
                                   changeState(comp, id, ShooterAIState::Searching);
                               },
                               1});
    };

    m_change_state_callbacks_laser[ShooterAIState::Searching] = [this](LaserAIComponent &comp, int id)
    {
        comp.timers.clear();
        comp.timers.push_back({12.69, [this, id](float t, int count)
                               {
                                   auto &comp = m_world.m_systems.get<LaserAIComponent>(id);
                                   auto &target_comp = m_world.m_systems.get<TargetComponent>(id);
                                   target_comp.p_target = nullptr;
                                   target_comp.target_pos = comp.pos + 269.f * utils::angle2dir(randf(0, 360));
                               },
                               TimedEventType::Infinite});
    };
}

void AISystem::initializeShooterAI()
{
    
    m_change_state_callbacks[ShooterAIState::FollowingPlayer] = [this](ShootPlayerAIComponent &comp, int id)
    {
        comp.timers.clear();
        comp.timers.push_back({comp.cooldown, [this, id](float t, int count)
            {
                                   std::vector<ColorByte> proj_colors = {ColorByte{255,20,0,255}, ColorByte{20,255,0,255}, ColorByte{255,0,255,255}, ColorByte{20,20,255,255}};
                                   auto &comp = m_world.m_systems.get<ShootPlayerAIComponent>(id);
                                   auto &bullet = m_bullet_factory.create2(comp.projectile_type, comp.pos, proj_colors.at(rand()%proj_colors.size()));
                                   bullet.setTarget(m_world.m_player);
                                   bullet.m_collision_resolvers[ObjectType::Enemy] =
                                       [&bullet, this, id](GameObject &obj, CollisionData &c_data)
                                   {
                                       if (obj.getId() == id || bullet.getTime() < 1.f)
                                       {
                                           return;
                                       }
                                       bullet.kill();
                                       m_world.p_messenger->send(DamageReceivedEvent{ObjectType::Bullet, bullet.getId(),
                                                                                     ObjectType::Enemy, obj.getId(), 3.});
                                   };

                                   auto dr_to_player = m_world.m_player->getPosition() - comp.pos;
                                   bullet.setAngle(utils::dir2angle(dr_to_player));
                                   bullet.m_vel = (dr_to_player) / utils::norm(dr_to_player) * bullet.m_max_vel;
                               },
                               1});
    };
    m_change_state_callbacks[ShooterAIState::Shooting] = [](ShootPlayerAIComponent &comp, int id) {

    };
    m_change_state_callbacks[ShooterAIState::Escaping] = [this](ShootPlayerAIComponent &comp, int id)
    {
        comp.timers.clear();
        auto &target_comp = m_world.m_systems.get<TargetComponent>(id);
        target_comp.p_target = nullptr;
        target_comp.target_pos = comp.pos + 269.f * utils::angle2dir(randf(0, 360));

        comp.timers.push_back({10., [this, id](float t, int count)
                               {
                                   auto &target_comp = m_world.m_systems.get<TargetComponent>(id);
                                   auto &comp = m_world.m_systems.get<ShootPlayerAIComponent>(id);
                                   target_comp.target_pos = comp.pos + 269.f * utils::angle2dir(randf(0, 360));
                               },
                               2});

        comp.timers.push_back({30., [this, id, &comp, &target_comp](float t, int count)
                               {
                                   target_comp.p_target = m_world.m_player;
                                   changeState(comp, id, ShooterAIState::FollowingPlayer);
                               },
                               1});
    };
    m_change_state_callbacks[ShooterAIState::Searching] = [this](ShootPlayerAIComponent &comp, int id)
    {
        comp.timers.clear();
        comp.timers.push_back({12.69, [this, id](float t, int count)
                               {
                                   auto &comp = m_world.m_systems.get<ShootPlayerAIComponent>(id);
                                   auto &target_comp = m_world.m_systems.get<TargetComponent>(id);
                                   target_comp.p_target = nullptr;
                                   target_comp.target_pos = comp.pos + 269.f * utils::angle2dir(randf(0, 360));
                               },
                               TimedEventType::Infinite});
    };

    m_dmg_postbox = std::make_unique<PostBox<DamageReceivedEvent>>(*m_world.p_messenger, [this](const auto &messages)
                                                                   {
   for(const auto& msg : messages)
   {
       if(m_world.m_systems.has<ShootPlayerAIComponent>(msg.receiver_id))
       {
        //    changeState(m_shooters.get(msg.receiver_id), msg.receiver_id, ShooterAIState::Escaping);
       }
   } });
}

void AISystem::updateLaserAI(LaserAIComponent &comp, int id)
{
    utils::Vector2f dr_to_player = m_world.m_player->getPosition() - comp.pos;
    float dist_to_player = utils::norm(dr_to_player);
    bool player_in_range = dist_to_player < comp.vision_radius;
    if (comp.state == ShooterAIState::FollowingPlayer)
    {
        if (!player_in_range)
        {
            changeState(comp, id, ShooterAIState::Searching);
        }
    }
    else if (comp.state == ShooterAIState::Searching)
    {
        if (player_in_range)
        {
            changeState(comp, id, ShooterAIState::FollowingPlayer);
        }
    }
}

void AISystem::updateShooterAI(ShootPlayerAIComponent &comp, int id)
{
    utils::Vector2f dr_to_player = m_world.m_player->getPosition() - comp.pos;
    float dist_to_player = utils::norm(dr_to_player);
    bool player_in_range = dist_to_player < comp.vision_radius;
    if (comp.state == ShooterAIState::FollowingPlayer)
    {
        if (!player_in_range)
        {
            changeState(comp, id, ShooterAIState::Searching);
        }
    }
    else if (comp.state == ShooterAIState::Searching)
    {
        if (player_in_range)
        {
            changeState(comp, id, ShooterAIState::FollowingPlayer);
        }
    }
}

void AISystem::changeState(LaserAIComponent &comp, int id, ShooterAIState target_state)
{
    //! if the callback exists and an actual change happens
    if (m_change_state_callbacks_laser.contains(target_state) && target_state != comp.state)
    {
        m_change_state_callbacks_laser.at(target_state)(comp, id);
    }
    comp.state = target_state;
}

void AISystem::changeState(ShootPlayerAIComponent &comp, int id, ShooterAIState target_state)
{
    //! if the callback exists and an actual change happens
    if (m_change_state_callbacks.contains(target_state) && target_state != comp.state)
    {
        m_change_state_callbacks.at(target_state)(comp, id);
    }
    comp.state = target_state;
}