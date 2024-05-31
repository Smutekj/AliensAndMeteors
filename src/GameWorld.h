#pragma once

#include "Geometry.h"
#include "Utils/Grid.h"
#include "Utils/ObjectPool.h"

#include <unordered_map>
#include <functional>

#include "CollisionSystem.h"
#include "GridNeighbourSearcher.h"
#include "Player.h"


class ObjectiveSystem
{

};

class GameWorld
{

    ObjectPool<std::shared_ptr<GameObject>> m_entities;

    std::unique_ptr<GridNeighbourSearcher> m_neighbour_searcher;
    Collisions::CollisionSystem m_collision_system;

    std::queue<std::shared_ptr<GameObject>> m_to_add;
    std::queue<std::shared_ptr<GameObject>> m_to_destroy;

    TextureHolder m_textures;

    Timer m_heart_timer;

public:
    PlayerEntity* m_player;
    GameWorld();

    void destroyObject(int entity_id);
    GameObject &addObject(ObjectType type);
    GameObject &addObjective(ObjectiveType type);
    void update(float dt);
    void draw(sf::RenderTarget &bloomy_target, sf::RenderTarget& window);

private:

    void addQueuedEntities();
    void removeQueuedEntities();
};

