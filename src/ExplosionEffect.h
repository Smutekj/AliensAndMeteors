#pragma once

#include "SFML/Graphics.hpp"
#include "ResourceIdentifiers.h"
#include "Utils/GayVector.h"
#include <iostream>

class AnimatedSprite
{

    int time = 0;
    int frame_time = 5;
    int life_time;
    int tex_x;
    int tex_y;
    int n_sprites_x;
    int n_sprites_y;

    int n_repeats = -1;

    sf::RectangleShape sprite;
    sf::Vector2i texture_rect_size;

public:
    AnimatedSprite(sf::Texture &texture, sf::Vector2f pos, sf::Vector2f scale,
                   int n_sprites_x = 4, int n_sprites_y = 4, int life_time = 120, int n_repeats = 1)
        : tex_x(n_sprites_x - 1), tex_y(n_sprites_y - 1), n_sprites_x(n_sprites_x), n_sprites_y(n_sprites_y), life_time(life_time), n_repeats(n_repeats)
    {
        texture_rect_size = static_cast<sf::Vector2i>(texture.getSize());
        texture_rect_size.x /= n_sprites_x;
        texture_rect_size.y /= n_sprites_y;
        sprite.setSize(scale);
        sprite.setTexture(&texture);
        sprite.setTextureRect(getCurrentTextureRect());
        // sprite.setScale(scale);
        sprite.setPosition(pos - scale / 2.f);

        // sprite.setColor(sf::Color::Green);
    }

    bool isDone() const
    {
        return time > life_time || n_repeats == 0;
    }

    void update()
    {
        time++;
        if (time % frame_time == frame_time - 1)
        {
            animateSprite();
        }
    }

    void draw(sf::RenderTarget &target)
    {
        target.draw(sprite);
    }

    void animateSprite()
    {
        tex_x--;
        if (tex_x < 0)
        {
            tex_y--;
            tex_x = n_sprites_x - 1;
        }
        if (tex_y < 0)
        {
            tex_y = n_sprites_y - 1;
        }
        if (tex_x == n_sprites_x - 1 && tex_y == n_sprites_y - 1)
        {
            n_repeats--;
        }

        sprite.setTextureRect(getCurrentTextureRect());
    }

    sf::IntRect getCurrentTextureRect() const
    {
        return {{texture_rect_size.x * tex_x, texture_rect_size.y * tex_y},
                {texture_rect_size.x, texture_rect_size.y}};
    }
};

class Effect
{

public:
    virtual void update() = 0;
    virtual bool isDone() = 0;
    virtual void draw(sf::RenderTarget &window) = 0;
    virtual ~Effect() = 0;
};

class ExplosionEffect2 : public Effect
{
    AnimatedSprite animation;
    sf::Vector2f r_center;
    float radius;

public:
    ExplosionEffect2(sf::Vector2f center, float radius, sf::Texture &texture)
        : r_center(center), radius(radius), animation(texture, center, {radius, radius})
    {
    }

    virtual ~ExplosionEffect2() {}

    virtual bool isDone() override
    {
        return animation.isDone();
    }

    virtual void update() override
    {
        animation.update();
    }

    virtual void draw(sf::RenderTarget &window) override
    {
        animation.draw(window);
    }
};

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

        ds[0].create(width/2, height/2);;
        ds[0].setSmooth(true);
    }

    void onResize(sf::Vector2u new_size){
        rts[0].create(new_size.x, new_size.y);
        // rts[1].create(new_size.x, new_size.y);
        rts[0].setSmooth(true);
        rts[1].setSmooth(true);
        ds[0].create(new_size.x/2, new_size.y/2);
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
        
        verts[0] = sf::Vertex{{0, 0}, sf::Color::Transparent, {0, 0}};
        verts[3] = sf::Vertex{{ds[0].getSize().x, 0}, sf::Color::Transparent, {1, 0}};
        verts[2] = sf::Vertex{{ds[0].getSize().x, ds[0].getSize().y}, sf::Color::Transparent, {1, 1}};
        verts[1] = sf::Vertex{{0, ds[0].getSize().y}, sf::Color::Transparent, {0, 1}};


        sf::RenderStates states;

        states.shader = &test;
        states.blendMode = sf::BlendNone;
        test.setUniform("image", input.getTexture());
        ds[0].draw(verts, states);
        ds[0].display();
        full_pass.setUniform("image", ds[0].getTexture());
        states.shader = &full_pass;
        // states.blendMode = sf
        // verts[0] = sf::Vertex{{0, 0}, sf::Color::Transparent, {0, 0}};
        // verts[3] = sf::Vertex{{input.getSize().x, 0}, sf::Color::Transparent, {1, 0}};
        // verts[2] = sf::Vertex{{input.getSize().x, input.getSize().y}, sf::Color::Transparent, {1, 1}};
        // verts[1] = sf::Vertex{{0, input.getSize().y}, sf::Color::Transparent, {0, 1}};

        verts[0] = sf::Vertex{{0, 0}, sf::Color::Transparent, {0, 0}};
        verts[3] = sf::Vertex{{rts[0].getSize().x, 0}, sf::Color::Transparent, {1, 0}};
        verts[2] = sf::Vertex{{rts[0].getSize().x, rts[0].getSize().y}, sf::Color::Transparent, {1, 1}};
        verts[1] = sf::Vertex{{0, rts[0].getSize().y}, sf::Color::Transparent, {0, 1}};


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
        // output.draw(verts, &add);
        // return;
        //! WTF!?????
        verts[0] = sf::Vertex{{0, 0}, sf::Color::Transparent, {0, 0}};
        verts[3] = sf::Vertex{{input.getSize().x, 0}, sf::Color::Transparent, {1, 0}};
        verts[2] = sf::Vertex{{input.getSize().x, input.getSize().y}, sf::Color::Transparent, {1, 1}};
        verts[1] = sf::Vertex{{0, input.getSize().y}, sf::Color::Transparent, {0, 1}};

        rts[1].display();
        states.shader = &add;
        states.blendMode = sf::BlendAdd;
        output.draw(verts, states);
        output.setView(old_view);
    }
};

