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

class PlayerEntity;

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

using EntityTypes = TypeList<Enemy, Meteor, SpaceStation, Boss, Bullet, Laser, Bomb, Explosion, PlayerEntity, Heart, EMP>;
// using EntityTypes = TypeList<Enemy>;

using EntityTuple = rebind<EntityTypes, std::tuple, ComponentBlock>;
using EntityQueue = rebind<EntityTypes, std::tuple, std::queue>;

class GameWorld
{
    EntityTuple m_entities2;
    EntityQueue m_entities_to_add;
    EntityQueue m_entities_to_remove;

    utils::DynamicObjectPool<std::shared_ptr<GameObject>, 5000> m_entities;

    std::unique_ptr<GridNeighbourSearcher> m_neighbour_searcher;
    Collisions::CollisionSystem m_collision_system;

    std::queue<std::shared_ptr<GameObject>> m_to_add;
    std::queue<std::shared_ptr<GameObject>> m_to_destroy;

    TextureHolder m_textures;

public:
    PlayerEntity *m_player;
    utils::DynamicObjectPool<std::function<void(ObjectType, int)>, 100> m_entitydestroyed_events;

    GameWorld();

    template <class EntityType>
    EntityType &addObject2()
    {
        auto &queue = std::get<std::queue<EntityType>>(m_entities_to_add);
        EntityType new_entity = {this, m_textures, &m_collision_system, m_player};
        queue.push(new_entity);
        EntityType &thing = queue.back();
        return thing;
    }

    template <class EntityType>
    EntityType &addObjectForced()
    {
        auto &block = std::get<ComponentBlock<EntityType>>(m_entities2);
        int new_id = block.insert({this, m_textures, &m_collision_system, m_player});
        EntityType &new_entity = block.get(new_id);
        new_entity.m_id = new_id;
        if (new_entity.collides())
        {
            m_collision_system.insertObject(new_entity);
        }
        return new_entity;
    }

    template <class EntityType>
    void addX(std::queue<EntityType> &to_add)
    {
        while (!to_add.empty())
        {
            auto &entity_block = std::get<ComponentBlock<EntityType>>(m_entities2);
            int new_id = entity_block.insert(to_add.front());

            EntityType &new_entity = entity_block.get(new_id);
            new_entity.m_id = new_id;
            if (new_entity.collides())
            {
                m_collision_system.insertObject(new_entity);
            } 
            new_entity.onCreation();
            to_add.pop();
        }
    }
    template <class EntityType>
    void removeX(std::queue<EntityType> &to_remove)
    {
        auto &entity_block = std::get<ComponentBlock<EntityType>>(m_entities2);
        while (!to_remove.empty())
        {
            auto &entity = to_remove.front();
            entity.onDestruction();

            //! notify observers that entity got destroyed
            for (auto callback_id : m_entitydestroyed_events.getEntityIds())
            {
                m_entitydestroyed_events.at(callback_id)(entity.getType(), entity.getId());
            }

            if (entity.collides())
            {
                m_collision_system.removeObject(entity);
            }
            entity_block.deactivate(entity.getId());
            to_remove.pop();
        }
    }

    void addQueuedEntities2()
    {
        std::apply([this](auto &...entity_queue)
                   { ((addX(entity_queue)), ...); }, m_entities_to_add);
    }
    void removeQueuedEntities2()
    {
        std::apply([this](auto &...entity_queue)
                   { ((removeX(entity_queue)), ...); }, m_entities_to_remove);
    }
    template <class EntityType>
    void updateX(ComponentBlock<EntityType> &entity_block, float dt)
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
    void update2(float dt)
    {
        m_collision_system.update();


        std::apply([this, dt](auto &...entity_queue)
                   { ((updateX(entity_queue, dt)), ...); }, m_entities2);



        addQueuedEntities2();
        removeQueuedEntities2();
    }
    template <class EntityType>
    void drawX(ComponentBlock<EntityType> &entity_block, LayersHolder &layers)
    {
        auto &block = entity_block.getData();
        int first_ind = block.at(0).next;
        for (int ind = first_ind; ind <= 750; ind += block[ind].next) // block[ind].next)
        {
            block.at(ind).comp.draw(layers);
        }
    }
    void draw2(LayersHolder &layers)
    {
        std::apply([this, &layers](auto &&...ents)
                   { ((drawX(ents, layers)), ...); }, m_entities2);
    }

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

    int addEntityDestroyedCallback(std::function<void(ObjectType, int)> callback);
    void removeEntityDestroyedCallback(int callback_id);

private:
    void addQueuedEntities();
    void removeQueuedEntities();
    void loadTextures();

    std::unordered_map<EffectType, std::function<std::shared_ptr<VisualEffect>()>> m_effect_factories;
};

template <class TriggerType, class... Args>
TriggerType &GameWorld::addTrigger(Args... args)
{
    auto new_trigger = std::make_shared<TriggerType>(this, m_textures, args...);
    m_to_add.push(new_trigger);
    return *new_trigger;
}