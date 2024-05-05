#pragma once

#include "core.h"
#include "Geometry.h"

struct Player
{
    sf::Vector2f pos = {Geometry::BOX[0] / 2.f, Geometry::BOX[1] / 2.f};
    float speed = 0.f;
    float angle = 0.f;
    float radius = 3.f;
    sf::Vector2f orientation = {1,0};
    int health = 100;
};
