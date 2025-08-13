#pragma once

// #include "Geometry.h"
#include "Utils/Grid.h"
#include "Utils/ObjectPool.h"

#include <unordered_map>
#include <functional>
#include <queue>

#include <Texture.h>

#include "CollisionSystem.h"
#include "GridNeighbourSearcher.h"

#include "Entities/Entities.h"
#include "Entities/Meteor.h"
#include "Entities/Enemy.h"
#include "Entities/Player.h"
#include "Entities/Attacks.h"
#include "Entities/VisualEffects.h"
#include "Entities/Triggers.h"
#include "ComponentSystem.h"

#include "Systems/TargetSystem.h"

#include "PostOffice.h"

struct PlayerEntity;

// Poor man's MPL vector.
template <class... Ts>
struct TypeList
{
    static const int size = sizeof...(Ts);
};

template <class TList, class Element>
struct push_back_impl;

template <template <class...> class TList, class... Ts, class Element>
struct push_back_impl<TList<Ts...>, Element>
{
    using type = TList<Ts..., Element>;
};
template <class TList, class Element>
using push_back_t = typename push_back_impl<TList, Element>::type;

template <class A, template <class...> class B, template <class> class Outer>
struct rebind_;

template <template <class...> class A, class... T, template <class...> class B, template <class> class Outer>
struct rebind_<A<T...>, B, Outer>
{
    using type = B<Outer<T>...>;
};
template <class A, template <class...> class B, template <class> class Outer>
using rebind = typename rebind_<A, B, Outer>::type;

#include <tuple>
#include <typeinfo>
#include "Utils/Colony.h"

using EntityTypes = TypeList<Enemy, Turret, Meteor, SpaceStation, Boss, Bullet,
                             Laser, Bomb, Explosion, PlayerEntity, Heart, EMP>;
// using EntityTypes = TypeList<Enemy>;

using EntityTuple = rebind<EntityTypes, std::tuple, ComponentBlock>;
using EntityQueue = rebind<EntityTypes, std::tuple, std::queue>;

class GameWorld
{

public:
    GameWorld(PostOffice &messenger);

    Collisions::CollisionSystem &getCollisionSystem()
    {
        return m_collision_system;
    }

    GameObject* get(int entity_id) 
    {
        return m_entities.at(entity_id).get();
    }

    //! checks whether components that exist have existing entities
    void checkComponentsConsistency();

    template <class EntityType>
    EntityType &addObject2();
    template <class EntityType>
    EntityType createEntity();
    template <class EntityType>
    std::shared_ptr<EntityType> createEntity2();
    template <class EntityType>
    EntityType &addObjectForced();
    template <class EntityType>
    void addX(std::queue<EntityType> &to_add);
    template <class EntityType>
    void updateX(ComponentBlock<EntityType> &entity_block, float dt);
    template <class EntityType>
    void removeX(std::queue<EntityType> &to_remove);
    template <class EntityType>
    void drawX(ComponentBlock<EntityType> &entity_block, LayersHolder &layers, View camera_view);

    void addQueuedEntities2();
    void removeQueuedEntities2();
    void update2(float dt);
    void draw2(LayersHolder &layers, View camera_view);

    ///!!!
    void destroyObject(int entity_id);
    GameObject &addObject(ObjectType type);
    VisualEffect &addVisualEffect(EffectType type);
    template <class TriggerType, class... Args>
    TriggerType &addTrigger(Args... args);

    std::size_t getNActiveEntities(ObjectType type);

    template <class EntityType>
    std::size_t getActiveCount()
    {
        return std::get<ComponentBlock<EntityType>>(m_entities2).activeCount();
    }

    void update(float dt);
    void draw(LayersHolder &window);

private:
    void registerSystems();
    void addQueuedEntities();
    void removeQueuedEntities();
    void loadTextures();

public:
    GameSystems m_systems;
    PlayerEntity *m_player;
    TextureHolder m_textures;

    Collisions::CollisionSystem m_collision_system;
    PostOffice *p_messenger = nullptr;

private:
    std::unordered_map<EffectType, std::function<std::shared_ptr<VisualEffect>()>> m_effect_factories;

    EntityTuple m_entities2;
    EntityQueue m_entities_to_add;
    EntityQueue m_entities_to_remove;

    EntityRegistryT m_entities;

    std::unique_ptr<GridNeighbourSearcher> m_neighbour_searcher;
    std::shared_ptr<TargetSystem> m_ts;

    std::queue<std::shared_ptr<GameObject>> m_to_add;
    std::queue<std::shared_ptr<GameObject>> m_to_destroy;

};

template <class TriggerType, class... Args>
TriggerType &GameWorld::addTrigger(Args... args)
{
    auto new_trigger = std::make_shared<TriggerType>(this, m_textures, args...);
    new_trigger->m_id = m_entities.reserveIndexForInsertion();
    m_to_add.push(new_trigger);
    return *new_trigger;
}

