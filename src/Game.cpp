#include "Game.h"

#include "Entities/Player.h"
#include "Entities/Attacks.h"

#include "Utils/RandomTools.h"

// #include "SoundModule.h"

void Game::initializeLayers()
{
    auto width = m_window.getTargetSize().x;
    auto height = m_window.getTargetSize().y;

    TextureOptions options;
    options.wrap_x = TexWrapParam::ClampEdge;
    options.wrap_y = TexWrapParam::ClampEdge;

    TextureOptions text_options;
    text_options.data_type = TextureDataTypes::UByte;
    text_options.format = TextureFormat::RGBA;
    text_options.internal_format = TextureFormat::RGBA;
    std::cout << "TEST0!" << std::endl;

    // std::filesystem::path shaders_directory = {__FILE__};
    std::filesystem::path shaders_directory = {"../Resources/Shaders/"};
    // shaders_directory.remove_filename().append();

    auto &unit_layer = m_layers.addLayer("Unit", 3, options);
    unit_layer.m_canvas.setShadersPath(shaders_directory);
    std::cout << "TEST1!" << std::endl;
    unit_layer.m_canvas.addShader("Instanced", "basicinstanced.vert", "texture.frag");
    std::cout << "TEST!" << std::endl;

    auto &shiny_layer = m_layers.addLayer("Bloom", 5, options);
    shiny_layer.m_canvas.setShadersPath(shaders_directory);
    shiny_layer.m_canvas.addShader("Instanced", "basicinstanced.vert", "texture.frag");
    shiny_layer.addEffect(std::make_unique<Bloom3>(width, height));

    // // auto &light_layer = m_layers.addLayer("Light", 150, options);
    // // light_layer.m_canvas.m_blend_factors = {BlendFactor::SrcAlpha, BlendFactor::OneMinusSrcAlpha};
    // // light_layer.addEffect(std::make_unique<SmoothLight>(width, height));
    // // light_layer.addEffect(std::make_unique<LightCombine>(width, height));
    // // light_layer.m_canvas.addShader("VisionLight", "basictex.vert", "fullpassLight.frag");
    // // light_layer.m_canvas.addShader("combineBloomBetter", "basicinstanced.vert", "combineBloomBetter.frag");

    // // auto &ui_layer = m_layers.addLayer("UI", 1000, text_options);
    // // water_layer.addEffect(std::make_unique<WaterEffect>(width, height));
    // //
    m_window.setShadersPath(shaders_directory);
    m_window.addShader("Shiny", "basicinstanced.vert", "shiny.frag");
    m_window.addShader("Arrow", "basicinstanced.vert", "texture.frag");
    m_window.addShader("LastPass", "basicinstanced.vert", "lastPass.frag");
    m_scene_canvas.setShadersPath(shaders_directory);
}

using namespace utils;

Game::Game(Renderer &window, KeyBindings &bindings)
    : m_window(window), m_key_binding(bindings),
      m_scene_pixels(window.getTargetSize().x, window.getTargetSize().y),
      m_scene_canvas(m_scene_pixels)
{
    m_background.loadFromFile("../Resources/Textures/background.png");

    initializeLayers();

    m_world = std::make_unique<GameWorld>();
    //! PLAYER NEEDS TO BE FIRST BECAUSE OTHER OBJECTS MIGHT REFERENCE IT!
    m_player = &m_world->addObjectForced<PlayerEntity>();
    m_player->setPosition({500, 500});
    m_world->m_player = m_player;
    
    for (int i = 0; i < 3; ++i)
    {
        auto &meteor = m_world->addObject2<Meteor>();
    }
    
    // std::filesystem::path texture_directory = {__FILE__};
    std::filesystem::path texture_directory = {};
    texture_directory.remove_filename().append("../Resources/Textures/");
    m_textures.setBaseDirectory(std::filesystem::path{"../Resources/Textures/"});
    m_textures.add("Arrow", "arrow.png");

    // std::filesystem::path font_path = {__FILE__};
    std::filesystem::path font_path = {};
    font_path.remove_filename().append("../Resources/Fonts/arial.ttf");
    m_font = std::make_unique<Font>(font_path);

    spawnNextObjective();
    // addDestroyNObjective(ObjectType::SpaceStation, 2);

    for (int i = 0; i < 500; ++i)
    {
        auto &enemy = m_world->addObject2<Enemy>();
        auto spawn_pos = m_player->getPosition() + randf(100, 400) * angle2dir(randf(0, 360));
        enemy.setPosition(spawn_pos);
    }

    auto &heart_spawner = m_world->addTrigger<Timer>();
    heart_spawner.setCallback(
        [this]()
        {
             auto &heart = m_world->addObject2<Heart>();
            auto spawn_pos = m_player->getPosition() + randf(20, 200) * angle2dir(randf(0, 360));
            heart.setPosition(spawn_pos);
        });

    auto &enemy_spawner = m_world->addTrigger<Timer>();
    enemy_spawner.m_cooldown = 5.f;
    enemy_spawner.setCallback(
        [this]()
        {
            if (m_world->getActiveCount<Enemy>() < 100) //! max 100 enemies
            {
                auto &enemy = m_world->addObject2<Enemy>();
                auto spawn_pos = m_player->getPosition() + randf(50, 200) * angle2dir(randf(0, 360));
                enemy.setPosition(spawn_pos);
            }
        });

    auto view = window.m_view;
    view.setCenter(m_player->getPosition());
    view.setSize(150.f, 150.f * window.getTargetSize().y / window.getTargetSize().x);
    window.m_view = view;
    m_default_view = view;

    m_health_text.setFont(m_font.get());
    m_health_text.setColor({255, 0, 0, 255});
}

