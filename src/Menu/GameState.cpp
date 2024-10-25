#include "GameState.h"
#include "MenuState.h"
#include "StateStack.h"
#include "ScoreBoard.h"

#include "../Game.h"
#include <Texture.h>


GameState::GameState(StateStack &stack, State::Context context)
    : State(stack, context)
{
    mp_game = std::make_unique<Game>(*context.window, *context.bindings);    
    std::cout << "Game Created!" << std::endl;
}

GameState::~GameState() {}

void GameState::update(float dt)
{
    auto &window = *m_context.window;

    mp_game->update(dt, window);

    if(mp_game->getState() == Game::GameState::PLAYER_DIED)
    {
        m_context.score->setCurrentScore(mp_game->getScore());
        m_stack->popState();
        m_stack->pushState(States::ID::Player_Died);
    }

}

void GameState::handleEvent(const SDL_Event &event)
{
    auto &window = *m_context.window;

    if (event.type == SDL_KEYUP)
    {
        if (event.key.keysym.sym  == SDLK_ESCAPE)
        {
            m_stack->pushState(States::ID::Pause);
        }
    }
    mp_game->handleEvent(event);
}

void GameState::draw()
{
    auto& window = *m_context.window;
    window.clear({0,0,0,0});
    mp_game->draw(window);
}
