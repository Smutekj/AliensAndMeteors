#include "GameObject.h"

#include "Polygon.h"
#include "GameWorld.h"

GameObject::GameObject(GameWorld* world, TextureHolder& textures, ObjectType type)
 :
  m_world(world), m_textures(textures), m_type(type)
  {}

void GameObject::updateAll(float dt)
{
    if (m_collision_shape)
    {
        m_collision_shape->setPosition(m_pos);
        m_collision_shape->setRotation(m_angle);
    }

    update(dt);
}

    void GameObject::move(sf::Vector2f by)
    {
        m_pos += by;
        if(m_collision_shape)
        {
            m_collision_shape->setPosition(m_pos);
        }
    }

bool GameObject::collides() const
{
    if(m_collision_shape)
    {
        return true;
    }
    return false;
}

const sf::Vector2f& GameObject::getPosition() const
{
    return m_pos;
}

float GameObject::getAngle() const
{
    return m_angle;
}

Polygon &GameObject::getCollisionShape()
{
    return *m_collision_shape;
}

int GameObject::getId() const
{
    return m_id;
}
ObjectType GameObject::getType() const
{
    return m_type;
}
