#include "QuestFactory.h"

#include "ObjectiveSystem.h"
#include "UISystem.h"
#include "GameWorld.h"

#include "Entities/Factories.h"
#include "Entities/Bosses.h"

#include "Utils/RandomTools.h"
#include "Camera.h"
#include "Systems/TimedEventManager.h"

QuestFactory::QuestFactory(ObjectiveSystem &objective, PostOffice &messanger, UISystem &ui, GameWorld &world, TextureHolder &textures, Camera &camera, Font &font, TimedEventManager &timers)
    : m_objectives(objective), m_messanger(messanger),
      m_ui_system(ui), m_world(world),
      m_enemy_factory(world, textures),
      m_textures(textures), m_camera(camera), m_font(&font),
      m_timers(timers)
{
    registerFactories();
}

std::shared_ptr<Quest> QuestFactory::create(QuestType type)
{
    if (m_creators.count(type) == 0)
    {
        return nullptr;
    }
    return m_creators.at(type)();
}

void QuestFactory::registerFactories()
{
    using namespace utils;
    auto spawn_enemies = [this](utils::Vector2f pos, int count)
    {
        for (int i = 0; i < count; ++i)
        {
            m_enemy_factory.create2(EnemyType::ShooterEnemy, pos + randf(100, 200) * utils::angle2dir(randf(0, 360)));
            m_enemy_factory.create2(EnemyType::EnergyShooter, pos + randf(100, 200) * utils::angle2dir(randf(0, 360)));
        }
    };

    auto add_timer = [this](float time_limit, Task &objective)
    {
        float timer_period = 0.1f;
        int n_timer_ticks = (int)(time_limit / timer_period);
        objective.m_timer_component.emplace_back(timer_period, [this, time_limit, n_timer_ticks, &objective](float t, int n)
                                                 {
                                                         if (n >= n_timer_ticks - 1)
                                                         {
                                                             objective.fail();
                                                             return;
                                                         }

                                                         auto el = m_ui_system.getTextElement("TimerBar");
                                                         if (el)
                                                         {
                                                             int seconds_left = (int)std::floor(time_limit - t);
                                                             int minutes_left = seconds_left / 60;
                                                             std::string minutes_string = minutes_left < 10 ? "0" + std::to_string(minutes_left) : std::to_string(minutes_left);
                                                             std::string seconds_string = seconds_left % 60 < 10 ? "0" + std::to_string(seconds_left % 60) : std::to_string(seconds_left % 60);
                                                             el->m_text.setText(minutes_string + ":" + seconds_string);
                                                         }
                                                         else
                                                         {
                                                             m_ui_system.addTimeBar();
                                                         } }, n_timer_ticks);
    };

    m_creators[QuestType::TimeRace1] = [this, spawn_enemies, add_timer]() -> std::shared_ptr<Quest>
    {
        std::shared_ptr<Quest> quest = std::make_shared<Quest>(m_messanger);

        auto player_pos = m_world.m_player->getPosition();
        float m_timerace_timer = 90.;
        int objective_count = 4;

        //! update timer in UI preiodically
        auto &spot1 = m_world.addTrigger<ReachPlace>();
        spot1.setPosition(m_world.m_player->getPosition() + 1000.f * angle2dir(randf(0, 150)));
        auto prev_objective = std::make_shared<ReachSpotTask>(spot1, *m_font, m_messanger, quest.get());
        spot1.attach(prev_objective);
        //! create Timer in UI on reaching first spot
        add_timer(60.f, *prev_objective);

        quest->addTask(prev_objective);
        spot1.activate();
        auto *prev_spot = &spot1;
        utils::Vector2f prev_position = spot1.getPosition();

        for (int i = 0; i < objective_count - 1; ++i)
        {
            auto &spot = m_world.addTrigger<ReachPlace>();
            spot.setSize({10});
            spot.setPosition(prev_position + 1000.f * angle2dir(randf(60, 150)));

            auto objective = std::make_shared<ReachSpotTask>(spot, *m_font, m_messanger, quest.get());

            objective->m_on_activation = [this, objective, prev_objective, add_timer]()
            {
                add_timer(prev_objective->m_timer_component.at(0).getTimeLeft() + 30.f, *objective);
            };
            spot.attach(objective);

            quest->addTask(objective, prev_objective.get());
            prev_objective->m_on_completion_callback = [&spot, prev_position, spawn_enemies]()
            {
                spot.activate();
                spawn_enemies(prev_position, 5);
            };
            prev_position = spot.getPosition();
            prev_objective = objective;
        }

        quest->m_on_completion = [this, spawn_enemies]
        {
            spawn_enemies(m_world.m_player->getPosition(), 10);
            auto &star_effect = m_world.addVisualEffect(EffectType::ParticleEmiter);
            star_effect.setPosition(m_world.m_player->getPosition());
            star_effect.setLifeTime(2.f);

            m_world.m_player->m_money += 100;
            m_ui_system.removeTimeBar();
        };
        quest->m_on_start = [this](Quest &q)
        {
            m_ui_system.addTimeBar();
        };

        return quest;
    };
    m_creators[QuestType::BossFight1] = [this, spawn_enemies, add_timer]() -> std::shared_ptr<Quest>
    {
        return createBossAndQuest<Boss1>({150.f, 0.f});
    };
    m_creators[QuestType::BossFight2] = [this, spawn_enemies, add_timer]() -> std::shared_ptr<Quest>
    {
        return createBossAndQuest<Boss2>({0.f, 150.f});
    };
    m_creators[QuestType::Survival1] = [this, spawn_enemies, add_timer]() -> std::shared_ptr<Quest>
    {
        float time_limit = 5.f;

        auto quest = std::make_shared<Quest>(m_messanger);
        auto task = std::make_shared<SurviveTask>(*m_world.m_player, m_messanger, quest.get());
        task->m_timer_component.emplace_back(time_limit, [task](float t, int c)
                                             { task->complete(); }, 1);

        int time_updater_id = m_timers.addTimedEvent(0.1f, [this, time_limit](float t, int c)
                                                     {
                auto el = m_ui_system.getTextElement("TimerBar");
                    if (el)
                    {
                        int seconds_left = (int)std::floor(time_limit - t);
                        int minutes_left = seconds_left / 60;
                        std::string minutes_string = minutes_left < 10 ? "0" + std::to_string(minutes_left) : std::to_string(minutes_left);
                        std::string seconds_string = seconds_left % 60 < 10 ? "0" + std::to_string(seconds_left % 60) : std::to_string(seconds_left % 60);
                        el->m_text.setText(minutes_string + ":" + seconds_string);
                    }else{
                        m_ui_system.addTimeBar();
                    } }, (int)(time_limit / 0.1f));

        auto finish_tiemr_id = m_timers.addTimedEvent(time_limit, [this, task](float t, int c)
                                                      { task->complete(); }, 1);

        auto spawner_timer_id = m_timers.addInfiniteEvent(1.f, [this](float t, int c)
                                                          {
                std::vector<EnemyType> types{EnemyType::LaserEnemy, EnemyType::ShooterEnemy, EnemyType::EnergyShooter};
                utils::Vector2f spawn_pos = m_world.m_player->getPosition() + utils::Vector2f{randf(-300.f, 300.f), randf(-300.f, 300.f)};
                auto& e = m_enemy_factory.create2(randomValue(types), spawn_pos);
                e.m_max_vel = 150.f; });

        auto wall_ids = buildTheWall(m_world.m_player->getPosition(), {1000, 1000});

        task->m_on_completion_callback = [time_updater_id, wall_ids, spawner_timer_id, this]()
        {
            for (auto id : wall_ids)
            {
                m_world.get(id)->kill();
            }
            m_ui_system.removeTimeBar();
            m_timers.removeEvent(spawner_timer_id);
        };
        task->m_on_failure_callback = [time_updater_id, wall_ids, spawner_timer_id, this]()
        {
            for (auto id : wall_ids)
            {
                m_world.get(id)->kill();
            }
            m_ui_system.removeTimeBar();
            m_timers.removeEvent(spawner_timer_id);
        };
        quest->addTask(task);

        quest->start();
        return quest;
    };
};

