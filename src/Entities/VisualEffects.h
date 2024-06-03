#pragma once

#include "../GameObject.h"
#include "../Particles.h"
#include "../Animation.h"

class VisualEffect : public GameObject
{

public:
    VisualEffect(GameWorld *world, TextureHolder &textures);
    virtual ~VisualEffect() = default;

    virtual void update(float dt) override
    {
        m_time += dt;
        if (m_time > m_lifetime)
        {
            kill();
        }
    }
    virtual void draw(sf::RenderTarget &target) override{}
    virtual void onCreation() override {}
    virtual void onDestruction() override {}
    virtual void onCollisionWith(GameObject &obj, CollisionData &c_data) override {}

    void setLifeTime(float lifetime)
    {
        m_lifetime = lifetime;
    }

protected:
    float m_time = 0.f;
    float m_lifetime = -1.f;
    Textures::ID m_texture_id;
};

class AnimatedSprite : public VisualEffect
{

public:
    AnimatedSprite(GameWorld *world, TextureHolder &textures)
    : VisualEffect(world, m_textures)
    {

    }


    virtual ~AnimatedSprite() override;


    virtual void update(float dt) override
    {
        m_animation->update(dt);
    }
    virtual void onCreation() override{}
    virtual void onDestruction() override{}
    virtual void draw(sf::RenderTarget &target) override{}
    virtual void onCollisionWith(GameObject &obj, CollisionData &c_data) override;

    void setType(Textures::ID type);
private:
    std::unique_ptr<Animation> m_animation;

};

class StarEmitter : public VisualEffect
{

public:
    StarEmitter(GameWorld *world, TextureHolder &textures)
        : m_particles(textures.get(Textures::ID::Star)), VisualEffect(world, textures)
    {
        m_particles.setColor(sf::Color::White);
        m_particles.setVel(30.f);
        m_particles.setTexture(textures.get(Textures::ID::Star));
        m_particles.setRepeat(false);
        m_particles.setSize(10.f);
    }

    virtual void update(float dt) final
    {
        m_time += dt;
        if (m_lifetime > 0.f && m_time > m_lifetime)
        {
            kill();
        }
        m_particles.setSpawnPos(m_pos);
        m_particles.update(dt);
    }
    virtual void draw(sf::RenderTarget &target) override
    {
        m_particles.draw(target);
    }
    virtual void onCreation() override {}
    virtual void onDestruction() override {}
    virtual void onCollisionWith(GameObject &obj, CollisionData &c_data) override {}

private:
    TexturedParticles m_particles;
};