#ifndef BOIDS_UI_H
#define BOIDS_UI_H

#include <vector>
#include <memory>
#include <unordered_map>
#include <array>

#include "BoidSystem.h"
#include "SFML/Graphics.hpp"

class Game;
class VisionSystem;
class DebugInfo;
namespace sf
{
    class RenderWindow;
}

enum class UIWindowType
{
    PHYSICS = 0,
    PATHFINDING,
    DEBUG,
    SHADERS,
    GRAPHICS,
    UNIT_MAKER,
    WALLS,
    COUNT
};

constexpr int N_UI_WINDOWS = static_cast<int>(UIWindowType::COUNT);

class UIWindow
{

protected:
    std::string name;
    bool is_active = false;
    std::vector<std::unique_ptr<UIWindow>> children;

    std::array<void *, N_UI_WINDOWS> controled_data;

public:
    virtual void draw() = 0;
    virtual ~UIWindow() = 0;

    UIWindow(std::string name);

    const std::string& getName()const{
        return name;
    } 
};

// struct PhysicsSystem;
// struct PhysicsSystem : System{
//     enum class Multiplier;
// };


class PhysicsWindow : public UIWindow
{

    float *gravity = nullptr;

    enum Data
    {
        GRAVITY = 0,
        MAX_SPEED,
    };

    std::unordered_map<BoidSystem::Multiplier, float>& force_multipliers;
    std::unordered_map<BoidSystem::Multiplier, std::pair<float, float>> mulitplier2slider_min_max;

    std::unordered_map<BoidSystem::Multiplier, float>& force_ranges;

public:
    PhysicsWindow(BoidSystem &ps);

    ~PhysicsWindow();

    virtual void draw() override;
};


class Shader;

struct ColorData{
    std::string uniform_name;
    sf::Color value = sf::Color::Green;
};

struct ValueData{
    std::string uniform_name;
    float value;
};

struct ShaderUIData{
    Shader* p_program;
    std::string filename;
    std::vector<ColorData> colors;
    std::vector<ValueData> values;
};

struct BuildingLayer;

class ShadersWindow : public UIWindow
{

    std::vector<ShaderUIData> shaders;
    enum Data
    {
        COLOR1,
        COLOR2,
    };

public:
    ShadersWindow();

    virtual ~ShadersWindow();

    virtual void draw() override;
};






class UI
{

    struct UIWindowData
    {
        std::unique_ptr<UIWindow> p_window;
        bool is_active = false;
        std::string name;
    };

    friend UIWindowType;

    sf::Vector2f mouse_coords_on_click_;

    std::unordered_map<UIWindowType, std::unique_ptr<UIWindow>> windows;
    // std::unordered_map<UIWindowType, >
    std::unordered_map<UIWindowType, bool> is_active;
    std::unordered_map<UIWindowType, std::string> names;
    std::unordered_map<UIWindowType, UIWindowData> window_data;

    bool show_demo_window = true;

public:
    UI(sf::RenderWindow &window, Game &game);

    void showWindow();
    void draw(sf::RenderWindow &window);
};

#endif // BOIDS_UI_H
