#include "AABB.h"


    AABB::AABB(sf::Vector2f r_min, sf::Vector2f r_max)
     : r_min(r_min), r_max(r_max)
    {
    }

    AABB::AABB(sf::Vector2f r_min, float width, float height) : r_min(r_min), r_max({r_min.x + width, r_min.y + height})
    {
    }


    AABB& AABB::inflate(float scale)
    {
        auto dr = (r_max - r_min);
        auto new_dr = dr*scale;
        auto overlap_dr = (new_dr - dr)/2.f;
        r_min = r_min - overlap_dr;
        r_max = r_max + overlap_dr;
        return *this;
    }

    bool AABB::isIn(const sf::Vector2f &r) const
    {
        const auto &dr = r - r_min;
        return r.x <= r_max.x && r.x > r_min.x && r.y <= r_max.y && r.y > r_min.y;
    }

    float AABB::volume() const
    {
        return (r_max.x - r_min.x) * (r_max.y - r_min.y);
    }

