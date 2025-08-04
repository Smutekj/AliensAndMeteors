#include "GameWorld.h"

#include <chrono>

#include "Utils/RandomTools.h"

#include "Systems/BoidSystem.h"
#include "Systems/MeteorAvoidanceSystem.h"
#include "Systems/HealthSystem.h"
#include "Systems/TargetSystem.h"
#include "Systems/TimedEventSystem.h"

GameWorld::GameWorld(PostOffice& messenger)
: p_messenger(&messenger), m_collision_system(messenger), m_systems(m_entities)
{
    loadTextures();

    m_neighbour_searcher = std::make_unique<GridNeighbourSearcher>();

    m_systems.registerSystem(std::make_shared<BoidSystem>(m_systems.getComponents<BoidComponent>()));
    m_systems.registerSystem(std::make_shared<AvoidanceSystem>(m_systems.getComponents<AvoidMeteorsComponent>(), m_collision_system));
    m_systems.registerSystem(std::make_shared<HealthSystem>(m_systems.getComponents<HealthComponent>(), messenger));
    m_systems.registerSystem(std::make_shared<TargetSystem>(m_systems.getComponents<TargetComponent>()));
    m_systems.registerSystem(std::make_shared<TimedEventSystem>(m_systems.getComponents<TimedEventComponent>()));

    m_effect_factories[EffectType::ParticleEmiter] =
        [this]()
    { return std::make_shared<StarEmitter>(this, m_textures); };
    m_effect_factories[EffectType::AnimatedSprite] =
        [this]()
    { return std::make_shared<AnimatedSprite>(this, m_textures); };
}

std::size_t GameWorld::getNActiveEntities(ObjectType type)
{
    auto &entities = m_entities.data();
    auto n_enemies = std::count_if(entities.begin(), entities.end(), [type](auto &obj)
                                   { return obj->getType() == type; });
    return n_enemies;
}


void GameWorld::addQueuedEntities2()
{
    std::apply([this](auto &...entity_queue)
               { ((addX(entity_queue)), ...); }, m_entities_to_add);
}
void GameWorld::removeQueuedEntities2()
{
    std::apply([this](auto &...entity_queue)
               { ((removeX(entity_queue)), ...); }, m_entities_to_remove);
}
template <class EntityType>
void GameWorld::updateX(ComponentBlock<EntityType> &entity_block, float dt)
{
    static_assert(std::is_base_of_v<GameObject, EntityType>);

    auto &block = entity_block.getData();
    int first_ind = block.at(0).next;
    for (int ind = first_ind; ind <= 750; ind += block.at(ind).next) // block[ind].next)
    {
        auto &entity = block.at(ind).comp;
        entity.updateAll(dt);
        entity.update(dt);
        if (block.at(ind).comp.isDead())
        {
            std::get<std::queue<EntityType>>(m_entities_to_remove).push(entity);
        }
    }
}

void GameWorld::update2(float dt)
{
    // m_collision_system.update();

    std::apply([this, dt](auto &...entity_queue)
               { ((updateX(entity_queue, dt)), ...); }, m_entities2);

    addQueuedEntities2();
    removeQueuedEntities2();
}
template <class EntityType>
void GameWorld::drawX(ComponentBlock<EntityType> &entity_block, LayersHolder &layers, View camera_view)
{
    camera_view.setSize(camera_view.getSize() * 1.2); //! draw also a little bit beyond the camera

    auto &block = entity_block.getData();
    int first_ind = block.at(0).next;
    for (int ind = first_ind; ind <= 750; ind += block[ind].next) // block[ind].next)
    {
        auto &drawable = block.at(ind).comp;

        Rectf bounding_rect = {drawable.getPosition().x, drawable.getPosition().y,
                               drawable.getSize().x, drawable.getSize().y};

        if (camera_view.intersects(bounding_rect))
        {
            drawable.draw(layers);
        }
    }
}
void GameWorld::draw2(LayersHolder &layers, View camera_view)
{
    std::apply([this, &layers, camera_view](auto &&...ents)
               { ((drawX(ents, layers, camera_view)), ...); }, m_entities2);
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
        auto new_id = new_object->getId(); //! the object already has id because we reserved it
        m_entities.insertAt(new_id, new_object);
        assert(new_id == m_entities.at(new_id)->getId());
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

        if (object->collides())
        {
            m_collision_system.removeObject(*object);
        }
        m_systems.removeEntity(object->getId());
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

    m_systems.preUpdate(dt);

    m_collision_system.update();
    m_systems.update(dt);
    m_systems.postUpdate(dt);
    
    for (auto &obj : m_entities.data())
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
    for (auto &obj : m_entities.data())
    {
        obj->draw(layers);
    }
}

VisualEffect &GameWorld::addVisualEffect(EffectType type)
{
    assert(m_effect_factories.count(type) > 0);

    auto new_effect = m_effect_factories.at(type)();
    new_effect->m_id = m_entities.reserveIndexForInsertion();
    m_to_add.push(new_effect);
    return *new_effect;
}

void GameWorld::loadTextures()
{
    m_textures.setBaseDirectory(std::string(RESOURCES_DIR) + "/Textures/");
    m_textures.add("Bomb", "bomb.png");
    m_textures.add("EnemyShip", "EnemyShip.png");
    m_textures.add("Boss1", "Ships/Boss1.png");
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