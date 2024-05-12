#pragma once

#include "SFML/Graphics.hpp"
#include "ResourceIdentifiers.h"
#include "Utils/GayVector.h"
#include <iostream>
#include <unordered_map>
#include <filesystem>

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
    virtual bool isDone() const = 0;
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

    virtual bool isDone() const override
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

class AnimatedSpriteEffect : public Effect, public sf::Sprite
{

    int time = 0;
    int frame_time = 5;
    int life_time;
    int tex_x;
    int tex_y;
    int n_sprites_x;
    int n_sprites_y;

    int n_repeats = -1;

    int dx = -1;
    int dy = -1;

    bool inverted = false;

    // sf::RectangleShape sprite;
    sf::Vector2i texture_rect_size;

public:
    AnimatedSpriteEffect(const sf::Texture &texture, sf::Vector2f pos, sf::Vector2f scale,
                         int n_sprites_x = 4, int n_sprites_y = 4,
                         int life_time = 120, int n_repeats = 1,
                         bool inverted = false)
        : tex_x(n_sprites_x - 1), tex_y(n_sprites_y - 1),
          n_sprites_x(n_sprites_x), n_sprites_y(n_sprites_y),
          life_time(life_time), n_repeats(n_repeats), inverted(inverted)
    {
        if (inverted)
        {
            dx = 1;
            dy = 1;
            tex_x = 0;
            tex_y = 0;
        }
        texture_rect_size = static_cast<sf::Vector2i>(texture.getSize());
        texture_rect_size.x /= n_sprites_x;
        texture_rect_size.y /= n_sprites_y;

        sf::Vector2f real_scale = {scale.x / texture_rect_size.x, scale.y / texture_rect_size.y}; 
        setScale(real_scale);
        setTexture(texture);
        setTextureRect(getCurrentTextureRect());
        setPosition(pos - scale / 2.f);

        // sprite.setColor(sf::Color::Green);
    }

    void setFrameTime(int new_frame_time)
    {
        frame_time = new_frame_time;
    }

    void setLifeTime(int new_life_time)
    {
        life_time = new_life_time;
        if (n_repeats == 1)
        {
            frame_time = new_life_time / (n_sprites_x * n_sprites_y);
        }
    }

    void setTime(int new_time)
    {
        time = new_time;
        auto frame_ind = (new_time / frame_time);
        tex_x = frame_ind % n_sprites_x;
        tex_y = frame_ind / n_sprites_x;
        setTextureRect(getCurrentTextureRect());
    }

    virtual bool isDone() const
    {
        return time > life_time || n_repeats == 0;
    }

    virtual void update()
    {
        time++;
        if (time % frame_time == frame_time - 1)
        {
            animateSprite();
        }
    }

    virtual void draw(sf::RenderTarget &target)
    {
        target.draw(*this);
    }

private:
    void animateSprite()
    {
        tex_x += dx;
        if (tex_x >= n_sprites_x || tex_x < 0)
        {
            tex_y += dy;
            tex_x = (tex_x + n_sprites_x) % n_sprites_x;
        }
        if (tex_y >= n_sprites_y || tex_y < 0)
        {
            tex_y = (tex_y + n_sprites_y) % n_sprites_y;
        }

        if (inverted)
        {
            if (tex_x == 0 && tex_y == 0)
            {
                n_repeats--;
            }
        }
        else
        {
            if (tex_x == n_sprites_x - 1 && tex_y == n_sprites_y - 1)
            {
                n_repeats--;
            }
        }

        setTextureRect(getCurrentTextureRect());
    }

    sf::IntRect getCurrentTextureRect() const
    {
        return {{texture_rect_size.x * tex_x, texture_rect_size.y * tex_y},
                {texture_rect_size.x, texture_rect_size.y}};
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
        std::filesystem::path path("../Resources/basic.vert");
        vert_pass.loadFromFile(path.generic_string(), "../Resources/discreteVert.frag");
        horiz_pass.loadFromFile("../Resources/basic.vert", "../Resources/discreteHoriz.frag");
        color_switch.loadFromFile("../Resources/basic.vert", "../Resources/brigthness.frag");

        rts[0].create(100, 100);
        rts[1].create(100, 100);
        rts[0].setSmooth(true);
        rts[1].setSmooth(true);
        texture_rect.setSize({1, 1});
        texture_rect.setOrigin({0, 1. / 2.f});
        texture_rect.setPosition(center);
        texture_rect.setFillColor(sf::Color{255, 255, 255, 225});
        texture_rect.setRotation(angle);
    }

    virtual ~LaserEffect() {}

    virtual bool isDone() const override
    {
        return time > life_time;
    }

    virtual void update() override
    {
        float x = time / (float)(life_time);
        float x4 = x*x*x*x;
        auto w = x4 * (max_width - width) + width;
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
        textures.load(Textures::ID::Explosion2, "../Resources/explosion2.png");
        textures.load(Textures::ID::Bomb, "../Resources/bomb.png");
    }

    void onResize(sf::Vector2i new_size)
    {
    }

    void createExplosion(sf::Vector2f at, float radius,
                         Textures::ID explosion_type = Textures::ID::Explosion, int life_time = 180)
    {

        auto dice_throw = rand() % 2;
        if (explosion_type == Textures::ID::Explosion)
        {

            auto new_effect = std::make_unique<ExplosionEffect2>(at, radius, textures.get(explosion_type));
            effects2.addObject(std::move(new_effect));
        }
        else if (explosion_type == Textures::ID::Explosion2)
        {

            auto new_effect = std::make_unique<AnimatedSpriteEffect>(textures.get(explosion_type),
                                                                     at, sf::Vector2f{radius, radius},
                                                                     12, 1, life_time, 1, true);
            effects2.addObject(std::move(new_effect));
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
            effects2.remove(ind);
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
    }
};
