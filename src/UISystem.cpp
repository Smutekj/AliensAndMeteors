#include "UISystem.h"
#include "Renderer.h"
#include "Entities/Player.h"

UISystem::UISystem(Renderer &window, TextureHolder &textures, PostOffice &messenger, PlayerEntity *player, Font &font)
    : ui(window), p_post_office(&messenger), p_player(player), window_canvas(window)
{
    auto top_bar = std::make_shared<UIElement>();
    auto fuel_bars = std::make_shared<UIElement>();
    auto health_bars = std::make_shared<UIElement>();

    auto health_bar = std::make_shared<SpriteUIELement>("healthBar");
    auto shield_bar = std::make_shared<SpriteUIELement>("healthBar");
    health_bar->bounding_box = {0, 0, 200, 40};
    shield_bar->bounding_box = {0, 0, 200, 40};
    auto money = std::make_shared<TextUIELement>(font, std::to_string(p_player->m_money) + " $");
    money->id = "Money";
    money->align = Alignement::Center;
    money->dimensions = {Pixels{100}, Pixels{30}};
    

    auto fuel_bar = std::make_shared<SpriteUIELement>("fuelBar", textures.get("FireNoise").get());
    auto boost_bar = std::make_shared<SpriteUIELement>("boostBar", textures.get("FireNoise").get());
    fuel_bar->dimensions = {Percentage{1.}, Percentage{0.4}};
    boost_bar->dimensions = {Percentage{1.}, Percentage{0.4}};

    fuel_bars->dimensions = {Percentage{0.2}, Percentage{1.}};
    fuel_bars->layout = Layout::Y;
    fuel_bars->align = Alignement::Left;
    fuel_bars->content_align_x = Alignement::Center;
    fuel_bars->content_align_y = Alignement::Center;
    // fuel_bars->sizing = Sizing::SCALE_TO_FIT;
    fuel_bars->margin.x = {50};
    fuel_bars->id = "FB";
    health_bars->layout = Layout::Y;
    health_bars->align = Alignement::Right;
    health_bars->sizing = Sizing::SCALE_TO_FIT;
    health_bars->margin.x = {50};
    health_bars->id = "HB";

    fuel_bars->addChildren(fuel_bar, boost_bar);
    health_bars->addChildren(health_bar, shield_bar);

    top_bar->bounding_box = {0, 0, window.getTargetSize().x, 120};
    top_bar->layout = Layout::X;
    top_bar->align = Alignement::CenterX;
    top_bar->content_align_y = Alignement::Center;
    top_bar->addChildren(fuel_bars, money, health_bars);

    ui.root->bounding_box = {0, 0, window.getTargetSize().x, window.getTargetSize().y};
    ui.root->addChildren(top_bar);
    ui.root->content_align_y = Alignement::Top;
}

void UISystem::draw(Renderer &window)
{
    auto old_view = window.m_view;
    window.m_view = window.getDefaultView();
    window.m_view.setSize(window.m_view.getSize().x, -window.m_view.getSize().y);
    ui.drawUI();

    window.m_view = old_view;
}

void UISystem::update(float dt)
{

    SizeVariant element_dimensions = Pixels{2.f};

    
    std::visit([](auto&& value) {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<T, Percentage>)
            std::cout << "Percentage: " << value << "%\n";
        else if constexpr (std::is_same_v<T, Pixels>)
            std::cout << "Pixels: " << value << "px\n";
        else if constexpr (std::is_same_v<T, Viewport>)
            std::cout << "Viewport: " << value << "vw\n";
    }, element_dimensions);



    float booster_ratio = std::min({1.f, p_player->boost_heat / p_player->max_boost_heat});
    window_canvas.getShader("boostBar").use();
    window_canvas.getShader("boostBar").setUniform2("u_booster_disabled", (int)(p_player->booster == BoosterState::Disabled));
    window_canvas.getShader("boostBar").setUniform2("u_booster_ratio", booster_ratio);
    window_canvas.getShader("boostBar").setUniform2("u_bar_aspect_ratio", 2.f);

    // draw fuel bar
    float fuel_ratio = std::min({1.f, p_player->m_fuel / p_player->m_max_fuel});
    window_canvas.getShader("fuelBar").use();
    window_canvas.getShader("fuelBar").setUniform2("u_fuel_ratio", fuel_ratio);

    float health_ratio = std::min({1.f, p_player->health / p_player->max_health});
    window_canvas.getShader("healthBar").use();
    window_canvas.getShader("healthBar").setUniform2("u_health_ratio", health_ratio);
}
