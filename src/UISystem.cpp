#include "UISystem.h"
#include "Renderer.h"
#include "Entities/Player.h"
#include "GameWorld.h"

UISystem::UISystem(Renderer &window, TextureHolder &textures,
                   PostOffice &messenger, PlayerEntity *player,
                   Font &font, GameWorld &world)
    : ui(window), p_post_office(&messenger),
      p_player(player), window_canvas(window),
      m_textures(textures), p_world(&world)
{
    auto m_boss_postbox = std::make_unique<PostBox<StartedBossFightEvent>>(messenger, [this](const auto &events)
                                                                           {
        for(const auto&  e : events)
        {
            boss_id = e.boss_id;
            addBossBar();
        } });
    auto m_died_postbox = std::make_unique<PostBox<EntityDiedEvent>>(messenger, [this](const auto &events)
        {
        for (const auto &e : events)
        {
            if(e.id == boss_id)
            {
                removeBossBar();
                boss_id = -1;
            }
        };
    });
    m_post_boxes.push_back(std::move(m_boss_postbox));
    m_post_boxes.push_back(std::move(m_died_postbox));

    auto top_bar = std::make_shared<UIElement>();

    auto bottom_bar = std::make_shared<UIElement>();
    auto fuel_bars = std::make_shared<UIElement>();
    auto health_bars = std::make_shared<UIElement>();

    auto health_bar = std::make_shared<SpriteUIELement>("healthBar");
    auto shield_bar = std::make_shared<SpriteUIELement>("healthBar");
    health_bar->dimensions = {Percentage{1.f}, Percentage{0.4f}};
    health_bar->id = "HealthBar";
    shield_bar->dimensions = {Percentage{1.f}, Percentage{0.4f}};
    auto money = std::make_shared<TextUIELement>(font, std::to_string(p_player->m_money) + " $");
    money->id = "Money";
    money->align_x = Alignement::Center;
    money->dimensions = {Pixels{100}, Pixels{30}};

    auto fuel_bar = std::make_shared<SpriteUIELement>("fuelBar", textures.get("FireNoise").get());
    auto boost_bar = std::make_shared<SpriteUIELement>("boostBar", textures.get("FireNoise").get());
    fuel_bar->dimensions = {Percentage{1.f}, Percentage{0.4f}};
    boost_bar->dimensions = {Percentage{1.f}, Percentage{0.4f}};

    fuel_bars->dimensions = {Percentage{0.2f}, Percentage{1.f}};
    fuel_bars->layout = Layout::Y;
    fuel_bars->align_x = Alignement::Left;
    fuel_bars->content_align_x = Alignement::Center;
    fuel_bars->content_align_y = Alignement::Center;
    // fuel_bars->sizing = Sizing::SCALE_TO_FIT;
    fuel_bars->margin.x = {50};
    fuel_bars->id = "FB";
    health_bars->layout = Layout::Y;
    health_bars->align_x = Alignement::Right;
    health_bars->content_align_x = Alignement::Center;
    health_bars->content_align_y = Alignement::Center;
    health_bars->dimensions = {Percentage{0.2f}, Percentage{1.f}};
    health_bars->margin.x = {50};
    health_bars->id = "HB";

    fuel_bars->addChildren(fuel_bar, boost_bar);
    health_bars->addChildren(health_bar, shield_bar);

    bottom_bar->dimensions = {Percentage{1.f}, Percentage{0.1f}};
    bottom_bar->id = "BottomBar";
    bottom_bar->layout = Layout::X;
    bottom_bar->align_x = Alignement::CenterX;
    bottom_bar->content_align_y = Alignement::Center;
    bottom_bar->addChildren(fuel_bars, money, health_bars);

    top_bar->dimensions = {Percentage{0.8f}, Percentage{0.1f}};
    top_bar->id = "TopBar";
    top_bar->layout = Layout::Y;
    top_bar->align_y = Alignement::Top;
    top_bar->align_x = Alignement::Center;
    top_bar->content_align_y = Alignement::Center;
    top_bar->addChildren();

    ui.root->dimensions = {Pixels{(float)window.getTargetSize().x}, Pixels{(float)window.getTargetSize().y}};
    ui.root->addChildren(top_bar, bottom_bar);
    ui.root->content_align_y = Alignement::Bottom;
    ui.root->content_align_x = Alignement::CenterX;
    ui.root->layout = Layout::Y;
}

void UISystem::draw(Renderer &window)
{
    auto old_view = window.m_view;
    window.m_view = window.getDefaultView();
    window.m_view.setSize(window.m_view.getSize().x, -window.m_view.getSize().y);

#if DEBUG
    ui.drawBoundingBoxes(); //! TODO: WHY THE FUCK IS IT FLIPPED WHEN I PUT IT AFTER drawUI???
#endif

    ui.drawUI();

    window.m_view = old_view;
}

void UISystem::addBossBar()
{
    auto boss_bar = std::make_shared<SpriteUIELement>("healthBar", m_textures.get("FireNoise").get());
    boss_bar->dimensions = {Percentage{1.f}, Percentage{0.4f}};
    boss_bar->id = "BossHealth";
    boss_bar->m_shader_id = "bossHealthBar";

    if (auto el = ui.getElementById("TopBar"))
    {
        el->addChild(boss_bar);
    }
}
void UISystem::removeBossBar()
{
    auto boss_bar = ui.getElementById("BossHealth");
    boss_bar->dimensions = {Percentage(0.f), Percentage(0.f)};
}

void UISystem::update(float dt)
{

    float booster_ratio = std::min({1.f, p_player->boost_heat / p_player->max_boost_heat});
    window_canvas.getShader("boostBar").use();
    window_canvas.getShader("boostBar").setUniform2("u_booster_disabled", (int)(p_player->booster == BoosterState::Disabled));
    window_canvas.getShader("boostBar").setUniform2("u_booster_ratio", booster_ratio);
    window_canvas.getShader("boostBar").setUniform2("u_bar_aspect_ratio", 2.f);

    // draw fuel bar
    float fuel_ratio = std::min({1.f, p_player->m_fuel / p_player->m_max_fuel});
    window_canvas.getShader("fuelBar").use();
    window_canvas.getShader("fuelBar").setUniform2("u_fuel_ratio", fuel_ratio);

    float health_ratio = std::min({1.f, p_player->getHpRatio()});
    window_canvas.getShader("healthBar").use();
    window_canvas.getShader("healthBar").setUniform2("u_health_ratio", health_ratio);

    if (boss_id != -1 && p_world->contains(boss_id))
    {
        if( p_world->m_systems.has<HealthComponent>(boss_id))
        {
            auto &h_comp = p_world->m_systems.get<HealthComponent>(boss_id);
            float health_ratio = h_comp.hp / h_comp.max_hp;
    
            window_canvas.getShader("bossHealthBar").use();
            window_canvas.getShader("bossHealthBar").setUniform2("u_health_ratio", health_ratio);
        }
    }

    //!
}
