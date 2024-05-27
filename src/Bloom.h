#pragma once

#include <SFML/Graphics.hpp>

#include <iostream>
#include <unordered_map>
#include <filesystem>

#include "ResourceIdentifiers.h"


class Bloom
{

    sf::Shader brightness;
    sf::Shader horiz_pass;
    sf::Shader vert_pass;
    sf::Shader full_pass;
    sf::Shader add;
    sf::Shader test;

    sf::RenderTexture rts[2];
    sf::RenderTexture ds[2];

    sf::Vector2i texture_size;

public:
    Bloom(int width = 800, int height = 800)
        : texture_size(width, height)
    {
        vert_pass.loadFromFile("../Resources/basic.vert", "../Resources/discreteVert.frag");
        full_pass.loadFromFile("../Resources/basic.vert", "../Resources/fullpass.frag");
        test.loadFromFile("../Resources/basic.vert", "../Resources/downsample.frag");
        horiz_pass.loadFromFile("../Resources/basic.vert", "../Resources/discreteHoriz.frag");
        brightness.loadFromFile("../Resources/basic.vert", "../Resources/brigthness.frag");
        add.loadFromFile("../Resources/basic.vert", "../Resources/add.frag");

        rts[0].setSmooth(true);
        rts[1].setSmooth(true);

        rts[0].create(width, height);
        rts[1].create(width, height);

        ds[0].create(width / 2, height / 2);
        ;
        ds[0].setSmooth(true);
    }

    void onResize(sf::Vector2u new_size)
    {
        rts[0].create(new_size.x, new_size.y);
        // rts[1].create(new_size.x, new_size.y);
        rts[0].setSmooth(true);
        rts[1].setSmooth(true);
        ds[0].create(new_size.x / 2, new_size.y / 2);
        ds[0].setSmooth(true);
    }

    void doTheThing(const sf::RenderTexture &input, sf::RenderTarget &output)
    {
        rts[0].clear(sf::Color::Transparent);
        rts[1].clear(sf::Color::Transparent);

        auto old_view = output.getView();
        output.setView(output.getDefaultView());

        sf::VertexArray verts;
        verts.resize(4);
        verts.setPrimitiveType(sf::Quads);
        // verts[0] = sf::Vertex{{0, 0}, sf::Color::Transparent, {0, 0}};
        // verts[3] = sf::Vertex{{input.getSize().x/2, 0}, sf::Color::Transparent, {1, 0}};
        // verts[2] = sf::Vertex{{input.getSize().x/2, input.getSize().y/2}, sf::Color::Transparent, {1, 1}};
        // verts[1] = sf::Vertex{{0, input.getSize().y/2}, sf::Color::Transparent, {0, 1}};
        sf::Vector2f rect_size;
        rect_size.x = ds[0].getSize().x;
        rect_size.y = ds[0].getSize().y;
        verts[0] = sf::Vertex{{0, 0}, sf::Color::Transparent, {0, 1}};
        verts[1] = sf::Vertex{{rect_size.x, 0}, sf::Color::Transparent, {1, 1}};
        verts[2] = sf::Vertex{{rect_size.x, rect_size.y}, sf::Color::Transparent, {1, 0}};
        verts[3] = sf::Vertex{{0, rect_size.y}, sf::Color::Transparent, {0, 0}};

        sf::RenderStates states;

        states.shader = &test;
        states.blendMode = sf::BlendNone;
        test.setUniform("image", input.getTexture());
        ds[0].draw(verts, states);
        ds[0].display();
        full_pass.setUniform("image", ds[0].getTexture());
        states.shader = &full_pass;
        // states.blendMode = sf
        verts[0] = sf::Vertex{{0, 0}, sf::Color::Transparent, {0, 1}};
        verts[1] = sf::Vertex{{rect_size.x, 0}, sf::Color::Transparent, {1, 1}};
        verts[2] = sf::Vertex{{rect_size.x, rect_size.y}, sf::Color::Transparent, {1, 0}};
        verts[3] = sf::Vertex{{0, rect_size.y}, sf::Color::Transparent, {0, 0}};

        rect_size.x = rts[0].getSize().x;
        rect_size.y = rts[0].getSize().y;
        verts[0] = sf::Vertex{{0, 0}, sf::Color::Transparent, {0, 1}};
        verts[1] = sf::Vertex{{rect_size.x, 0}, sf::Color::Transparent, {1, 1}};
        verts[2] = sf::Vertex{{rect_size.x, rect_size.y}, sf::Color::Transparent, {1, 0}};
        verts[3] = sf::Vertex{{0, rect_size.y}, sf::Color::Transparent, {0, 0}};


        // rts[1].draw(verts, states);
        // rts[1].display();

        brightness.setUniform("image", ds[0].getTexture());
        states.blendMode = sf::BlendNone;
        states.shader = &brightness;

        rts[0].draw(verts, states);
        // rts[0].display();

        for (int i = 0; i < 5; ++i)
        {
            vert_pass.setUniform("image", rts[0].getTexture());
            // texture_rect.setTexture(&rts[0].getTexture());
            states.shader = &vert_pass;
            rts[1].draw(verts, states);
            // rts[1].display();

            horiz_pass.setUniform("image", rts[1].getTexture());
            // texture_rect.setTexture(&rts[1].getTexture());
            states.shader = &horiz_pass;
            rts[0].draw(verts, states);
            // rts[0].display();
        }

        add.setUniform("image1", rts[0].getTexture());
        add.setUniform("image2", input.getTexture());

        rect_size.x = input.getSize().x;
        rect_size.y = input.getSize().y;
        verts[0] = sf::Vertex{{0, 0}, sf::Color::Transparent, {0, 1}};
        verts[1] = sf::Vertex{{rect_size.x, 0}, sf::Color::Transparent, {1, 1}};
        verts[2] = sf::Vertex{{rect_size.x, rect_size.y}, sf::Color::Transparent, {1, 0}};
        verts[3] = sf::Vertex{{0, rect_size.y}, sf::Color::Transparent, {0, 0}};


        rts[1].display();
        states.shader = &add;
        states.blendMode = sf::BlendAdd;
        output.draw(verts, states);
        output.setView(old_view);
    }
};
