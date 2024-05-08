#include <omp.h>
#include <iostream>
#include <fstream>
#include <unordered_set>

#include "Game.h"
// #include "UI.h"
#include "SoundModule.h"

#include "imgui.h"
#include "imgui-SFML.h"

#include <iostream>

Game::Game(sf::Vector2i n_cells, sf::Vector2f box_size, sf::RenderWindow &window)
    : bloom()
{
    t.create(window.getSize().x, window.getSize().y);
    t.setSmooth(true);

    player_particles = std::make_unique<Particles>(player.pos, 100);

    const sf::Vector2f cell_size = {box_size.x / asFloat(n_cells).x, box_size.y / asFloat(n_cells).y};

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
    window.setView(view);

    player_shape.setOrigin({player.radius / 2.f, player.radius / 2.f});
    player_shape.setPosition(player.pos);
    player_shape.setSize({4.f, 4.f});
    player_shape.setRotation(0);
    player_shape.setFillColor(sf::Color::Red);

    std::cout << player_shape.getPoint(2).x << " " << player_shape.getPoint(2).y << "\n";

    boid_world.player = &player;
    boid_world.polygons = &poly_manager;
    boid_world.p_bs = &bullet_world;

    bullet_world.p_boids = &boid_world;
    bullet_world.p_meteors = &poly_manager;
    bullet_world.p_effects = &effects;
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

sf::Vector2f angleToDir(float angle)
{
    sf::Vector2f dir;
    dir.x = std::cos(angle * M_PI / 180.f);
    dir.y = std::sin(angle * M_PI / 180.f);
    return dir;
}

void Game::parseEvents(sf::RenderWindow &window)
{

    sf::Event event;
    auto mouse_position = window.mapPixelToCoords(sf::Mouse::getPosition(window));

    while (window.pollEvent(event))
    {
        ImGui::SFML::ProcessEvent(event);

        if (event.type == sf::Event::Closed)
        {
            game_is_running = false;
        }

        if (event.type == sf::Event::Resized)
        {
            // t.create(window.getSize().x, window.getSize().y);
            // effects.onResize(window.getSize());
            // bloom.onResize(window.getSize());
            // auto view = window.getView();
            // view.setCenter(player.pos);
            // window.setView(view);
        }

        if (event.type == sf::Event::KeyPressed)
        {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
            {
                bullet_world.spawnBullet(-1, player.pos, angleToDir(player.angle));
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
        }
        if (event.type == sf::Event::MouseButtonPressed)
        {

            if (event.mouseButton.button == sf::Mouse::Right &&
                sf::Mouse::isButtonPressed(sf::Mouse::Right))
            {
                // auto nearest_neigbhours = p_the_god->getSystem<PhysicsSystem>(ComponentID::PHYSICS).p_ns_->getNeighboursIndsFull(click_position, 5 * RHARD);

                // if (nearest_neigbhours.empty())
                // {
                boid_world.spawnBoss(mouse_position);
                // }
            }

            else if (event.mouseButton.button == sf::Mouse::Right)
            {
            }
        }
        if (event.type == sf::Event::MouseButtonPressed)
        {

            if (event.mouseButton.button == sf::Mouse::Left)
            {
                const auto mouse_coords = window.mapPixelToCoords(sf::Mouse::getPosition(window));
                if (!selection_pending)
                {
                    start_position = mouse_coords;
                    end_position = start_position;
                    selection_pending = true;
                }
            }
            else if (event.mouseButton.button == sf::Mouse::Right)
            {
                click_position = window.mapPixelToCoords(sf::Mouse::getPosition(window));
                // p_the_god->getSystem<SeekSystem>(ComponentID::PATHFINDING).issuePaths(selection.getSelectedInds(), click_position);
                boid_world.setTargetOf(selection, player.pos);
            }
        }
        else if (event.type == sf::Event::MouseMoved)
        {
            // ui.onMouseHold(window);
            if (selection_pending)
            {
                end_position = window.mapPixelToCoords(sf::Mouse::getPosition(window));
            }
        }
        else if (event.type == sf::Event::MouseButtonReleased)
        {
            // ui.onRelease(window);
            if (selection_pending)
            {
                end_position = window.mapPixelToCoords(sf::Mouse::getPosition(window));
            }
            if (event.mouseButton.button == sf::Mouse::Left)
            {
                end_position = window.mapPixelToCoords(sf::Mouse::getPosition(window));
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
            auto view2 = window.getView();
            if (event.mouseWheelScroll.wheel > 0)
            {
                view2.zoom(0.9f);
            }
            else
            {
                view2.zoom(1. / 0.9f);
            }
            window.setView(view2);
        }
    }
}

//! \brief parse events and normal input
//! \note  right now this is just a placeholder code until I make a nice OOP solution with bindings and stuff
void Game::parseInput(sf::RenderWindow &window)
{
    moveView(window);

    if (game_is_stopped_)
    {
        return;
    }

    parseEvents(window);

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
    {
        player.speed += 15.f;
        player.speed = std::min(player.speed, 25.f);
    }
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
    {
        player.speed -= 15.f;
        player.speed = std::max(player.speed, -25.f);
    }
    else
    {
        player.speed = 0.f;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
    {
        player.angle -= 5.f;
        if (player.angle < -180.f)
        {
            player.angle = 180.f;
        }
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
    {
        player.angle += 5.f;
        if (player.angle > 180.f)
        {
            player.angle = -180.f;
        }
    }
}

void Game::update(const float dt, sf::RenderWindow &window)
{
    if (game_is_stopped_)
    {
        return;
    }

    effects.update();
    player_particles->update(dt);

    player.pos.x += player.speed * std::cos(player.angle * M_PI / 180.) * dt;
    player.pos.y += player.speed * std::sin(player.angle * M_PI / 180.) * dt;

    player_shape.setPosition(player.pos);
    player_shape.setRotation(player.angle);

    poly_manager.update(dt);

    boid_world.update(dt);
    bullet_world.update();

    while (!boid_world.to_destroy.empty())
    {
        auto entity_ind = boid_world.to_destroy.front();

        effects.createExplosion(boid_world.entity2boid_data.at(entity_ind).r, 5.f);
        boid_world.removeBoid(entity_ind);
        boid_world.to_destroy.pop();
    }

    sf::RectangleShape rect;
    boid_vertices.setPrimitiveType(sf::Quads);
    const auto &boids = boid_world.getBoids();
    boid_vertices.resize(4 * boids.size());
    sf::Color color = sf::Color::Green;

    for (int boid_ind = 0; boid_ind < boids.size(); ++boid_ind)
    {
        const auto &boid = boids[boid_ind];

        rect.setSize({boid.radius, boid.radius});
        rect.setOrigin(rect.getSize() / 2.f);

        sf::Transform a;
        a.rotate(boid.orientation);

        boid_vertices[boid_ind * 4 + 0] = {boid.r + a.transformPoint(rect.getPoint(0)), color};
        boid_vertices[boid_ind * 4 + 1] = {boid.r + a.transformPoint(rect.getPoint(1)), color};
        boid_vertices[boid_ind * 4 + 2] = {boid.r + a.transformPoint(rect.getPoint(2)), color};
        boid_vertices[boid_ind * 4 + 3] = {boid.r + a.transformPoint(rect.getPoint(3)), color};
    }

    bullet_vertices.setPrimitiveType(sf::Quads);
    const auto &bullets = bullet_world.bullets.data;
    bullet_vertices.resize(4 * bullets.size());
    color = sf::Color::Red;

    for (int boid_ind = 0; boid_ind < bullets.size(); ++boid_ind)
    {
        const auto &boid = bullets[boid_ind];

        rect.setSize({boid.radius, boid.radius});
        rect.setOrigin(rect.getSize() / 2.f);

        sf::Transform a;
        a.rotate(boid.orientation);

        bullet_vertices[boid_ind * 4 + 0] = {boid.pos + a.transformPoint(rect.getPoint(0)), color};
        bullet_vertices[boid_ind * 4 + 1] = {boid.pos + a.transformPoint(rect.getPoint(1)), color};
        bullet_vertices[boid_ind * 4 + 2] = {boid.pos + a.transformPoint(rect.getPoint(2)), color};
        bullet_vertices[boid_ind * 4 + 3] = {boid.pos + a.transformPoint(rect.getPoint(3)), color};
    }

    auto meteors = poly_manager.getNearestMeteors(player.pos, player.radius);
    for (auto *meteor : meteors)
    {
        auto mvt = meteor->getMVTOfSphere(player.pos, player.radius / 2.f);
        if (norm2(mvt) > 0.0001f)
        {
            player.health = 0;
        }
    }
    // if (player.health == 0)
    // {
    //     endGame(EndGameType::DIED);
    // }

    if (boss_spawner_timer-- < 0)
    {
        boss_spawner_timer = 600;
        boid_world.spawnBoss(player.pos + angle2dir(rand() % 180) * randf(20, 30));
        boid_world.spawnBoss(player.pos + angle2dir(rand() % 180) * randf(20, 30));
    }

    if (normal_spawner_timer-- < 0)
    {
        normal_spawner_timer = 30;
        boid_world.addBoid(player.pos + angle2dir(rand() % 180) * randf(5, 10));
    }
    if (group_spawner_timer-- < 0)
    {
        group_spawner_timer = 300;
        // boid_world.addGroupOfBoids( 10, player.pos + angle2dir(rand()%180)*randf(5, 10), 5);
    }
}

void Game::draw(sf::RenderWindow &window)
{

    t.setView(window.getView());
    t.clear(sf::Color::Black);

    t.draw(player_shape);
    t.draw(boid_vertices);
    t.draw(bullet_vertices);

    player_particles->draw(t);
    effects.draw(t);
    poly_manager.draw(t);
    bullet_world.draw(t);
    t.display();

    sf::RectangleShape r;
    r.setSize({t.getSize().x, t.getSize().y});
    r.setTexture(&t.getTexture());

    bloom.doTheThing(t, window);
    // auto old_view = window.getView();
    // window.setView(window.getDefaultView());
    // window.draw(r);
    // window.setView(old_view);
}

void Game::addEnemy(sf::Vector2f at)
{
    boid_world.addBoid(at);
}

void Game::addGroupOfEnemies(sf::Vector2f at, float radius, int n_in_group)
{
    boid_world.addGroupOfBoids(n_in_group, at, radius);
}
