#include "ToolBoxUI.h"

#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl2.h>

#include <unordered_map>

#include <Window.h>
#include <Texture.h>

#include "GameWorld.h"

#include "nlohmann/json.hpp"

ToolBoxUI::ToolBoxUI(Window &window, TextureHolder &textures)
    : m_sprite_pixels(400, 300, TextureOptions{.internal_format = TextureFormat::RGBA, .data_type = TextureDataTypes::UByte}),
      m_sprite_canvas(m_sprite_pixels)
{

        m_sprite_canvas.m_view.setCenter(m_sprite_pixels.getSize() / 2.f);
        m_sprite_canvas.m_view.setSize(utils::Vector2f{m_sprite_pixels.getSize().x, -m_sprite_pixels.getSize().y});

        // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
        // GL ES 2.0 + GLSL 100 (WebGL 1.0)
        const char *glsl_version = "#version 100";
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(IMGUI_IMPL_OPENGL_ES3)
        // GL ES 3.0 + GLSL 300 es (WebGL 2.0)
        const char *glsl_version = "#version 300 es";
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(__APPLE__)
        // GL 3.2 Core + GLSL 150
        const char *glsl_version = "#version 150";
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
        // GL 3.0 + GLSL 130
        const char *glsl_version = "#version 130";
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        // ImGui::StyleColorsLight();

        // float main_scale = ImGui_ImplSDL2_GetContentScaleForDisplay(0);
        // Setup scaling
        ImGuiStyle &style = ImGui::GetStyle();
        // style.ScaleAllSizes(10.f); // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)

        // Setup Platform/Renderer backends
        ImGui_ImplSDL2_InitForOpenGL(window.getHandle(), window.getContext());
        ImGui_ImplOpenGL3_Init(glsl_version);

        m_textures.setBaseDirectory(m_texture_directory);
}

ToolBoxUI::~ToolBoxUI()
{
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
}

namespace fs = std::filesystem;
using json = nlohmann::json;

struct ComponentBuilder
{
        virtual std::string build()
        {
                return "";
        }

        virtual void addToUI() {
                // m_name = component_name;

                // for (auto &el : component_js.items())
                // {
                // }
        };

        void addValue(std::string attribute, std::string data_type)
        {
                if (data_type == "float")
                {
                        ui_data[attribute];
                        ImGui::InputFloat(attribute.c_str(), (float *)ui_data.at(attribute));
                        // ui_data[attribute] =
                }
        }

        std::string m_name;
        std::unordered_map<std::string, void *> ui_data;
};

struct HealthBuilder : public ComponentBuilder
{
        virtual std::string build() override
        {
                return "HealthComponent{.max_hp = " + std::to_string(comp.max_hp) + ",.hp_regen = " + std::to_string(comp.hp_regen) + "};";
        }

        virtual void addToUI() override
        {
                ImGui::Begin("HealthComponent");
                ImGui::InputFloat("max_hp", &comp.max_hp);
                ImGui::InputFloat("hp_regen", &comp.hp_regen);
                ImGui::End();
        }

        float max_hp_value = 0.f;

        HealthComponent comp;
};

std::vector<fs::path> readTexturesFromDirectory(std::filesystem::path directory_path)
{
        if (!fs::exists(directory_path) || !fs::is_directory(directory_path))
        {
                std::cout << "Cannot read textures in: " << directory_path.string() << " Path does not exist or is not directory!" << std::endl;
                return {};
        }

        std::vector<fs::path> texture_paths;

        std::cout << "Reading textures in path: " << directory_path << std::endl;
        for (auto const &dir_entry : fs::directory_iterator{directory_path})
        {
                if (fs::path(dir_entry).extension() == ".png")
                {
                        texture_paths.push_back(dir_entry);
                }
        }

        return texture_paths;
}

template <typename T>
void eraseIndices(std::deque<T> &vec, const std::unordered_set<size_t> &to_remove)
{
        std::size_t idx = 0;
        vec.erase(
            std::remove_if(vec.begin(), vec.end(),
                           [&](const T &) mutable
                           {
                                   return to_remove.count(idx++); // remove if index is in set
                           }),
            vec.end());
}

