#pragma once

#include "Geometry.h"
#include "Utils/Grid.h"
#include "Utils/GayVector.h"

#include <unordered_map>
#include <functional>

#include "CollisionSystem.h"
#include "GridNeighbourSearcher.h"
#include "Player.h"


class GameWorld
{

    ObjectPool<std::shared_ptr<GameObject>> m_entities;

    std::unique_ptr<GridNeighbourSearcher> m_neighbour_searcher;

    Collisions::CollisionSystem m_collision_system;

    PlayerEntity* m_player;

    std::queue<std::shared_ptr<GameObject>> to_add;
    std::queue<std::shared_ptr<GameObject>> to_destroy;

    // Messenger m_messenger;

    std::unordered_set<int> free_ids; 

    TextureHolder m_textures;

public:
    GameWorld();

    void destroyObject(int entity_id);
    GameObject &addObject(ObjectType type);
    void update(float dt);
    void draw(sf::RenderTarget &bloomy_target, sf::RenderTarget& window);

private:

    void addQueuedEntities();
    void removeQueuedEntities();
};
