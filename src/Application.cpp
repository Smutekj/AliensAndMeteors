#include "Application.h"

#include "Menu/MenuState.h"
#include "Menu/GameState.h"
#include "Menu/PauseState.h"

Application::Application(float fps)

{
    sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
    m_window = std::make_unique<sf::RenderWindow>(desktop, "My window"); // sf::Style::Fullscreen);
    m_window->setFramerateLimit(fps);
    m_window->setActive(true);
    m_dt = 1. / fps;

    m_textures.load(Textures::ID::BackGround, "../Resources/Starbasesnow.png");

    m_font.loadFromFile("../Resources/DigiGraphics.ttf");
    State::Context context(*m_window, m_textures, m_bindings, m_font, m_score);
    m_state_stack = std::make_unique<StateStack>(context);

    registerStates();
    m_state_stack->pushState(States::ID::Menu);

}

void Application::run()
{
    while (m_window->isOpen())
    {
        m_window->clear(sf::Color::Black);
        sf::Event event;
        while(m_window->pollEvent(event)){
            m_state_stack->handleEvent(event);
        }
        m_state_stack->update(m_dt);
        m_state_stack->draw();
        m_window->display();
    }
}

void Application::registerStates()
{
    m_state_stack->registerState<EndScreenState>(States::Exit);
    m_state_stack->registerState<MenuState>(States::Menu);
    m_state_stack->registerState<GameState>(States::Game);
    m_state_stack->registerState<PauseState>(States::Pause);
    m_state_stack->registerState<ScoreBoardState>(States::Score);
    m_state_stack->registerState<PlayerDiedState>(States::Player_Died);
    m_state_stack->registerState<SettingsState>(States::Settings);
}