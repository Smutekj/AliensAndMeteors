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

#include "Entities/Meteor.h"
#include "Entities/Entities.h"
#include "Entities/VisualEffects.h"
#include "Entities/Triggers.h"

class PlayerEntity;

class GameWorld
{

    utils::DynamicObjectPool<std::shared_ptr<GameObject>, 5000> m_entities;

    std::unique_ptr<GridNeighbourSearcher> m_neighbour_searcher;
    Collisions::CollisionSystem m_collision_system;

    std::queue<std::shared_ptr<GameObject>> m_to_add;
    std::queue<std::shared_ptr<GameObject>> m_to_destroy;

    TextureHolder m_textures;

    PlayerEntity *m_player;

public:

    utils::DynamicObjectPool<std::function<void(ObjectType, int)>, 100> m_entitydestroyed_events;

    GameWorld();

    void destroyObject(int entity_id);
    GameObject &addObject(ObjectType type);
    VisualEffect &addVisualEffect(EffectType type);
    template <class TriggerType, class... Args>
    TriggerType &addTrigger(Args... args);
    
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