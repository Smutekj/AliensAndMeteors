#include "GameState.h"
#include "MenuState.h"
#include "StateStack.h"
#include "ScoreBoard.h"

#include "../Entities/Player.h"

#include "../Game.h"
#include <Texture.h>

PlayerEntity *p_player = nullptr;

GameState::GameState(StateStack &stack, State::Context context)
    : State(stack, context)
{
    try
    {
        mp_game = std::make_shared<Game>(*context.window, *context.bindings);
        p_player = mp_game->getPlayer();
    }
    catch (std::exception &e)
    {
        std::cout << "ERROR DURING GAME CREATION: \n"
                  << e.what() << "\n";
        throw std::runtime_error("GAME CREATION FAILED");
    }
    std::cout << "Game Created!" << std::endl;
}

GameState::~GameState() {}

void GameState::update(float dt)
{
    auto &window = *m_context.window;

    mp_game->update(dt, window);

    if (mp_game->getState() == Game::GameState::PLAYER_DIED)
    {
        m_context.score->setCurrentScore(mp_game->getScore());
        m_stack->popState();
        m_stack->pushState(States::ID::Player_Died);
    }
    else if (mp_game->getState() == Game::GameState::SHOPPING)
    {
        m_stack->pushState(States::ID::Shop);
    }
}

void GameState::handleEvent(const SDL_Event &event)
{
    auto &window = *m_context.window;

    if (event.type == SDL_KEYUP)
    {
        if (event.key.keysym.sym == SDLK_ESCAPE)
        {
            m_stack->pushState(States::ID::Pause);
        }
    }
    mp_game->handleEvent(event);
}

void GameState::draw()
{
    auto &window = *m_context.window;
    window.clear({0, 0, 0, 0});
    mp_game->draw(window);
}

ShopState::ShopState(StateStack &stack, State::Context context)
    : State(stack, context), document(*context.window)
{

    auto shop_header = std::make_shared<SpriteUIELement>();
    shop_header->layout = Layout::X;
    shop_header->dimensions = {Pixels{800}, Pixels{200}};
    shop_header->content_align_x = Alignement::Right;
    shop_header->content_align_y = Alignement::Center;
    shop_header->setTexture(*m_context.textures->get("HeaderFrame"));
    // shop_header->sizing = Sizing::SCALE_TO_FIT;
    shop_header->id = "header";
    shop_header->padding = {50, 20};

    auto exit_button = std::make_shared<SpriteUIELement>();
    exit_button->setTexture(*m_context.textures->get("Close"));
    exit_button->dimensions = {Pixels{60}, Pixels{60}};
    // exit_button->margin = {20, 20};
    // exit_button->align = Alignement::Center;
    exit_button->id = "closeButton";

    // auto money_text = std::make_shared<TextUIELement>(*context.font, std::to_string(p_player->m_money) + " $");
    auto money_text = std::make_shared<TextUIELement>(*context.font, std::to_string(p_player->m_money) + " $");
    money_text->dimensions = {Pixels{120}, Pixels{60}};
    money_text->id = "moneyText";
    money_text->align = Alignement::Left;
    // money_text->margin = {20, 20};
    shop_header->addChildren(money_text,exit_button);


    auto grid_holder = std::make_shared<SpriteUIELement>();
    grid_holder->setTexture(*m_context.textures->get("ShopItemFrame"));
    grid_holder->layout = Layout::Grid;
    grid_holder->align = Alignement::CenterX;
    grid_holder->dimensions = {Percentage{0.9}, Pixels{600}};
    // grid_holder->sizing = Sizing::SCALE_TO_FIT;
    grid_holder->padding = {20, 50};

    std::vector<std::string> icon_textures = {"Fuel", "Heart", "Arrow", "Coin"};
    for (int i = 0; i < 4; ++i)
    {
        auto button_holder = std::make_shared<SpriteUIELement>();
        button_holder->id = "buttonHolder";
        button_holder->event_callbacks[UIEvent::MOUSE_ENETERED] = [button_holder](UIElement::UIElementP node)
        {
            node->padding.x = 11;
            node->padding.y = 6;
            button_holder->image.setColor({255,255, 255, 255});
        };
        button_holder->event_callbacks[UIEvent::MOUSE_LEFT] = [button_holder](UIElement::UIElementP node)
        {
            node->padding.x = 10;
            node->padding.y = 5;
            button_holder->image.setColor({125,125, 125, 255});
        };

        button_holder->setTexture(*m_context.textures->get("ShopItemFrame"));
        button_holder->padding = {10, 5};
        button_holder->margin.x = 10;
        button_holder->id = icon_textures.at(i);
        button_holder->layout = Layout::Y;
        button_holder->content_align_x = Alignement::CenterX;
        button_holder->sizing = Sizing::SCALE_TO_FIT;

        auto icon = std::make_shared<SpriteUIELement>();
        icon->setTexture(*m_context.textures->get(icon_textures.at(i)));
        icon->dimensions = {Pixels{80}, Pixels{80}};
        icon->align = Alignement::CenterX;

        auto text = std::make_shared<TextUIELement>(*m_context.font, "100");
        text->dimensions = {Pixels{40}, Pixels{40}};
        text->margin.x = 5;
        text->id = "amountText";

        auto control_bar = std::make_shared<UIElement>();
        control_bar->id = "control";
        control_bar->sizing = Sizing::SCALE_TO_FIT;
        control_bar->margin.y = 20;

        auto buy_button = std::make_shared<SpriteUIELement>();
        buy_button->setTexture(*m_context.textures->get("Forward"));
        buy_button->dimensions = {Pixels{40}, Pixels{40}};
        buy_button->id = "buyButton";

        auto sell_button = std::make_shared<SpriteUIELement>(*buy_button);
        sell_button->setTexture(*m_context.textures->get("Back"));
        sell_button->id = "sellButton";

        control_bar->addChildren(sell_button,text, buy_button);
        button_holder->addChildren(icon, control_bar);
        grid_holder->addChildren(button_holder);

        //! create a corresponding shop item
        m_items[icon_textures.at(i)].ui_node = button_holder.get();
    }

    auto shop_background = std::make_shared<UIElement>();
    shop_background->bounding_box = {0,0,1000,1000};
    shop_background->sizing = Sizing::SCALE_TO_FIT;
    shop_background->layout = Layout::Y;
    shop_background->content_align_x = Alignement::Center;
    shop_background->addChildren(shop_header, grid_holder);

    document.root->layout = Layout::Y;
    document.root->content_align_x = Alignement::CenterX;
    document.root->addChildren(shop_background);
    document.root->padding = {50, 50};

    initButtons();
}

