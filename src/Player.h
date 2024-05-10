#pragma once

#include "core.h"
#include "Geometry.h"

struct Player
{
    sf::Vector2f pos = {Geometry::BOX[0] / 2.f, Geometry::BOX[1] / 2.f};
    sf::Vector2f vel = {0.f, 0.f};
    float speed = 0.f;
    float angle = 0.f;
    float radius = 3.f;
    sf::Vector2f orientation = {1,0};
    int health = 100;
    int max_health = 100;

    void update(float dt){
        
    }
};
