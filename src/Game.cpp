#include "Game.h"

#include "Entities/Player.h"
#include "Entities/Attacks.h"

#include "Utils/RandomTools.h"

// #include "SoundModule.h"

void Game::initializeLayersAndTextures()
{
    m_textures.setBaseDirectory(std::string(RESOURCES_DIR) + "/Textures/");
    m_textures.add("Arrow", "arrow.png");
    m_textures.add("FireNoise", "fireNoise.png");

    std::filesystem::path font_path = std::string(RESOURCES_DIR) + "/Fonts/arial.ttf";
    m_font = std::make_unique<Font>(font_path);

    auto width = m_window.getTargetSize().x;
    auto height = m_window.getTargetSize().y;

    TextureOptions options;
    options.wrap_x = TexWrapParam::ClampEdge;
    options.wrap_y = TexWrapParam::ClampEdge;

    TextureOptions text_options;
    text_options.data_type = TextureDataTypes::UByte;
    text_options.format = TextureFormat::RGBA;
    text_options.internal_format = TextureFormat::RGBA;

    std::filesystem::path shaders_directory = {RESOURCES_DIR};
    shaders_directory.append("Shaders/");

    auto &unit_layer = m_layers.addLayer("Unit", 3, text_options, width, height);
    unit_layer.m_canvas.setShadersPath(shaders_directory);
    unit_layer.m_canvas.addShader("Instanced", "basicinstanced.vert", "texture.frag");
    unit_layer.m_canvas.addShader("Meteor", "basictex.vert", "Meteor.frag");
    // unit_layer.addEffect(std::make_unique<EdgeDetect>(width, height));
    // unit_layer.addEffect(std::make_unique<BloomFinal>(width, height));

    auto &shiny_layer = m_layers.addLayer("Bloom", 5, options, width, height);
    shiny_layer.m_canvas.setShadersPath(shaders_directory);
    shiny_layer.m_canvas.addShader("Instanced", "basicinstanced.vert", "texture.frag");
    shiny_layer.addEffect(std::make_unique<BloomFinal>(width, height));

    auto &base_layer = m_ui_layers.addLayer("Base", 0, options, width, height);
    base_layer.m_canvas.setShadersPath(shaders_directory);
    auto &bloom_layer = m_ui_layers.addLayer("Bloom", 1, options, width, height);
    bloom_layer.m_canvas.setShadersPath(shaders_directory);
    bloom_layer.m_canvas.addShader("boostBar2", "basicinstanced.vert", "boostBar2.frag");
    bloom_layer.m_canvas.addShader("fuelBar", "basicinstanced.vert", "fuelBar.frag");
    // bloom_layer.addEffect(std::make_unique<Bloom>(width, height));

    // //
    m_window.setShadersPath(shaders_directory);
    m_window.addShader("Shiny", "basicinstanced.vert", "shiny.frag");
    m_window.addShader("Arrow", "basicinstanced.vert", "texture.frag");
    m_window.addShader("LastPass", "basicinstanced.vert", "lastPass.frag");
    m_window.addShader("healthBar", "basicinstanced.vert", "healthBar.frag");
    m_window.addShader("boostBar", "basicinstanced.vert", "boostBar.frag");
    m_window.addShader("fuelBar", "basicinstanced.vert", "fuelBar.frag");
    m_window.addShader("boostBar2", "basicinstanced.vert", "boostBar2.frag");

    m_scene_canvas.setShadersPath(shaders_directory);

    glCheckErrorMsg("ERROR In initialize layers");
}

using namespace utils;

constexpr utils::Vector2f PLAYER_START_POS = {500, 500};
constexpr float START_VIEW_SIZE = 200.f;