void ToolBoxUI::handleEvent(SDL_Event event)
{
        if (event.type == SDL_KEYUP && event.key.keysym.scancode == SDL_SCANCODE_DELETE)
        {
                if (m_clicker_state == ImageClickerState::SelectingPoints)
                {
                        eraseIndices(m_clicked_points, m_selected_point_inds);
                        m_selected_point_inds.clear();
                }
        }

        if (event.type == SDL_MOUSEBUTTONUP)
        {
                if (m_clicker_state == ImageClickerState::SelectingPoints && event.button.button == SDL_BUTTON_LEFT)
                {
                        m_selected_point_inds.clear();
                        for (int i = 0; i < m_clicked_points.size(); ++i)
                        {
                                auto &point = m_clicked_points.at(i);
                                if (m_selection_rect.contains(point))
                                {
                                        m_selected_point_inds.insert(i);
                                }
                        }
                        m_selection_state = PointSelection::SelectingFirst;
                }
                if (event.button.button == SDL_BUTTON_RIGHT)
                {
                        m_clicker_state = ImageClickerState::AddingPoints;
                        m_draw_next_point = true;
                }
        }
        if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT && m_clicker_state == ImageClickerState::SelectingPoints)
        {
                m_selection_state = PointSelection::SelectingSecond;
                ImVec2 mouse_pos = ImGui::GetIO().MousePos;
                m_selection_rect.pos_x = mouse_pos.x - m_image_min.x;
                m_selection_rect.pos_y = mouse_pos.y - m_image_min.y;
        }
        if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT && m_clicker_state == ImageClickerState::AddingPoints)
        {
                ImVec2 mouse_pos = ImGui::GetIO().MousePos;
                if (isInImage(mouse_pos))
                {
                        m_clicked_points.push_back({mouse_pos.x - m_image_min.x, mouse_pos.y - m_image_min.y});
                }
        }
}

bool ToolBoxUI::isInImage(ImVec2 point)
{
        return point.x >= m_image_min.x && point.x < m_image_min.x + m_image_size.x &&
               point.y >= m_image_min.y && point.y < m_image_min.y + m_image_size.y;
}

void drawRectBorder(Rectf rect, Renderer &canvas)
{
        utils::Vector2f p1 = {rect.pos_x, rect.pos_y};
        utils::Vector2f p2 = {rect.pos_x, rect.pos_y + rect.height};
        utils::Vector2f p3 = {rect.pos_x + rect.width, rect.pos_y + rect.height};
        utils::Vector2f p4 = {rect.pos_x + rect.width, rect.pos_y};

        canvas.drawLineBatched(p1, p2, 1., {0, 1, 0.1, 1});
        canvas.drawLineBatched(p2, p3, 1., {0, 1, 0.1, 1});
        canvas.drawLineBatched(p3, p4, 1., {0, 1, 0.1, 1});
        canvas.drawLineBatched(p4, p1, 1., {0, 1, 0.1, 1});
}

void ToolBoxUI::redrawImage()
{
        if (m_selected_texture_name.empty())
        {
                return;
        }
        m_sprite_canvas.clear({0, 0, 0, 1});

        if (m_clicked_points.size() > 0)
        {
                std::size_t n_points = m_clicked_points.size();
                for (std::size_t id = 0; id < n_points; ++id)
                {
                        auto &point = m_clicked_points.at(id);
                        auto &prev_point = m_clicked_points.at((id + 1) % n_points);
                        m_sprite_canvas.drawLineBatched(point, prev_point, 1, {0, 1., 0.1, 1});

                        Color point_color = m_selected_point_inds.contains(id) ? Color{0, 1, 0.1, 1} : Color{1., 0, 0.1, 1};
                        m_sprite_canvas.drawCricleBatched(point, 5, point_color);
                }
        }

        if (m_clicker_state == ImageClickerState::SelectingPoints && m_selection_state == PointSelection::SelectingSecond)
        {
                drawRectBorder(m_selection_rect, m_sprite_canvas);
        }

        //! draw sprite into canvas;
        Sprite sprite(*m_textures.get(m_selected_texture_name));
        utils::Vector2f tex_size = m_textures.get(m_selected_texture_name)->getSize();
        float aspect_ratio = tex_size.y / tex_size.x;
        float element_aspect_ratio = m_image_size.y / m_image_size.x;

        utils::Vector2f image_size = {m_image_size.x, m_image_size.y * aspect_ratio};
        if (aspect_ratio > 1.)
        {
                image_size = {m_image_size.y / aspect_ratio, m_image_size.y};
        }
        else
        {
                image_size = {m_image_size.y, m_image_size.y * aspect_ratio};
        }

        sprite.setScale(image_size.x / 2.f, image_size.y / 2.f);
        sprite.setPosition(image_size.x / 2.f, image_size.y / 2.f);
        m_sprite_canvas.drawSprite(sprite);
        m_sprite_canvas.m_blend_factors = {BlendFactor::SrcAlpha, BlendFactor::OneMinusSrcAlpha};
        m_sprite_canvas.drawAll();
}

