#include "ObjectiveSystem.h"

#include <Renderer.h>

#include "PostBox.h"

#include <nlohmann/json.hpp>

#include <numbers>

#ifndef M_PIf
#define M_PIf 3.14159265358979323846f
#endif

bool Task::isFinished() const
{
    return m_is_finished;
}

bool Objective::isFinished() const
{
    return m_is_finished;
}

void drawArrowTo(const Texture &arrow_texture, utils::Vector2f location, Renderer &window, Color color = {1, 1, 0, 1})
{
    auto ui_view = window.getDefaultView();

    auto center = window.m_view.getCenter();

    auto angle = utils::dir2angle(location - center);

    auto view_size = ui_view.getSize() * 0.95f;
    auto angle_threshold = utils::dir2angle(view_size);

    float distance_on_view;
    if (angle >= -angle_threshold && angle < angle_threshold)
    {
        distance_on_view = view_size.x / 2.1f / std::cos(angle * M_PIf / 180.f);
    }
    else if (angle >= angle_threshold && angle < 180 - angle_threshold)
    {
        distance_on_view = view_size.y / 2.05f / std::sin(angle * M_PIf / 180.f);
    }
    else if ((angle >= 180 - angle_threshold || angle < -180 + angle_threshold))
    {
        distance_on_view = -view_size.x / 2.1f / std::cos(angle * M_PIf / 180.f);
    }
    else
    {
        distance_on_view = -view_size.y / 2.05f / std::sin(angle * M_PIf / 180.f);
    }

    float actual_dist = utils::dist(location, center);
    if (!window.m_view.contains(location))
    { //! draw only when not on screen
        Sprite arrow_rect(arrow_texture);
        arrow_rect.setScale(15, 15);
        arrow_rect.m_color = color;
        arrow_rect.setRotation(glm::radians(angle));
        arrow_rect.setPosition(ui_view.getCenter() + distance_on_view * utils::angle2dir(angle));
        window.m_blend_factors.src_factor = BlendFactor::SrcAlpha;
        window.drawSprite(arrow_rect, "Arrow");
    }
}

ReachSpotTask::ReachSpotTask(GameObject &spot, Font &font, PostOffice &messenger, Quest* parent)
    : m_location(spot.getPosition()), Task(messenger, parent)
{
    m_font = &font;
}

void ReachSpotTask::onObservation(Trigger *trig)
{
    m_is_finished = true;
    trig->kill();
    complete();
}
// void ReachSpotObjective::fail()
// {
//     m_is_finished = true;
//     m_on_failure_callback();
// }

void ReachSpotTask::draw(Renderer &window, const TextureHolder &textures)
{
    auto old_view = window.m_view;

    drawArrowTo(*textures.get("Arrow"), m_location, window);

    window.m_view = window.getDefaultView();

    Text text;
    text.setFont(m_font);
    text.setText("Survey Area");
    text.setColor({255, 0, 0, 255});
    utils::Vector2f window_size = {
        static_cast<float>(window.getTargetSize().x),
        static_cast<float>(window.getTargetSize().y)};
    text.setPosition(window_size.x / 20.f, m_text_y_pos);
    window.drawText(text);

    window.m_view = old_view;
}

SurveySpotTask::SurveySpotTask(GameObject &spot, Font &font, PostOffice &messenger, Quest* parent)
    : m_surveyed(spot), Task(messenger, parent)
{
    m_font = &font;
}

void SurveySpotTask::onObservation(Trigger *trig)
{
    m_is_finished = true;
    trig->kill();
}

DestroyEntityTask::DestroyEntityTask(GameObject &target, Font &font, PostOffice &messenger, Quest* parent)
    : m_target(target), Task(messenger, parent)
{
    m_font = &font;

    target.setDestructionCallback([this](int id, ObjectType type)
                                  { m_is_finished = true; });
}

void DestroyEntityTask::onObservation(Trigger *trig)
{
    m_is_finished = true;
    trig->kill();
}

void DestroyEntityTask::draw(Renderer &window, const TextureHolder &textures)
{

    auto old_view = window.m_view;
    if (!m_is_finished)
    {
        drawArrowTo(*textures.get("Arrow"), m_target.getPosition(), window, {1, 0, 0, 1});
    }
    window.m_view = window.getDefaultView();

    Text text;
    text.setFont(m_font);
    text.setText("Kill  Alien");
    if (m_is_finished)
    {
        text.setColor({0, 255, 0, 255});
    }
    utils::Vector2f window_size = {
        static_cast<float>(window.getTargetSize().x),
        static_cast<float>(window.getTargetSize().y)};
    text.setPosition(window_size.x / 20.f, m_text_y_pos);

    window.drawText(text);
    window.m_view = old_view;
}

