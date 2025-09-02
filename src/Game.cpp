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

#include "SoundSystem.h"

void Game::initializeSounds()
{
    std::filesystem::path sounds_path = std::string(RESOURCES_DIR) + "/Sounds";
    SoundSystem::registerSound(SoundID::Explosion1, sounds_path / "Explode.wav");
    SoundSystem::registerSound(SoundID::Laser1, sounds_path / "Lasers/Laser_09.wav");
    SoundSystem::registerSound(SoundID::Laser2, sounds_path / "Lasers/Laser_01.wav");
    SoundSystem::registerSound(SoundID::Laser3, sounds_path / "Lasers/Laser_04.wav");
    SoundSystem::registerSound(SoundID::Electro, sounds_path / "Electro/Laser_03.wav");
    SoundSystem::registerSound(SoundID::Rocket1, sounds_path / "Rockets/Rocket_1.wav");
    SoundSystem::registerSound(SoundID::Rocket2, sounds_path / "Rockets/Rocket_2.wav");
    SoundSystem::registerSound(SoundID::Rocket3, sounds_path / "Rockets/Rocket_3.wav");
    SoundSystem::registerSound(SoundID::Rocket4, sounds_path / "Rockets/Rocket_4.wav");
};
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
    shiny_layer.addEffect(std::make_unique<Bloom>(width / 2, height / 2));
    auto &laser_layer = m_layers.addLayer("Bloom2",5, options, width, height);
    laser_layer.m_canvas.setShadersPath(shaders_directory);
    laser_layer.m_canvas.addShader("Instanced", "basicinstanced.vert", "texture.frag");
    laser_layer.addEffect(std::make_unique<BloomFinal>(width, height));

    auto &base_layer = m_ui_layers.addLayer("Base", -1, options, width, height);
    base_layer.m_canvas.setShadersPath(shaders_directory);
    auto &bloom_layer = m_ui_layers.addLayer("Bloom", 0, options, width, height);
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
constexpr float START_VIEW_SIZE = 300.f;

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
      m_camera(PLAYER_START_POS, {START_VIEW_SIZE, START_VIEW_SIZE * window.getTargetSize().y / window.getTargetSize().x}, messanger),
      m_ui(static_cast<Window &>(window.getTarget()), m_textures)
{
    messanger.registerEvents<EntityDiedEvent,
                             QuestCompletedEvent,
                             CollisionEvent,
                             DamageReceivedEvent,
                             HealthChangedEvent,
                             StartedBossFightEvent,
                             StartedTimerEvent,
                             EntityLeftViewEvent>();

    initializeSounds();
    initializeLayersAndTextures();
    m_world = std::make_unique<GameWorld>(messanger, m_textures);
    //! PLAYER NEEDS TO BE FIRST BECAUSE OTHER OBJECTS MIGHT REFERENCE IT!
    m_player = &m_world->addObjectForced<PlayerEntity>();
    m_player->setPosition({500, 500});
    m_world->m_player = m_player;
    m_player->setDestructionCallback([this](int id, ObjectType type)
                                     { m_state = GameState::PLAYER_DIED; });

    m_ui_system = std::make_unique<UISystem>(window, m_textures, messanger, m_player, *m_font, *m_world);
    m_ui.initWorld(*m_world);

    m_enemy_factory = std::make_unique<EnemyFactory>(*m_world, m_textures);
    m_pickup_factory = std::make_unique<PickupFactory>(*m_world, m_textures);
    m_laser_factory = std::make_unique<LaserFactory>(*m_world, m_textures);
    m_bullet_factory = std::make_unique<ProjectileFactory>(*m_world, m_textures);
    m_quest_factory = std::make_unique<QuestFactory>(*m_objective_system, messanger, *m_ui_system, *m_world, m_textures, m_camera, *m_font, m_timers);
    registerCollisions();
    registerSystems();

    m_objective_system = std::make_unique<ObjectiveSystem>(messanger);

    m_background = std::make_unique<Texture>(std::string(RESOURCES_DIR) + "/Textures/background.png");

    for (int i = 0; i < 300; ++i)
    {
        auto &meteor = m_world->addObject2<Meteor>();
        auto spawn_pos = m_player->getPosition() + randf(200, 3000) * angle2dir(randf(0, 360));
        meteor.setPosition(spawn_pos);
    }
    for (int i = 0; i < 0; ++i)
    {
        auto spawn_pos = m_player->getPosition() + randf(100, 2000) * angle2dir(randf(0, 360));
        m_enemy_factory->create2(EnemyType::ShooterEnemy, spawn_pos);
    }

    m_camera.setSpeed(10);
    // auto& bul = m_bullet_factory->create2(ProjectileType::LaserBullet, m_window.getMouseInWorld(), {255,0,0,255});
    // b = &bul;

    // spawnNextObjective();
    // addDestroyNObjective(ObjectType::SpaceStation, 2);

    auto &heart_spawner = m_world->addTrigger<Timer>();
    heart_spawner.setCallback(
        [this]()
        {
            auto spawn_pos = m_player->getPosition() + randf(20, 200) * angle2dir(randf(0, 360));
            auto &heart = m_pickup_factory->create2(Pickup::Shield, spawn_pos);
        });

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
    auto build_the_wall = [this](utils::Vector2f center)
    {
        //! create walls
        auto &wall_up = m_world->addObject3(ObjectType::Wall);
        auto &wall_down = m_world->addObject3(ObjectType::Wall);
        auto &wall_left = m_world->addObject3(ObjectType::Wall);
        auto &wall_right = m_world->addObject3(ObjectType::Wall);

        CollisionComponent c_comp_left;
        CollisionComponent c_comp_right;
        CollisionComponent c_comp_up;
        CollisionComponent c_comp_down;
        c_comp_left.type = ObjectType::Wall;
        c_comp_right.type = ObjectType::Wall;
        c_comp_up.type = ObjectType::Wall;
        c_comp_down.type = ObjectType::Wall;

        c_comp_left.shape.convex_shapes = {4};
        c_comp_right.shape.convex_shapes = {4};
        c_comp_up.shape.convex_shapes = {4};
        c_comp_down.shape.convex_shapes = {4};

        wall_right.setPosition(center + utils::Vector2f{400, 0});
        wall_left.setPosition(center - utils::Vector2f{390, 0});
        wall_up.setPosition(center + utils::Vector2f{0, 290});
        wall_down.setPosition(center - utils::Vector2f{0, 290});

        wall_right.setSize({400, 300});
        wall_left.setSize({400, 300});
        wall_up.setSize({400, 300});
        wall_down.setSize({400, 300});

        SpriteComponent s_compl = {.layer_id = "Unit", .shader_id = "fuelBar", .sprite = Sprite(*m_textures.get("FireNoise"))};
        SpriteComponent s_compr = {.layer_id = "Unit", .shader_id = "fuelBar", .sprite = Sprite(*m_textures.get("FireNoise"))};
        SpriteComponent s_compu = {.layer_id = "Unit", .shader_id = "fuelBar", .sprite = Sprite(*m_textures.get("FireNoise"))};
        SpriteComponent s_compd = {.layer_id = "Unit", .shader_id = "fuelBar", .sprite = Sprite(*m_textures.get("FireNoise"))};

        m_world->m_systems.addEntityDelayed(wall_up.getId(), c_comp_up, s_compl);
        m_world->m_systems.addEntityDelayed(wall_left.getId(), c_comp_left, s_compr);
        m_world->m_systems.addEntityDelayed(wall_down.getId(), c_comp_down, s_compd);
        m_world->m_systems.addEntityDelayed(wall_right.getId(), c_comp_right, s_compu);
    };

    auto &boss = m_world->addObject2<Boss2>();
    auto player_pos = m_player->getPosition();
    auto boss_pos = player_pos + utils::Vector2f{0, 150};

    boss.setPosition(boss_pos);
    build_the_wall(player_pos);

    m_camera.startMovingTo(player_pos, 1., [](auto &camera)
                           {
        camera.m_move_state = Camera::MoveState::Fixed;
        
        camera.startChangingSize({410, 310}, 2., [](auto& camera){camera.m_view_size_state = Camera::SizeState::Fixed;}); });

    messanger.send(StartedBossFightEvent{boss.getId()});
    // auto kill_task = std::make_shared<DestroyEntityTask>(boss, *m_font, messanger, &q);
    // q.addTask(kill_task);

    auto &entities = m_world->getEntities();
    for (auto obj : entities.data())
    {
        if (obj->getType() == ObjectType::Meteor)
        {
            obj->kill();
        }
    }
    //    auto& quest_giver = createQuestGiver(m_quest_factory->create(QuestType::BossFight1));
    //    quest_giver.setPosition(m_player->getPosition() + utils::Vector2f{300, 0});
    //    auto& quest_giver = createQuestGiver(m_quest_factory->create(QuestType::BossFight2));
    //    quest_giver.setPosition(m_player->getPosition() + utils::Vector2f{0, 300});
}

