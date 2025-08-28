#pragma once

#include <random>

#include "../Vertex.h"

inline float randf(float min = 0, float max = 1)
{
    return (rand() / static_cast<float>(RAND_MAX)) * (max - min) + min;
}

inline Vec2 randomPosInBox(Vec2 ul_corner,
                           Vec2 box_size)
{
    return {ul_corner.x + rand() / static_cast<float>(RAND_MAX) * box_size.x,
            ul_corner.y + rand() / static_cast<float>(RAND_MAX) * box_size.y};
}

inline int randi(int min, int max)
{
    return rand() % (max - min + 1) + min;
}

inline int randi(int max)
{
    return randi(0, max);
}

// template<template<class> class ContainerType, typename DataType>
// concept  Indexable = requires(ContainerType<DataType> a)
// {
//     { a.at() } -> DataType&;
//     { a.size() } -> std::size_t;
// };

template <class T>
inline auto randomValue(T &&values)
{
    return values.at(randi(0, values.size() - 1));
}

class Random
{
public:
    Random()
        : rng(std::random_device{}())
    {
    }

    Random(int seed)
        : rng(seed)
    {
    }

    int getInt(int min, int max)
    {
        std::uniform_int_distribution<std::mt19937::result_type> dist(min, max); // distribution in range [1, 6]
        return dist(rng);
    }

private:
    std::mt19937 rng;
};