Game::Game(Renderer &window, KeyBindings &bindings)
    : m_window(window), m_key_binding(bindings),
      m_scene_pixels(window.getTargetSize().x, window.getTargetSize().y),
      m_scene_canvas(m_scene_pixels),
      m_camera(PLAYER_START_POS, {START_VIEW_SIZE, START_VIEW_SIZE * window.getTargetSize().y / window.getTargetSize().x})
{
    messanger.registerEvents<EntityDiedEvent, EntityDiedEvent, QuestCompletedEvent, CollisionEvent, DamageReceivedEvent>();

    initializeLayersAndTextures();
    m_world = std::make_unique<GameWorld>(messanger);
    //! PLAYER NEEDS TO BE FIRST BECAUSE OTHER OBJECTS MIGHT REFERENCE IT!
    m_player = &m_world->addObjectForced<PlayerEntity>();
    m_player->setPosition({500, 500});
    m_world->m_player = m_player;

    m_objective_system = std::make_unique<ObjectiveSystem>(messanger);
    m_ui_system = std::make_unique<UISystem>(window, m_textures, messanger, m_player, *m_font);

    m_background = std::make_unique<Texture>(std::string(RESOURCES_DIR) + "/Textures/background.png");

    for (int i = 0; i < 200; ++i)
    {
        auto &meteor = m_world->addObject2<Meteor>();
        auto spawn_pos = m_player->getPosition() + randf(50, 1000) * angle2dir(randf(0, 360));
        meteor.setPosition(spawn_pos);
    }

    spawnNextObjective();
    // addDestroyNObjective(ObjectType::SpaceStation, 2);

    // for (int i = 0; i < 100; ++i)
    // {
    //     auto &enemy = m_world->addObject2<Enemy>();
    //     auto spawn_pos = m_player->getPosition() + randf(100, 400) * angle2dir(randf(0, 360));
    //     enemy.setPosition(spawn_pos);
    // }

    auto &heart_spawner = m_world->addTrigger<Timer>();
    heart_spawner.setCallback(
        [this]()
        {
            auto &heart = m_world->addObject2<Heart>();
            heart.type = Pickup::Fuel;
            auto spawn_pos = m_player->getPosition() + randf(20, 200) * angle2dir(randf(0, 360));
            heart.setPosition(spawn_pos);
        });

    // auto &enemy_spawner = m_world->addTrigger<Timer>();
    // enemy_spawner.m_cooldown = 5.f;
    // enemy_spawner.setCallback(
    //     [this]()
    //     {
    //         if (m_world->getActiveCount<Enemy>() < 100) //! max 100 enemies
    //         {
    //             auto &enemy = m_world->addObject2<Enemy>();
    //             auto spawn_pos = m_player->getPosition() + randf(50, 200) * angle2dir(randf(0, 360));
    //             enemy.setPosition(spawn_pos);
    //         }
    //     });

    m_health_text.setFont(m_font.get());
    m_health_text.setColor({255, 0, 0, 255});
}

void Game::addDestroyNObjective(ObjectType type, int count)
{
    auto quest = std::make_shared<Quest>(messanger);
    auto destroy_stations = std::make_shared<DestroyNOfTypeTask>(type, "Space Station", count, *m_font, messanger, quest.get());
    quest->addTask(destroy_stations);

    destroy_stations->m_on_completion_callback =
        [this, type, count, id = destroy_stations->m_id]()
    {
        addDestroyNObjective(type, count + 2);
    };
}

void Game::spawnBossObjective()
{
    auto &boss = m_world->addObject2<Boss>();
    boss.setPosition(m_player->getPosition() + 50.f * angle2dir(randf(0, 150)));

    // auto destroy_enemy_obj = std::make_shared<DestroyEntityTask>(boss, *m_font, messanger, quest.get());

    // destroy_enemy_obj->m_on_completion_callback =
    //     [this, destroy_enemy_obj]()
    // {
    //     if (m_score > 50 && rand() % 2 == 0)
    //     {
    //         spawnBossObjective();
    //     }
    //     m_score += 10;
    // };

    // m_objective_system->add(destroy_enemy_obj);
}

void Game::changeStage(GameStage target_stage)
{

    if (target_stage == GameStage::TIME_RACE)
    {
        auto &new_trigger = m_world->addTrigger<ReachPlace>();
        auto new_pos = m_player->getPosition() + 400.f * utils::angle2dir(randf(0, 150));
        new_trigger.setPosition(new_pos);

        // auto objective = std::make_shared<ReachSpotTask>(new_trigger, *m_font, messanger);
        // // m_objective_system->add(objective);
        // new_trigger.setCallback([this, new_pos]()
        //                         {
        //     m_score += 3;
        //     spawnNextObjective(); });

        // objective->m_on_completion_callback = [this, new_pos]()
        // {
        //     auto &star_effect = m_world->addVisualEffect(EffectType::ParticleEmiter);
        //     star_effect.setPosition(new_pos);
        //     star_effect.setLifeTime(2.f);
        // };
        // new_trigger.attach(objective);

        // auto &time_ran_out = m_world->addTrigger<Timer>();
        // time_ran_out.m_cooldown = 60.f;
        // time_ran_out.setCallback([this, objective_id = objective->m_id]()
        //                          {

        //         m_objective_system->remove(objective_id);
        //         changeStage(GameStage::FREE); });
    }

    stage = target_stage;
}

void Game::spawnNextObjective()
{
    auto &spot1 = m_world->addTrigger<ReachPlace>();
    auto &spot2 = m_world->addTrigger<ReachPlace>();
    spot1.setPosition(m_player->getPosition() + 400.f * angle2dir(randf(0, 150)));
    spot2.setPosition(spot1.getPosition() + 500. * angle2dir(randf(0, 180.)));

    auto spawn_enemies = [this](utils::Vector2f pos)
    {
        for (int i = 0; i < 10; ++i)
        {
            auto &enemy = m_world->addObject2<Enemy>();
            enemy.setPosition(pos + randf(30, 50) * angle2dir(randf(0, 360)));
        }
    };

    spot1.setCallback([this, spawn_enemies, pos = spot1.getPosition()]()
                      { spawn_enemies(pos); });
    spot2.setCallback([this, spawn_enemies, pos = spot2.getPosition()]()
                      { spawn_enemies(pos); });

    std::shared_ptr<Quest> quest = std::make_shared<Quest>(messanger);
    auto objective = std::make_shared<ReachSpotTask>(spot1, *m_font, messanger, quest.get());
    auto objective2 = std::make_shared<ReachSpotTask>(spot2, *m_font, messanger, quest.get());

    quest->addTask(objective);
    quest->addTask(objective2, objective.get());
    objective->m_on_completion_callback = [this, new_pos = spot1.getPosition()]()
    {
        auto &star_effect = m_world->addVisualEffect(EffectType::ParticleEmiter);
        star_effect.setPosition(new_pos);
        star_effect.setLifeTime(2.f);
    };
    m_objective_system->add(quest);

    spot1.attach(objective);
    spot2.attach(objective2);
}

