#include <omp.h>
#include <iostream>
#include <fstream>
#include <unordered_set>

#include "Game.h"
#include "UI.h"
#include "SoundModule.h"
#include "Geometry.h"

#include "imgui.h"
#include "imgui-SFML.h"

#include "Entities.h"

Game::Game(sf::RenderWindow &window, KeyBindings &bindings)
    : m_window(window), key_binding(bindings), m_ui(window)
{

    ImGui::SFML::Init(m_window);

    t.create(window.getSize().x, window.getSize().y);
    light_texture.create(window.getSize().x, window.getSize().y);
    t.setSmooth(true);
    light_texture.setSmooth(true);

    sf::Vector2f box_size = {static_cast<float>(Geometry::BOX[0]), static_cast<float>(Geometry::BOX[1])};
    sf::Vector2i n_cells = {Geometry::N_CELLS[0], Geometry::N_CELLS[1]};
    sf::Vector2f cell_size = {
        static_cast<float>(Geometry::BOX[0] / n_cells.x),
        static_cast<float>(Geometry::BOX[1] / n_cells.y)};

    m_world = std::make_unique<GameWorld>();

    m_player = &static_cast<PlayerEntity &>(m_world->addObject(ObjectType::Player));
    m_player->setPosition({box_size.x / 2.f, box_size.y / 2.f});

    auto view = window.getView();
    view.setCenter(m_player->getPosition());
    view.setSize({100.f, 100.f * window.getSize().y / window.getSize().x});
    window.setView(view);
    default_view = view;

    font.loadFromFile("../Resources/DigiGraphics.ttf");
    health_text.setFont(font);
    health_text.setFillColor(sf::Color::Red);
}

void Game::moveView(sf::RenderWindow &window)
{
    auto view = window.getView();
    auto vp1 = window.getViewport(view);

    const sf::Vector2f view_size = view.getSize(); // * (view.getViewport().width);

    auto threshold = view.getSize() / 2.f - view.getSize() / 3.f;
    auto dx = m_player->getPosition().x - view.getCenter().x;
    auto dy = m_player->getPosition().y - view.getCenter().y;
    auto view_max = view.getCenter() + view.getSize() / 2.f;
    auto view_min = view.getCenter() - view.getSize() / 2.f;

    //! move view when approaching sides
    if (dx > threshold.x && view_max.x < Geometry::BOX[0])
    {
        view.setCenter(view.getCenter() + sf::Vector2f{dx - threshold.x, 0});
    }
    else if (dx < -threshold.x && view_min.x > 0)
    {
        view.setCenter(view.getCenter() + sf::Vector2f{dx + threshold.x, 0});
    }
    if (dy > threshold.y && view_max.y < Geometry::BOX[1])
    {
        view.setCenter(view.getCenter() + sf::Vector2f{0, dy - threshold.y});
    }
    else if (dy < -threshold.y && view_min.y > 0)
    {
        view.setCenter(view.getCenter() + sf::Vector2f{0, dy + threshold.y});
    }

    //! look from higher distance when boosting
    float booster_ratio = m_player->speed / m_player->max_speed;
    // view.setSize(default_view.getSize()*(1 + booster_ratio/3.f));

    window.setView(view);
}

void Game::handleEvent(const sf::Event &event)
{
    auto mouse_position = m_window.mapPixelToCoords(sf::Mouse::getPosition(m_window));
    ImGui::SFML::ProcessEvent(event);

    if (event.type == sf::Event::KeyPressed)
    {
        if (event.key.code == key_binding[PlayerControl::BOOST])
        {
            m_player->is_boosting = true;
        }
    }
    if (event.type == sf::Event::KeyReleased)
    {
        if (event.key.code == key_binding[PlayerControl::SHOOT_LASER])
        {
            auto dir = angle2dir(m_player->m_angle);

            auto &laser = static_cast<Laser2 &>(m_world->addObject(ObjectType::Laser));
            laser.setPosition(m_player->getPosition());
            laser.setAngle(m_player->m_angle);
            laser.setOwner(m_player);
        }
        if (event.key.code == key_binding[PlayerControl::THROW_BOMB])
        {
            auto dir = angle2dir(m_player->m_angle);

            auto &bomb = m_world->addObject(ObjectType::Bomb);
            bomb.setPosition(m_player->getPosition());
            bomb.m_vel = (50.f + m_player->speed) * angle2dir(m_player->m_angle);
            bomb.setAngle(m_player->m_angle);
        }
        if (event.key.code == key_binding[PlayerControl::BOOST])
        {
            m_player->is_boosting = false;
        }
    }
    // if (event.type == sf::Event::MouseButtonPressed)
    // {

    //     if (event.mouseButton.button == sf::Mouse::Right &&
    //         sf::Mouse::isButtonPressed(sf::Mouse::Right))
    //     {
    //         auto &new_enemy = m_world->addObject(ObjectType::Enemy);
    //         new_enemy.setPosition(mouse_position);
    //     }

    // }
    if (event.type == sf::Event::MouseButtonReleased)
    {
        if (event.mouseButton.button == sf::Mouse::Right)
        {
            auto &station = m_world->addObject(ObjectType::SpaceStation);
            station.setPosition(mouse_position);
        }
    }

    if (event.type == sf::Event::MouseWheelMoved)
    {
        auto view = m_window.getView();
        if (event.mouseWheelScroll.wheel > 0)
        {
            view.zoom(0.9f);
        }
        else
        {
            view.zoom(1. / 0.9f);
        }
        m_window.setView(view);
    }
}

