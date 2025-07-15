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
    button_holder->setTexture(*m_context.textures->get("ShopItemFrame"));
    button_holder->bounding_box = {0, 0, 140, 200};
    button_holder->padding = {30, 10};
    button_holder->layout = Layout::Y;
    auto fuel_text = std::make_shared<TextUIELement>(*m_context.font, "Fuel");
    fuel_text->bounding_box = {0, 0, 80, 40};
    fuel_text->margin = {0, 0};
    fuel_text->id = "TEXT";


    auto heart_text = std::make_shared<TextUIELement>(*m_context.font, "Health");
    auto speed_text = std::make_shared<TextUIELement>(*m_context.font, "Speed");
    heart_text->bounding_box = {0, 0, 80, 40};
    speed_text->bounding_box = {0, 0, 80, 40};

    auto fuel_button = std::make_shared<SpriteUIELement>();
    auto heart_button = std::make_shared<SpriteUIELement>();
    auto speed_button = std::make_shared<SpriteUIELement>();
    auto money_button = std::make_shared<SpriteUIELement>();
    fuel_button->setTexture(*m_context.textures->get("Fuel"));
    fuel_button->bounding_box = {0, 0, 80, 80};
    heart_button->setTexture(*m_context.textures->get("Heart"));
    heart_button->bounding_box = {0, 0, 80, 80};
    // speed_button->setTexture(*m_context.textures->get("Arrow"));
    // speed_button->bounding_box = {0,0, 80, 80};
    // money_button->setTexture(*m_context.textures->get("Coin"));
    // money_button->bounding_box = {0,0, 80, 80};
    auto control_bar = std::make_shared<UIElement>();
    control_bar->bounding_box = {0, 0, 140, 40};
    control_bar->size_style = SizeStyle::FIT_CHIDLREN;

    auto buy_button = std::make_shared<SpriteUIELement>();
    buy_button->setTexture(*m_context.textures->get("Coin"));
    buy_button->bounding_box = {0, 0, 30, 20};
    buy_button->id = "BUY";
    auto sell_button = std::make_shared<SpriteUIELement>();
    sell_button->setTexture(*m_context.textures->get("Coin"));
    sell_button->bounding_box = {0, 0,30, 20};
    sell_button->id = "SELL";
    fuel_text->bounding_box = {0, 0, 40, 20};
    // control_bar->margin.y = 30;
    buy_button->margin.x = 5;
    sell_button->margin.x = 5;
    fuel_text->margin.x = 5;

    control_bar->addChildren(buy_button, fuel_text, sell_button);
    control_bar->id = "HOLDER";
    button_holder->addChildren(control_bar);
    // speed_button->margin = {10, 0};
    button_holder->margin.x = 10;
    document.root->layout = Layout::Grid;
    document.root->bounding_box.width = 800;
    document.root->max_width = 800;
    document.root->addChildren(control_bar); //, heart_button, speed_button, money_button);
    // for (int i = 0; i < 1; ++i)
    // {
    //     auto button_holder2 = std::make_shared<SpriteUIELement>(*button_holder);
    //     document.root->addChildren(button_holder2); //, heart_button, speed_button, money_button);
    // }
    document.root->padding = {200, 200};

    int panning_x = 20;
    int panning_y = 15;

    int left_corner_x = 400;
    int left_corner_y = 800;

    int x = left_corner_x;
    int y = left_corner_y;

    //! set box positions
    for (int i = 0; i < m_ui_elements.size(); ++i)
    {
        auto &box = m_ui_elements.at(i).bounding_box;

        box.pos_x = x;
        box.pos_y = y;

        if (i % n_elements_per_row == n_elements_per_row - 1)
        {
            y -= (panning_y + box.height);
            x = left_corner_x;
        }
        else
        {
            x += (panning_x + box.width);
        }
    }
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

        for (auto &[box, text, sprite_name] : m_ui_elements)
        {
            if (box.contains(mouse_position))
            {
                std::cout << "Mouse is Inside: " << text << "\n";
            }
        }
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
    document.drawUI();
    Text pica("Penis");
    pica.setFont(m_context.font);
    pica.setScale(1, -1);
    pica.centerAround(window.getMouseInScreen());
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
