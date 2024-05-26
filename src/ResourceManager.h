#pragma once

#include <SFML/Graphics/Shader.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Audio.hpp>

#include <unordered_map>
#include <memory>

#include "ResourceIdentifiers.h"


enum class ShaderType
{
    BASIC,
    TEXTURED,
    INSTANCED
};


class SoundManager
{


    std::unordered_map<int, sf::SoundBuffer> m_resources;
    std::unordered_map<int, sf::Sound> m_sounds;


public:
    static void play(int type)
    {

        static SoundManager instance;

        instance.m_sounds[type].setBuffer(instance.m_resources.at(type));
        // instance.m_sounds[type].play();

    }


    private:
    SoundManager()
    {
        if(!m_resources[0].loadFromFile("../Resources/Sound/pop.wav"))
        {
            throw std::runtime_error("soundfile not found");
        }
    }

public:
    SoundManager(SoundManager const &) = delete;
    void operator=(SoundManager const &) = delete;
};
