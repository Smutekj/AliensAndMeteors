#include "GameState.h"
#include "MenuState.h"
#include "StateStack.h"
#include "ScoreBoard.h"

#include "../Game.h"
#include <Texture.h>

GameState::GameState(StateStack &stack, State::Context context)
    : State(stack, context)
{
    try
    {
        mp_game = std::make_shared<Game>(*context.window, *context.bindings);
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

    Rect<int> frame_box = {0, 0, 150, 180};

    m_ui_elements.push_back({frame_box, "Fuel", "Fuel"});
    m_ui_elements.push_back({frame_box, "Health", "Heart"});
    m_ui_elements.push_back({frame_box, "Speed", "Arrow"});
    m_ui_elements.push_back({frame_box, "Money", "Coin"});

    auto button_holder = std::make_shared<SpriteUIELement>();
    button_holder->id = "buttonHolder";
    button_holder->event_callbacks[UIEvent::MOUSE_ENETERED] = [](UIElement::UIElementP node)
    {
        node->padding.x = 39;
    };
    button_holder->event_callbacks[UIEvent::MOUSE_LEFT] = [](UIElement::UIElementP node)
    {
        node->padding.x = 30;
    };

    button_holder->setTexture(*m_context.textures->get("ShopItemFrame"));
    button_holder->bounding_box = {0, 0, 150, 200};
    button_holder->padding = {30, 10};
    button_holder->margin.x = 10;
    button_holder->layout = Layout::Y;
    button_holder->sizing = Sizing::SCALE_TO_FIT;

    auto fuel_text = std::make_shared<TextUIELement>(*m_context.font, "Fuel");
    fuel_text->bounding_box = {0, 0, 100, 40};
    fuel_text->margin = {0, 0};
    fuel_text->padding = {5, 5};
    fuel_text->margin = {10, 0};
    auto heart_text = std::make_shared<TextUIELement>(*fuel_text);
    auto speed_text = std::make_shared<TextUIELement>(*fuel_text);
    auto money_text = std::make_shared<TextUIELement>(*fuel_text);
    heart_text->m_text.setText("Health");
    speed_text->m_text.setText("Speed");
    money_text->m_text.setText("Money");

    auto fuel_button = std::make_shared<SpriteUIELement>();
    auto heart_button = std::make_shared<SpriteUIELement>();
    auto speed_button = std::make_shared<SpriteUIELement>();
    auto money_button = std::make_shared<SpriteUIELement>();
    fuel_button->setTexture(*m_context.textures->get("Fuel"));
    fuel_button->bounding_box = {0, 0, 80, 80};
    heart_button->setTexture(*m_context.textures->get("Heart"));
    heart_button->bounding_box = {0, 0, 80, 80};
    speed_button->setTexture(*m_context.textures->get("Arrow"));
    speed_button->bounding_box = {0, 0, 80, 80};
    money_button->setTexture(*m_context.textures->get("Coin"));
    money_button->bounding_box = {0, 0, 80, 80};

    auto control_bar = std::make_shared<UIElement>();
    control_bar->bounding_box = {0, 0, 80, 60};
    control_bar->sizing = Sizing::SCALE_TO_FIT;
    auto buy_button = std::make_shared<SpriteUIELement>();
    buy_button->setTexture(*m_context.textures->get("Forward"));
    buy_button->bounding_box = {0, 0, 40, 40};
    auto sell_button = std::make_shared<SpriteUIELement>();
    sell_button->setTexture(*m_context.textures->get("Back"));
    sell_button->bounding_box = {0, 0, 40, 40};
    control_bar->margin.y = 30;

    auto button_holder2 = std::make_shared<SpriteUIELement>(*button_holder);
    auto button_holder3 = std::make_shared<SpriteUIELement>(*button_holder);
    auto button_holder4 = std::make_shared<SpriteUIELement>(*button_holder);

    auto control_bar2 = std::make_shared<UIElement>(*control_bar);
    auto control_bar3 = std::make_shared<UIElement>(*control_bar);
    auto control_bar4 = std::make_shared<UIElement>(*control_bar);

    control_bar->addChildren(sell_button, fuel_text, buy_button);
    control_bar2->addChildren(sell_button, heart_text, buy_button);
    control_bar3->addChildren(sell_button, speed_text, buy_button);
    control_bar4->addChildren(sell_button, money_text, buy_button);

    button_holder->addChildren(fuel_button, control_bar);
    button_holder2->addChildren(heart_button, control_bar2);
    button_holder3->addChildren(speed_button, control_bar3);
    button_holder4->addChildren(money_button, control_bar4);

    auto grid_holder = std::make_shared<UIElement>();
    grid_holder->layout = Layout::Grid;
    grid_holder->bounding_box.width = 600;
    grid_holder->bounding_box.height = 600;
    grid_holder->max_width = 800;
    grid_holder->addChildren(button_holder, button_holder2, button_holder3, button_holder4);

    document.root->layout = Layout::Y;
    // document.root->sizing = Sizing::SCALE_TO_FIT;
    document.root->addChildren(grid_holder);
    // for(int i = 0; i < 10; ++i)
    // {
    //     auto button_holder2 = std::make_shared<SpriteUIELement>(*button_holder);
    //     document.root->addChildren(button_holder2);//, heart_button, speed_button, money_button);
    // }
    document.root->padding = {50, 50};
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
    pica.setScale(0.5, -0.5);
    pica.centerAround(mouse_pos);
    window.drawText(pica);

    // window.drawCricleBatched(window.getMouseInScreen(), 20, {1,0,0,1});

    // Sprite frame(*m_context.textures->get("ShopItemFrame"));
    // Text frame_text;
    // frame_text.setFont(m_context.font);
    // for(auto& [box, text, sprite_name] : m_ui_elements)
    // {
    //     utils::Vector2f el_center = {box.pos_x + box.width/2., box.pos_y + box.height/2.};
    //     frame.setPosition(el_center);
    //     frame.setTexture(*m_context.textures->get(sprite_name));
    //     frame.setScale(box.width/2, -box.height/2);
    //     window.drawSprite(frame);

    //     frame_text.setText(text);
    //     frame_text.centerAround({el_center.x, el_center.y - 20});
    //     window.drawText(frame_text);
    // }

    window.drawAll();
    window.m_view = old_view;
}
