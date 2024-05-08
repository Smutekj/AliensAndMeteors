#include <iostream>
#include <memory>
#include "Game.h"
#include "UI.h"
#include "Geometry.h"
#include "Utils/RandomTools.h"
#include "Utils/Grid.h"

#include "imgui.h"
#include "imgui-SFML.h"

constexpr float FRAME_RATE = 60;
#define RES_X 1920
#define RES_Y 1080

// #include "Systems/GraphicsSystem.h"
// #include "Systems/VisionSystem.h"

// #include "Graphics/SceneLayer.h"

// #include "imgui.h"
// #include "imgui_impl_glfw.h"
// #include "imgui_impl_opengl3.h"

struct Circle
{
    sf::Vector2f r = {0, 0}; //! center
    float radius_sq = 1;
};

inline Circle randomCircle(const float r_max)
{
    return {randomPosInBox(), randf(0, r_max * r_max)};
}
inline void generateRandomPositionsInCircles(const float density,
                                             sf::Vector2f box_size,
                                             int n_positions,
                                             Game &game)
{
    const int n_to_insert = n_positions;
    while (n_positions > 0)
    {
        const auto circle = randomCircle(100);
        int n_in_circle = std::min({static_cast<int>(std::floor(M_PI * circle.radius_sq * density)), n_positions});
        const auto range_x = 2 * std::sqrt(circle.radius_sq);
        const sf::Vector2f r0 = {circle.r.x - range_x / 2.0f, circle.r.y - range_x / 2.0f};
        sf::Vector2f r = r0;
        int player_ind = 0;
        const float dx = std::sqrt(1.f / density);
        while (n_in_circle > 0 and r.y < r0.y + range_x)
        {
            n_positions > n_to_insert / 3 ? player_ind = 0 : player_ind = 1;
            if (dist2(r, circle.r) < circle.radius_sq)
            {
                game.addEnemy(r);
                n_positions--;
                n_in_circle--;
            }
            r.x += dx;
            if (r.x > (r0.x + range_x) || r.x > Geometry::BOX[0])
            {
                r.x -= range_x;
                r.y += dx;
            }
        }
    }
}

inline void generateRandomGroups(sf::Vector2f box_size,
                                 int n_groups,
                                 Game &game)
{
    const int n_to_insert = n_groups;
    for (int i = 0; i < n_groups; ++i)
    {
        auto center = randomPosInBox();
        game.addGroupOfEnemies(center, 10.f, 10);
    }
}


#include <variant>

int main()
{

    const auto N_CELLS = Geometry::N_CELLS;
    const auto BOX = Geometry::BOX;
    sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
    sf::RenderWindow window(sf::VideoMode(800, 600, desktop.bitsPerPixel), "My window", sf::Style::Fullscreen); // sf::Style::Fullscreen);
    window.setFramerateLimit(60);
    window.setActive(true);
    ImGui::SFML::Init(window);

    sf::Vector2f box_size = {static_cast<float>(BOX[0]), static_cast<float>(BOX[1])};
    sf::Vector2i n_cells = {N_CELLS[0], N_CELLS[1]};
    sf::Vector2f cell_size = {static_cast<float>(BOX[0] / n_cells.x), static_cast<float>(BOX[1] / n_cells.y)};

    auto view = window.getView();
    view.setCenter(box_size / 2.f);
    view.setSize({box_size.x, box_size.y});

    //! time and clocks stuff
    const auto fps = 60;                //! fixed fps
    const size_t fps_calc_period = 369; //! how often we update fps
    const auto time_of_frame = 1. / fps;
    const auto maximum_frame_time = time_of_frame * 1e6;

    //! create game world and some helper stuff
    Game game(n_cells, box_size, window);
    UI ui(window, game);
    generateRandomPositionsInCircles(0.02f, box_size, 0, game);
    // generateRandomGroups(box_size, 50, game);


    int frame_i = 0;
    unsigned long long time_of_n_frames = 0;
    float real_fps = fps;

    double last_update_time = 0;

    sf::RectangleShape background;
    background.setSize(box_size);
    sf::Texture background_texture;
    background_texture.loadFromFile("../Resources/Starbasesnow.png");
    background.setTexture(&background_texture);
    background_texture.setRepeated(true);
    background_texture.setSmooth(true);
    

    sf::Clock clock;
    sf::Clock deltaClock;

    while (game.game_is_running)
    {
        window.clear(sf::Color::Black);
        window.draw(background);

        game.parseInput(window);
        game.update(time_of_frame, window);

        ImGui::SFML::Update(window, deltaClock.restart());
        ui.draw(window);
        game.draw(window);

        sf::Time elapsed1 = clock.getElapsedTime();

        ImGui::SFML::Render(window);
        window.display();

        std::cout << "frame took: " << elapsed1.asMilliseconds() << std::endl;

        clock.restart();
    }

    // ! cleanup
    // ImGui_ImplOpenGL3_Shutdown();
    // ImGui_ImplGlfw_Shutdown();
    // ImGui::DestroyContext();

    // glfwDestroyWindow(window.handle);
    // glfwTerminate();

    return 0;
}


// std::vector<sf::RectangleShape> splitBackground(sf::Vector2f box_size, sf::View view){

//     auto view_center = view.getCenter();
//     auto view_size = view.getSize();

//     if(view_center.x + view_size.x/2.f > box_size.x){

//     }

// }