#include "Game.h"

#include "Entities/Player.h"
#include "Entities/Attacks.h"
#include "Entities/Bosses.h"

#include "Utils/RandomTools.h"

#include "Systems/BoidSystem.h"
#include "Systems/MeteorAvoidanceSystem.h"
#include "Systems/HealthSystem.h"
#include "Systems/TargetSystem.h"
#include "Systems/TimedEventSystem.h"
#include "Systems/EnemyAISystems.h"
#include "Systems/SpriteSystem.h"

#include <imgui_impl_sdl2.h>

void Game::initializeLayersAndTextures()
{
    m_textures.setBaseDirectory(std::string(RESOURCES_DIR) + "/Textures/");
    loadTextures();

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

    auto &shields_layer = m_layers.addLayer("Shields", 4, text_options, width, height);
    shields_layer.m_canvas.setShadersPath(shaders_directory);

    auto &unit_layer = m_layers.addLayer("Unit", 3, text_options, width, height);
    unit_layer.m_canvas.setShadersPath(shaders_directory);
    unit_layer.m_canvas.addShader("Instanced", "basicinstanced.vert", "texture.frag");
    unit_layer.m_canvas.addShader("Meteor", "basictex.vert", "Meteor.frag");
    // unit_layer.addEffect(std::make_unique<EdgeDetect>(width, height));
    // unit_layer.addEffect(std::make_unique<BloomFinal>(width, height));

    auto &shiny_layer = m_layers.addLayer("Bloom", 2, options, width, height);
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
    m_window.addShader("bossHealthBar", "basicinstanced.vert", "bossHealthBar.frag");
    m_window.addShader("boostBar", "basicinstanced.vert", "boostBar.frag");
    m_window.addShader("fuelBar", "basicinstanced.vert", "fuelBar.frag");
    m_window.addShader("boostBar2", "basicinstanced.vert", "boostBar2.frag");

    m_scene_canvas.setShadersPath(shaders_directory);

    glCheckErrorMsg("ERROR In initialize layers");
}

using namespace utils;

constexpr utils::Vector2f PLAYER_START_POS = {500, 500};
constexpr float START_VIEW_SIZE = 200.f;

GameObject &Game::createQuestGiver(std::shared_ptr<Quest> quest)
{
    GameObject &quest_giver = m_world->addObject3(ObjectType::SpaceStation);
    quest_giver.setSize({50, 50});

    CollisionComponent c_comp;
    Polygon shape = {32};
    c_comp.type = ObjectType::SpaceStation;
    c_comp.shape.convex_shapes = {shape};

    SpriteComponent s_comp = {.layer_id = "Unit", .sprite = {*m_textures.get("QuestGiver")}};

    m_world->m_systems.addEntityDelayed(quest_giver.getId(), c_comp, s_comp);

    quest_giver.m_collision_resolvers[ObjectType::Player] = [quest, this](auto &obj, auto &c_data)
    {
        if (!m_objective_system->contains(quest))
        {
            m_objective_system->add(quest);
        }
    };

    return quest_giver;
}

