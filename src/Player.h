#pragma once

#include "core.h"
#include "Geometry.h"
#include "PolygonObstacleManager.h"

struct Player
{
    sf::Vector2f pos = {Geometry::BOX[0] / 2.f, Geometry::BOX[1] / 2.f};
    sf::Vector2f vel = {0.f, 0.f};
    float speed = 0.f;
    float max_speed = 30.f;
    bool is_boosting = false;
    float boost_factor = 2.f;
    float slowing_factor = 0.03f;
    float acceleration = 0.5f;

    float boost_time = 0.f;
    float max_boost_time = 100.f;
    
    float boost_heat = 0.f;
    float max_boost_heat = 100.f;

    float angle = 0.f;
    float radius = 3.f;
    sf::Vector2f orientation = {1, 0};
    int health = 100;
    int max_health = 100;


    void update(float dt)
    {
        if (angle < -180.f)
        {
            angle = 180.f;
        }
        else if (angle > 180.f)
        {
            angle = -180.f;
        }

        if(is_boosting)
        {
            boost_time++;
            boost_heat++;

            if(boost_heat > max_boost_heat)
            {
                is_boosting = false;
                boost_time = 0;
            }
        }else{
            if(boost_heat >= 0.f)
            {
                boost_heat--;
            }
        }

        speed += acceleration;
        float real_max_speed = max_speed*(1+boost_factor*is_boosting);
        speed += boost_factor*is_boosting;
        // speed = std::min(speed, real_max_speed);
        // speed = std::max(speed, -real_max_speed/2.f);
        speed -= speed*slowing_factor;
                
        pos += speed * angle2dir(angle) * dt;

    }

};

struct Ligthning
{
    sf::RenderTexture light_texture;

    int n_rays = 69;
    float distance = 69.69f;
    std::vector<Edgef> rays;
    sf::Vector2f center;

    PolygonObstacleManager *p_meteors;

    sf::Shader light_shader;

    Ligthning(PolygonObstacleManager &meteors) : p_meteors(&meteors)
    {
        light_shader.loadFromFile("../Resources/basic.vert", "../Resources/light.frag");

        light_texture.create(800, 600);
        light_texture.setSmooth(true);
        rays.resize(n_rays);
    }
    void update()
    {

        float d_angle = 360.f / n_rays;

        int i = 0;
        for (auto &ray : rays)
        {
            ray.from = center;
            ray.t = angle2dir(d_angle * i);
            ray.l = distance;

            auto hit_point = p_meteors->findClosestIntesection(ray.from, ray.t, ray.l);
            ray.l = dist(ray.from, hit_point);

            i++;
        }
    }

    void draw(sf::RenderTexture &input, sf::RenderTarget &window)
    {
        sf::VertexArray vertices;
        vertices.setPrimitiveType(sf::TriangleFan);
        vertices.resize(n_rays + 2);
        vertices[0].position = center;
        vertices[0].color = sf::Color(255, 255, 255, 125);

        for (int i = 0; i < n_rays; ++i)
        {
            vertices[i + 1].position = rays[i].to();
            float factor = rays[i].l / distance;
            float color = factor * 50;
            vertices[i + 1].color = sf::Color(color, color, color, (1 - factor) * 255);
        }
        vertices[n_rays + 1] = vertices[1];

        light_texture.setView(window.getView());
        light_texture.clear(sf::Color::Black);
        light_texture.draw(vertices);
        light_texture.display();

        sf::RenderStates state;
        state.blendMode = sf::BlendAdd;
        light_shader.setUniform("image", input.getTexture());
        light_shader.setUniform("light_data", light_texture.getTexture());
        state.shader = &light_shader;
        // state.texture

        sf::Vector2f rect_size;
        rect_size.x = input.getSize().x;
        rect_size.y = input.getSize().y;
        sf::VertexArray quad_verts;
        quad_verts.setPrimitiveType(sf::PrimitiveType::Quads);
        quad_verts.resize(4);
        quad_verts[0] = sf::Vertex{{0, 0}, sf::Color::Transparent, {0, 1}};
        quad_verts[1] = sf::Vertex{{rect_size.x, 0}, sf::Color::Transparent, {1, 1}};
        quad_verts[2] = sf::Vertex{{rect_size.x, rect_size.y}, sf::Color::Transparent, {1, 0}};
        quad_verts[3] = sf::Vertex{{0, rect_size.y}, sf::Color::Transparent, {0, 0}};

        auto old_view = window.getView();
        window.setView(input.getDefaultView());
        window.draw(quad_verts, state);
        window.setView(old_view);
    }
};

void inline darken(const sf::RenderTexture &input, sf::RenderTexture &result)
{
    sf::Vector2f rect_size;
    rect_size.x = input.getSize().x;
    rect_size.y = input.getSize().y;
    sf::VertexArray quad_verts;
    quad_verts.setPrimitiveType(sf::PrimitiveType::Quads);
    quad_verts.resize(4);
    quad_verts[0] = sf::Vertex{{0, 0}, sf::Color::Transparent, {0, 1}};
    quad_verts[1] = sf::Vertex{{rect_size.x, 0}, sf::Color::Transparent, {1, 1}};
    quad_verts[2] = sf::Vertex{{rect_size.x, rect_size.y}, sf::Color::Transparent, {1, 0}};
    quad_verts[3] = sf::Vertex{{0, rect_size.y}, sf::Color::Transparent, {0, 0}};

    sf::Shader darkener;
    darkener.loadFromFile("../Resources/basic.vert", "../Resources/darken.frag");
    darkener.setUniform("image", input.getTexture());
    darkener.setUniform("factor", 0.5f);

    // auto old_view = result.getView();
    // result.setView(input.getDefaultView());
    result.draw(quad_verts, &darkener);
    // result.setView(old_view);
    result.display();
}

struct LightSource
{
    std::vector<Edgef> rays;

    sf::Vector2f center;
};
