#pragma once

#include "SFML/Graphics/Shader.hpp"

#include <unordered_map>
#include <memory>

enum class ShaderType
{
    BASIC,
    TEXTURED,
    INSTANCED
};

class ShaderManager
{

public:
    std::unordered_map<ShaderType, sf::Shader> shaders;

    static ShaderManager &getInstance()
    {
        static ShaderManager instance; // Guaranteed to be destroyed.
                                       // Instantiated on first use.
        return instance;
    }

    sf::Shader &getShader(ShaderType type)
    {
        static ShaderManager instance;
        return instance.shaders.at(type);
    }

private:
    ShaderManager()
    {

        // shaders.insert({ShaderType::BASIC, sf::Shader()});
        // shaders.at(ShaderType::BASIC).loadFromFile(s_vertexShaderSourceViewTransform, s_framgentShaderSource);
    }

public:
    ShaderManager(ShaderManager const &) = delete;
    void operator=(ShaderManager const &) = delete;
};
