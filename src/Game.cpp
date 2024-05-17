#include <omp.h>
#include <iostream>
#include <fstream>
#include <unordered_set>

#include "Game.h"
// #include "UI.h"
#include "SoundModule.h"
#include "Geometry.h"

#include "imgui.h"
#include "imgui-SFML.h"

#include <iostream>

Game::Game(sf::RenderWindow &window, KeyBindings &bindings)
    : bullet_world(player), m_window(window), lights(poly_manager), key_binding(bindings)
{

    t.create(window.getSize().x, window.getSize().y);
    light_texture.create(window.getSize().x, window.getSize().y);
    t.setSmooth(true);
    light_texture.setSmooth(true);

    sf::Vector2f box_size = {static_cast<float>(Geometry::BOX[0]), static_cast<float>(Geometry::BOX[1])};
    sf::Vector2i n_cells = {Geometry::N_CELLS[0], Geometry::N_CELLS[1]};
    sf::Vector2f cell_size = {
        static_cast<float>(Geometry::BOX[0] / n_cells.x),
        static_cast<float>(Geometry::BOX[1] / n_cells.y)};

    player_particles = std::make_unique<Particles>(player.pos, 100);

    bool player_is_inside_meteor = true;
    while (player_is_inside_meteor)
    {
        player.pos = randomPosInBox();

        player_is_inside_meteor = false;
        auto meteors = poly_manager.getNearestMeteors(player.pos, player.radius);
        for (auto *meteor : meteors)
        {
            auto mvt = meteor->getMVTOfSphere(player.pos, player.radius);
            if (norm2(mvt) > 0.0001f)
            {
                player_is_inside_meteor = true;
            }
        }
    }
    player.pos = {box_size.x / 2.f, box_size.y / 2.f};

    poly_manager.addRegularObstacle(ObstacleType::POWERUP, player.pos);

    auto view = window.getView();
    view.setCenter(player.pos);
    view.setSize({100.f, 100.f * window.getSize().y / window.getSize().x});
    window.setView(view);
    default_view = view;

    textures.load(Textures::ID::PlayerShip, "../Resources/playerShip.png");

    player_shape.setOrigin({player.radius / 2.f, player.radius / 2.f});
    player_shape.setPosition(player.pos);
    player_shape.setSize({4.f, 4.f});
    player_shape.setRotation(0);
    player_shape.setTexture(&textures.get(Textures::ID::PlayerShip));

    boid_world.player = &player;
    boid_world.polygons = &poly_manager;
    boid_world.p_bs = &bullet_world;

    bullet_world.p_boids = &boid_world;
    bullet_world.p_meteors = &poly_manager;
    bullet_world.p_effects = &effects;
    bullet_world.p_game = this;

    font.loadFromFile("../Resources/DigiGraphics.ttf");
    health_text.setFont(font);
    health_text.setFillColor(sf::Color::Red);
}

void Game::moveView(sf::RenderWindow &window)
{
    auto view = window.getView();
    auto vp1 = window.getViewport(view);

    const sf::Vector2f view_size = view.getSize(); // * (view.getViewport().width);
    const auto left_border = window.getViewport(view).left;
    const auto right_border = left_border + window.getViewport(view).width;
    const auto top_border = window.getViewport(view).top;
    const auto bottom_border = top_border + window.getViewport(view).height;

    const auto &mouse_pos = sf::Mouse::getPosition(window);
    const auto &mouse_coords = window.mapPixelToCoords(sf::Mouse::getPosition(window));

    bool mouse_in_left_border = mouse_pos.x < 0 + 5;
    bool mouse_in_top_border = mouse_pos.y < top_border + 5;
    bool mouse_in_right_border = mouse_pos.x > right_border - 5;
    bool mouse_in_bottom_border = mouse_pos.y > bottom_border - 5;

    auto threshold = view.getSize() / 2.f - view.getSize() / 3.f;
    auto dx = player.pos.x - view.getCenter().x;
    auto dy = player.pos.y - view.getCenter().y;
    auto view_max = view.getCenter() + view.getSize() / 2.f;
    auto view_min = view.getCenter() - view.getSize() / 2.f;
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

    float booster_ratio = player.speed/player.max_speed;
    view.setSize(default_view.getSize()*(1 + booster_ratio/3.f));

    window.setView(view);
}