//! \brief parse events and normal input
//! \note  right now this is just a placeholder code until I make a nice OOP solution with bindings and stuff
void Game::parseInput(sf::RenderWindow &window)
{

    if (sf::Keyboard::isKeyPressed(key_binding[PlayerControl::MOVE_FORWARD]))
    {
        m_player->speed += m_player->acceleration;
    }
    else if (sf::Keyboard::isKeyPressed(key_binding[PlayerControl::MOVE_BACK]))
    {
        m_player->speed -= m_player->acceleration;
    }
    else
    {
        m_player->speed /= 1.1f;
    }
    if (sf::Keyboard::isKeyPressed(key_binding[PlayerControl::STEER_LEFT]))
    {
        m_player->m_angle -= 5.f;
    }
    if (sf::Keyboard::isKeyPressed(key_binding[PlayerControl::STEER_RIGHT]))
    {
        m_player->m_angle += 5.f;
    }
}

void Game::update(const float dt, sf::RenderWindow &window)
{

    moveView(window);
    parseInput(window);

    if (m_player->health < 0)
    {
        state = GameState::PLAYER_DIED;
    }

    auto tic = std::chrono::high_resolution_clock::now();
    m_world->update(dt);
    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(
                     tic - std::chrono::high_resolution_clock::now())
              << " ms\n";
}

void Game::draw(sf::RenderWindow &window)
{

    t.setView(window.getView());
    t.clear(sf::Color::Black);
    m_world->draw(t, window);

    t.display();

    bloom.doTheThing(t, window);
    // darken(t, light_texture);
    // lights.draw(light_texture, window);

    m_ui.draw(window);
    drawUI(window);
}

void Game::drawUI(sf::RenderWindow &window)
{

    auto old_view = window.getView();
    window.setView(window.getDefaultView());

    std::string hp_text = "HP: " + std::to_string(m_player->health);

    sf::RectangleShape health_rect;

    sf::Vector2f healt_comp_uisize = {(float)window.getSize().x * 1.f / 6.f, (float)window.getSize().y * 1.f / 10.f};
    sf::Vector2f healt_comp_min = {(float)window.getSize().x * 5.f / 6.f, (float)window.getSize().y * 9.f / 10.f};

    health_text.setPosition(healt_comp_min);
    health_text.setString(hp_text);

    health_rect.setPosition({healt_comp_min.x, healt_comp_min.y + healt_comp_uisize.y / 2.f});
    float health_ratio = m_player->health / (float)m_player->max_health;
    health_rect.setSize({health_ratio * healt_comp_uisize.x * 3.f / 4.f, healt_comp_uisize.y / 3.f});
    health_rect.setFillColor(sf::Color::Red);
    health_rect.setOutlineThickness(1.f);
    health_rect.setOutlineColor(sf::Color::Black);
    window.draw(health_text);
    window.draw(health_rect);

    sf::Vector2f booster_size = {(float)window.getSize().x * 1.f / 6.f, health_rect.getSize().y};
    sf::Vector2f booster_pos = {(float)window.getSize().x * 1.f / 6.f, health_rect.getPosition().y};

    sf::VertexArray booster_rect;
    booster_rect.setPrimitiveType(sf::Quads);
    booster_rect.resize(4);

    sf::RectangleShape booster_boundary(booster_size);
    booster_boundary.setOutlineThickness(3.f);
    booster_boundary.setOutlineColor(sf::Color::Black);
    booster_boundary.setPosition(booster_pos);
    booster_boundary.setFillColor(sf::Color::Transparent);
    window.draw(booster_boundary);

    float booster_ratio = m_player->boost_heat / m_player->max_boost_heat;
    booster_size.x *= booster_ratio;
    sf::Color booster_color(255 * (booster_ratio), 255 * (1 - booster_ratio), 0);

    booster_rect[0] = {{booster_pos.x, booster_pos.y}, sf::Color::Yellow};
    booster_rect[1] = {{booster_pos.x + booster_size.x, booster_pos.y}, booster_color};
    booster_rect[2] = {{booster_pos.x + booster_size.x, booster_pos.y + booster_size.y}, booster_color};
    booster_rect[3] = {{booster_pos.x, booster_pos.y + booster_size.y}, sf::Color::Yellow};
    window.draw(booster_rect);

    sf::Vector2f score_comp_uisize = {(float)window.getSize().x * 1.f / 6.f, (float)window.getSize().y * 1.f / 10.f};
    sf::Vector2f score_comp_min = {(float)window.getSize().x / 2.f - score_comp_uisize.x / 2.f,
                                   (float)window.getSize().y * 1.f / 20.f};
    health_text.setString("Score: " + std::to_string(m_score));
    health_text.setPosition(score_comp_min.x - health_text.getLocalBounds().width / 2.f, score_comp_min.y);

    window.draw(health_text);

    window.setView(old_view);
}
