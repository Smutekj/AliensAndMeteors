#pragma once

// #include "Geometry.h"
#include "Utils/Grid.h"
#include "Utils/ObjectPool.h"
#include "Utils/ContiguousColony.h"

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

class ToolBoxUI;

struct PlayerEntity;

class GameWorld
{

public:
    GameWorld(PostOffice &messenger, TextureHolder& textures);

    Collisions::CollisionSystem &getCollisionSystem()
    {
        return m_collision_system;
    }

    GameObject* get(int entity_id) 
    {
        return m_entities.at(entity_id).get();
    }
    EntityRegistryT& getEntities() 
    {
        return m_entities;
    }
    bool contains(int entity_id) const
    {
        return m_entities.contains(entity_id);
    }

    //! checks whether components that exist have existing entities
    void checkComponentsConsistency();
    
    template <class EntityType>
    EntityType &addObject2();
    template <class EntityType>
    std::shared_ptr<EntityType> createEntity2();
    template <class EntityType>
    EntityType &addObjectForced();

    ///!!!
    void destroyObject(int entity_id);
    GameObject &addObject(ObjectType type);
    GameObject &addObject3(ObjectType type);
    VisualEffect &addVisualEffect(EffectType type);
    template <class TriggerType, class... Args>
    TriggerType &addTrigger(Args... args);
    
    std::size_t getNActiveEntities(ObjectType type);

    // template <class EntityType>
    // std::size_t getActiveCount()
    // {
    //     return std::get<ComponentBlock<EntityType>>(m_entities2).activeCount();
    // }

    void update(float dt);
    void draw(LayersHolder &window, const View& camera_view);

    void removeParent(GameObject& child);

private:
    void addQueuedEntities();
    void removeQueuedEntities();
    void loadTextures();

public:
    GameSystems m_systems;
    PlayerEntity *m_player;
    TextureHolder& m_textures;

    Collisions::CollisionSystem m_collision_system;
    PostOffice *p_messenger = nullptr;

private:
    std::unordered_map<EffectType, std::function<std::shared_ptr<VisualEffect>()>> m_effect_factories;


    EntityRegistryT m_entities;
    DynamicObjectPool2<int> m_root_entities;

    std::shared_ptr<TargetSystem> m_ts;

    std::deque<std::shared_ptr<GameObject>> m_to_add;
    std::deque<std::shared_ptr<GameObject>> m_to_destroy;

    friend ToolBoxUI;

};

template <class TriggerType, class... Args>
TriggerType &GameWorld::addTrigger(Args... args)
{
    auto new_trigger = std::make_shared<TriggerType>(this, m_textures, args...);
    new_trigger->m_id = m_entities.reserveIndexForInsertion();
    m_to_add.push_back(new_trigger);
    return *new_trigger;
}

template <class EntityType>
EntityType &GameWorld::addObject2()
{
    
    static_assert(std::is_base_of_v<GameObject, EntityType> || std::is_same_v<GameObject, EntityType>);

    auto new_entity = createEntity2<EntityType>();
    new_entity->m_id = m_entities.reserveIndexForInsertion();
    m_to_add.push_back(new_entity);
    return *new_entity;
}

template <class EntityType>
std::shared_ptr<EntityType> GameWorld::createEntity2()
{
    if constexpr (std::is_same_v<EntityType, Enemy> || std::is_same_v<EntityType, SpaceStation>)
    {
        return std::make_shared<EntityType>(this, m_textures, m_player, m_systems);
    }
    else
    {
        return std::make_shared<EntityType>(this, m_textures, m_player);
    }
}

template <class EntityType>
EntityType &GameWorld::addObjectForced()
{

    int new_id = m_entities.reserveIndexForInsertion();
    auto new_entity = createEntity2<EntityType>();
    new_entity->m_id = new_id;

    new_entity->onCreation();
    if (m_systems.has<CollisionComponent>(new_id))
    {
        m_collision_system.insertObject(*new_entity);
    }
    m_root_entities.insertAt(new_id, new_id);
    m_entities.insertAt(new_id, new_entity);
    
    return static_cast<EntityType&>(*m_entities.at(new_id));
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