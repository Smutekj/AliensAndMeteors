#pragma once

#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

#include <iostream>

class TextureAtlas
{

    sf::RenderTexture m_atlas;

    public:

    TextureAtlas(std::vector<std::string>& filenames)
    {

        m_atlas.create(4000, 4000);

        sf::RectangleShape rect;
        sf::Texture texture;

        sf::View view;


        for(auto& filename : filenames)
        {
            if(!texture.loadFromFile(filename)){
                throw std::runtime_error("file " + filename + " with texture not found!");
            }
            auto size = texture.getSize();
            rect.setTexture(&texture);
            rect.setSize({(float)size.x, (float)size.y});
            
            view.setSize(rect.getSize());
            
            m_atlas.draw(rect);

        }

        m_atlas.display();
    }



};