void Game::addDestroyNObjective(ObjectType type, int count)
{
    auto destroy_stations = std::make_shared<DestroyNOfType>(type, "Space Station", count, *m_font);
    int callback_id = m_world->addEntityDestroyedCallback(
        [destroy_stations](ObjectType type, int id)
        {
            destroy_stations->entityDestroyed(type, id);
        });
    m_objective_system.add(destroy_stations);
    destroy_stations->m_on_completion_callback =
        [this, callback_id, type, count, id = destroy_stations->m_id]()
    {
        m_objective_system.remove(id);
        m_world->removeEntityDestroyedCallback(callback_id);
        addDestroyNObjective(type, count + 2);
    };
}

void Game::spawnBossObjective()
{
    auto &boss = m_world->addObject2<Boss>();
    boss.setPosition(m_player->getPosition() + 50.f * angle2dir(randf(0, 150)));
    auto &t2 = m_world->addTrigger<EntityDestroyed>(&boss);
    t2.setCallback(
        [this]()
        {
            if (m_score > 50 && rand() % 2 == 0)
            {
                spawnBossObjective();
            }
            m_score += 10;
        });

    auto destroy_enemy_obj = std::make_shared<DestroyEntity>(boss, *m_font);
    destroy_enemy_obj->m_on_completion_callback =
        [this, destroy_enemy_obj]()
    {
        m_objective_system.remove(destroy_enemy_obj->m_id);
    };

    t2.attach(destroy_enemy_obj);
    m_objective_system.add(destroy_enemy_obj);
}

void Game::spawnNextObjective()
{
    auto &new_trigger = m_world->addTrigger<ReachPlace>();
    auto new_pos = m_player->getPosition() + 400.f * angle2dir(randf(0, 150));
    new_trigger.setPosition(new_pos);

    auto spawn_enemies = [this](utils::Vector2f pos)
    {
        for (int i = 0; i < 10; ++i)
        {
            auto &enemy = m_world->addObject2<Enemy>();
            enemy.setPosition(pos + randf(30, 50) * angle2dir(randf(0, 360)));
        }
    };

    auto objective = std::make_shared<ReachSpotObjective>(new_trigger, *m_font);
    m_objective_system.add(objective);
    new_trigger.setCallback([this, spawn_enemies, new_pos, id = objective->m_id]()
                            {
        spawn_enemies(new_pos);
        m_objective_system.remove(id);
        m_score += 3;
        spawnNextObjective();
        if(m_score > 20 && rand()%3 == 0)
        {
            spawnBossObjective();
        } });

    objective->m_on_completion_callback = [this, new_pos]()
    {
        auto &star_effect = m_world->addVisualEffect(EffectType::ParticleEmiter);
        star_effect.setPosition(new_pos);
        star_effect.setLifeTime(2.f);
    };
    new_trigger.attach(objective);
}

void Game::moveView(Renderer &window)
{
    auto view = window.m_view;

    const utils::Vector2f view_size = view.getSize(); // * (view.getViewport().width);

    //! look from higher distance when boosting
    float booster_ratio = m_player->speed / m_player->max_speed;
    view.setSize(m_default_view.getSize() * (1 + booster_ratio / 2.f));

    auto threshold = view.getSize() / 2.f - view.getSize() / 3.f;
    auto dx = m_player->getPosition().x - view.getCenter().x;
    auto dy = m_player->getPosition().y - view.getCenter().y;
    auto view_max = view.getCenter() + view.getSize() / 2.f;
    auto view_min = view.getCenter() - view.getSize() / 2.f;

    //! move view when approaching sides
    if (dx > threshold.x)
    {
        view.setCenter(view.getCenter() + utils::Vector2f{dx - threshold.x, 0});
    }
    else if (dx < -threshold.x)
    {
        view.setCenter(view.getCenter() + utils::Vector2f{dx + threshold.x, 0});
    }
    if (dy > threshold.y)
    {
        view.setCenter(view.getCenter() + utils::Vector2f{0, dy - threshold.y});
    }
    else if (dy < -threshold.y)
    {
        view.setCenter(view.getCenter() + utils::Vector2f{0, dy + threshold.y});
    }

    window.m_view = view;
}

