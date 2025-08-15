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

struct DamageReceivedEvent
{
    ObjectType cause_type;
    int cause_entity_id;
    ObjectType receiver_type;
    int receiver_id;

    float dmg = 0.f;
};

struct HealthChangedEvent
{
    int entity_id;
    float old_hp;
    float new_hp;
};

struct StartedBossFightEvent
{
    int boss_id;
    int boss_fight_id = -1; ///???
};


