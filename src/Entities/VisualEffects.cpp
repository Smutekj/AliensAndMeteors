#include "VisualEffects.h"
#include "../DrawLayer.h"
#include "../Utils/RandomTools.h"

VisualEffect::VisualEffect(GameWorld *world, TextureHolder &textures)
    : GameObject(world, textures, ObjectType::Count)
{
}

AnimatedSprite::AnimatedSprite(GameWorld *world, TextureHolder &textures)
    : VisualEffect(world, m_textures)
{
}

AnimatedSprite::~AnimatedSprite() {}

void AnimatedSprite::update(float dt)
{
    m_animation->update(dt);
}

void AnimatedSprite::draw(LayersHolder &target)
{
    auto& canvas = target.getCanvas("Unit");
    auto texture = m_textures.get(m_texture_id);
    Sprite the_sprite;
    if(texture)
    {
        the_sprite.setTexture(*texture);
    }
    the_sprite.setPosition(m_pos);
    the_sprite.setRotation(m_angle);
    the_sprite.setScale(m_size/2.f);

    canvas.drawSprite(the_sprite, "Instanced");
}

StarEmitter::StarEmitter(GameWorld *world, TextureHolder &textures)
    : m_particles(*textures.get("Star")), VisualEffect(world, textures)
{
    m_particles.setEmitter([](utils::Vector2f spawn_pos){
        Particle p;
        auto rand_angle =  randf(0, 360);
        p.pos = spawn_pos + 20 * utils::angle2dir(rand_angle);
        p.color = {1,1,1,1};
        p.scale = 10.f;
        p.vel = 30.*utils::angle2dir(rand_angle);
        return p;
    });
    m_particles.setUpdater([](Particle& p, float dt)
    {
        p.pos += p.vel*dt;
        p.color.a -= 255 / (p.life_time / dt);
        if(p.color.a < 0)
        {
            p.color.a = 0;
        }
    });

    m_particles.setRepeat(false);
}

void StarEmitter::update(float dt)
{
    m_time += dt;
    if (m_lifetime > 0.f && m_time > m_lifetime)
    {
        kill();
    }
    m_particles.setSpawnPos(m_pos);
    m_particles.update(dt);
}
void StarEmitter::draw(LayersHolder &layers)
{
    m_particles.draw(layers.getCanvas("Unit"));
}