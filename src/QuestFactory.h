#pragma once

#include "ObjectiveSystem.h"
#include "UISystem.h"
#include "GameWorld.h"

#include "Entities/Factories.h"

#include "Utils/RandomTools.h"

enum class QuestType
{
    TimeRace1,
    BossFight1,
};

class QuestFactory
{

public:
    QuestFactory(ObjectiveSystem &objective, PostOffice &messanger, UISystem &ui, GameWorld &world, TextureHolder &textures)
        : m_objectives(objective), m_messanger(messanger), m_ui_system(ui), m_world(world), m_enemy_factory(world, textures)
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

            for (int i = 0; i < objective_count - 1; ++i)
            {
                auto &spot = m_world.addTrigger<ReachPlace>();
                spot.setSize({10});
                spot.setPosition(m_world.m_player->getPosition() + 1000.f * angle2dir(randf(60, 150)));

                auto objective = std::make_shared<ReachSpotTask>(spot, *m_font, m_messanger, quest.get());

                objective->m_on_activation = [this, objective, prev_objective, add_timer] ()
                {
                    add_timer(prev_objective->m_timer_component->getTimeLeft() + 30.f, *objective);
                };
                spot.attach(objective);

                quest->addTask(objective, prev_objective.get());
                prev_objective->m_on_completion_callback = [&spot, prev_spot, spawn_enemies]()
                {
                    spot.activate();
                    spawn_enemies(prev_spot->getPosition(), 5);
                };
                prev_spot = &spot;
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
            quest->m_on_start = [this](Quest& q)
            {
                m_ui_system.addTimeBar();
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
    Font *m_font;
    // TimedEventMa
};
