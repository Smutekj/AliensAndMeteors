#pragma once

#include <functional>

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/CircleShape.hpp>

#include "../GameObject.h"

class PlayerEntity;
class GameWorld;
class Trigger;

template <class T>
class Observer
{
public:
    Observer() {}
    virtual ~Observer() {}
    virtual void update(T *subject) = 0;
};

template <class T>
class Subject
{
public:
    Subject() {}
    virtual ~Subject() {}
    void attach(Observer<T> &observer)
    {
        m_observers.push_back(&observer);
    }

    void notify()
    {
        for (auto &observer : m_observers)
        {
            observer->update(static_cast<T *>(this));
        }
    }

private:
    std::vector<Observer<T> *> m_observers;
};

class Trigger : public GameObject, public Subject<Trigger>
{

protected:
    std::function<void()> m_callback = []() {};

public:
    Trigger(GameWorld *world, TextureHolder &textures);

    virtual void update(float dt) = 0;
    virtual void onCreation() = 0;
    virtual void onDestruction() = 0;
    virtual void draw(sf::RenderTarget &target) = 0;
    virtual void onCollisionWith(GameObject &obj, CollisionData &c_data) = 0;

    void setCallback(std::function<void()> new_callback)
    {
        m_callback = new_callback;
    }

    void callback()
    {
        m_callback();
        notify();
    }
    const std::function<void()> &getCallback() const
    {
        return m_callback;
    }
};

struct Timer : public Trigger
{
    float m_cooldown = 5.f;
    float m_time = 0;

    Timer(GameWorld *world, TextureHolder &textures);
    virtual ~Timer() = default;

    virtual void onCreation() override;
    virtual void onDestruction() override;
    virtual void draw(sf::RenderTarget &target) override;
    virtual void onCollisionWith(GameObject &obj, CollisionData &c_data) override;

    virtual void update(float dt) override;
};

class EntityDestroyed : public Trigger
{

public:
    EntityDestroyed(GameWorld *world, TextureHolder &textures, GameObject *entity);
    virtual ~EntityDestroyed() override;

    virtual void update(float dt) override;
    virtual void onCreation() override;
    virtual void onDestruction() override;
    virtual void draw(sf::RenderTarget &target) override;
    virtual void onCollisionWith(GameObject &obj, CollisionData &c_data) override;

private:
    GameObject *m_watched_entity = nullptr;
};

class ReachPlace : public Trigger
{

public:
    ReachPlace(GameWorld *world, TextureHolder &textures, PlayerEntity *player = nullptr);
    virtual ~ReachPlace() override;

    virtual void update(float dt) override;
    virtual void onCreation() override;
    virtual void onDestruction() override;
    virtual void draw(sf::RenderTarget &target) override;
    virtual void onCollisionWith(GameObject &obj, CollisionData &c_data) override;

private:
    sf::CircleShape m_spot;
    sf::RectangleShape m_arrow_rect;
    PlayerEntity *m_player;
};

class StayAtPlace : public Trigger
{

public:
    StayAtPlace(GameWorld *world, TextureHolder &textures, PlayerEntity *player, float trigger_time);
    virtual ~StayAtPlace() override = default;

    virtual void update(float dt) override;
    virtual void onCreation() override;
    virtual void onDestruction() override;
    virtual void draw(sf::RenderTarget &target) override;
    virtual void onCollisionWith(GameObject &obj, CollisionData &c_data) override;

private:
    sf::CircleShape m_spot;
    sf::RectangleShape m_arrow_rect;
    PlayerEntity *m_player;
    float m_trigger_time = 0.f;
    float m_stay_time = 0.f;
};