void Game::handleEvent(const SDL_Event &event)
{
    auto mouse_position = m_window.getMouseInWorld();

    if (event.type == SDL_KEYDOWN)
    {
        if (event.key.keysym.sym == m_key_binding[PlayerControl::BOOST])
        {
            m_player->onBoostDown();
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
            m_player->onBoostUp();
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
    if (event.type == SDL_MOUSEBUTTONUP)
    {

        if (isKeyPressed(SDL_SCANCODE_LCTRL) && event.button.button == SDL_BUTTON_RIGHT)
        {
            auto &new_enemy = m_world->addObject2<Boss>();
            new_enemy.setPosition(mouse_position);
        }
        if (event.button.button == SDL_BUTTON_RIGHT)
        {
            m_camera.startMovingTo(m_window.getMouseInWorld(), 1.);
            // auto &station = m_world->addObject2<Turret>();
            // station.setPosition(mouse_position);
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
        m_player->acceleration = 15.f;
    }
    else if (isKeyPressed(m_key_binding[PlayerControl::MOVE_BACK]))
    {
        m_player->acceleration = -15.f;
    }
    else
    {
        m_player->acceleration = 0.;
    }
}

void Game::update(const float dt, Renderer &window)
{
    m_camera.update(dt, m_player);
    window.m_view = m_camera.getView();

    parseInput(window, dt);

    if (m_player->health < 0)
    {
        m_state = GameState::PLAYER_DIED;
    }

    m_world->update2(dt);
    m_world->update(dt);

    messanger.distributeMessages();

    m_objective_system->update();
};

void Game::draw(Renderer &window)
{

    Sprite background_rect;
    auto old_view = window.m_view;
    window.m_view = window.getDefaultView();
    background_rect.setPosition(window.getTargetSize() / 2.f);
    background_rect.setScale(window.getTargetSize() / 2.f);
    background_rect.setTexture(*m_background);
    //! move background with view
    utils::Vector2f view_size_rel = utils::Vector2f{0.2, 0.2} + old_view.getSize() / 1415.f;
    utils::Vector2f rel_pos = old_view.getCenter() / 1400.;
    utils::Vector2f size = m_background->getSize();
    background_rect.m_tex_rect = {(int)(size.x * rel_pos.x),
                                  (int)(size.y * rel_pos.y),
                                  (int)(size.x * view_size_rel.x),
                                  (int)(size.y * view_size_rel.y)};
    window.drawSprite(background_rect);
    window.drawAll();
    window.m_view = old_view;

    m_layers.clearAllLayers();
    // m_player->draw(m_layers);
    m_world->draw(m_layers);
    m_world->draw2(m_layers, window.m_view);
    // Enemy::m_neighbour_searcher.drawGrid(*m_layers.getLayer("Unit"));

    // auto &test_canvas = m_layers.getCanvas("Bloom");
    // Sprite fire_sprite(*m_textures.get("FireNoise"));
    // fire_sprite.setPosition(m_player->getPosition());
    // fire_sprite.setScale(utils::Vector2f{m_player->m_radius * 5.});
    // test_canvas.drawSprite(fire_sprite, "fireEffect");

    //! clear and draw into scene
    // m_scene_canvas.clear({0, 0, 0, 0});
    // m_scene_canvas.m_view = old_view;
    m_window.m_blend_factors = {BlendFactor::One, BlendFactor::OneMinusSrcAlpha};
    m_layers.setView(m_window.m_view);
    m_layers.drawInto(m_window);

    // //! draw everything to a window quad
    // auto scene_size = m_scene_pixels.getSize();
    // Sprite screen_sprite(m_scene_pixels.getTexture());
    // screen_sprite.setPosition(scene_size / 2.f);
    // screen_sprite.setScale(scene_size / 2.f);
    // m_window.m_view.setCenter(screen_sprite.getPosition());
    // m_window.m_view.setSize(scene_size);
    // m_window.drawSprite(screen_sprite, "LastPass");
    //! draw the scene into the window
    auto old_factors = m_window.m_blend_factors;
    // m_window.m_blend_factors = {BlendFactor::One, BlendFactor::OneMinusSrcAlpha};
    // m_window.drawAll();

    //
    m_window.m_view = old_view;
    m_ui_system->update(0.016);
    m_ui_system->draw(window);
    // m_window.m_blend_factors = old_factors;

    m_objective_system->draw(window, m_textures);
    m_window.drawAll();
    // m_ui.draw(window);
}


Game::GameState Game::getState() const
{
    return m_state;
}

int Game::getScore() const
{
    return m_score;
}

PlayerEntity *Game::getPlayer()
{
    return m_player;
}