class LaserEffect : public Effect
{
    sf::RectangleShape laser_rect;
    sf::RectangleShape texture_rect;

    sf::Vector2f r_center;
    float radius;

    sf::RenderTexture t1;
    sf::RenderTexture t2;

    sf::RenderTexture rts[2];

    sf::Shader vert_pass;
    sf::Shader color_switch;
    sf::Shader horiz_pass;

    int time = 0;
    int life_time = 90;

    float length;
    float width;
    float max_width;

public:
    LaserEffect(sf::Vector2f center, float length, float width, float angle, sf::Texture *texture = nullptr)
        : r_center(center), length(length), width(width), max_width(2 * width)
    {
        vert_pass.loadFromFile("../Resources/basic.vert", "../Resources/discreteVert.frag");
        horiz_pass.loadFromFile("../Resources/basic.vert", "../Resources/discreteHoriz.frag");
        color_switch.loadFromFile("../Resources/basic.vert", "../Resources/brigthness.frag");

        rts[0].create(100, 100);
        rts[1].create(100, 100);
        rts[0].setSmooth(true);
        rts[1].setSmooth(true);
        texture_rect.setSize({1, 1});
        texture_rect.setOrigin({0, 1./2.f});
        texture_rect.setPosition(center);
        texture_rect.setFillColor(sf::Color{255, 255, 255, 225});

        texture_rect.setRotation(angle);
        laser_rect.setSize({rts[0].getSize().x * 8. / 9., rts[0].getSize().y * 8. / 9.});
        laser_rect.setPosition({rts[0].getSize().x / 18., rts[0].getSize().y / 18.});
        laser_rect.setFillColor(sf::Color::Green);

        rts[0].clear(sf::Color::Black);
        rts[0].draw(laser_rect);
        rts[0].display();
        rts[1].clear(sf::Color::Transparent);

        laser_rect.setSize({rts[0].getSize().x, rts[0].getSize().y});
        laser_rect.setPosition({0, 0});

        // for (int i = 0; i < 10; ++i)
        // {

        //     vert_pass.setUniform("image", rts[0].getTexture());
        //     // rts[(i+1)%2].clear(sf::Color::Transparent);
        //     laser_rect.setTexture(&rts[0].getTexture());
        //     rts[1].draw(laser_rect, &vert_pass);

        //     horiz_pass.setUniform("image", rts[1].getTexture());
        //     laser_rect.setTexture(&rts[1].getTexture());
        //     rts[0].draw(laser_rect, &horiz_pass);
        // }

        // color_switch.setUniform("image", rts[0].getTexture());
        // laser_rect.setTexture(&rts[0].getTexture());
        // rts[1].draw(laser_rect, &color_switch);
        // // rts[1].clear(sf::Color::White);

        // texture_rect.setTexture(&rts[1].getTexture());
    }

    virtual ~LaserEffect() {}

    virtual bool isDone() override
    {
        return time > life_time;
    }

    virtual void update() override
    {
        float x = time / (float)(life_time);
        auto w = x * (max_width - width) + width;
        auto dy = (max_width - width) / (float)life_time;
        auto angle = texture_rect.getRotation();
        // texture_rect.rotate(-angle);
        // texture_rect.move({0, -dy/2.f});
        texture_rect.setScale({length, w});
        // texture_rect.rotate(angle);
        time++;
    }

    virtual void draw(sf::RenderTarget &window) override
    {
        window.draw(texture_rect);
    }
};

