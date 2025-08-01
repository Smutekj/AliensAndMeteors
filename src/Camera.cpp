#include "Camera.h"

#include "Entities/Player.h"


void Camera::startMovingTo(utils::Vector2f target, float duration)
{
    m_view_state = MoveState::MOVING_TO_POSITION;
    m_move_view_duration = duration;
    m_move_view_time = 0.f;
    m_view_target = target;
}
void Camera::moveToTarget(float dt)
{
    m_move_view_time += dt;

    float view_speed = utils::norm(m_view_velocity);
    float view_acceleration = 50.;

    utils::Vector2f dr_to_target = m_view_target - m_view.getCenter();
    float dist_to_target = utils::norm(dr_to_target);

    if (dist_to_target > 20)
    {
        view_speed = 100.f;
    }
    else if (dist_to_target > 2)
    {
        view_speed -= view_speed * dt;
    }
    else
    {
        view_speed = 0.;
        m_view_velocity = {0};
        m_view_state = MoveState::FOLLOWING_PLAYER;
    }

    m_view_velocity += dr_to_target / dist_to_target * view_speed * dt;
    utils::truncate(m_view_velocity, m_max_view_speed);

    m_view.setCenter(m_view.getCenter() + m_view_velocity * dt);
}

void Camera::update(float dt, PlayerEntity *player)
{
  if (m_view_state == MoveState::FOLLOWING_PLAYER)
  {
    followPlayer(dt, player);
  }
  else if (m_view_state == MoveState::MOVING_TO_POSITION)
  {
    moveToTarget(dt);
  }
}
View Camera::getView() const
{
  return m_view;
}


void Camera::followPlayer(float dt, PlayerEntity *m_player)
{
    const utils::Vector2f view_size = m_view.getSize();

    //! look from higher distance when boosting
    float booster_ratio = m_player->speed / m_player->max_speed;
    m_view.setSize(m_default_view.getSize() * (1 + booster_ratio / 2.f));

    auto threshold = m_view.getSize() / 2.f - m_view.getSize() / 3.f;
    auto dx = m_player->getPosition().x - m_view.getCenter().x;
    auto dy = m_player->getPosition().y - m_view.getCenter().y;

    m_view_velocity = {0};
    utils::Vector2f m_view_acc = {0};

    //! move view when approaching sides
    if (dx > threshold.x)
    {
        m_view_acc.x = dx - threshold.x;
    }
    else if (dx < -threshold.x)
    {
        m_view_acc.x = dx + threshold.x;
    }
    if (dy > threshold.y)
    {
        m_view_acc.y = dy - threshold.y;
    }
    else if (dy < -threshold.y)
    {
        m_view_acc.y = dy + threshold.y;
    }

    m_view_acc *= 100.;
    m_view_velocity += m_view_acc * dt;
    utils::truncate(m_view_velocity, m_max_view_speed);
    m_view.setCenter(m_view.getCenter() + m_view_velocity * dt);
}
