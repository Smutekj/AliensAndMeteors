

#include "Application.h"

#include "Menu/MenuState.h"
#include "Menu/GameState.h"
#include "Menu/PauseState.h"
#include "PostEffects.h"
#include "DrawLayer.h"

#include <IncludesGl.h>
#include <Utils/RandomTools.h>
#include <Utils/IO.h>

#include <time.h>
#include <chrono>
#include <queue>
#include <filesystem>

void Application::run()
{

#ifdef __EMSCRIPTEN__
    int fps = 0; // Use browser's requestAnimationFrame
    emscripten_set_main_loop_arg(gameLoop, (void *)this, fps, true);
#else
    while (!m_window.shouldClose())
        gameLoop((void *)this);
#endif
}

void Application::handleInput()
{

    m_state_stack->update(m_dt);
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        m_state_stack->handleEvent(event);
        if (event.type == SDL_WINDOWEVENT)
        {
            if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
            {
                m_window.setSize(event.window.data1, event.window.data2);
                // m_window_canvas.m_view = m_window_canvas;
                // m_window_canvas.m_view
            }
        }
    }
    m_state_stack->draw();
    Shader::m_time += m_dt;
}

static std::size_t s_frame_count = 0;
clock_t tic = 0;

void inline gameLoop(void *mainLoopArg)
{
    Application *p_app = (Application *)mainLoopArg;

    p_app->handleInput();
    double dt = (double)(SDL_GetPerformanceCounter() - tic) / SDL_GetPerformanceFrequency();
    tic = SDL_GetPerformanceCounter();
    p_app->m_dt = dt;

    // Swap front/back framebuffers
    SDL_GL_SwapWindow(p_app->m_window.getHandle());

    if (s_frame_count++ > 300)
    {
        std::cout << "avg frame time: " << p_app->m_avg_frame_time.getAverage() << " ms" << std::endl;
        s_frame_count = 0;
    }
    // auto toc = clock();
    // if (dt < 16.6666)
    // {
    //     SDL_Delay(16.6666 - dt);
    // }
    p_app->m_avg_frame_time.addNumber((double)(clock() - tic) / CLOCKS_PER_SEC * 1000.);
}

Application::Application(int width, int height)
    : m_window(width, height), m_window_canvas(m_window)
{
    m_dt = 0.0166667f;

    m_textures.add("background", "../Resources/Textures/background.png");

    m_font = std::make_shared<Font>("arial.ttf");
    State::Context context(m_window_canvas, m_window, m_textures, m_bindings, *m_font, m_score);
    m_state_stack = std::make_unique<StateStack>(context);

    registerStates();
    m_state_stack->pushState(States::ID::Menu);

    //     //! set view and add it to renderers
    m_window_canvas.m_view.setSize(m_window.getSize());
    m_window_canvas.m_view.setCenter(m_window.getSize() / 2);

    // m_window_canvas.addShader("circle", "basicinstanced.vert", "circle.frag");
    m_window_canvas.addShader("Shiny", "basicinstanced.vert", "shiny.frag");
    m_window_canvas.addShader("Instanced", "basicinstanced.vert", "texture.frag");
    m_window_canvas.addShader("LastPass", "basicinstanced.vert", "lastPass.frag");
    m_window_canvas.addShader("VertexArrayDefault", "basictex.vert", "fullpass.frag");
    m_window_canvas.addShader("Text", "basicinstanced.vert", "textBorder.frag");
    //     m_ui = std::make_unique<UI>(m_window, m_textures, m_layers, m_window_renderer);
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