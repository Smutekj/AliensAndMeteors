#include "ObjectiveSystem.h"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Text.hpp>

bool Objective::isFinished() const
{
    return m_is_finished;
}

void Objective::drawArrowTo(sf::Vector2f location, sf::RenderWindow &window, sf::Color color = sf::Color::Green)
{

    auto center = window.getView().getCenter();

    sf::RectangleShape m_arrow_rect;
    m_arrow_rect.setSize({6, 6});
    m_arrow_rect.setOrigin({3, 3});
    m_arrow_rect.setFillColor(color);
    m_arrow_rect.setTexture(&m_textures.get(Textures::ID::Arrow));

    auto angle = dir2angle(location - center);
    m_arrow_rect.setRotation(angle);

    auto view_size = window.getView().getSize() * 0.95f;
    auto angle_threshold = dir2angle(view_size);

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

    float actual_dist = dist(location, center);
    if (actual_dist > distance_on_view)
    { //! draw only when not on screen
        m_arrow_rect.setPosition(center + distance_on_view * angle2dir(angle));
        window.draw(m_arrow_rect);
    }
}

ReachSpotObjective::ReachSpotObjective(GameObject &spot, sf::Font &font)
    : m_location(spot.getPosition())
{
    m_font = &font;
    m_textures.load(Textures::ID::Arrow, "../Resources/arrow.png");
}

void ReachSpotObjective::update(Trigger *trig)
{
    m_is_finished = true;
    trig->kill();
    m_on_completion_callback();
}

void ReachSpotObjective::draw(sf::RenderWindow &window)
{
    auto old_view = window.getView();

    drawArrowTo(m_location, window);

    window.setView(window.getDefaultView());
    sf::Text text;
    text.setFont(*m_font);
    text.setString("Survey Area");
    sf::Vector2f window_size = {
        static_cast<float>(window.getSize().x),
        static_cast<float>(window.getSize().y)};
    text.setPosition({window_size.x / 20.f, m_text_y_pos});
    window.draw(text);
    window.setView(old_view);
}

SurveySpot::SurveySpot(GameObject &spot, sf::Font &font)
    : m_surveyed(spot)
{
    m_font = &font;
    m_textures.load(Textures::ID::Arrow, "../Resources/arrow.png");
}

void SurveySpot::update(Trigger *trig)
{
    m_is_finished = true;
    trig->kill();
    m_on_completion_callback();
}

DestroyEntity::DestroyEntity(GameObject &target, sf::Font &font)
    : m_target(target)
{
    m_font = &font;
    m_textures.load(Textures::ID::Arrow, "../Resources/arrow.png");
}

void DestroyEntity::update(Trigger *trig)
{
    m_is_finished = true;
    trig->kill();
}

void DestroyEntity::draw(sf::RenderWindow &window)
{

    auto old_view = window.getView();
    if (!m_is_finished)
    {
        drawArrowTo(m_target.getPosition(), window, sf::Color::Red);
    }
    window.setView(window.getDefaultView());

    sf::Text text;
    text.setFont(*m_font);
    text.setString("Kill  Alien");
    if (m_is_finished)
    {
        text.setFillColor(sf::Color::Green);
    }
    sf::Vector2f window_size = {
        static_cast<float>(window.getSize().x),
        static_cast<float>(window.getSize().y)};
    text.setPosition({window_size.x / 20.f, m_text_y_pos});

    window.draw(text);
    window.setView(old_view);
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

void ObjectiveSystem::draw(sf::RenderWindow &window)
{
    float y_pos = window.getSize().y / 20.f;
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

void DestroyNOfType::draw(sf::RenderWindow &window)
{
    auto old_view = window.getView();
    window.setView(window.getDefaultView());
    sf::Text text;
    text.setFont(*m_font);
    text.setString("Destroyed  " +
                   std::to_string(m_destroyed_count) + "/" + std::to_string(m_destroyed_target) +
                   +"  " + m_entity_name + "s");
    if (m_is_finished)
    {
        text.setFillColor(sf::Color::Green);
    }
    sf::Vector2f window_size = {
        static_cast<float>(window.getSize().x),
        static_cast<float>(window.getSize().y)};
    text.setPosition({window_size.x / 20.f, m_text_y_pos});

    window.draw(text);
    window.setView(old_view);
}