Game::Game(Renderer &window, KeyBindings &bindings)
    : m_window(window), m_key_binding(bindings),
      m_scene_pixels(window.getTargetSize().x, window.getTargetSize().y),
      m_scene_canvas(m_scene_pixels),
      m_camera(PLAYER_START_POS, {START_VIEW_SIZE, START_VIEW_SIZE * window.getTargetSize().y / window.getTargetSize().x}),
      m_ui(static_cast<Window &>(window.getTarget()), m_textures)
{
    messanger.registerEvents<EntityDiedEvent,
                             QuestCompletedEvent,
                             CollisionEvent,
                             DamageReceivedEvent,
                             HealthChangedEvent,
                             StartedBossFightEvent>();

    initializeLayersAndTextures();
    m_world = std::make_unique<GameWorld>(messanger, m_textures);
    m_enemy_factory = std::make_unique<EnemyFactory>(*m_world, m_textures);
    m_pickup_factory = std::make_unique<PickupFactory>(*m_world, m_textures);
    m_laser_factory = std::make_unique<LaserFactory>(*m_world, m_textures);
    registerCollisions();
    registerSystems();

    //! PLAYER NEEDS TO BE FIRST BECAUSE OTHER OBJECTS MIGHT REFERENCE IT!
    m_player = &m_world->addObjectForced<PlayerEntity>();
    m_player->setPosition({500, 500});
    m_world->m_player = m_player;
    m_player->setDestructionCallback([this](int id, ObjectType type)
                                     { m_state = GameState::PLAYER_DIED; });
    m_ui.initWorld(*m_world);

    m_objective_system = std::make_unique<ObjectiveSystem>(messanger);
    m_ui_system = std::make_unique<UISystem>(window, m_textures, messanger, m_player, *m_font, *m_world);

    m_background = std::make_unique<Texture>(std::string(RESOURCES_DIR) + "/Textures/background.png");

    for (int i = 0; i < 2; ++i)
    {
        auto &meteor = m_world->addObject2<Meteor>();
        auto spawn_pos = m_player->getPosition() + randf(200, 5000) * angle2dir(randf(0, 360));
        meteor.setPosition(spawn_pos);
    }
    for (int i = 0; i < 0; ++i)
    {
        auto spawn_pos = m_player->getPosition() + randf(100, 6000) * angle2dir(randf(0, 360));
        m_enemy_factory->create2(EnemyType::ShooterEnemy, spawn_pos);
    }

    spawnNextObjective();
    // addDestroyNObjective(ObjectType::SpaceStation, 2);

    // auto &heart_spawner = m_world->addTrigger<Timer>();
    // heart_spawner.setCallback(
    //     [this]()
    //     {
    //         auto spawn_pos = m_player->getPosition() + randf(20, 200) * angle2dir(randf(0, 360));
    //         auto &heart = m_pickup_factory->create2(Pickup::Heart, spawn_pos);
    //     });

    // auto &enemy_spawner = m_world->addTrigger<Timer>();
    // enemy_spawner.m_cooldown = 5.f;
    // enemy_spawner.setCallback(
    //     [this]()
    //     {
    //         {
    //             auto spawn_pos = m_player->getPosition() + randf(50, 200) * angle2dir(randf(0, 360));
    //             if(rand()%2 == 0)
    //             {
    //                 auto &enemy = m_enemy_factory->create2(EnemyType::EnergyShooter, spawn_pos);
    //             }else{
    //                 auto &enemy = m_enemy_factory->create2(EnemyType::ShooterEnemy, spawn_pos);
    //             }
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

    auto &qg = createQuestGiver(quest);
    qg.setPosition(m_player->getPosition() + utils::Vector2f{500, 0});
}

void Game::startBossFight()
{
    m_stage = GameStage::BossFight;

    auto &boss = m_world->addObject2<Boss1>();
    auto player_pos = m_player->getPosition();
    auto boss_pos = m_player->getPosition() + utils::Vector2f{100, 0};
    boss.setPosition(boss_pos);
    m_camera.startMovingTo(boss_pos - utils::Vector2f{150, 0}, 1., [](auto &camera)
                           {
        camera.m_view_state = Camera::MoveState::Fixed;
        camera.startChangingSize({400, 300}, 2., [](auto& camera){camera.m_view_size_state = Camera::SizeState::Fixed;}); });

    messanger.send(StartedBossFightEvent{boss.getId()});
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

    if (target_stage == GameStage::TimeRace)
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

    m_stage = target_stage;
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
            auto &enemy = m_enemy_factory->create2(EnemyType::ShooterEnemy, pos + randf(100, 200) * angle2dir(randf(0, 360)));
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
    quest->m_on_completion = [this, new_pos = spot2.getPosition()]()
    {
        auto &star_effect = m_world->addVisualEffect(EffectType::ParticleEmiter);
        star_effect.setPosition(new_pos);
        star_effect.setLifeTime(2.f);
    };
    spot1.attach(objective);
    spot2.attach(objective2);

    auto &quest_giver = createQuestGiver(quest);
    quest_giver.setPosition(m_player->getPosition() + Vec2{500, 0});
}

