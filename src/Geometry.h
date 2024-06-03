#pragma once

#include <cmath>

#include <SFML/System/Vector2.hpp>

#ifndef M_PIf
#define M_PIf std::numbers::pi_v<float>
#endif 


namespace Geometry{
    constexpr int N_CELLS[2] = {64, 64};
    constexpr int CELL_SIZE = 20;
    constexpr int BOX[2] = {CELL_SIZE*N_CELLS[0], CELL_SIZE*N_CELLS[1]};
}; //! Namespace Geometry


template <typename T> inline float dot(const T& a, const T& b) { return a.x * b.x + a.y * b.y; }
template <typename T> inline float dot(const T&& a, const T&& b) { return a.x * b.x + a.y * b.y; }

template <typename T> inline float norm2(const T& a) { return dot(a, a); }
template <typename T> inline float norm(const T& a) { return std::sqrt(norm2(a)); }

template <typename T> inline float dist2(const T& a, const T& b) { return dot(a - b, a - b); }
template <typename T> inline float dist2(const T&& a, const T&& b) { return dot(a - b, a - b); }

template <typename T> inline float dist(const T& a, const T& b) { return std::sqrt(dist2(a, b)); }
template <typename T> inline float dist(const T&& a, const T&& b) { return std::sqrt(dist2(a, b)); }

struct Edgef {
    sf::Vector2f from;
    sf::Vector2f t = {0.f, 0.f};
    float l = 0;

    Edgef(const sf::Vector2f& v1, const sf::Vector2f& v2)
        : from(v1) {
        const auto t_new = static_cast<sf::Vector2f>(v2 - v1);
        l = norm(t_new);
        t = t_new / l;
    }
    Edgef() = default;
    sf::Vector2f to() const { return from + t * l; }
};


inline void truncate(sf::Vector2f &vec, float max_value)
{
    auto speed = norm(vec);
    if (speed > max_value)
    {
        vec *= max_value / speed;
    }
}

inline sf::Vector2f getIntersection(sf::Vector2f r1s, sf::Vector2f r1e, sf::Vector2f r2s, sf::Vector2f r2e)
{

    auto dr12 = r1s - r2s;
    auto t1 = r1e - r1s;
    auto l1 = norm(t1);
    t1 /= norm(t1);
    auto t2 = r2e - r2s;
    auto l2 = norm(t2);
    t2 /= norm(t2);
    sf::Vector2f n1 = {t1.y, -t1.x};
    sf::Vector2f n2 = {t2.y, -t2.x};
    auto alpha1 = dot(-dr12, n2) / dot(n2, t1);
    auto alpha2 = dot(dr12, n1) / dot(n1, t2);
    if (alpha1 >= 0 && alpha1 <= l1 && alpha2 >= 0 && alpha2 <= l2)
    {
        return r1s + alpha1 * t1;
    }
    return {-1, -1};
}



inline sf::Vector2f angle2dir(float angle){
    sf::Vector2f dir;
    dir.x = std::cos(angle*M_PIf/180.f);
    dir.y = std::sin(angle*M_PIf/180.f);
    return dir;
}

inline float dir2angle(const sf::Vector2f& dir){
    return std::atan2(dir.y, dir.x)*180.f/M_PIf;
}



inline sf::Vector2f asFloat(const sf::Vector2i& r) { return static_cast<sf::Vector2f>(r); }

template <class T>
inline sf::Vector2f asFloat(const T& r) { return {static_cast<float>(r.x), static_cast<float>(r.y)}; }

template <typename T>
inline float angleBetween(const T& v1,
                          const T& v2) { //! I should just tabelate this, because this is stupidely expensive
    return 180.f / (M_PIf)*std::acos(dot(v2, v1) / norm(v1) / norm(v2)) *
           (2.f * ((v1.x * v2.y - v2.x * v1.y) > 0) - 1.f);
}