void Game::handleEvent(const SDL_Event &event)
{
    auto mouse_position = m_window.getMouseInWorld();

    if (event.type == SDL_KEYDOWN)
    {
        if (event.key.keysym.sym == m_key_binding[PlayerControl::BOOST])
        {
            m_player->is_boosting = true;
        }
        if (event.key.keysym.sym == m_key_binding[PlayerControl::STEER_LEFT])
        {
            m_player->m_is_turning_left = true;
        }
        if (event.key.keysym.sym == m_key_binding[PlayerControl::STEER_RIGHT])
        {
            m_player->m_is_turning_right = true;
        }
    }
    if (event.type == SDL_KEYUP)
    {
        auto dir = utils::angle2dir(m_player->getAngle());
        if (event.key.keysym.sym == m_key_binding[PlayerControl::SHOOT_LASER])
        {
            auto &laser = m_world->addObject2<Laser>();
            laser.setPosition(m_player->getPosition());
            laser.setAngle(m_player->getAngle());
            laser.setOwner(m_player);
            laser.m_rotates_with_owner = true;
            laser.m_max_dmg = 1.;
            m_player->m_is_shooting_laser = true;
            m_player->m_laser_timer = laser.m_life_time;
        }
        if (event.key.keysym.sym == m_key_binding[PlayerControl::THROW_BOMB])
        {
            auto &bomb = m_world->addObject2<Bomb>();
            bomb.setPosition(m_player->getPosition());
            bomb.m_vel = (50.f + m_player->speed) * angle2dir(m_player->getAngle());
            bomb.setAngle(m_player->getAngle());
        }
        if (event.key.keysym.sym == SDLK_e) // m_key_binding[PlayerControl::THROW_EMP])
        {
            auto &bomb = m_world->addObject2<EMP>();
            bomb.setPosition(m_player->getPosition());
            bomb.m_vel = (50.f + m_player->speed) * angle2dir(m_player->getAngle());
            bomb.setAngle(m_player->getAngle());
        }
        if (event.key.keysym.sym == m_key_binding[PlayerControl::BOOST])
        {
            m_player->is_boosting = false;
        }
        if (event.key.keysym.sym == m_key_binding[PlayerControl::STEER_LEFT])
        {
            m_player->m_is_turning_left = false;
        }
        if (event.key.keysym.sym == m_key_binding[PlayerControl::STEER_RIGHT])
        {
            m_player->m_is_turning_right = false;
        }
    }
    if (event.type == SDL_MOUSEBUTTONUP && isKeyPressed(SDL_SCANCODE_LCTRL))
    {

        if (event.button.button == SDL_BUTTON_RIGHT)
        {
            auto &new_enemy = m_world->addObject2<Boss>();
            new_enemy.setPosition(mouse_position);
        }
    }
    else if (event.type == SDL_MOUSEBUTTONUP)
    {
        if (event.button.button == SDL_BUTTON_RIGHT)
        {
            auto &station = m_world->addObject2<SpaceStation>();
            station.setPosition(mouse_position);
        }
    }

    // if (event.type == SDL_MOUSEWHEEL)
    // {
    //     auto view = m_window.m_view;
    //     if (event.wheel.preciseY > 0)
    //     {
    //         view.zoom(0.9f);
    //     }
    //     else
    //     {
    //         view.zoom(1. / 0.9f);
    //     }
    //     m_window.m_view = view;
    // }
}

//! \brief parse events and normal input
//! \note  right now this is just a placeholder code until I make a nice OOP solution with bindings and stuff
void Game::parseInput(Renderer &window, float dt)
{

    if (isKeyPressed(m_key_binding[PlayerControl::MOVE_FORWARD]))
    {
        m_player->speed += m_player->acceleration;
    }
    else if (isKeyPressed(m_key_binding[PlayerControl::MOVE_BACK]))
    {
        m_player->speed -= m_player->acceleration;
    }
}

void Game::update(const float dt, Renderer &window)
{
    moveView(window);
    parseInput(window, dt);

    if (m_player->health < 0)
    {
        m_state = GameState::PLAYER_DIED;
    }

    m_world->update2(dt);
    m_world->update(dt);
    m_objective_system.update();
};

