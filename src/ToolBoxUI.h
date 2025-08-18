#pragma once

#include <Window.h>
#include <Texture.h>
#include <Renderer.h>

#include <filesystem>

#include <unordered_set>
#include <imgui.h>

class GameWorld;

class ToolBoxUI
{
public:
        ToolBoxUI(Window &window, TextureHolder &textures);

        void draw();
        void initWorld(GameWorld &world);

        void handleEvent(SDL_Event event);

private:
        void drawEntityDesigner();

        void redrawImage();

        bool isInImage(ImVec2 point);

private:
        GameWorld *p_world = nullptr;
        bool show_demo_window = true;

        bool m_selecting_texture = false;

        
        int m_selected_tex_id = -1;
        std::string m_selected_texture_name = "";
        std::vector<std::filesystem::path> m_texture_paths; 
        std::filesystem::path m_texture_directory = "../Resources/Textures/";
        
        TextureHolder m_textures;
        

        FrameBuffer m_sprite_pixels;
        Renderer m_sprite_canvas;

        
        enum class ImageClickerState
        {
                AddingPoints,
                MovingPoints,
                DeletingPoints,
                SelectingPoints,
        };
        enum class PointSelection
        {
                SelectingFirst,
                SelectingSecond,
                AppendingPoints,
        };
        std::deque<utils::Vector2f> m_clicked_points;
        std::unordered_set<std::size_t> m_selected_point_inds;

        PointSelection m_selection_state = PointSelection::SelectingFirst;
        ImageClickerState m_clicker_state = ImageClickerState::AddingPoints;
        Rectf m_selection_rect;
        bool m_draw_next_point = false;
        bool get_first_point = false;

        ImVec2 m_image_size;
        ImVec2 m_image_min;

};