void Game::handleEvent(const sf::Event &event)
{
    auto mouse_position = m_window.mapPixelToCoords(sf::Mouse::getPosition(m_window));

    if (event.type == sf::Event::Resized)
    {
    }

    if (event.type == sf::Event::KeyPressed)
    {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
        {
            bullet_world.spawnBullet(-1, player.pos, angle2dir(player.angle));
        }
        if (event.key.code == key_binding[PlayerControl::BOOST])
        {
            player.is_boosting = true;
        }
    }
    if (event.type == sf::Event::KeyReleased)
    {

        if (event.key.code == key_binding[PlayerControl::SHOOT_LASER])
        {
            auto dir = angle2dir(player.angle);

            bullet_world.createLaser(-1, player.pos, player.pos + dir * 200.f);
        }
        if (event.key.code == key_binding[PlayerControl::THROW_BOMB])
        {
            auto dir = angle2dir(player.angle);

            bullet_world.createBomb(-1, player.pos, 150.f * angle2dir(player.angle));
        }
        if (event.key.code == key_binding[PlayerControl::BOOST])
        {
            player.is_boosting = false;
        }
    }
    if (event.type == sf::Event::MouseButtonPressed)
    {

        if (event.mouseButton.button == sf::Mouse::Right &&
            sf::Mouse::isButtonPressed(sf::Mouse::Right))
        {
            boid_world.spawnBoss(mouse_position);
        }

        else if (event.mouseButton.button == sf::Mouse::Right)
        {
        }
    }

    else if (event.type == sf::Event::MouseButtonReleased)
    {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl))
        {

            poly_manager.addRandomMeteorAt(mouse_position, {5, 5});
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
        player.speed += player.acceleration;
    }
    else if (sf::Keyboard::isKeyPressed(key_binding[PlayerControl::MOVE_BACK]))
    {
        player.speed -= player.acceleration;
    }
    else
    {
        player.speed /= 1.1f;
    }
    if (sf::Keyboard::isKeyPressed(key_binding[PlayerControl::STEER_LEFT]))
    {
        player.angle -= 5.f;
    }
    if (sf::Keyboard::isKeyPressed(key_binding[PlayerControl::STEER_RIGHT]))
    {
        player.angle += 5.f;
    }
}

void Game::update(const float dt, sf::RenderWindow &window)
{

    moveView(window);

    parseInput(window);

    effects.update();
    player_particles->update(dt);

    player.update(dt);

    player_shape.setPosition(player.pos);
    player_shape.setRotation(player.angle);

    poly_manager.update(dt);
    boid_world.update(dt);
    bullet_world.update(dt);

    while (!boid_world.to_destroy.empty())
    {
        auto entity_ind = boid_world.to_destroy.front();

        effects.createExplosion(boid_world.entity2boid_data.at(entity_ind).r, 5.f);
        boid_world.removeBoid(entity_ind);
        score++;
        boid_world.to_destroy.pop();
    }

    poly_manager.collideWithPlayer(player);

    if (player.health == 0)
    {
        state = GameState::PLAYER_DIED;
    }

    if (boss_spawner_timer-- < 0)
    {
        auto spawn_position = player.pos + angle2dir(randf(0, 180)) * randf(50, 100);
        boss_spawner_timer = 600;
        if (rand() % 2 == 0)
        {
            boid_world.spawn(Boid::EnemyType::BOMBER, spawn_position);
        }
        else
        {
            boid_world.spawn(Boid::EnemyType::LASERBOI, spawn_position);
        }
    }

    if (normal_spawner_timer-- < 0)
    {
        normal_spawner_timer = 60;
        auto spawn_position = player.pos + angle2dir(randf(0, 180)) * randf(50, 100);
        boid_world.spawn(Boid::EnemyType::BASIC, spawn_position);
    }
    if (group_spawner_timer-- < 0)
    {
        group_spawner_timer = 300;
        // boid_world.addGroupOfBoids( 10, player.pos + angle2dir(rand()%180)*randf(5, 10), 5);
    }
    if(power_spawner_timer-- < 0)
    {
        auto spawn_position = player.pos + angle2dir(randf(0, 180)) * randf(50, 200);
        poly_manager.addRegularObstacle(ObstacleType::POWERUP, spawn_position);
        power_spawner_timer = 500;
    }

    lights.center = player.pos;
    lights.update();
}