class EffectsManager
{

    enum class ID
    {
        EXPLOSION,
        EXPLOSION2,
        LASER,
    };

    ResourceHolder<sf::Texture, Textures::ID> textures;
    //
    ObjectPool<std::unique_ptr<Effect>, 5000> effects2;
    std::unordered_map<int, std::unique_ptr<Effect>> effects;
    // std::unordered_set<std::unique_ptr<Effect>> effects2;
    int first_free_ind = 0;

public:
    EffectsManager()
    {
        textures.load(Textures::ID::Explosion, "../Resources/explosion.png");
    }

    void onResize(sf::Vector2i new_size){
        // for (auto ind : effects2.active_inds)
        // {
        //     effects2.at(ind)->update();
        //     if (effects2.at(ind)->isDone())
        //     {

        //     }
        // }
    }

    void createExplosion(sf::Vector2f at, float radius)
    {
        auto new_effect = std::make_unique<ExplosionEffect2>(at, radius, textures.get(Textures::ID::Explosion));
        effects2.addObject(std::move(new_effect));
        // effects[first_free_ind] =std::make_unique<ExplosionEffect2>(at, radius, textures.get(Textures::ID::Explosion));
        while (effects.count(first_free_ind) != 0)
        {
            first_free_ind++;
        }
    }

    void createLaser(sf::Vector2f at, float length, float width, float angle)
    {
        auto new_effect = std::make_unique<LaserEffect>(at, length, width, angle);
        effects2.addObject(std::move(new_effect));
        // effects[first_free_ind] =std::make_unique<ExplosionEffect2>(at, radius, textures.get(Textures::ID::Explosion));
        while (effects.count(first_free_ind) != 0)
        {
            first_free_ind++;
        }
    }

    void update()
    {

        std::vector<int> to_destroy;
        // for (auto& [ind, effect] : effects)
        // {
        //     effect->update();
        //     if (effect->isDone())
        //     {
        //         to_destroy.push_back(ind);
        //     }
        // }
        for (auto ind : effects2.active_inds)
        {
            effects2.at(ind)->update();
            if (effects2.at(ind)->isDone())
            {
                to_destroy.push_back(ind);
            }
        }

        for (auto ind : to_destroy)
        {
            // effects2.at(ind%).reset();
            effects2.remove(ind);
            // effects.erase(ind);
            first_free_ind = ind;
        }
        to_destroy.clear();
    }

    void draw(sf::RenderTarget &window)
    {
        for (auto ind : effects2.active_inds)
        {
            auto &wtf = effects2.at(ind);
            effects2.at(ind)->draw(window);
        }
        // for (auto& [ind, effect] : effects)
        // {
        //     effect->draw(window);
        // }
    }
};

class ExplosionEffect
{

    sf::Vector2f r_center;
    float radius;
    int time = 0;
    int animation_time = 0;
    int life_time = 60;
    int animation_period = life_time / 16;

    sf::Vector2i texture_rect_size;
    int ix = 3;
    int iy = 3;

    sf::Shader effect;

    sf::RectangleShape effect_rect;
    sf::Texture explosion_texture;

public:
    ExplosionEffect(sf::Vector2f center, float radius = 3.0)
        : r_center(center), radius(radius), effect_rect({radius, radius})
    {
        effect_rect.setPosition(center - effect_rect.getSize() / 2.f);
        explosion_texture.loadFromFile("../Resources/explosion.png");
        texture_rect_size.x = explosion_texture.getSize().x / 4;
        texture_rect_size.y = explosion_texture.getSize().y / 4;

        auto init_rect = sf::IntRect{{0, 0}, texture_rect_size};
        effect_rect.setTexture(&explosion_texture);

        auto new_rect = sf::IntRect{
            {texture_rect_size.x * ix, texture_rect_size.y * iy},
            {texture_rect_size.x, texture_rect_size.y}};
        effect_rect.setTextureRect(new_rect);

        effect.loadFromFile("../Resources/basic.vert", "../Resources/explosion.frag");
        // effect_rect.setFillColor(sf::Color::Red);
    }
    bool isDone() const
    {
        return time > life_time;
    }

    void update()
    {

        time++;
        if (time - animation_time * animation_period > animation_period)
        {
            animation_time++;
            animateSprite();
        }
    }

    void animateSprite()
    {
        ix--;
        if (ix < 0)
        {
            iy--;
            ix = 3;
        }
        auto new_rect = sf::IntRect{
            {texture_rect_size.x * ix, texture_rect_size.y * iy},
            {texture_rect_size.x, texture_rect_size.y}};
        effect_rect.setTextureRect(new_rect);
    }

    void draw(sf::RenderWindow &window)
    {
        effect.setParameter("time", time);
        // window.draw(effect_rect, &effect);
        window.draw(effect_rect);
    }
};