void Game::startTimer()
{
}

void Game::startSurvival()
{

    auto build_boundaryx = [this](utils::Vector2f start, utils::Vector2f finish, float width)
    {
        float length = utils::norm(finish - start);
        utils::Vector2f dir = (finish - start) / length;
        float angle = utils::dir2angle(dir);
        utils::Vector2f center = (start + finish) / 2.f;

        auto &wall_left = m_world->addObject3(ObjectType::Wall);
        CollisionComponent c_comp_left;
        c_comp_left.type = ObjectType::Wall;
        c_comp_left.shape.convex_shapes = {4};

        wall_left.setPosition(center);
        wall_left.setAngle(angle);
        wall_left.setSize({length, 20});
        SpriteComponent s_compl = {.layer_id = "Bloom2", .shader_id = "fireEffect", .sprite = Sprite(*m_textures.get("FireNoise"))};
        m_world->m_systems.addEntityDelayed(wall_left.getId(), c_comp_left, s_compl);
    };
    auto build_boundary = [this](utils::Vector2f start, utils::Vector2f finish, float width)
    {
        float length = utils::norm(finish - start);
        utils::Vector2f dir = (finish - start) / length;
        utils::Vector2f perp_dir = {dir.y, -dir.x};
        float angle = utils::dir2angle(dir);
        utils::Vector2f center_l = (start + finish) / 2.f - perp_dir * width / 2.f;
        utils::Vector2f center_r = (start + finish) / 2.f + perp_dir * width / 2.f;

        auto &wall_left = m_world->addObject3(ObjectType::Wall);
        auto &wall_right = m_world->addObject3(ObjectType::Wall);

        CollisionComponent c_comp_left;
        CollisionComponent c_comp_right;
        c_comp_left.type = ObjectType::Wall;
        c_comp_right.type = ObjectType::Wall;

        c_comp_left.shape.convex_shapes = {4};
        c_comp_right.shape.convex_shapes = {4};

        wall_right.setPosition(center_l);
        wall_right.setAngle(angle);
        wall_right.setSize({length, 20});
        wall_left.setPosition(center_r);
        wall_left.setAngle(angle);
        wall_left.setSize({length, 20});

        SpriteComponent s_compl = {.layer_id = "Unit", .shader_id = "fuelBar", .sprite = Sprite(*m_textures.get("FireNoise"))};
        SpriteComponent s_compr = {.layer_id = "Unit", .shader_id = "fuelBar", .sprite = Sprite(*m_textures.get("FireNoise"))};

        m_world->m_systems.addEntityDelayed(wall_right.getId(), c_comp_right, s_compr);
        m_world->m_systems.addEntityDelayed(wall_left.getId(), c_comp_left, s_compl);
    };

    //! generate path
    std::deque<utils::Vector2f> path = {m_player->getPosition() + utils::Vector2f{100, 0}};
    int direction = 1;
    path.push_back(m_player->getPosition() + utils::Vector2f{200, 0});
    for (int i = 1; i < 200; ++i)
    {
        auto prev_point = path.at(i);
        auto prev_dir = prev_point - path.at(i - 1);
        float prev_angle = utils::dir2angle(prev_dir);
        auto next_point = prev_point + utils::angle2dir(prev_angle + direction * randf(5, 10)) * 100.f;
        // build_boundary(prev_point, next_point, 300.f);
        path.push_back(next_point);
        
        if (randi(5) == 0)
        {
            direction *= -1;
        }
    }
    
    auto get_intersection = [](utils::Vector2f r0, utils::Vector2f v0, utils::Vector2f r1, utils::Vector2f v1)
    {
        utils::Vector2f dr = r0 - r1;
        utils::Vector2f n0 = {v0.y, -v0.x};
        float beta = utils::dot(dr, n0) / utils::dot(v1, n0);
        return r1 + beta * v1;
    } ;

    float width = 100.f;
    auto start_l = path.at(1) + utils::Vector2f{0, width};
    auto start_r = path.at(1) - utils::Vector2f{0, width};
    for (int i = 1; i < path.size() - 1; ++i)
    {
        auto prev_point = path.at(i-1);
        auto curr_point = path.at(i);
        auto next_point = path.at(i+1);
        auto prev_dir = curr_point - prev_point;
        auto next_dir = next_point - curr_point;
        prev_dir /= utils::norm(prev_dir);
        next_dir /= utils::norm(next_dir);
        utils::Vector2f prev_perp_dir = {prev_dir.y, -prev_dir.x};
        utils::Vector2f next_perp_dir = {next_dir.y, -next_dir.x};
        
        utils::Vector2f end_l = get_intersection(start_l, prev_dir, curr_point, next_perp_dir); 
        utils::Vector2f end_r = get_intersection(start_r, prev_dir, curr_point, next_perp_dir); 
        build_boundaryx(start_l, end_l, 300.f);
        build_boundaryx(start_r, end_r, 300.f);
        start_l = end_l;
        start_r = end_r;
    }


    m_timers.addInfiniteEvent(1.f, [this](float t, int c)
                              {
        m_enemy_factory->create2(EnemyType::ShooterEnemy, m_player->getPosition() + utils::Vector2f{-100.f, 0.f});
        m_enemy_factory->create2(EnemyType::EnergyShooter, m_player->getPosition() + utils::Vector2f{-100.f, 0.f}); });
    m_timers.addInfiniteEvent(3.f, [this](float t, int c)
                              { m_enemy_factory->create2(EnemyType::LaserEnemy, m_player->getPosition() + utils::Vector2f{+100.f, 0.f}); });
    m_camera.setSpeed(75);
    m_camera.startFollowingPath(path, 1.f);
    // auto &quest_giver = createQuestGiver(m_quest_factory->create(QuestType::Survival1));
    // quest_giver.setPosition(m_player->getPosition() + Vec2{500, 0});
}
void Game::startTimeRace()
{
    m_stage = GameStage::TimeRace;

    auto &quest_giver = createQuestGiver(m_quest_factory->create(QuestType::TimeRace1));
    quest_giver.setPosition(m_player->getPosition() + Vec2{500, 0});
    // messanger.send(StartedTimeRaceEvent{});
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

    // auto &quest_giver = createQuestGiver(quest);
    // quest_giver.setPosition(m_player->getPosition() + Vec2{500, 0});
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
        if (event.key.keysym.sym == SDL_KeyCode::SDLK_r)
        {
            if (!m_player->shield_active)
            {
                m_player->activateShield();
            }
        }
        if (event.key.keysym.sym == m_key_binding[PlayerControl::SHOOT_LASER])
        {
            auto &bullet = m_bullet_factory->create2(ProjectileType::Rocket, m_player->getPosition(), ColorByte{});
            bullet.m_max_vel = m_player->speed;
            bullet.m_vel = utils::angle2dir(m_player->getAngle()) * bullet.m_max_vel;
            bullet.setAngle(utils::dir2angle(bullet.m_vel));
            bullet.m_collision_resolvers[ObjectType::Player] = [](auto &obj, auto &c_data)
            {
                return;
            };
            // auto &laser = m_laser_factory->create2(LaserType::Basic, m_player->getPosition(), {0, 125, 255, 255});
            // m_player->addChild(&laser);
            // laser.m_stopping_types.push_back(ObjectType::Shield);
            // laser.m_stopping_types.push_back(ObjectType::Boss);
            // laser.m_rotates_with_owner = true;
            // laser.m_max_dmg = 0.2;
            // laser.m_life_time = 3.;
            // laser.m_max_length = 400.;
            // m_player->m_is_shooting_laser = true;
            // m_player->m_laser_timer = laser.m_life_time;
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
            startSurvival();
            // startTimeRace();
            // startBossFight();
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

    m_world->update(dt);

    m_timers.update(dt);

    messanger.distributeMessages();

    m_objective_system->update(dt);
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
    m_world->draw(m_layers, m_camera.getView());
    // Enemy::m_neighbour_searcher.drawGrid(*m_layers.getLayer("Unit"));

    auto &test_canvas = m_layers.getCanvas("Bloom");
    Sprite fire_sprite(*m_textures.get("FireNoise"));
    fire_sprite.setPosition(m_player->getPosition());
    fire_sprite.setScale(utils::Vector2f{m_player->getSize().x});
    test_canvas.drawSprite(fire_sprite, "fireShield");

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
    colllider.registerResolver(ObjectType::Meteor, ObjectType::Wall, [](GameObject &obj1, GameObject &obj2, CollisionData c_data)
                               { 
                                //! bounce meteor off the wall
                                auto mvt = c_data.separation_axis;
                                if (dot(mvt, obj1.m_vel) > 0.f)
                                {
                                    obj1.m_vel -= 2.f * dot(mvt, obj1.m_vel) * mvt;
                                } });
    colllider.registerResolver(ObjectType::Player, ObjectType::Wall, [](GameObject &obj1, GameObject &obj2, CollisionData c_data)
                               { 
                                //! bounce meteor off the wall
                                auto mvt = -c_data.separation_axis;
                                if (dot(mvt, obj1.m_vel) < 0.f)
                                {
                                    obj1.m_vel -= 2.f * dot(mvt, obj1.m_vel) * mvt;
                                    obj1.setAngle(utils::dir2angle(obj1.m_vel));

                                } });

    colllider.registerResolver(ObjectType::Shield, ObjectType::Meteor);
    colllider.registerResolver(ObjectType::Shield, ObjectType::Bullet);
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
    systems.registerSystem(std::make_shared<ParticleSystem>(systems.getComponents<ParticleComponent>(),
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
    animation_system->registerAnimation("FrontShield.png", AnimationId::FrontShield, "FrontShield.json");
    animation_system->registerAnimation("FrontShield2.png", AnimationId::FrontShield2, "FrontShield2.json");

    systems.registerSystem(animation_system);
}

void Game::loadTextures()
{
    m_textures.setBaseDirectory(std::string(RESOURCES_DIR) + "/Textures/");
    m_textures.add("Bomb", "bomb.png");
    m_textures.add("EnemyShip", "EnemyShip.png");
    m_textures.add("QuestGiver", "Buildings/QuestGiver.png");
    m_textures.add("Boss1", "Ships/Boss1.png");
    m_textures.add("Boss2", "Ships/Boss2.png");
    m_textures.add("EnemyLaser", "EnemyLaser.png");
    m_textures.add("EnemyBomber", "EnemyBomber.png");
    m_textures.add("Meteor", "Meteor.png");
    m_textures.add("BossShip", "BossShip.png");
    m_textures.add("Explosion2", "explosion2.png");
    m_textures.add("Explosion", "explosion.png");
    m_textures.add("PlayerShip", "playerShip.png");
    m_textures.add("Heart", "Heart.png");
    m_textures.add("ShieldPickup", "ShieldPickup.png");
    m_textures.add("Station", "Station.png");
    m_textures.add("Missile", "Missile.png");
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
