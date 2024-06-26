#include "Game.h"

#include "imgui.h"
#include "imgui-SFML.h"

#include "Entities/Player.h"
#include "Entities/Attacks.h"

#include "UI.h"
#include "SoundModule.h"
#include "Geometry.h"

Game::Game(sf::RenderWindow &window, KeyBindings &bindings)
    : m_window(window), m_key_binding(bindings), m_ui(window)
{

    ImGui::SFML::Init(m_window);

    m_bloom_texture.create(window.getSize().x, window.getSize().y);
    m_bloom_texture.setSmooth(true);

    sf::Vector2f box_size = {static_cast<float>(Geometry::BOX[0]), static_cast<float>(Geometry::BOX[1])};
    sf::Vector2i n_cells = {Geometry::N_CELLS[0], Geometry::N_CELLS[1]};
    sf::Vector2f cell_size = {
        static_cast<float>(Geometry::BOX[0] / n_cells.x),
        static_cast<float>(Geometry::BOX[1] / n_cells.y)};

    m_world = std::make_unique<GameWorld>();

    m_player = &static_cast<PlayerEntity &>(m_world->addObject(ObjectType::Player));
    m_player->setPosition({box_size.x / 2.f, box_size.y / 2.f});

    m_textures.load(Textures::ID::Arrow, "../Resources/arrow.png");

    spawnNextObjective();
    spawnBossObjective();
    addDestroyNObjective(ObjectType::SpaceStation, 2);

    auto &heart_spawner = m_world->addTrigger<Timer>();
    heart_spawner.setCallback(
        [this]()
        {
            auto &heart = m_world->addObject(ObjectType::Heart);
            auto spawn_pos = m_player->getPosition() + randf(20, 200) * angle2dir(randf(0, 360));
            heart.setPosition(spawn_pos);
        });

    auto &enemy_spawner = m_world->addTrigger<Timer>();
    enemy_spawner.m_cooldown = 1.f;
    enemy_spawner.setCallback(
        [this]()
        {
            auto &enemy = m_world->addObject(ObjectType::Enemy);
            auto spawn_pos = m_player->getPosition() + randf(20, 200) * angle2dir(randf(0, 360));
            enemy.setPosition(spawn_pos);
        });

    auto view = window.getView();
    view.setCenter(m_player->getPosition());
    view.setSize({100.f, 100.f * window.getSize().y / window.getSize().x});
    window.setView(view);
    m_default_view = view;

    m_font.loadFromFile("../Resources/DigiGraphics.ttf");
    m_health_text.setFont(m_font);
    m_health_text.setFillColor(sf::Color::Red);
}

void Game::addDestroyNObjective(ObjectType type, int count)
{
    auto destroy_stations = std::make_shared<DestroyNOfType>(type, "Space Station", count, m_font);
    int callback_id = m_world->addEntityDestroyedCallback(
        [destroy_stations](ObjectType type, int id){
           destroy_stations->entityDestroyed(type, id) ;
        }
    ) ;    
    m_objective_system.add(destroy_stations);
    destroy_stations->m_on_completion_callback =
     [this, callback_id, type, count, id = destroy_stations->m_id]()
    {
        m_objective_system.remove(id);
        m_world->removeEntityDestroyedCallback(callback_id);
        addDestroyNObjective(type, count+2);
    };

}

void Game::spawnBossObjective()
{
    auto &boss = m_world->addObject(ObjectType::Boss);
    boss.setPosition(m_player->getPosition() + 50.f * angle2dir(randf(0, 150)));
    auto &t2 = m_world->addTrigger<EntityDestroyed>(&boss);
    t2.setCallback(
        [this]()
        {
            // spawnBossObjective();
            m_score++;
        });
    
    auto destroy_enemy_obj = std::make_shared<DestroyEntity>(boss, m_font);
    destroy_enemy_obj->m_on_completion_callback = 
    [this, destroy_enemy_obj](){
        m_objective_system.remove(destroy_enemy_obj->m_id);
    };

    t2.attach(*destroy_enemy_obj);
    m_objective_system.add(std::move(destroy_enemy_obj));
}


void Game::spawnNextObjective()
{
    auto &new_trigger = m_world->addTrigger<ReachPlace>();
    auto new_pos = m_player->getPosition() + 300.f * angle2dir(randf(0, 150));
    new_trigger.setPosition(new_pos);
    m_score++;

    auto spawn_enemies = [this](sf::Vector2f pos)
    {
        for (int i = 0; i < 10; ++i)
        {
            auto &enemy = m_world->addObject(ObjectType::Enemy);
            enemy.setPosition(pos + randf(30, 50) * angle2dir(randf(0, 360)));
        }
    };
    auto objective = std::make_shared<ReachSpotObjective>(new_trigger, m_font);
    m_objective_system.add(objective);

    new_trigger.setCallback([this, spawn_enemies, new_pos, id = objective->m_id]()
                            {
        spawn_enemies(new_pos);
        m_objective_system.remove(id);
        spawnNextObjective();
         });

    objective->m_on_completion_callback = [this, new_pos](){
        auto& star_effect = m_world->addVisualEffect(EffectType::ParticleEmiter);
        star_effect.setPosition(new_pos);
        star_effect.setLifeTime(2.f);
    };
    new_trigger.attach(*objective);
}

void Game::moveView(sf::RenderWindow &window)
{
    auto view = window.getView();
    auto vp1 = window.getViewport(view);

    const sf::Vector2f view_size = view.getSize(); // * (view.getViewport().width);

    //! look from higher distance when boosting
    float booster_ratio = m_player->speed / m_player->max_speed;
    view.setSize(m_default_view.getSize() * (1 + booster_ratio / 3.f));

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

    window.setView(view);
}

