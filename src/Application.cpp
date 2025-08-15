

#include "Application.h"

#include "Menu/MenuState.h"
#include "Menu/GameState.h"
#include "Menu/PauseState.h"
#include "PostEffects.h"
#include "DrawLayer.h"

#include <IncludesGl.h>
#include <Utils/RandomTools.h>

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

void Application::iterate()
{
    m_state_stack->update(m_dt);

    //! poll and events let state stack handle them
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        // ImGui_ImplSDL2_ProcessEvent(&event);

        
        m_state_stack->handleEvent(event);
        if (event.type == SDL_WINDOWEVENT)
        {
            if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
            {
                m_window.setSize(event.window.data1, event.window.data2);
            }
        }
    }

    m_state_stack->draw();
    // m_ui->draw(m_window);

    Shader::m_time += m_dt;
}

static std::size_t s_frame_count = 0;

void inline gameLoop(void *mainLoopArg)
{

#ifdef __EMSCRIPTEN__
    auto tic = emscripten_get_now();
#else
    auto tic = std::chrono::high_resolution_clock::now();
#endif

    Application *p_app = (Application *)mainLoopArg;

    p_app->iterate();

    SDL_GL_SwapWindow(p_app->m_window.getHandle()); // Swap front/back framebuffers

#ifdef __EMSCRIPTEN__
    double dt = emscripten_get_now() - tic;
#else
    double dt = std::chrono::duration_cast<std::chrono::microseconds>(
                    std::chrono::high_resolution_clock::now() - tic)
                    .count() /
                1e3;
#endif

    p_app->m_avg_frame_time.addNumber(dt);
    s_frame_count++;
    if (s_frame_count > 200)
    {
        std::cout << "frame time " <<  dt << " ms" << std::endl;
        std::cout << "avg frame time: " << p_app->m_avg_frame_time.getAverage() << " ms" << std::endl;
        std::cout << "max frame time: " << p_app->m_avg_frame_time.getMax() << " ms" << std::endl;
        p_app->m_avg_frame_time.averaging_interval = 200;
        p_app->m_avg_frame_time.reset();
        s_frame_count = 0;
    }

    double dt2 = dt;
    if (dt < 33)
    {
#ifdef __EMSCRIPTEN__
        emscripten_sleep(33 - dt);
        dt2 = emscripten_get_now() - tic;
#else
        // SDL_Delay(33. - dt);
        dt2 = std::chrono::duration_cast<std::chrono::microseconds>(
                         std::chrono::high_resolution_clock::now() - tic)
                         .count() /
                     1e3;
#endif
    }

    p_app->m_dt = std::min(0.03, dt2 / 1000.); //! limit the timestep for debugging
}

#define TO_STRING(x) #x

#ifndef RESOURCES_DIR
static_assert(false)
#endif

    Application::Application(int width, int height)
    : m_window(width, height), m_window_canvas(m_window)
{
    std::filesystem::path font_path = {RESOURCES_DIR};
    font_path.append("Fonts/DigiGraphics.ttf");
    std::cout << std::filesystem::current_path() << "\n";

    m_font = std::make_shared<Font>(font_path);
    State::Context context(m_window_canvas, m_window, m_textures, m_bindings, *m_font, m_score);
    m_state_stack = std::make_unique<StateStack>(context);

    registerStates();
    m_state_stack->pushState(States::ID::Menu);

    //     //! set view and add it to renderers
    m_window_canvas.m_view.setSize(m_window_canvas.getTargetSize());
    m_window_canvas.m_view.setCenter(m_window_canvas.getTargetSize() / 2);

    // m_window_canvas.addShader("circle", "basicinstanced.vert", "circle.frag");
    m_window_canvas.setShadersPath(std::string(RESOURCES_DIR) + "/Shaders/");
    m_window_canvas.addShader("Shiny", "basicinstanced.vert", "shiny.frag");
    m_window_canvas.addShader("Instanced", "basicinstanced.vert", "texture.frag");
    m_window_canvas.addShader("LastPass", "basicinstanced.vert", "lastPass.frag");
    m_window_canvas.addShader("VertexArrayDefault", "basictex.vert", "fullpass.frag");
    m_window_canvas.addShader("Text", "basicinstanced.vert", "textBorder.frag");

    m_textures.setBaseDirectory(std::string(RESOURCES_DIR) + "/Textures/");
    m_textures.add("TestArrow", "Arrow.png");
    m_textures.add("Fuel", "fuel.png");
    m_textures.add("Heart", "Heart.png");
    m_textures.add("ShopItemFrame", "UIShopFrame.png");
    m_textures.add("HeaderFrame", "UIHeaderFrame.png");
    m_textures.add("Arrow", "arrow.png");
    m_textures.add("Coin", "coin.png");
    m_textures.add("Back", "UIBack.png");
    m_textures.add("Forward", "UIForward.png");
    m_textures.add("Ok", "UIOk.png");
    m_textures.add("Close", "UIClose.png");
    m_textures.add("Menu", "UIMenu.png");
    m_textures.add("Dot", "UIDot.png");
        m_ui = std::make_unique<ToolBoxUI>(m_window, m_textures);


    

}


void Application::registerStates()
{
    m_state_stack->registerState<EndScreenState>(States::ID::Exit);
    m_state_stack->registerState<MenuState>(States::ID::Menu);
    m_state_stack->registerState<GameState>(States::ID::Game);
    m_state_stack->registerState<PauseState>(States::ID::Pause);
    m_state_stack->registerState<ScoreBoardState>(States::ID::Score);
    m_state_stack->registerState<PlayerDiedState>(States::ID::Player_Died);
    m_state_stack->registerState<SettingsState>(States::ID::Settings);
    m_state_stack->registerState<KeyBindingState>(States::ID::KeyBindings);
    m_state_stack->registerState<GraphicsState>(States::ID::Graphics);
    m_state_stack->registerState<ShopState>(States::ID::Shop);
}