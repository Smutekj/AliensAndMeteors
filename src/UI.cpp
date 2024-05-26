#include "UI.h"
#include "Game.h"

#include <iostream>
#include <fstream>
#include <sstream>

#include "imgui.h"
#include "imgui-SFML.h"

#include "Utils/magic_enum.hpp"
#include "Utils/magic_enum_utility.hpp"

#include "Entities.h"

std::vector<std::string> separateLine(std::string line, char delimiter = ' ')
{
    std::vector<std::string> result;
    int start = 0;
    int end = 0;

    while ((start = line.find_first_not_of(' ', end)) != std::string::npos)
    {
        end = line.find(' ', start);
        result.push_back(line.substr(start, end - start));
    }
    return result;
}

UIWindow::UIWindow(std::string name) : name(name)
{
}

UI::UI(sf::RenderWindow &window)
{

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
                                                          // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // IF using Docking Branch
                                                          // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    // // // Setup Platform/Renderer backends

    auto physics = std::make_unique<PhysicsWindow>(Enemy::m_force_multipliers, Enemy::m_force_ranges);

    windows[UIWindowType::PHYSICS] = std::move(physics);
    // windows[UIWindowType::SHADERS] = std::move(shaders_window);

    is_active[UIWindowType::PHYSICS] = true;
    // is_active[UIWindowType::SHADERS] = true;

    names[UIWindowType::PHYSICS] = "Physics";
    // names[UIWindowType::SHADERS] = "Shaders";
}

UIWindow::~UIWindow()
{
}

void UI::showWindow()
{
}

void UI::draw(sf::RenderWindow &window)
{

    ImGui::SFML::Update(window, m_clock.restart());
    ImGui::Begin("Control Panel"); // Create a window called "Hello, world!" and append into it.
    for (auto &[window_type, p_window] : windows)
    {
        if (ImGui::Button(p_window->getName().c_str()))
            is_active[window_type] = !is_active[window_type];
    }

    ImGui::End();

    for (auto &[window_type, p_window] : windows)
    {
        if (is_active[window_type])
        {
            p_window->draw();
        }
    }
    ImGui::SFML::Render(window);
}

PhysicsWindow::PhysicsWindow(std::unordered_map<Multiplier, float> &force_multipliers,
                             std::unordered_map<Multiplier, float> &force_ranges) : m_force_multipliers(force_multipliers), m_force_ranges(force_ranges), UIWindow("Physics")
{
    for (auto &[multiplier_type, value] : force_multipliers)
    {
        mulitplier2slider_min_max[multiplier_type] = {0.0f, 100.f};
    }
}

PhysicsWindow::~PhysicsWindow() {}

void PhysicsWindow::draw()
{

    ImGui::Begin(name.c_str());

    if (ImGui::BeginListBox("Force Multipliers"))
    {
        for (auto &[multiplier_type, value] : m_force_multipliers)
        {
            auto multiplier_name = static_cast<std::string>(magic_enum::enum_name(multiplier_type));
            auto &min_value = mulitplier2slider_min_max[multiplier_type].first;
            auto &max_value = mulitplier2slider_min_max[multiplier_type].second;

            ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.66);
            ImGui::SliderFloat(multiplier_name.c_str(), &value, min_value, max_value);

            ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.33f);
            ImGui::InputFloat(("min##" + multiplier_name).c_str(), &min_value);
            ImGui::SameLine();
            ImGui::InputFloat(("max##" + multiplier_name).c_str(), &max_value);
        }
        ImGui::EndListBox();
    }

    if (ImGui::BeginListBox("Force Ranges"))
    {
        for (auto &[multiplier_type, value] : m_force_ranges)
        {
            auto multiplier_name = static_cast<std::string>(magic_enum::enum_name(multiplier_type));

            ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.66);
            ImGui::SliderFloat(multiplier_name.c_str(), &value, 0, 50);
        }
        ImGui::EndListBox();
    }
    ImGui::End();
}

ShadersWindow::ShadersWindow() : UIWindow("Shaders")
{
}

ShadersWindow::~ShadersWindow() {}

static void drawStuff(sf::Color &color)
{
    // ImGui::SeparatorText("Color picker");
    static bool alpha = true;
    static bool alpha_bar = true;
    static bool side_preview = true;
    static bool ref_color = true;
    static ImVec4 ref_color_v(1.0f, 0.0f, 1.0f, 0.5f);
    static int display_mode = 0;
    static int picker_mode = 0;

    ImGui::Combo("Display Mode", &display_mode, "Auto/Current\0None\0RGB Only\0HSV Only\0Hex Only\0");
    ImGuiColorEditFlags flags;
    if (!alpha)
        flags |= ImGuiColorEditFlags_NoAlpha; // This is by default if you call ColorPicker3() instead of ColorPicker4()
    if (alpha_bar)
        flags |= ImGuiColorEditFlags_AlphaBar;
    if (!side_preview)
        flags |= ImGuiColorEditFlags_NoSidePreview;
    if (picker_mode == 1)
        flags |= ImGuiColorEditFlags_PickerHueBar;
    if (picker_mode == 2)
        flags |= ImGuiColorEditFlags_PickerHueWheel;
    if (display_mode == 1)
        flags |= ImGuiColorEditFlags_NoInputs; // Disable all RGB/HSV/Hex displays
    if (display_mode == 2)
        flags |= ImGuiColorEditFlags_DisplayRGB; // Override display mode
    if (display_mode == 3)
        flags |= ImGuiColorEditFlags_DisplayHSV;
    if (display_mode == 4)
        flags |= ImGuiColorEditFlags_DisplayHex;
    ImGui::ColorPicker4("MyColor##4", (float *)&color, flags, ref_color ? &ref_color_v.x : NULL);
}

