#pragma once

#include <SFML/System/NonCopyable.hpp>

namespace sf{
    class RenderTexture;
    class Shader;
    class RenderTarget;
}

class PostEffect : sf::NonCopyable
{
	public:
		virtual					~PostEffect();
		virtual void			apply(const sf::RenderTexture& input, sf::RenderTarget& output) = 0;
        virtual void            update() = 0;
		static bool				isSupported();
		

	protected:
		static void				applyShader(const sf::Shader& shader, sf::RenderTarget& output);
};


class ExplosionEffect : PostEffect{


public:

    ExplosionEffect(){

    }

    void apply(const sf::RenderTexture& input, sf::RenderTarget& output) override{

    }


private:
    sf::Shader test;
};


class PostEffectManager{

    ObjectPool<std::unique_ptr<PostEffect>, 1000> effects;

};


