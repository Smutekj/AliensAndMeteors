#pragma once

#include <chrono>
#include <vector>
#include <numeric>
#include <unordered_map>
#include <memory>

#include "Window.h"
#include "DrawLayer.h"

#include "Menu/StateStack.h"
#include "Menu/ScoreBoard.h"
#include "Utils/Statistics.h"
#include "Commands.h"

void gameLoop(void *mainLoopArg);

class Application
{

public:
    Application(int widht, int height);
    void run();
    void iterate();

private:
    void registerStates();

public:
    Window m_window;
    Renderer m_window_canvas;

    friend void gameLoop(void *);

private:
    std::unique_ptr<StateStack> m_state_stack; //! state stack for menu navigation

    TextureHolder m_textures;
    float m_dt;             //! time step
    KeyBindings m_bindings; //! defines key->command bindings
    ScoreBoard m_score;     //! keeps score;
    std::shared_ptr<Font> m_font;

    Statistics m_avg_frame_time;
};