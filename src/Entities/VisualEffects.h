#pragma once

#include "../GameObject.h"
#include "../Animation.h"
#include <Particles.h>

class LayersHolder;

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
    virtual void draw(LayersHolder &target) override{}
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
    std::string m_texture_id;
};

class AnimatedSprite : public VisualEffect
{

public:
    AnimatedSprite(GameWorld *world, TextureHolder &textures);
    virtual ~AnimatedSprite() override;

    virtual void update(float dt) override;
    virtual void onCreation() override{}
    virtual void onDestruction() override{}
    virtual void draw(LayersHolder &target) override;
    virtual void onCollisionWith(GameObject &obj, CollisionData &c_data) override{}

    // void setType(std::string type);
private:
    std::unique_ptr<Animation> m_animation;

};

class StarEmitter : public VisualEffect
{

public:
    StarEmitter(GameWorld *world, TextureHolder &textures);

    virtual void update(float dt) final;
    virtual void draw(LayersHolder &target) override;
    virtual void onCreation() override {}
    virtual void onDestruction() override {}
    virtual void onCollisionWith(GameObject &obj, CollisionData &c_data) override {}

private:
    TexturedParticles m_particles;
};