void Game::handleEvent(const SDL_Event &event)
{
    auto mouse_position = m_window.getMouseInWorld();

    ImGui_ImplSDL2_ProcessEvent(&event);
    if (ImGui::GetIO().WantCaptureMouse)
    {
        m_ui.handleEvent(event);
        return;
    }

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
            auto &laser = m_laser_factory->create2(LaserType::Basic, m_player->getPosition(), {0, 125, 255, 255});
            m_player->addChild(&laser);
            laser.m_stopping_types.push_back(ObjectType::Shield);
            laser.m_stopping_types.push_back(ObjectType::Boss);
            laser.m_rotates_with_owner = true;
            laser.m_max_dmg = 0.2;
            laser.m_life_time = 3.;
            laser.m_max_length = 400.;
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
        if (isKeyPressed(SDLK_LCTRL) && event.button.button == SDL_BUTTON_RIGHT)
        {
            auto &new_enemy = m_enemy_factory->create2(EnemyType::EnergyShooter, mouse_position);
        }
        else if (event.button.button == SDL_BUTTON_LEFT)
        {
            startBossFight();
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
        m_player->acceleration = -35.f;
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
    m_window.m_blend_factors = {BlendFactor::SrcAlpha, BlendFactor::OneMinusSrcAlpha};
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
    m_ui.draw();
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

void Game::registerCollisions()
{
    auto &colllider = m_world->getCollisionSystem();

    colllider.registerResolver(ObjectType::Meteor, ObjectType::Meteor, [](GameObject &obj1, GameObject &obj2, CollisionData c_data)
                               { Collisions::bounce(obj1, obj2, c_data); });
    colllider.registerResolver(ObjectType::Meteor, ObjectType::Bullet);

    colllider.registerResolver(ObjectType::Player, ObjectType::SpaceStation);
    
    colllider.registerResolver(ObjectType::Player, ObjectType::Meteor);
    colllider.registerResolver(ObjectType::Player, ObjectType::Bullet);
    colllider.registerResolver(ObjectType::Player, ObjectType::Laser);
    colllider.registerResolver(ObjectType::Player, ObjectType::Explosion);
    colllider.registerResolver(ObjectType::Player, ObjectType::Trigger);
    colllider.registerResolver(ObjectType::Player, ObjectType::Heart);

    colllider.registerResolver(ObjectType::Boss, ObjectType::Laser);
    colllider.registerResolver(ObjectType::Boss, ObjectType::Bullet);

    colllider.registerResolver(ObjectType::Enemy, ObjectType::Meteor);
    colllider.registerResolver(ObjectType::Enemy, ObjectType::Bullet);
    colllider.registerResolver(ObjectType::Enemy, ObjectType::Laser);
    colllider.registerResolver(ObjectType::Enemy, ObjectType::Explosion);
}

void Game::registerSystems()
{
    auto &systems = m_world->m_systems;

    systems.registerSystem(std::make_shared<BoidSystem>(systems.getComponents<BoidComponent>()));
    systems.registerSystem(std::make_shared<AvoidanceSystem>(systems.getComponents<AvoidMeteorsComponent>(),
                                                             systems, m_world->getCollisionSystem()));
    systems.registerSystem(std::make_shared<HealthSystem>(systems.getComponents<HealthComponent>(), messanger));
    systems.registerSystem(std::make_shared<TargetSystem>(systems.getComponents<TargetComponent>(), m_world->getEntities()));
    systems.registerSystem(std::make_shared<TimedEventSystem>(systems.getComponents<TimedEventComponent>()));
    systems.registerSystem(std::make_shared<AISystem>(*m_world,
                                                      systems.getComponents<ShootPlayerAIComponent>(),
                                                      systems.getComponents<LaserAIComponent>()));
    systems.registerSystem(std::make_shared<SpriteSystem>(systems.getComponents<SpriteComponent>(),
                                                          m_layers));

    std::filesystem::path animation_directory = {RESOURCES_DIR};
    animation_directory /= "Textures/Animations/";
    auto animation_system = std::make_shared<AnimationSystem>(
        systems.getComponents<AnimationComponent>(),
        animation_directory, animation_directory);

    animation_system->registerAnimation("LongShield.png", AnimationId::Shield, "LongShield.json");
    animation_system->registerAnimation("BlueExplosion.png", AnimationId::BlueExplosion, "BlueExplosion.json");
    animation_system->registerAnimation("PurpleExplosion.png", AnimationId::PurpleExplosion, "PurpleExplosion.json");
    animation_system->registerAnimation("GreenBeam.png", AnimationId::GreenBeam, "GreenBeam.json");

    systems.registerSystem(animation_system);
}

void Game::loadTextures()
{
    m_textures.setBaseDirectory(std::string(RESOURCES_DIR) + "/Textures/");
    m_textures.add("Bomb", "bomb.png");
    m_textures.add("EnemyShip", "EnemyShip.png");
    m_textures.add("QuestGiver", "Buildings/QuestGiver.png");
    m_textures.add("Boss1", "Ships/Boss1.png");
    m_textures.add("EnemyLaser", "EnemyLaser.png");
    m_textures.add("EnemyBomber", "EnemyBomber.png");
    m_textures.add("Meteor", "Meteor.png");
    m_textures.add("BossShip", "BossShip.png");
    m_textures.add("Explosion2", "explosion2.png");
    m_textures.add("Explosion", "explosion.png");
    m_textures.add("PlayerShip", "playerShip.png");
    m_textures.add("Heart", "Heart.png");
    m_textures.add("Station", "Station.png");
    m_textures.add("BoosterYellow", "effectYellow.png");
    m_textures.add("BoosterPurple", "effectPurple.png");
    m_textures.add("Arrow", "arrow.png");
    m_textures.add("Emp", "emp.png");
    m_textures.add("Star", "star.png");
    m_textures.add("Fuel", "fuel.png");
    m_textures.add("Turrets", "Turrets.png");
    m_textures.add("Arrow", "arrow.png");
    m_textures.add("EnergyBullet", "EnergyBullet1.png");
    m_textures.add("EnergyBall", "EnergyBall.png");

    m_textures.add("LongShield", "Animations/LongShield.png");

    m_textures.add("Arrow", "arrow.png");
    m_textures.add("FireNoise", "fireNoise.png");
}
