#include "GameWorld.h"

#include <chrono>

#include <SFML/Graphics/Texture.hpp>

#include "Entities/Entities.h"
#include "Entities/Enemy.h"
#include "Entities/Player.h"
#include "Entities/Attacks.h"

GameWorld::GameWorld()
{

    for (int i = 0; i < 500; ++i)
    {
        auto &meteor = addObject(ObjectType::Meteor);
    }

    loadTextures();
   
    m_neighbour_searcher = std::make_unique<GridNeighbourSearcher>();

    m_effect_factories[EffectType::ParticleEmiter] =
        [this]()
    { return std::make_shared<StarEmitter>(this, m_textures); };
}

GameObject &GameWorld::addObject(ObjectType type)
{
    std::shared_ptr<GameObject> new_object;

    switch (type)
    {
    case ObjectType::Enemy:
        new_object = std::make_shared<Enemy>(this, m_textures, m_collision_system, *m_neighbour_searcher, m_player);
        break;
    case ObjectType::Meteor:
        new_object = std::make_shared<Meteor>(this, m_textures);
        break;
    case ObjectType::Bullet:
        new_object = std::make_shared<Bullet>(this, m_textures, m_player);
        break;
    case ObjectType::Laser:
        new_object = std::make_shared<Laser>(this, m_textures, m_collision_system);
        break;
    case ObjectType::Bomb:
        new_object = std::make_shared<Bomb>(this, m_textures, m_collision_system);
        break;
    case ObjectType::Explosion:
        new_object = std::make_shared<Explosion>(this, m_textures);
        break;
    case ObjectType::Player:
    {
        new_object = std::make_shared<PlayerEntity>(this, m_textures);
        m_player = &static_cast<PlayerEntity &>(*new_object);
        break;
    }
    case ObjectType::Heart:
        new_object = std::make_shared<Heart>(this, m_textures);
        break;
    case ObjectType::SpaceStation:
        new_object = std::make_shared<SpaceStation>(this, m_textures);
        break;
    case ObjectType::Boss:
        new_object = std::make_shared<Boss>(this, m_textures, m_player);
        break;
    case ObjectType::Trigger:
        new_object = std::make_shared<ReachPlace>(this, m_textures, m_player);
        break;
    case ObjectType::EMP:
        new_object = std::make_shared<EMP>(this, m_textures, &m_collision_system);
        break;
    default:
        throw std::runtime_error("You forgot to add the new object here!");
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
            m_collision_system.insertObject(m_entities.at(new_id));
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

    m_collision_system.update();

    for (auto &obj : m_entities.getObjects())
    {
        obj->updateAll(dt);
        if (obj->isDead())
        {
            destroyObject(obj->getId());
        }
    }

    addQueuedEntities();
    removeQueuedEntities();
}

void GameWorld::draw(sf::RenderTarget &bloomy_target, sf::RenderTarget &window)
{
    for (auto &obj : m_entities.getObjects())
    {
        if (obj->isBloomy())
        {
            obj->draw(bloomy_target);
        }
        else
        {
            obj->draw(window);
        }
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
     m_textures.load(Textures::ID::Bomb, "../Resources/bomb.png");
    m_textures.load(Textures::ID::EnemyShip, "../Resources/EnemyShip.png");
    m_textures.load(Textures::ID::BossShip, "../Resources/BossShip.png");
    m_textures.load(Textures::ID::Explosion2, "../Resources/explosion2.png");
    m_textures.load(Textures::ID::Explosion, "../Resources/explosion.png");
    m_textures.load(Textures::ID::PlayerShip, "../Resources/playerShip.png");
    m_textures.load(Textures::ID::Heart, "../Resources/Heart.png");
    m_textures.load(Textures::ID::Station, "../Resources/Station.png");
    m_textures.load(Textures::ID::BoosterYellow, "../Resources/effectYellow.png");
    m_textures.load(Textures::ID::BoosterPurple, "../Resources/effectPurple.png");
    m_textures.load(Textures::ID::Arrow, "../Resources/arrow.png");
    m_textures.load(Textures::ID::Emp, "../Resources/emp.png");
    m_textures.load(Textures::ID::Star, "../Resources/star.png");

}