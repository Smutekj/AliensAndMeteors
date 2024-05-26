#include "GameWorld.h"

#include "Entities.h"

GameWorld::GameWorld()
{

    for (int i = 0; i < 300; ++i)
    {
        auto &meteor = addObject(ObjectType::Meteor);
    }

    m_textures.load(Textures::ID::Bomb, "../Resources/bomb.png");
    m_textures.load(Textures::ID::EnemyShip, "../Resources/EnemyShip.png");
    m_textures.load(Textures::ID::Explosion2, "../Resources/explosion2.png");
    m_textures.load(Textures::ID::Explosion, "../Resources/explosion.png");
    m_textures.load(Textures::ID::PlayerShip, "../Resources/playerShip.png");
    m_textures.load(Textures::ID::Heart, "../Resources/Heart.png");
    m_textures.load(Textures::ID::Station, "../Resources/Station.png");
    m_textures.load(Textures::ID::BoosterYellow, "../Resources/effectYellow.png");
    m_textures.load(Textures::ID::BoosterPurple, "../Resources/effectPurple.png");

    m_neighbour_searcher = std::make_unique<GridNeighbourSearcher>();
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
        new_object = std::make_shared<Bullet2>(this, m_textures, m_player);
        break;
    case ObjectType::Laser:
        new_object = std::make_shared<Laser2>(this, m_textures, m_collision_system);
        break;
    case ObjectType::Bomb:
        new_object = std::make_shared<Bomb2>(this, m_textures, m_collision_system);
        break;
    case ObjectType::Explosion:
        new_object = std::make_shared<Explosion>(this, m_textures);
        break;
    case ObjectType::Heart:
        new_object = std::make_shared<Heart>(this, m_textures);
        break;
    case ObjectType::Player:
        new_object = std::make_shared<PlayerEntity>(this, m_textures);
        m_player = &static_cast<PlayerEntity &>(*new_object);
        break;
    case ObjectType::SpaceStation:
        new_object = std::make_shared<SpaceStation>(this, m_textures);
        break;
    }

    // auto new_id = *free_ids.begin();
    // free_ids.erase(free_ids.begin());
    // new_object->m_id = new_id;

    to_add.push(new_object);
    return *to_add.back();
}

void GameWorld::addQueuedEntities()
{
    while (!to_add.empty())
    {
        auto new_object = to_add.front();
        auto new_id = m_entities.addObject(new_object);
        m_entities.at(new_id)->m_id = new_id;
        if (m_entities.at(new_id)->collides())
        {
            m_collision_system.insertObject(*m_entities.at(new_id));
        }

        m_entities.at(new_id)->onCreation();
        to_add.pop();
    }
}

void GameWorld::removeQueuedEntities()
{
    while (!to_destroy.empty())
    {
        auto object = to_destroy.front();
        object->onDestruction();
        if (object->collides())
        {
            m_collision_system.removeObject(*object);
        }
        m_entities.remove(object->getId());
        to_destroy.pop();
    }
}

void GameWorld::destroyObject(int entity_id)
{
    to_destroy.push(m_entities.at(entity_id));
}

void GameWorld::update(float dt)
{
    for (auto ind : m_entities.active_inds)
    {
        m_entities.at(ind)->updateAll(dt);
        if (m_entities.at(ind)->isDead())
        {
            destroyObject(ind);
        }
    }

    m_collision_system.update();
    addQueuedEntities();
    removeQueuedEntities();
}

void GameWorld::draw(sf::RenderTarget &bloomy_target, sf::RenderTarget &window)
{
    for (auto ind : m_entities.active_inds)
    {
        if (m_entities.at(ind)->isBloomy())
        {
            m_entities.at(ind)->draw(bloomy_target);
        }
        else
        {
            m_entities.at(ind)->draw(window);
        }
    }
}