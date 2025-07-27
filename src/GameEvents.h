#pragma once

#include "Vector2.h"
#include "GameObject.h"

enum class DeathCause
{
    TooMuchAlcohol,
    CrashedIntoObject,
    KilledByPlayer,
    Timeout,
};

struct EntityDiedEvent
{
    ObjectType type;
    int id;
    utils::Vector2f where;
};

struct EntityCreatedEvent
{
    // EntityType type;
    int id;
    utils::Vector2f where;
};

enum class ObjectiveEndCause
{
    Completed,
    Progressed,
    Failed
};

struct ObjectiveFinishedEvent
{
    int id;
    ObjectiveEndCause reason;
};

struct ObjectiveProgressedEvent
{
    int id;
};

struct QuestProgressedEvent
{
    int id;
};

struct QuestFailedEvent
{
    int id;
};
struct QuestCompletedEvent
{
    int id;
};

//! collision events
struct CollisionEventEntities
{
    int id_a;
    int id_b;
};
struct CollisionEventTypes
{
    ObjectType type_a;
    ObjectType type_b;
};

struct CollisionEventTypeEntity
{
    int id_a;
    ObjectType type_b;
};


