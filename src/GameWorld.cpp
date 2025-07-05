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

        //! move objects far away from player somewhere around the player
        auto player_pos = m_player->getPosition();
        auto player_vel = m_player->m_vel;
        auto obj_pos = obj->getPosition();
        auto obj_vel = obj->m_vel;
        auto obj_moves_away = utils::dot(player_vel, obj_vel) < 0;
        auto player_obj_dist = utils::dist(obj_pos, player_pos);
        if (obj_moves_away && player_obj_dist > 450)
        {
            auto rand_radius = randf(400, 500);
            auto rand_angle = randf(0, 360);
            // auto x = randomPosInBox(player_pos - utils::Vector2f(300.),player_pos + utils::Vector2f(300.));
            auto new_obj_pos = player_pos + rand_radius * utils::angle2dir(rand_angle);
            obj->setPosition(new_obj_pos);
            if (obj->getType() == ObjectType::Enemy)
            {
                obj->kill();
            }
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
    m_textures.add("Bomb", "../Resources/Textures/bomb.png");
    m_textures.add("EnemyShip", "../Resources/Textures/EnemyShip.png");
    m_textures.add("EnemyLaser", "../Resources/Textures/EnemyLaser.png");
    m_textures.add("EnemyBomber", "../Resources/Textures/EnemyBomber.png");
    m_textures.add("Meteor", "../Resources/Textures/Meteor.png");
    m_textures.add("BossShip", "../Resources/Textures/BossShip.png");
    m_textures.add("Explosion2", "../Resources/Textures/explosion2.png");
    m_textures.add("Explosion", "../Resources/Textures/explosion.png");
    m_textures.add("PlayerShip", "../Resources/Textures/playerShip.png");
    m_textures.add("Heart", "../Resources/Textures/Heart.png");
    m_textures.add("Station", "../Resources/Textures/Station.png");
    m_textures.add("BoosterYellow", "../Resources/Textures/effectYellow.png");
    m_textures.add("BoosterPurple", "../Resources/Textures/effectPurple.png");
    m_textures.add("Arrow", "../Resources/Textures/arrow.png");
    m_textures.add("Emp", "../Resources/Textures/emp.png");
    m_textures.add("Star", "../Resources/Textures/star.png");
}