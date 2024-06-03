#pragma once

#include <SFML/System/Vector2.hpp>
#include <cmath>


struct AABB
{
    sf::Vector2f r_min = {0, 0};
    sf::Vector2f r_max = {0, 0};

    AABB() = default;
    AABB(sf::Vector2f r_min, sf::Vector2f r_max);
    AABB(sf::Vector2f r_min, float width, float height);

    AABB &inflate(float scale);
    bool isIn(const sf::Vector2f &r) const;
    float volume() const;
};

inline AABB makeUnion(const AABB &r1, const AABB &r2)
{
    AABB r12;
    r12.r_min.x = std::min(r1.r_min.x, r2.r_min.x);
    r12.r_min.y = std::min(r1.r_min.y, r2.r_min.y);

    r12.r_max.x = std::max(r1.r_max.x, r2.r_max.x);
    r12.r_max.y = std::max(r1.r_max.y, r2.r_max.y);
    return r12;
}