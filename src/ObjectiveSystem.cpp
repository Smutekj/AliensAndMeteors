#include "ObjectiveSystem.h"

#include <Renderer.h>

#include <numbers>

#ifndef M_PIf
#define M_PIf  3.14159265358979323846f
#endif

bool Objective::isFinished() const
{
    return m_is_finished;
}

void Objective::drawArrowTo(utils::Vector2f location, Renderer &window, Color color = {0,1,0,1})
{
    auto center = window.m_view.getCenter();

    Sprite m_arrow_rect;
    m_arrow_rect.setScale(6, 6);
    m_arrow_rect.m_color = color;
    m_arrow_rect.setTexture(*m_textures.get("Arrow"));

    auto angle = utils::dir2angle(location - center);
    m_arrow_rect.setRotation(glm::radians(angle));

    auto view_size = window.m_view.getSize() * 0.95f;
    auto angle_threshold = utils::dir2angle(view_size);

    float distance_on_view;
    if (angle >= -angle_threshold && angle < angle_threshold)
    {
        distance_on_view = view_size.x / 2.1 / std::cos(angle * M_PIf / 180.f);
    }
    else if (angle >= angle_threshold && angle < 180 - angle_threshold)
    {
        distance_on_view = view_size.y / 2.05f / std::sin(angle * M_PIf / 180.f);
    }
    else if ((angle >= 180 - angle_threshold || angle < -180 + angle_threshold))
    {
        distance_on_view = -view_size.x / 2.1 / std::cos(angle * M_PIf / 180.f);
    }
    else
    {
        distance_on_view = -view_size.y / 2.05 / std::sin(angle * M_PIf / 180.f);
    }

    float actual_dist = utils::dist(location, center);
    if (actual_dist > distance_on_view)
    { //! draw only when not on screen
        m_arrow_rect.setPosition(center + distance_on_view * utils::angle2dir(angle));
        window.drawSprite(m_arrow_rect, "Instanced");
    }
}

ReachSpotObjective::ReachSpotObjective(GameObject &spot, Font &font)
    : m_location(spot.getPosition())
{
    m_font = &font;
    m_textures.add("Arrow", "../Resources/Textures/arrow.png");
}

void ReachSpotObjective::update(Trigger *trig)
{
    m_is_finished = true;
    trig->kill();
    m_on_completion_callback();
}

void ReachSpotObjective::draw(Renderer &window)
{
    auto old_view = window.m_view;

    drawArrowTo(m_location, window);

    window.m_view = window.getDefaultView();
    Text text;
    text.setFont(m_font);
    text.setText("Survey Area");
    utils::Vector2f window_size = {
        static_cast<float>(window.getTargetSize().x),
        static_cast<float>(window.getTargetSize().y)};
    text.setPosition(window_size.x / 20.f, m_text_y_pos);
    window.drawText(text, "Text");
    window.m_view = old_view;
}

SurveySpot::SurveySpot(GameObject &spot, Font &font)
    : m_surveyed(spot)
{
    m_font = &font;
    m_textures.add("Arrow", "../Resources/Textures/arrow.png");
}

void SurveySpot::update(Trigger *trig)
{
    m_is_finished = true;
    trig->kill();
    m_on_completion_callback();
}

DestroyEntity::DestroyEntity(GameObject &target, Font &font)
    : m_target(target)
{
    m_font = &font;
    m_textures.add("Arrow", "../Resources/Textures/arrow.png");
}

void DestroyEntity::update(Trigger *trig)
{
    m_is_finished = true;
    trig->kill();
}

void DestroyEntity::draw(Renderer &window)
{

    auto old_view = window.m_view;
    if (!m_is_finished)
    {
        drawArrowTo(m_target.getPosition(), window, {1,0,0,1});
    }
    window.m_view = window.getDefaultView();

    Text text;
    text.setFont(m_font);
    text.setText("Kill  Alien");
    if (m_is_finished)
    {
        text.setColor({0,255,0,255});
    }
    utils::Vector2f window_size = {
        static_cast<float>(window.getTargetSize().x),
        static_cast<float>(window.getTargetSize().y)};
    text.setPosition(window_size.x / 20.f, m_text_y_pos);

    window.drawText(text, "Text");
    window.m_view = old_view;
}

void ObjectiveSystem::add(std::shared_ptr<Objective> obj)
{

    int objective_id = m_objectives.addObject(obj);
    m_objectives.at(objective_id)->m_id = objective_id;
}

void ObjectiveSystem::remove(int id)
{
    m_objectives.remove(id);
}

void ObjectiveSystem::draw(Renderer &window)
{
    float y_pos = window.getTargetSize().y / 20.f;
    for (auto id : m_objectives.active_inds)
    {
        m_objectives.at(id)->m_text_y_pos = y_pos;
        m_objectives.at(id)->draw(window);
        y_pos += 50.f;
    }
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

void DestroyNOfType::draw(Renderer &window)
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
        text.setColor({0,255,0,255});
    }
    utils::Vector2f window_size = {
        static_cast<float>(window.getTargetSize().x),
        static_cast<float>(window.getTargetSize().y)};
    text.setPosition(window_size.x / 20.f, m_text_y_pos);

    window.drawText(text, "Text");
    window.m_view = old_view;
}


    DestroyNOfType::DestroyNOfType(ObjectType type, std::string name, int destroyed_target_count, Font &font)
        : m_type(type), m_destroyed_target(destroyed_target_count), m_entity_name(name)
    {
        m_font = &font;
    }

    void DestroyNOfType::entityDestroyed(ObjectType type, int id)
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