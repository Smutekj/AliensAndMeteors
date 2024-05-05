#pragma once
#include "SFML/Graphics.hpp"

class ExplosionEffect{

    sf::Vector2f r_center;
    float radius;
    int time = 0;
    int life_time = 200;

    sf::Shader effect;

    sf::RectangleShape effect_rect;

public:
    ExplosionEffect(sf::Vector2f center, float radius = 3.0) 
    : 
    r_center(center), radius(radius), effect_rect({radius, radius})
     {
        effect_rect.setPosition(center);
        effect.loadFromFile("../Resources/basic.vert", "../Resources/explosion.frag");
        effect_rect.setFillColor(sf::Color::Red);
        effect_rect.setTextureRect(sf::IntRect{{0,0}, {600,600}});
    
    }
    bool isDone()const{
        return time > life_time;
    }

    void update(){

        time++;
        
    }


    void draw(sf::RenderWindow& window){
        effect.setParameter("time", time);
        window.draw(effect_rect, &effect);

    }

};