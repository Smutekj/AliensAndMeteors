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

Game::Game(sf::RenderWindow &window)
    : bullet_world(player), m_window(window)
{
    t.create(window.getSize().x, window.getSize().y);
    t.setSmooth(true);

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

    auto view = window.getView();
    view.setCenter(player.pos);
    view.setSize({50., 50. * window.getSize().y / window.getSize().x});
    window.setView(view);

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
    window.setView(view);
}

void Game::handleEvent(const sf::Event &event)
{
    auto mouse_position = m_window.mapPixelToCoords(sf::Mouse::getPosition(m_window));

    if (event.type == sf::Event::Closed)
    {
        game_is_running = false;
    }

    if (event.type == sf::Event::Resized)
    {
    }

    if (event.type == sf::Event::KeyPressed)
    {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
        {
            bullet_world.spawnBullet(-1, player.pos, angle2dir(player.angle));
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Delete))
        {
            boid_world.removeBoids(selection);
            for (auto i : selected_meteors)
            {
                onMeteorDestruction(i);
            }
            selected_meteors.clear();
            selection.clear();
        }
    }
    if (event.type == sf::Event::KeyReleased)
    {
        if (last_pressed_was_space)
        {
            game_is_stopped_ = false;
            last_pressed_was_space = false;
        }
        if (event.key.code == sf::Keyboard::Key::E)
        {
            addExplosion(mouse_position, 0);
        }
        if (event.key.code == sf::Keyboard::Key::LShift)
        {
            auto dir = angle2dir(player.angle);

            bullet_world.createLaser(-1, player.pos, player.pos + dir * 200.f);
        }
        if (event.key.code == sf::Keyboard::Key::B)
        {
            auto dir = angle2dir(player.angle);

            bullet_world.createBomb(-1, player.pos, 150.f * angle2dir(player.angle));
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
    if (event.type == sf::Event::MouseButtonPressed)
    {

        if (event.mouseButton.button == sf::Mouse::Left)
        {
            if (!selection_pending)
            {
                start_position = mouse_position;
                end_position = start_position;
                selection_pending = true;
            }
        }
        else if (event.mouseButton.button == sf::Mouse::Right)
        {
            boid_world.setTargetOf(selection, player.pos);
        }
    }
    else if (event.type == sf::Event::MouseMoved)
    {
        if (selection_pending)
        {
            end_position = mouse_position;
        }
    }
    else if (event.type == sf::Event::MouseButtonReleased)
    {
        // ui.onRelease(window);
        if (selection_pending)
        {
            end_position = mouse_position;
        }
        if (event.mouseButton.button == sf::Mouse::Left)
        {
            end_position = mouse_position;
            selection_pending = false;
            sf::FloatRect selection_rect{start_position, end_position - start_position};
            selection = boid_world.getBoidsIn(selection_rect);
            selected_meteors = poly_manager.getNearestMeteorInds(start_position, end_position);

            // selection.selectInRectangle(*p_the_god, start_position, end_position, 0);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl))
        {

            poly_manager.addRandomMeteorAt(mouse_position);
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
    void Game::parseInput(sf::RenderWindow & window)
    {
        moveView(window);

        if (game_is_stopped_)
        {
            return;
        }

        // parseEvents(window);

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
        {
            player.speed += 0.5f;
            player.speed = std::min(player.speed, 25.f);
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
        {
            player.speed -= 0.5f;
            player.speed = std::max(player.speed, -10.f);
        }
        else
        {
            player.speed = 0.f;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
        {
            player.angle -= 5.f;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
        {
            player.angle += 5.f;
        }
    }

    void Game::update(const float dt, sf::RenderWindow &window)
    {
        if (game_is_stopped_)
        {
            return;
        }

        parseInput(window);

        effects.update();
        player_particles->update(dt);

        player.pos += player.speed * angle2dir(player.angle) * dt;

        player_shape.setPosition(player.pos);
        player_shape.setRotation(player.angle);

        poly_manager.update(dt);
        boid_world.update(dt);
        bullet_world.update(dt);

        // poly_manager.collideWithPlayer(player);

        if (player.angle < -180.f)
        {
            player.angle = 180.f;
        }
        if (player.angle > 180.f)
        {
            player.angle = -180.f;
        }

        while (!boid_world.to_destroy.empty())
        {
            auto entity_ind = boid_world.to_destroy.front();

            effects.createExplosion(boid_world.entity2boid_data.at(entity_ind).r, 5.f);
            boid_world.removeBoid(entity_ind);
            boid_world.to_destroy.pop();
        }

        // if (player.health == 0)
        // {
        //     endGame(EndGameType::DIED);
        // }

        if (boss_spawner_timer-- < 0)
        {
            boss_spawner_timer = 600;
            // boid_world.spawnBoss(player.pos + angle2dir(rand() % 180) * randf(100, 150));
            // boid_world.spawnBoss(player.pos + angle2dir(rand() % 180) * randf(100, 150));
        }

        if (normal_spawner_timer-- < 0)
        {
            normal_spawner_timer = 60;
            boid_world.addBoid(player.pos + angle2dir(rand() % 180) * randf(50, 51));
        }
        if (group_spawner_timer-- < 0)
        {
            group_spawner_timer = 300;
            // boid_world.addGroupOfBoids( 10, player.pos + angle2dir(rand()%180)*randf(5, 10), 5);
        }
    }

    void Game::draw(sf::RenderWindow & window)
    {

        t.setView(window.getView());
        t.clear(sf::Color::Black);

        window.draw(player_shape);

        player_particles->draw(t);
        effects.draw(t);
        poly_manager.draw(t);
        bullet_world.draw(t);
        boid_world.draw(t);
        t.display();

        bloom.doTheThing(t, window);

        drawUI(window);
    }

    void Game::drawUI(sf::RenderWindow & window)
    {

        auto old_view = window.getView();
        window.setView(window.getDefaultView());

        std::string hp_text = "HP: " + std::to_string(player.health);

        sf::RectangleShape health_rect;

        sf::Vector2f healt_comp_uisize = {window.getSize().x * 1. / 6., window.getSize().y * 1. / 10.};
        sf::Vector2f healt_comp_min = {window.getSize().x * 5. / 6., window.getSize().y * 9. / 10.};

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
        window.setView(old_view);
    }

    void Game::addEnemy(sf::Vector2f at)
    {
        boid_world.addBoid(at);
    }

    void Game::addGroupOfEnemies(sf::Vector2f at, float radius, int n_in_group)
    {
        boid_world.addGroupOfBoids(n_in_group, at, radius);
    }
