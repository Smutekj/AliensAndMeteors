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

    auto grid_holder = std::make_shared<UIElement>();
    grid_holder->layout = Layout::Grid;
    grid_holder->bounding_box.width = 600;
    grid_holder->bounding_box.height = 600;

    std::vector<std::string> icon_textures = {"Fuel", "Heart", "Arrow", "Coin"};
    for (int i = 0; i < 4; ++i)
    {
        auto button_holder = std::make_shared<SpriteUIELement>();
        button_holder->id = "buttonHolder";
        button_holder->event_callbacks[UIEvent::MOUSE_ENETERED] = [](UIElement::UIElementP node)
        {
            node->padding.x = 32;
            node->padding.y = 32;
        };
        button_holder->event_callbacks[UIEvent::MOUSE_LEFT] = [](UIElement::UIElementP node)
        {
            node->padding.x = 30;
            node->padding.y = 30;
        };

        button_holder->setTexture(*m_context.textures->get("ShopItemFrame"));
        button_holder->bounding_box = {0, 0, 150, 200};
        button_holder->padding = {30, 10};
        button_holder->margin.x = 10;
        button_holder->id = icon_textures.at(i);
        button_holder->layout = Layout::Y;
        button_holder->sizing = Sizing::SCALE_TO_FIT;

        auto icon = std::make_shared<SpriteUIELement>();
        icon->setTexture(*m_context.textures->get(icon_textures.at(i)));
        icon->bounding_box = {0, 0, 80, 80};

        auto text = std::make_shared<TextUIELement>(*m_context.font, "100");
        text->bounding_box = {0, 0, 60, 40};
        text->id = "amountText";

        auto control_bar = std::make_shared<UIElement>();
        control_bar->id = "control";
        control_bar->bounding_box = {0, 0, 80, 60};
        control_bar->sizing = Sizing::SCALE_TO_FIT;
        control_bar->margin.y = 30;

        auto buy_button = std::make_shared<SpriteUIELement>();
        buy_button->setTexture(*m_context.textures->get("Forward"));
        buy_button->bounding_box = {0, 0, 40, 40};
        buy_button->id = "buyButton";

        auto sell_button = std::make_shared<SpriteUIELement>(*buy_button);
        sell_button->setTexture(*m_context.textures->get("Back"));
        sell_button->id = "sellButton";

        control_bar->addChildren(sell_button, buy_button);
        button_holder->addChildren(icon, text, control_bar);
        grid_holder->addChildren(button_holder);
    }

    document.root->layout = Layout::Y;
    document.root->addChildren(grid_holder);
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

    
    auto change_player_property = [this](UIElement *holder, auto &property, auto delta)
    {
        TextUIELement *text_el = dynamic_cast<TextUIELement *>(holder->getElementById("amountText"));
        if (text_el)
        {
            property += delta;
            text_el->m_text.setText(std::to_string((int)property));
        }
    };
    
    auto fuel_holder = document.root->getElementById("Fuel");
    fuel_holder->getElementById("buyButton")->event_callbacks[UIEvent::CLICK] = [fuel_holder, change_player_property](UIElement::UIElementP node_p)
    {
        change_player_property(fuel_holder, p_player->m_max_fuel, 1);
    };

    fuel_holder->getElementById("sellButton")->event_callbacks[UIEvent::CLICK] = [fuel_holder, change_player_property](UIElement::UIElementP node_p)
    {
        change_player_property(fuel_holder, p_player->m_max_fuel, -1);
    };

    auto speed_holder = document.root->getElementById("Arrow");
    speed_holder->getElementById("buyButton")->event_callbacks[UIEvent::CLICK] = [speed_holder, change_player_property](UIElement::UIElementP node_p)
    {
        change_player_property(speed_holder, p_player->acceleration, +0.1);
    };
    speed_holder->getElementById("buyButton")->event_callbacks[UIEvent::CLICK] = [speed_holder, change_player_property](UIElement::UIElementP node_p)
    {
        change_player_property(speed_holder, p_player->acceleration, -0.1);
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
