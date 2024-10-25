#include "VisualEffects.h"

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
}

StarEmitter::StarEmitter(GameWorld *world, TextureHolder &textures)
    : m_particles(*textures.get("Star")), VisualEffect(world, textures)
{
    // m_particles.setColor(sf::Color::White);
    // m_particles.setVel(30.f);
    // m_particles.setTexture(textures.get(Textures::ID::Star));
    // m_particles.setRepeat(false);
    // m_particles.setSize(10.f);
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
void StarEmitter::draw(LayersHolder &target)
{
    // m_particles.draw(target);
}