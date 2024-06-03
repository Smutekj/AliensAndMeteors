#pragma once

#include <SFML/Graphics.hpp>

#include <iostream>
#include <unordered_map>
#include <filesystem>

#include "ResourceIdentifiers.h"

class Bloom
{

public:
    Bloom(int width = 1200, int height = 1200 * 1080. / 1920.);
    void doTheThing(const sf::RenderTexture &input, sf::RenderTarget &output);

private:
    ShaderHolder m_shaders;

    sf::RenderTexture m_pass_textures[2];
    sf::RenderTexture m_downsized_texture;

    sf::VertexArray m_texture_rect;

    sf::Vector2i texture_size;
};
