#include "GameWorld.h"

#include <chrono>

#include "Utils/RandomTools.h"

GameWorld::GameWorld()
{
    loadTextures();

    m_neighbour_searcher = std::make_unique<GridNeighbourSearcher>();

    m_effect_factories[EffectType::ParticleEmiter] =
        [this]()
    { return std::make_shared<StarEmitter>(this, m_textures); };
    m_effect_factories[EffectType::AnimatedSprite] =
        [this]()
    { return std::make_shared<AnimatedSprite>(this, m_textures); };
}

std::size_t GameWorld::getNActiveEntities(ObjectType type)
{
    auto &entities = m_entities.getObjects();
    auto n_enemies = std::count_if(entities.begin(), entities.end(), [type](auto &obj)
                                   { return obj->getType() == type; });
    return n_enemies;
}

GameObject &GameWorld::addObject(ObjectType type)
{
    std::shared_ptr<GameObject> new_object;

    switch (type)
    {
    case ObjectType::Trigger:
        new_object = std::make_shared<ReachPlace>(this, m_textures, m_player);
        break;
        // default:
        //     throw std::runtime_error("You forgot to add the new object here!");
    }

    m_to_add.push(new_object);
    return *m_to_add.back();
}

void GameWorld::addQueuedEntities()
{
    while (!m_to_add.empty())
    {
        auto new_object = m_to_add.front();
        auto new_id = m_entities.addObject(new_object);
        m_entities.at(new_id)->m_id = new_id;
        if (m_entities.at(new_id)->collides())
        {
            m_collision_system.insertObject(*m_entities.at(new_id));
        }

        m_entities.at(new_id)->onCreation();
        m_to_add.pop();
    }
}

void GameWorld::removeQueuedEntities()
{
    while (!m_to_destroy.empty())
    {
        auto object = m_to_destroy.front();
        object->onDestruction();

        //! notify observers that entity got destroyed
        for (auto callback_id : m_entitydestroyed_events.getEntityIds())
        {
            m_entitydestroyed_events.at(callback_id)(object->getType(), object->getId());
        }

        if (object->collides())
        {
            m_collision_system.removeObject(*object);
        }
        m_entities.remove(object->getId());
        m_to_destroy.pop();
    }
}

void GameWorld::destroyObject(int entity_id)
{
    m_to_destroy.push(m_entities.at(entity_id));
}

void GameWorld::update(float dt)
{

    // m_collision_system.update();

    for (auto &obj : m_entities.getObjects())
    {
        obj->updateAll(dt);
        obj->update(dt);
        if (obj->isDead())
        {
            destroyObject(obj->getId());
        }
    }

    addQueuedEntities();
    removeQueuedEntities();
}

void GameWorld::draw(LayersHolder &layers)
{
    for (auto &obj : m_entities.getObjects())
    {
        obj->draw(layers);
    }
}

VisualEffect &GameWorld::addVisualEffect(EffectType type)
{
    assert(m_effect_factories.count(type) > 0);

    auto new_effect = m_effect_factories.at(type)();
    m_to_add.push(new_effect);
    return *new_effect;
}

int GameWorld::addEntityDestroyedCallback(std::function<void(ObjectType, int)> callback)
{
    return m_entitydestroyed_events.addObject(callback);
}

void GameWorld::removeEntityDestroyedCallback(int callback_id)
{
    m_entitydestroyed_events.remove(callback_id);
}

void GameWorld::loadTextures()
{
    m_textures.setBaseDirectory(std::string(RESOURCES_DIR) + "/Textures/");
    m_textures.add("Bomb", "bomb.png");
    m_textures.add("EnemyShip", "EnemyShip.png");
    m_textures.add("EnemyLaser", "EnemyLaser.png");
    m_textures.add("EnemyBomber", "EnemyBomber.png");
    m_textures.add("Meteor", "Meteor.png");
    m_textures.add("BossShip", "BossShip.png");
    m_textures.add("Explosion2", "explosion2.png");
    m_textures.add("Explosion", "explosion.png");
    m_textures.add("PlayerShip", "playerShip.png");
    m_textures.add("Heart", "Heart.png");
    m_textures.add("Station", "Station.png");
    m_textures.add("BoosterYellow", "effectYellow.png");
    m_textures.add("BoosterPurple", "effectPurple.png");
    m_textures.add("Arrow", "arrow.png");
    m_textures.add("Emp", "emp.png");
    m_textures.add("Star", "star.png");
    m_textures.add("Fuel", "fuel.png");
    m_textures.add("FireNoise", "fireNoise.png");
    m_textures.add("Turrets", "Turrets.png");
    m_textures.add("Arrow", "arrow.png");
}