void Game::handleEvent(const sf::Event &event)
{
    auto mouse_position = m_window.mapPixelToCoords(sf::Mouse::getPosition(m_window));
    ImGui::SFML::ProcessEvent(event);

    if (event.type == sf::Event::KeyPressed)
    {
        if (event.key.code == m_key_binding[PlayerControl::BOOST])
        {
            m_player->is_boosting = true;
        }
    }
    if (event.type == sf::Event::KeyReleased)
    {
        if (event.key.code == m_key_binding[PlayerControl::SHOOT_LASER])
        {
            auto dir = angle2dir(m_player->getAngle());

            auto &laser = static_cast<Laser &>(m_world->addObject(ObjectType::Laser));
            laser.setPosition(m_player->getPosition());
            laser.setAngle(m_player->getAngle());
            laser.setOwner(m_player);
        }
        if (event.key.code == m_key_binding[PlayerControl::THROW_BOMB])
        {
            auto dir = angle2dir(m_player->getAngle());

            auto &bomb = m_world->addObject(ObjectType::Bomb);
            bomb.setPosition(m_player->getPosition());
            bomb.m_vel = (50.f + m_player->speed) * angle2dir(m_player->getAngle());
            bomb.setAngle(m_player->getAngle());
        }
        if (event.key.code == sf::Keyboard::E) // m_key_binding[PlayerControl::THROW_EMP])
        {
            auto dir = angle2dir(m_player->getAngle());

            auto &bomb = m_world->addObject(ObjectType::EMP);
            bomb.setPosition(m_player->getPosition());
            bomb.m_vel = (50.f + m_player->speed) * angle2dir(m_player->getAngle());
            bomb.setAngle(m_player->getAngle());
        }
        if (event.key.code == m_key_binding[PlayerControl::BOOST])
        {
            m_player->is_boosting = false;
        }
    }
    if (event.type == sf::Event::MouseButtonReleased && sf::Keyboard::isKeyPressed(sf::Keyboard::LControl))
    {

        if (event.mouseButton.button == sf::Mouse::Right)
        {
            auto &new_enemy = m_world->addObject(ObjectType::Boss);
            new_enemy.setPosition(mouse_position);
        }
    }
    else if (event.type == sf::Event::MouseButtonReleased)
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

    if (sf::Keyboard::isKeyPressed(m_key_binding[PlayerControl::MOVE_FORWARD]))
    {
        m_player->speed += m_player->acceleration;
    }
    else if (sf::Keyboard::isKeyPressed(m_key_binding[PlayerControl::MOVE_BACK]))
    {
        m_player->speed -= m_player->acceleration;
    }
    else
    {
        m_player->speed /= 1.1f;
    }
    if (sf::Keyboard::isKeyPressed(m_key_binding[PlayerControl::STEER_LEFT]))
    {
        m_player->setAngle(m_player->getAngle() - 5.f);
    }
    if (sf::Keyboard::isKeyPressed(m_key_binding[PlayerControl::STEER_RIGHT]))
    {
        m_player->setAngle(m_player->getAngle() + 5.f);
    }
}

void Game::update(const float dt, sf::RenderWindow &window)
{

    moveView(window);
    parseInput(window);

    if (m_player->health < 0)
    {
        m_state = GameState::PLAYER_DIED;
    }

    m_world->update(dt);
    m_objective_system.update();
};

void Game::draw(sf::RenderWindow &window)
{

    m_bloom_texture.setView(window.getView());
    m_bloom_texture.clear(sf::Color::Black);
    m_world->draw(m_bloom_texture, window);
    m_bloom_texture.display();

    m_bloom.doTheThing(m_bloom_texture, window);

    m_ui.draw(window);
    drawUI(window);
    m_objective_system.draw(window);
}

void Game::drawUI(sf::RenderWindow &window)
{

    auto old_view = window.getView();
    window.setView(window.getDefaultView());

    std::string hp_text = "HP: " + std::to_string(static_cast<int>(m_player->health));

    sf::RectangleShape health_rect;

    sf::Vector2f healt_comp_uisize = {(float)window.getSize().x * 1.f / 6.f, (float)window.getSize().y * 1.f / 10.f};
    sf::Vector2f healt_comp_min = {(float)window.getSize().x * 5.f / 6.f, (float)window.getSize().y * 9.f / 10.f};

    m_health_text.setPosition(healt_comp_min);
    m_health_text.setString(hp_text);

    health_rect.setPosition({healt_comp_min.x, healt_comp_min.y + healt_comp_uisize.y / 2.f});
    float health_ratio = m_player->health / (float)m_player->max_health;
    health_rect.setSize({health_ratio * healt_comp_uisize.x * 3.f / 4.f, healt_comp_uisize.y / 3.f});
    health_rect.setFillColor(sf::Color::Red);
    health_rect.setOutlineThickness(1.f);
    health_rect.setOutlineColor(sf::Color::Black);
    window.draw(m_health_text);
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
    m_health_text.setString("Score: " + std::to_string(m_score));
    m_health_text.setPosition(score_comp_min.x - m_health_text.getLocalBounds().width / 2.f, score_comp_min.y);

    window.draw(m_health_text);
    window.setView(old_view);
}

Game::GameState Game::getState() const
{
    return m_state;
}

int Game::getScore() const
{
    return m_score;
}
