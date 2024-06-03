#pragma once

#include <queue>
#include <functional>

#include "Entities/Triggers.h"

#include "Utils/ObjectPool.h"

#include "ResourceIdentifiers.h"

namespace sf
{
    class RenderWindow;
    class Font;
}

class Objective
{

public:
    virtual void draw(sf::RenderWindow &window) = 0;
    bool isFinished() const;

protected:
    void drawArrowTo(sf::Vector2f location, sf::RenderWindow &window, sf::Color color);

public:
    int m_id;
    float m_text_y_pos;
    std::function<void()> m_on_completion_callback = []() {};

protected:
    sf::Font *m_font;
    bool m_is_finished = false;
    TextureHolder m_textures;
};

class ReachSpotObjective : public Objective, public Observer<Trigger>
{

public:
    ReachSpotObjective(GameObject &spot, sf::Font &font);
    virtual void update(Trigger *trig) override;
    virtual void draw(sf::RenderWindow &window) override;

private:
    sf::Vector2f m_location;
};

class SurveySpot : public Objective, public Observer<Trigger>
{

    SurveySpot(GameObject &spot, sf::Font &font);
    virtual void update(Trigger *trig) override;

private:
    float m_surveyed_timer = 0;
    GameObject &m_surveyed;
};

class DestroyEntity : public Objective, public Observer<Trigger>
{

public:
    DestroyEntity(GameObject& target, sf::Font &font);

    virtual void update(Trigger *trig) override;

    virtual void draw(sf::RenderWindow &window) override;

private:
    GameObject& m_target;
};

class DestroyNOfType : public Objective
{

public:
    DestroyNOfType(ObjectType type, std::string name, int destroyed_target_count, sf::Font &font)
        : m_type(type), m_destroyed_target(destroyed_target_count), m_entity_name(name)
    {
        m_font = &font;
    }

    void entityDestroyed(ObjectType type, int id)
    {
        if (type == m_type && !m_is_finished)
        {
            m_destroyed_count++;
            if (m_destroyed_count >= m_destroyed_target)
            {
                m_is_finished = true;
                m_on_completion_callback();
            }
        }
    }

    virtual void draw(sf::RenderWindow &window) override;

private:
    ObjectType m_type;
    int m_destroyed_count = 0;
    int m_destroyed_target;

    std::string m_entity_name;
};

class ObjectiveSystem
{

public:
    void add(std::shared_ptr<Objective> obj);
    void remove(int id);
    void draw(sf::RenderWindow &window);
    void update();
    bool allFinished() const;
    void entityDestroyed(ObjectType type, int id);

private:
    // std::deque<std::shared_ptr<Objective>> m_objectives;
    ObjectPool<std::shared_ptr<Objective>, 10> m_objectives;
    bool m_all_quests_finished = false;
};