void ToolBoxUI::drawEntityDesigner()
{
        auto &entities = p_world->getEntities();

        ImGui::Begin("Entity", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);
        if (ImGui::Button("Select Texture"))
        {
                m_selecting_texture = !m_selecting_texture;
                if (m_selecting_texture)
                {
                        m_texture_paths = readTexturesFromDirectory(m_texture_directory);
                }
        }
        ImGui::SameLine();
        if (ImGui::Button("Add Points"))
        {
                m_clicker_state = ImageClickerState::AddingPoints;
        }
        ImGui::SameLine();
        if (ImGui::Button("Select Points"))
        {
                m_clicker_state = ImageClickerState::SelectingPoints;
        }

        if (m_selected_tex_id != -1)
        {
                auto selected_tex_name = m_texture_paths.at(m_selected_tex_id).filename();
                auto selected_texture = m_textures.get(selected_tex_name.replace_extension(""));
                auto tex_handle = selected_texture->getHandle();
                auto image_size = ImVec2{selected_texture->getSize().x, selected_texture->getSize().y};
                float aspect_ratio = image_size.y / (float)image_size.x;

                ImGui::Image((ImTextureID)(intptr_t)m_sprite_pixels.getHandle(), ImVec2(400, 300), ImVec2(0, 1), ImVec2(1, 0));
                m_image_min = ImGui::GetItemRectMin();
                m_image_size = ImGui::GetItemRectSize();

                if (m_clicker_state == ImageClickerState::AddingPoints)
                {
                        // redrawImage();
                        m_draw_next_point = false;
                }
                else if (m_clicker_state == ImageClickerState::SelectingPoints)
                {
                        // redrawImage();
                }
        }

        ImVec2 mouse_pos = ImGui::GetIO().MousePos;
        if (m_selection_state == PointSelection::SelectingSecond)
        {
                m_selection_rect.width = mouse_pos.x - m_image_min.x - m_selection_rect.pos_x;
                m_selection_rect.height = mouse_pos.y - m_image_min.y - m_selection_rect.pos_y;
        }

        ImGui::End();

        if (ImGui::Begin("Textures", &m_selecting_texture))
        {
                if (ImGui::BeginListBox("Texture Files"))
                {

                        for (int id = 0; id < m_texture_paths.size(); ++id)
                        {
                                auto tex_filename = m_texture_paths.at(id).filename();
                                const bool is_selected = (m_selected_tex_id == id);

                                if (ImGui::Selectable(tex_filename.c_str(), is_selected))
                                {
                                        m_selected_tex_id = id;
                                        if (!m_textures.get(tex_filename))
                                        {
                                                auto path = tex_filename;
                                                m_textures.add(tex_filename.replace_extension("").string(), path);
                                        }
                                        m_selected_texture_name = tex_filename.replace_extension("").string();

                                        redrawImage();
                                        m_selecting_texture = false;
                                }
                                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                                if (is_selected)
                                {
                                        ImGui::SetItemDefaultFocus();
                                }
                        }
                        ImGui::EndListBox();
                }
        }
        ImGui::End();
}

void ToolBoxUI::draw()
{

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        // if (show_demo_window)
        //         ImGui::ShowDemoWindow(&show_demo_window);

        drawEntityDesigner();

        // Rendering
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        redrawImage();
}

void ToolBoxUI::initWorld(GameWorld &world)
{
        p_world = &world;
}
