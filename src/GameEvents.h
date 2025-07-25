#pragma once

#include "Vector2.h"


enum class DeathCause
{
    TooMuchAlcohol,
    CrashedIntoObject,
    KilledByPlayer,
    Timeout,
};

struct EntityDiedEvent
{
    // EntityType type;
    int id;
    utils::Vector2f where;
};

struct EntityCreatedEvent
{
    // EntityType type;
    int id;
    utils::Vector2f where;
};

struct ObjectiveFinishedEvent
{
    int id;
};