void setShaderVariableValue(sf::Shader &shader,
                            std::string fragment, std::string vertex, std::string var_name, sf::Color color)
{

    const auto filename = fragment;
    const auto tmp_filename = filename + ".tmp";
    std::ifstream file(filename);
    std::ofstream new_file(tmp_filename);

    std::string line;
    while (std::getline(file, line))
    {
        std::stringstream iss(line);

        auto words_on_line = separateLine(line);
        if (words_on_line.size() >= 3 && words_on_line.at(0) == "const")
        {
            if (words_on_line.at(2) == var_name)
            {

                std::string new_value = "vec4(" + std::to_string(color.r) + "," +
                                        std::to_string(color.g) + "," +
                                        std::to_string(color.b) + "," +
                                        std::to_string(color.a) + ");";

                words_on_line.at(4) = new_value;

                line = "";
                for (const auto &word : words_on_line)
                {
                    line += word;
                    line += " ";
                }
            }
        }

        new_file << line << "\n";
    }
    new_file.close();
    file.close();

    std::remove(filename.c_str());
    std::rename(tmp_filename.c_str(), filename.c_str());
    shader.loadFromFile(vertex, fragment);
}

void setShaderVariableValue(sf::Shader &shader, std::string var_name, float value)
{

    // const auto filename = shader.fragment_path;
    // const auto tmp_filename = filename + ".tmp";
    // std::ifstream file(filename);
    // std::ofstream new_file(tmp_filename);

    // std::string line;
    // while (std::getline(file, line))
    // {
    //     std::stringstream iss(line);

    //     auto words_on_line = separateLine(line);
    //     if (words_on_line.size() >= 3 && words_on_line.at(0) == "const")
    //     {
    //         if (words_on_line.at(2) == var_name)
    //         {

    //             std::string new_value = std::to_string(value);

    //             words_on_line.at(4) = new_value + ";";

    //             line = "";
    //             for (const auto &word : words_on_line)
    //             {
    //                 line += word;
    //                 line += " ";
    //             }
    //         }
    //     }

    //     new_file << line << "\n";
    // }
    // new_file.close();
    // file.close();

    // std::remove(filename.c_str());
    // std::rename(tmp_filename.c_str(), filename.c_str());
    // shader.recompile();
}

void ShadersWindow::draw()
{
    {
        ImGui::Begin("Shader!");

        static int chosen_shader_ind = 0; // Here we store our selection data as an index.
        static int chosen_field_ind = 0;  // Here we store our selection data as an index.
        if (ImGui::TreeNode("Shaders"))
        {
            if (ImGui::BeginListBox("Shaders"))
            {
                for (int n = 0; n < shaders.size(); n++)
                {
                    const bool is_selected = (chosen_shader_ind == n);
                    // if (ImGui::Selectable(shaders[n].p_program->shader_name.c_str(), is_selected))
                    // {
                    //     chosen_shader_ind = n;
                    //     chosen_field_ind = 0;
                    // }

                    // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                    if (is_selected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndListBox();
            }
            ImGui::TreePop();
        }

        auto &shader_data = shaders.at(chosen_shader_ind);

        if (ImGui::TreeNode("Field Picker"))
        {
            if (ImGui::BeginListBox("Color Fields"))
            {
                for (int n = 0; n < shader_data.colors.size(); n++)
                {
                    const bool is_selected = (chosen_field_ind == n);
                    if (ImGui::Selectable(shader_data.colors.at(n).uniform_name.c_str(), is_selected))
                        chosen_field_ind = n;
                    if (is_selected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndListBox();
            }

            for (int n = 0; n < shader_data.values.size(); n++)
            {
                const bool is_selected = (chosen_field_ind == n);
                // if (ImGui::InputFloat(shader_data.values.at(n).uniform_name.c_str(), &shader_data.values.at(n).value))
                //     setShaderVariableValue(*shader_data.p_program,
                //                            shader_data.values.at(n).uniform_name.c_str(),
                //                            shader_data.values.at(n).value);
            }

            ImGui::TreePop();
        }

        drawStuff(shader_data.colors.at(chosen_field_ind).value);
        // setShaderVariableValue(*shader_data.p_program,
        //                        shader_data.colors.at(chosen_field_ind).uniform_name.c_str(),
        //                        shader_data.colors.at(chosen_field_ind).value);

        ImGui::End();
    }
}