void Game::draw(Renderer &window)
{

    Sprite background_rect;
    auto old_view = window.m_view;
    window.m_view = window.getDefaultView();
    background_rect.setPosition(window.getTargetSize() / 2.f);
    background_rect.setScale(window.getTargetSize() / 2.f);
    background_rect.setTexture(m_background);
    //! move background with view
    utils::Vector2f view_size_rel = utils::Vector2f{0.2, 0.2} + old_view.getSize() / 1415.f;
    utils::Vector2f rel_pos = old_view.getCenter() / 1400.;
    utils::Vector2f size = m_background.getSize();
    background_rect.m_tex_rect = {(int)(size.x * rel_pos.x),
                                  (int)(size.y * rel_pos.y),
                                  (int)(size.x * view_size_rel.x),
                                  (int)(size.y * view_size_rel.y)};
    window.drawSprite(background_rect);
    window.drawAll();
    window.m_view = old_view;

    m_layers.clearAllLayers();
    m_world->draw(m_layers);
    m_world->draw2(m_layers);
    Enemy::m_neighbour_searcher.drawGrid(*m_layers.getLayer("Unit"));

    //! clear and draw into scene
    m_scene_canvas.clear({0, 0, 0, 0});
    m_scene_canvas.m_view = old_view;
    m_layers.setView(m_window.m_view);
    m_layers.drawInto(m_scene_canvas);

    // //! draw everything to a window quad
    auto scene_size = m_scene_pixels.getSize();

    Sprite screen_sprite(m_scene_pixels.getTexture());
    screen_sprite.setPosition(scene_size / 2.f);
    screen_sprite.setScale(scene_size / 2.f);
    m_window.m_view.setCenter(screen_sprite.getPosition());
    m_window.m_view.setSize(scene_size);
    m_window.drawSprite(screen_sprite, "LastPass", DrawType::Dynamic);
    auto old_factors = m_window.m_blend_factors;
    m_window.m_blend_factors = {BlendFactor::One, BlendFactor::OneMinusSrcAlpha};
    m_window.drawAll();
    drawUI(window);
    m_window.m_view = old_view;

    m_window.m_blend_factors = old_factors;

    m_objective_system.draw(window);
    m_window.drawAll();
    // m_ui.draw(window);
}

void Game::drawUI(Renderer &window)
{
    auto old_view = window.m_view;
    window.m_view = window.getDefaultView(); //! draw directly on screen

    std::string hp_text = "HP: " + std::to_string(static_cast<int>(m_player->health));

    Sprite health_rect;

    utils::Vector2f window_size = window.getTargetSize();
    utils::Vector2f healt_comp_uisize = {window_size.x * 1.f / 6.f, window_size.y * 1.f / 10.f};
    utils::Vector2f healt_comp_min = {window_size.x * 5.f / 6.f, window_size.y * 9.f / 10.f};

    auto mouse_pos = m_window.getMouseInWorld();
    m_health_text.setText(hp_text);
    m_health_text.centerAround(healt_comp_min);

    health_rect.setPosition(healt_comp_min.x, healt_comp_min.y);
    float health_ratio = m_player->health / (float)m_player->max_health;
    health_rect.setScale(healt_comp_uisize.x * 3.f / 4.f, healt_comp_uisize.y / 3.f);
    health_rect.m_tex_rect = {0, 0, 1, 1};
    health_rect.m_tex_size = {1, 1};
    // health_rect.m_color = {255, 0, 0, 255};
    window.drawSprite(health_rect, "healthBar");
    window.getShader("healthBar").use();
    window.getShader("healthBar").setUniform2("u_health_percentage", health_ratio);
    window.drawAll();
    window.drawText(m_health_text);

    utils::Vector2f booster_size = {window_size.x * 1.f / 6.f, health_rect.getScale().y * 2.f};
    utils::Vector2f booster_pos = {window_size.x * 1.f / 6.f, health_rect.getPosition().y};
    Sprite booster_rect;
    booster_rect.setTexture(*m_textures.get("Arrow"));
    float booster_ratio = std::min({1.f, m_player->boost_heat / m_player->max_boost_heat});
    booster_rect.setPosition(booster_pos.x - booster_size.x / 2.f * (1. - booster_ratio), booster_pos.y);
    booster_rect.setScale(booster_size.x / 2.f * booster_ratio, booster_size.y / 2.f);
    unsigned char factor = 255 * (booster_ratio);
    unsigned char factor2 = 255 * (1. - booster_ratio);
    booster_rect.m_color = ColorByte{factor, factor2, 0, 255};
    window.drawSprite(booster_rect, "boostBar");

    utils::Vector2f score_comp_uisize = {window_size.x * 1.f / 6.f, (float)window.getTargetSize().y * 1.f / 10.f};
    utils::Vector2f score_comp_min = {window_size.x / 2.f,
                                      (float)window.getTargetSize().y * 1.f / 20.f};
    m_health_text.setText("Score: " + std::to_string(m_score));
    m_health_text.centerAround(score_comp_min);

    window.drawText(m_health_text);
    window.drawAll();
    window.m_view = old_view;
}

Game::GameState Game::getState() const
{
    return m_state;
}

int Game::getScore() const
{
    return m_score;
}