template <class BossType>
std::shared_ptr<Quest> QuestFactory::createBossAndQuest(utils::Vector2f boss_offset)
{
    auto quest = std::make_shared<Quest>(m_messanger);

    quest->m_on_start = [this, boss_offset](Quest &q)
    {
        auto &player = *m_world.m_player;

        auto &spot1 = m_world.addTrigger<ReachPlace>();
        spot1.setPosition(player.getPosition() + 500.f * utils::angle2dir(randf(0, 360)));
        spot1.setSize(20);
        auto reach_spot_task = std::make_shared<ReachSpotTask>(spot1, *m_font, m_messanger, &q);
        spot1.attach(reach_spot_task);
        spot1.activate();

        reach_spot_task->m_on_completion_callback = [this, &q, &player, boss_offset]()
        {
            auto &boss = m_world.addObject2<BossType>();
            auto player_pos = player.getPosition();
            auto boss_pos = player.getPosition() + boss_offset;

            boss.setPosition(boss_pos);
            buildTheWall(player.getPosition(), {400, 300});

            m_camera.startMovingTo(player.getPosition(), 1., [](auto &camera)
                                   {
                    camera.m_view_state = Camera::MoveState::Fixed;
                    camera.startChangingSize({410, 310}, 2., [](auto& camera){camera.m_view_size_state = Camera::SizeState::Fixed;}); });

            m_messanger.send(StartedBossFightEvent{boss.getId()});
            auto kill_task = std::make_shared<DestroyEntityTask>(boss, *m_font, m_messanger, &q);
            q.addTask(kill_task);

            auto &entities = m_world.getEntities();
            for (auto obj : entities.data())
            {
                if (obj->getType() == ObjectType::Meteor)
                {
                    obj->kill();
                }
            }
        };

        q.addTask(reach_spot_task);
    };
    quest->m_on_completion = [this]() {

    };

    return quest;
}

