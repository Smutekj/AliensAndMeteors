#pragma once

#include "ObjectiveSystem.h"
#include "UISystem.h"
#include "GameWorld.h"

#include "Entities/Factories.h"
#include "Entities/Bosses.h"

#include "Utils/RandomTools.h"

enum class QuestType
{
    TimeRace1,
    BossFight1,
};

class QuestFactory
{

public:
    QuestFactory(ObjectiveSystem &objective, PostOffice &messanger, UISystem &ui, GameWorld &world, TextureHolder &textures, Camera &camera, Font& font)
        : m_objectives(objective), m_messanger(messanger),
          m_ui_system(ui), m_world(world),
          m_enemy_factory(world, textures),
          m_textures(textures), m_camera(camera), m_font(&font)
    {
        registerFactories();
    }

    std::shared_ptr<Quest> create(QuestType type)
    {
        if (m_creators.count(type) == 0)
        {
            return nullptr;
        }
        return m_creators.at(type)();
    }

private:
    void registerFactories()
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
            objective.m_timer_component = std::make_unique<TimedEvent>(timer_period, [this, time_limit, n_timer_ticks, &objective](float t, int n)
                                                                       {
                    if(n >= n_timer_ticks -1)
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
                    add_timer(prev_objective->m_timer_component->getTimeLeft() + 30.f, *objective);
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
            // m_stage = GameStage::BossFight;

            auto quest = std::make_shared<Quest>(m_messanger);

            auto build_the_wall = [this](utils::Vector2f boss_pos)
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

                auto center = boss_pos - utils::Vector2f{150, 0};
                wall_right.setPosition(center + utils::Vector2f{400, 0});
                wall_left.setPosition(center - utils::Vector2f{390, 0});
                wall_up.setPosition(center + utils::Vector2f{0, 290});
                wall_down.setPosition(center - utils::Vector2f{0, 290});

                wall_right.setSize({400, 300});
                wall_left.setSize({400, 300});
                wall_up.setSize({400, 300});
                wall_down.setSize({400, 300});

                SpriteComponent s_compl = {.layer_id = "Unit", .shader_id = "fuelBar", .sprite = Sprite(*m_textures.get("FireNoise"))};
                SpriteComponent s_compr = {.layer_id = "Unit", .shader_id = "fuelBar", .sprite = Sprite(*m_textures.get("FireNoise"))};
                SpriteComponent s_compu = {.layer_id = "Unit", .shader_id = "fuelBar", .sprite = Sprite(*m_textures.get("FireNoise"))};
                SpriteComponent s_compd = {.layer_id = "Unit", .shader_id = "fuelBar", .sprite = Sprite(*m_textures.get("FireNoise"))};

                m_world.m_systems.addEntityDelayed(wall_up.getId(), c_comp_up, s_compl);
                m_world.m_systems.addEntityDelayed(wall_left.getId(), c_comp_left, s_compr);
                m_world.m_systems.addEntityDelayed(wall_down.getId(), c_comp_down, s_compd);
                m_world.m_systems.addEntityDelayed(wall_right.getId(), c_comp_right, s_compu);
            };

            quest->m_on_start = [this, build_the_wall](Quest &q)
            {
                auto &player = *m_world.m_player;

                auto &spot1 = m_world.addTrigger<ReachPlace>();
                spot1.setPosition(player.getPosition() + 500.f * utils::angle2dir(randf(0, 360)));
                spot1.setSize(20);
                auto reach_spot_task = std::make_shared<ReachSpotTask>(spot1, *m_font, m_messanger, &q);
                spot1.attach(reach_spot_task);
                spot1.activate();

                reach_spot_task->m_on_completion_callback = [this, &q, &player, build_the_wall]()
                {
                    
                    auto &boss = m_world.addObject2<Boss1>();
                    auto player_pos = player.getPosition();
                    auto boss_pos = player.getPosition() + utils::Vector2f{100, 0};
                    boss.setPosition(boss_pos);
                    build_the_wall(boss_pos);

                    m_camera.startMovingTo(boss_pos - utils::Vector2f{150, 0}, 1., [](auto &camera)
                                           {
                        camera.m_view_state = Camera::MoveState::Fixed;
                        camera.startChangingSize({410, 310}, 2., [](auto& camera){camera.m_view_size_state = Camera::SizeState::Fixed;}); });

                    m_messanger.send(StartedBossFightEvent{boss.getId()});
                    auto kill_task = std::make_shared<DestroyEntityTask>(boss, *m_font, m_messanger, &q);
                    q.addTask(kill_task);
                };

                q.addTask(reach_spot_task);

            };
            quest->m_on_completion = [this]() {

            };

            return quest;
        };
    };

private:
    EnemyFactory m_enemy_factory;

    PostOffice &m_messanger;

    std::unordered_map<QuestType, std::function<std::shared_ptr<Quest>()>> m_creators;
    ObjectiveSystem &m_objectives;
    UISystem &m_ui_system;
    GameWorld &m_world;
    Camera &m_camera;
    Font *m_font;
    TextureHolder &m_textures;
    // TimedEventMa
};