template <class PropertyType>
void changePlayerProperty(PropertyType &property, PropertyType delta, UIElement::UIElementP ui_holder)
{
    TextUIELement *text_el = dynamic_cast<TextUIELement *>(ui_holder->getElementById("amountText"));
    if (text_el)
    {
        property += delta;
        std::stringstream ss;
        ss << std::setprecision(2) << property;
        text_el->m_text.setText(ss.str());
    }
}

void ShopState::initButtons()
{

    auto exit = document.root->getElementById("closeButton");
    exit->event_callbacks[UIEvent::CLICK] = [this, exit](auto node) {
        this->requestStackPop();
    };

    auto change_player_property = [this](UIElement *holder, auto &property, auto delta)
    {
        TextUIELement *text_el = dynamic_cast<TextUIELement *>(holder->getElementById("amountText"));
        if (text_el)
        {
            property += delta;
            text_el->m_text.setText(std::to_string((int)property));
        }
        TextUIELement *money_text = dynamic_cast<TextUIELement *>(document.getElementById("moneyText"));
        if (money_text)
        {
            money_text->m_text.setText(std::to_string(p_player->m_money) + " $");
        }
    };

    auto fuel_holder = document.root->getElementById("Fuel");
    fuel_holder->getElementById("buyButton")->event_callbacks[UIEvent::CLICK] =
        [this, fuel_holder, change_player_property](UIElement::UIElementP node_p)
    {
        if (m_items.at("Fuel").buy(p_player->m_money))
        {
            change_player_property(fuel_holder, p_player->m_max_fuel, 1);
        }
    };

    fuel_holder->getElementById("sellButton")->event_callbacks[UIEvent::CLICK] = [this, fuel_holder, change_player_property](UIElement::UIElementP node_p)
    {
        if (m_items.at("Fuel").sell(p_player->m_money))
        {
            change_player_property(fuel_holder, p_player->m_max_fuel, -1);
        }
    };

    auto speed_holder = document.root->getElementById("Arrow");
    speed_holder->getElementById("buyButton")->event_callbacks[UIEvent::CLICK] = [this, speed_holder, change_player_property](UIElement::UIElementP node_p)
    {
        if (m_items.at("Arrow").buy(p_player->m_money))
        {
            change_player_property(speed_holder, p_player->acceleration, +0.1);
        }
    };
    speed_holder->getElementById("sellButton")->event_callbacks[UIEvent::CLICK] = [this, speed_holder, change_player_property](UIElement::UIElementP node_p)
    {
        if (m_items.at("Arrow").sell(p_player->m_money))
        {
            change_player_property(speed_holder, p_player->acceleration, -0.1);
        }
    };
}

ShopState::~ShopState() {}

void ShopState::update(float dt)
{
    auto &window = *m_context.window;
}

void ShopState::handleEvent(const SDL_Event &event)
{
    auto &window = *m_context.window;
    auto old_view = window.m_view;
    window.m_view = window.getDefaultView();

    if (event.type == SDL_KEYUP)
    {
        if (event.key.keysym.sym == SDLK_ESCAPE)
        {
            m_stack->popState();
        }
    }
    if (event.type == SDL_MOUSEBUTTONUP)
    {
        auto mouse_position = window.getMouseInScreen();

        document.handleEvent(UIEvent::CLICK);
    }
    if (event.type == SDL_MOUSEMOTION)
    {
        document.handleEvent(UIEvent::MOUSE_ENETERED);
    }

    window.m_view = old_view;
}

void ShopState::draw()
{
    auto &window = *m_context.window;
    auto old_view = window.m_view;
    window.m_view = window.getDefaultView();
    window.m_view.setSize(window.m_view.getSize().x, -window.m_view.getSize().y);

    // window.m_view.m_view_matrix[1][1]
    window.clear({0, 0, 0, 0});
    // document.getElementById("buttonHolder")->bounding_box.width = window.getMouseInScreen().x - document.getElementById("buttonHolder")->bounding_box.pos_x;
    document.drawUI();

    auto mouse_pos = window.getMouseInScreen();
    Text pica(std::to_string(mouse_pos.x) + "   " + std::to_string(mouse_pos.y));
    pica.setFont(m_context.font);
    pica.setScale(0.4, -0.4);
    pica.centerAround(mouse_pos);
    window.drawText(pica);

    window.drawAll();
    window.m_view = old_view;
}
