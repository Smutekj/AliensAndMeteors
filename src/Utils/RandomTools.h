#pragma once

#include "../core.h"
#include "../Geometry.h"
// #include "../Game.h"
// #include "Graphics/SceneLayer.h"


inline float randf(const float min = 0, const float max = 1){
    return (rand() / static_cast<float>(RAND_MAX)) * (max - min) + min;
}


inline  sf::Vector2f randomPosInBox(sf::Vector2f ul_corner = {0,0},
                            sf::Vector2f box_size = {Geometry::BOX[0], Geometry::BOX[1]}
                            ){
    return {ul_corner.x + rand() / static_cast<float>(RAND_MAX) * box_size.x,
            ul_corner.y + rand() / static_cast<float>(RAND_MAX) * box_size.y };
}