void ObjectiveSystem::add(std::shared_ptr<Quest> quest)
{
    m_quests.insert(quest);
    quest->start();
    // m_quests.at(objective_id)->m_id = objective_id;
}

void ObjectiveSystem::remove(int id)
{
    m_objectives.remove(id);
}


void Quest::draw(Renderer& window, const TextureHolder& textures)
{
    for(auto id : active_tasks_ids)
    {
        sub_tasks.at(id).task->draw(window, textures);
    }
}

void ObjectiveSystem::draw(Renderer &window, const TextureHolder &textures)
{

    float y_pos = window.getTargetSize().y / 20.f;
    for (auto& quest : m_quests)
    {
        quest->m_text_y_pos = y_pos;
        quest->draw(window, textures);
        y_pos += 50.f;
    }

    auto old_view = window.m_view;
    window.m_view = window.getDefaultView();
    window.drawAll();
    window.m_view = old_view;
}

void ObjectiveSystem::update()
{

    for (auto id : m_objectives.active_inds)
    {
        if (m_objectives.at(id)->isFinished())
        {
            // m_objectives.remove(id);
        }
    }
}

bool ObjectiveSystem::allFinished() const
{
    return m_all_quests_finished;
}

void DestroyNOfTypeTask::draw(Renderer &window, const TextureHolder &textures)
{
    auto old_view = window.m_view;
    window.m_view = window.getDefaultView();
    Text text;
    text.setFont(m_font);
    text.setText("Destroyed  " +
                 std::to_string(m_destroyed_count) + "/" + std::to_string(m_destroyed_target) +
                 +"  " + m_entity_name + "s");
    if (m_is_finished)
    {
        text.setColor({0, 255, 0, 255});
    }
    utils::Vector2f window_size = {
        static_cast<float>(window.getTargetSize().x),
        static_cast<float>(window.getTargetSize().y)};
    text.setPosition(window_size.x / 20.f, m_text_y_pos);

    window.drawText(text);
    window.m_view = old_view;
}

DestroyNOfTypeTask::DestroyNOfTypeTask(ObjectType type, std::string name, int destroyed_target_count,
                                       Font &font, PostOffice &messenger, Quest* parent)
    : m_type(type), m_destroyed_target(destroyed_target_count), m_entity_name(name), Task(messenger, parent)
{
    m_font = &font;
    m_on_entity_destroyed = std::make_unique<PostBox<EntityDiedEvent>>(messenger,
                                                                       [this](const auto &event_queue)
                                                                       {
            for(const auto& event : event_queue)
            {
                entityDestroyed(event.type, event.id);
            } });
}

void DestroyNOfTypeTask::entityDestroyed(ObjectType type, int id)
{

    if (m_active && type == m_type && !m_is_finished)
    {
        m_destroyed_count++;
        if (m_destroyed_count >= m_destroyed_target)
        {
            m_is_finished = true;
            m_on_completion_callback();
        }
    }
}



ObjectiveSystem::ObjectiveSystem(PostOffice &messanger)
    : p_messanger(&messanger)
{
    post_box = std::make_unique<PostBox<EntityDiedEvent>>(messanger, [](const auto &events)
                                                          {
        for(const EntityDiedEvent& event : events)
        {
            std::cout << "Entity: " << event.id << " DIED!" << std::endl; 
        } });
    m_objectives_postbox = std::make_unique<PostBox<QuestCompletedEvent>>(messanger, [this](const auto &events)
                                                                             {
        
        for(const QuestCompletedEvent& event : events)
        {
            // remove(event.id);
        } });
}

void Task::activate()
{
    m_active = true;
}
void Task::complete()
{
    assert(m_active);
    m_on_completion_callback();
    m_parent->onTaskCompletion(this);
};
void Task::fail() {
    // m_parent->onTaskCompletion(this);

};


// for convenience
using json = nlohmann::json;

void ObjectiveSystem::registerQuest(std::filesystem::path json_path)
{
    std::ifstream file("../quests.json");
    nlohmann::json quest_data = json::parse(file);

    assert(quest_data.is_array());

    // while()
}