void Game::draw(sf::RenderWindow &window)
{

    t.setView(window.getView());
    t.clear(sf::Color::Black);

    window.draw(player_shape);
    boid_world.draw(window);
    player_particles->draw(t);
    effects.draw(t);
    poly_manager.draw(t);
    bullet_world.draw(t);
    t.display();

    bloom.doTheThing(t, window);
    // darken(t, light_texture);

    // lights.draw(light_texture, window);

    drawUI(window);
}

void Game::drawUI(sf::RenderWindow &window)
{

    auto old_view = window.getView();
    window.setView(window.getDefaultView());

    std::string hp_text = "HP: " + std::to_string(player.health);

    sf::RectangleShape health_rect;

    sf::Vector2f healt_comp_uisize = {(float)window.getSize().x * 1.f / 6.f, (float)window.getSize().y * 1.f / 10.f};
    sf::Vector2f healt_comp_min = {(float)window.getSize().x * 5.f / 6.f, (float)window.getSize().y * 9.f / 10.f};

    health_text.setPosition(healt_comp_min);
    health_text.setString(hp_text);

    health_rect.setPosition({healt_comp_min.x, healt_comp_min.y + healt_comp_uisize.y / 2.f});
    float health_ratio = player.health / (float)player.max_health;
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

    float booster_ratio = player.boost_heat / player.max_boost_heat;
    booster_size.x *= booster_ratio;
    sf::Color booster_color(255 * (booster_ratio), 255 * (1 - booster_ratio), 0);

    booster_rect[0] = {{booster_pos.x, booster_pos.y}, sf::Color::Yellow};
    booster_rect[1] = {{booster_pos.x + booster_size.x, booster_pos.y}, booster_color};
    booster_rect[2] = {{booster_pos.x + booster_size.x, booster_pos.y + booster_size.y}, booster_color};
    booster_rect[3] = {{booster_pos.x, booster_pos.y + booster_size.y}, sf::Color::Yellow};
    window.draw(booster_rect);


    sf::RectangleShape score_rect;

    sf::Vector2f score_comp_uisize = {(float)window.getSize().x * 1.f / 6.f, (float)window.getSize().y * 1.f / 10.f};
    sf::Vector2f score_comp_min = {(float)window.getSize().x / 2.f - score_comp_uisize.x/2.f,
                                 (float)window.getSize().y * 1.f / 10.f};
    health_text.setPosition(score_comp_min);
    health_text.setString(std::to_string(score));

    window.setView(old_view);

}

void Game::onMeteorDestruction(int entity_ind)
{
    auto &meteor = poly_manager.obstacles.at(entity_ind);

    assert(meteor.type == ObstacleType::METEOR);

    if (meteor.radius <= 3.f)
    {
        poly_manager.destroyMeteor(entity_ind);
        return; //! no shattering of small meteors
    }

    auto old_center = meteor.getCenter();
    const auto &old_points = meteor.points;
    int n_points = old_points.size();

    std::vector<Polygon> new_meteors;

    for (int i = 0; i < 2; ++i)
    {
        Polygon new_meteor(0);
        auto new_center = (old_center + old_points[i] + old_points[(i + 1) % n_points]) / 3.f;
        new_meteor.points.push_back(old_center - new_center);
        new_meteor.points.push_back(old_points[i] - new_center);
        new_meteor.points.push_back(old_points[(i + 1) % n_points] - new_center);
        new_meteor.setPosition(meteor.getPosition());
        new_meteor.setScale(meteor.getScale());
        new_meteor.setRotation(meteor.getRotation());
        auto transform = meteor.getTransform();

        new_meteor.vel = transform.transformPoint(new_center - old_center);
        poly_manager.addRandomMeteorAt(meteor.getPosition(), meteor.getScale() / 2.f);

        // poly_manager.addMeteor(new_meteor);
    }

    poly_manager.destroyMeteor(entity_ind);
}