std::vector<int> QuestFactory::buildTheWall(utils::Vector2f center, utils::Vector2f size)
{
    //! create walls
    auto &wall_up = m_world.addObject3(ObjectType::Wall);
    auto &wall_down = m_world.addObject3(ObjectType::Wall);
    auto &wall_left = m_world.addObject3(ObjectType::Wall);
    auto &wall_right = m_world.addObject3(ObjectType::Wall);

    CollisionComponent c_comp_left;
    CollisionComponent c_comp_right;
    CollisionComponent c_comp_up;
    CollisionComponent c_comp_down;
    c_comp_left.type = ObjectType::Wall;
    c_comp_right.type = ObjectType::Wall;
    c_comp_up.type = ObjectType::Wall;
    c_comp_down.type = ObjectType::Wall;

    c_comp_left.shape.convex_shapes = {4};
    c_comp_right.shape.convex_shapes = {4};
    c_comp_up.shape.convex_shapes = {4};
    c_comp_down.shape.convex_shapes = {4};

    wall_right.setPosition(center + utils::Vector2f{size.x, 0});
    wall_left.setPosition(center - utils::Vector2f{size.x, 0});
    wall_up.setPosition(center + utils::Vector2f{0, size.y});
    wall_down.setPosition(center - utils::Vector2f{0, size.y});

    wall_right.setSize(size);
    wall_left.setSize(size);
    wall_up.setSize(size);
    wall_down.setSize(size);

    SpriteComponent s_compl = {.layer_id = "Unit", .shader_id = "fuelBar", .sprite = Sprite(*m_textures.get("FireNoise"))};
    SpriteComponent s_compr = {.layer_id = "Unit", .shader_id = "fuelBar", .sprite = Sprite(*m_textures.get("FireNoise"))};
    SpriteComponent s_compu = {.layer_id = "Unit", .shader_id = "fuelBar", .sprite = Sprite(*m_textures.get("FireNoise"))};
    SpriteComponent s_compd = {.layer_id = "Unit", .shader_id = "fuelBar", .sprite = Sprite(*m_textures.get("FireNoise"))};

    m_world.m_systems.addEntityDelayed(wall_up.getId(), c_comp_up, s_compl);
    m_world.m_systems.addEntityDelayed(wall_left.getId(), c_comp_left, s_compr);
    m_world.m_systems.addEntityDelayed(wall_down.getId(), c_comp_down, s_compd);
    m_world.m_systems.addEntityDelayed(wall_right.getId(), c_comp_right, s_compu);

    return {wall_left.getId(), wall_right.getId(), wall_down.getId(), wall_up.getId()};
};
