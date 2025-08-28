#pragma once

#include <SDL_mixer.h>

#include <unordered_map>
#include <filesystem>
#include <cassert>
#include <functional>
#include <iostream>

enum class SoundID
{
    Explosion1 = 0,
    Laser1,
    Laser2,
    Laser3,
    Electro,
    Rocket1,
    Rocket2,
    Rocket3,
    Rocket4,
};
enum class MusicID
{
    
};

struct Channel
{
    int id = 0;
    u_int8_t volume = 69;
    bool playing = false;

};


class SoundSystem
{
    static SoundSystem* p_instance;
    static std::array<Channel, MIX_CHANNELS> m_channels;


private:
    void onChannelFinish(int channel_id)
    {
        m_channels.at(channel_id).playing = false;
    }
    
    // static const std::function<void(int)> on_finish = [](int channel_id)
    // {
    //     m_channels.at(channel_id).playing = false;
    // };
    
    SoundSystem()
    {
        Mix_Volume(-1, 25);
    }
    ~SoundSystem(){
        delete p_instance;
    }
    

public:
    static void play(SoundID id)
    {
        play(id, 0.);
    }
    static void play(SoundID id, float distance)
    {
        assert(SoundSystem::get().m_chunks.contains(id));
     
        static float m_max_distance = 400.f;
        if(distance < 0.f)
        {
            std::cout << "WARNING! Cannot play sound at negative distance!" << std::endl;
            return; 
        }

        distance = std::min(m_max_distance, distance);//! truncate the distance
        auto max_dist_i =  std::numeric_limits<u_int8_t>::max();

        u_int8_t dist_i = ((distance / m_max_distance) * max_dist_i); 
        int channel_id = Mix_PlayChannel(-1, SoundSystem::get().m_chunks.at(id), 0);
        Mix_SetDistance(channel_id, dist_i);
    }

    static bool registerSound(SoundID id, std::filesystem::path wav_path)
    {
        SoundSystem::get().m_chunks[id] = Mix_LoadWAV((const char *)wav_path.c_str());
        if (!SoundSystem::get().m_chunks.at(id))
        {
            printf("Failed to load sound effect at path %s\n ! SDL_mixer Error: %s\n", wav_path.c_str(), Mix_GetError());
            return false;
        }
        return true;
    }

public:
    static SoundSystem &get()
    {
        if (!p_instance)
        {
            p_instance = new SoundSystem();
            // Mix_ChannelFinished(*on_finish.target<void(*)(int)>());

        }
        return *p_instance;
    }


private:
    std::unordered_map<SoundID, Mix_Chunk *> m_chunks;
    std::unordered_map<MusicID, Mix_Music *> m_songs;

};
