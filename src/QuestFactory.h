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
    Survival1,
    BossFight1,
    BossFight2,
};

class Camera;
class TimedEventManager;
class UISystem;
class ObjectiveSystem;
class Font;

class QuestFactory
{

public:
    QuestFactory(ObjectiveSystem &objective, PostOffice &messanger,
                 UISystem &ui, GameWorld &world,
                 TextureHolder &textures, Camera &camera,
                 Font &font, TimedEventManager &timers);

    std::shared_ptr<Quest> create(QuestType type);

private:
    void registerFactories();

private:
    template <class BossType>
    std::shared_ptr<Quest> createBossAndQuest(utils::Vector2f boss_offset);
    std::vector<int> buildTheWall(utils::Vector2f center, utils::Vector2f size);
    
private:
std::unordered_map<QuestType, std::function<std::shared_ptr<Quest>()>> m_creators;

    EnemyFactory m_enemy_factory;

    PostOffice &m_messanger;
    ObjectiveSystem &m_objectives;
    UISystem &m_ui_system;
    GameWorld &m_world;
    Camera &m_camera;
    Font *m_font;
    TextureHolder &m_textures;
    TimedEventManager &m_timers;
    // TimedEventMa
};