template <class EntityType>
EntityType &GameWorld::addObject2()
{
    static_assert(std::is_base_of_v<GameObject, EntityType>);
    // auto &queue = std::get<std::queue<EntityType>>(m_entities_to_add);
    // auto new_entity = createEntity<EntityType>();
    // queue.push(new_entity);
    // EntityType &thing = queue.back();
    // return thing;

    auto new_entity = createEntity2<EntityType>();
    new_entity->m_id = m_entities.reserveIndexForInsertion();
    m_to_add.push(new_entity);
    return *new_entity;
}

template <class EntityType>
EntityType GameWorld::createEntity()
{
    if constexpr (std::is_same_v<EntityType, Enemy>)
    {
        return {this, m_textures, &m_collision_system, m_player, m_systems};
    }
    else
    {
        return {this, m_textures, &m_collision_system, m_player};
    }
}

template <class EntityType>
std::shared_ptr<EntityType> GameWorld::createEntity2()
{
    if constexpr (std::is_same_v<EntityType, Enemy> || std::is_same_v<EntityType, SpaceStation>)
    {
        return std::make_shared<EntityType>(this, m_textures, &m_collision_system, m_player, m_systems);
    }
    else
    {
        return std::make_shared<EntityType>(this, m_textures, &m_collision_system, m_player);
    }
}

template <class EntityType>
EntityType &GameWorld::addObjectForced()
{
    // auto &block = std::get<ComponentBlock<EntityType>>(m_entities2);
    // int new_id = block.insert(createEntity<EntityType>());
    auto new_entity = createEntity2<EntityType>();
    int new_id = m_entities.insert(new_entity);
    // EntityType &new_entity = block.get(new_id);
    // new_entity.m_block_id = new_id;
    new_entity->m_id = new_id;

    new_entity->onCreation();
    if (m_systems.has<CollisionComponent>(new_id))
    {
        m_collision_system.insertObject(*new_entity);
    }
    return *new_entity;
}

template <class EntityType>
void GameWorld::addX(std::queue<EntityType> &to_add)
{
    while (!to_add.empty())
    {
        auto &entity_block = std::get<ComponentBlock<EntityType>>(m_entities2);
        int block_id = entity_block.insert(to_add.front());
        auto &new_entity = entity_block.get(block_id);

        std::shared_ptr<EntityType> entity_p(&new_entity);
        m_entities.insertAt(entity_p->getId(), entity_p);

        // new_entity.m_id = id;

        if (new_entity.collides())
        {
            m_collision_system.insertObject(new_entity);
        }
        new_entity.onCreation();
        to_add.pop();
    }
}
template <class EntityType>
void GameWorld::removeX(std::queue<EntityType> &to_remove)
{
    auto &entity_block = std::get<ComponentBlock<EntityType>>(m_entities2);
    while (!to_remove.empty())
    {
        assert(false); //! DO NOT USe THIs!!!
        auto &entity = to_remove.front();
        entity.onDestruction();

        p_messenger->send(EntityDiedEvent{entity.getType(), entity.getId(), entity.getPosition()});

        if (entity.collides())
        {
            m_collision_system.removeObject(entity);
        }

        entity_block.deactivate(entity.getId());
        m_entities.remove(entity.getId());
        to_remove.pop();
    }
}

struct EntityTreeNode
{
    std::vector<int> children;
    int parent = -1;
};

class SceneGraph
{
public:
    SceneGraph(GameWorld &world, EntityRegistryT &entities)
        : m_world(world), m_entities(entities)
    {
    }

    void update(float dt)
    {
        
    }

    void addEntity(int id)
    {
        EntityTreeNode node = {{}, id};
        m_scene.addObject(node);
    }

    void addNewChild(int parent, int child_id)
    {
        //! the child should not exist yet!
        assert(!m_scene.contains(child_id));
    
        addEntity(child_id);
        addExistingChild(parent, child_id);
    }

    void addExistingChild(int parent, int child_id)
    {
        //! the child should already exist!
        assert(m_scene.contains(child_id));
    
        auto& children = m_scene.at(parent).children;    
        //! do not add anything twice!
        assert(std::find(children.begin(), children.end(), child_id) == children.end());
        children.push_back(child_id);
        m_scene.at(child_id).parent = parent;
    }

    void removeEntity(int id)
    {
        
        std::deque<int> to_remove = {id};
        while(!to_remove.empty())
        {
            int removed_id = to_remove.front();
            to_remove.pop_front();
            
            auto& node = m_scene.at(removed_id);
            for(auto child_id : node.children)
            {
                to_remove.push_back(child_id);
            }
            m_scene.remove(id);
            m_entities.at(id)->kill();
        }
    }



private:

    utils::DynamicObjectPool<EntityTreeNode, MAX_ENTITY_COUNT> m_scene;

    EntityRegistryT &m_entities;
    GameWorld &m_world;
};