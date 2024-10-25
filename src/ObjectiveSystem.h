#pragma once

#include <queue>
#include <string>
#include <functional>

#include <Renderer.h>
#include <Font.h>

#include "Entities/Triggers.h"
#include "Utils/ObjectPool.h"


class Objective
{

public:
    virtual void draw(Renderer &window) = 0;
    virtual ~Objective() = default;
    bool isFinished() const;

protected:
    void drawArrowTo(utils::Vector2f location, Renderer &window, Color color);

public:
    int m_id;
    float m_text_y_pos;
    std::function<void()> m_on_completion_callback = []() {};

protected:
    Font *m_font = nullptr;
    bool m_is_finished = false;
    TextureHolder m_textures;
};

class ReachSpotObjective : public Objective, public Observer<Trigger>
{

public:
    ReachSpotObjective(GameObject &spot, Font &font);
    virtual ~ReachSpotObjective() = default;
    
    virtual void update(Trigger *trig) override;
    virtual void draw(Renderer &window) override;

private:
    utils::Vector2f m_location;
};

class SurveySpot : public Objective, public Observer<Trigger>
{

public:
    SurveySpot(GameObject &spot, Font &font);
    virtual ~SurveySpot() = default;

    virtual void update(Trigger *trig) override;
    virtual void draw(Renderer &window) override{}

private:
    float m_surveyed_timer = 0;
    GameObject &m_surveyed;
};

class DestroyEntity : public Objective, public Observer<Trigger>
{

public:
    DestroyEntity(GameObject& target, Font &font);
    virtual ~DestroyEntity() = default;

    virtual void update(Trigger *trig) override;

    virtual void draw(Renderer &window) override;

private:
    GameObject& m_target;
};

class DestroyNOfType : public Objective
{

public:
    DestroyNOfType(ObjectType type, std::string name, int destroyed_target_count, Font &font);
    virtual ~DestroyNOfType() = default;

    void entityDestroyed(ObjectType type, int id);

    virtual void draw(Renderer &window) override;

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
    void draw(Renderer &window);
    void update();
    bool allFinished() const;
    void entityDestroyed(ObjectType type, int id);

private:
    // std::deque<std::shared_ptr<Objective>> m_objectives;
    utils::ObjectPool<std::shared_ptr<Objective>, 10> m_objectives;
    bool m_all_quests_finished = false;
};