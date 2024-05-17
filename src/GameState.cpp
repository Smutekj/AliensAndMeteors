#include "GameState.h"
#include "Game.h"
#include "StateStack.h"

#include "SFML/Graphics/Texture.hpp"

GameState::GameState(StateStack &stack, State::Context context)
    : State(stack, context)
{
    m_is_final_state = true;
    mp_game = std::make_unique<Game>(*m_context.window, bindings);


    auto& background_texture = context.textures->get(Textures::ID::BackGround);
    background_texture.setRepeated(true);
    background_texture.setSmooth(true);
}

GameState::~GameState() {}

void GameState::update(float dt)
{
    auto &window = *getContext().window;

    mp_game->update(dt, window);

    if(mp_game->getState() == Game::GameState::PLAYER_DIED)
    {
        requestStackPop();
        //! requestStackPush(States::ID::Score)
    }

}

void GameState::handleEvent(const sf::Event &event)
{
    auto &window = *getContext().window;

    if (event.type == sf::Event::KeyReleased)
    {
        if (event.key.code == sf::Keyboard::Escape)
        {
            m_stack->pushState(States::ID::Pause);
        }
    }
    mp_game->handleEvent(event);
}

void GameState::draw()
{
    auto& background_texture = m_context.textures->get(Textures::ID::BackGround);
    auto& window = *m_context.window;
    sf::RectangleShape background;


    background.setSize({Geometry::BOX[0], Geometry::BOX[1]});
    background.setTexture(&background_texture);
    background.setTextureRect({0, 0, 2 * (int)background_texture.getSize().x, 2 * (int)background_texture.getSize().y});
    window.draw(background);

    mp_game->draw(window);
}
