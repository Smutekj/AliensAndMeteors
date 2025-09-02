#include "Camera.h"

#include "Entities/Player.h"

#include "PostOffice.h"

Camera::Camera(utils::Vector2f center, utils::Vector2f size, PostOffice &messanger)
    : m_messanger(messanger)
{
    m_view.setCenter(center);
    m_view.setSize(size);
    m_default_view = m_view;
}

void Camera::setSpeed(float speed)
{
    m_max_view_speed = speed;
}

void Camera::startFollowingPath(std::deque<utils::Vector2f> path, float duration, std::function<void(Camera &)> callback)
{
    m_path = path;
    m_on_reaching_target_callback = callback;
    
    m_move_state = MoveState::FollowingPath;
    m_move_view_duration = duration;
    m_move_view_time = 0.f;
    m_view_target = path.at(0);
    path.pop_front();

}
void Camera::startMovingTo(utils::Vector2f target, float duration, std::function<void(Camera &)> callback)
{
    m_on_reaching_target_callback = callback;

    m_move_state = MoveState::MovingToPosition;
    m_move_view_duration = duration;
    m_move_view_time = 0.f;
    m_view_target = target;
}
void Camera::startChangingSize(utils::Vector2f target, float duration, std::function<void(Camera &)> callback)
{
    m_on_reaching_target_callback = callback;

    m_view_size_state = SizeState::Resizing;
    m_resize_view_duration = duration;
    m_resize_view_time = 0.f;
    m_view_target_size = target;
}

void Camera::resizeToTarget(float dt)
{
    m_move_view_time += dt;

    utils::Vector2f dr_to_target = m_view_target_size - m_view.getSize();
    float dist_to_target = std::max(utils::norm(dr_to_target), 0.001f);
    float view_speed = 50.;

    if (dist_to_target < 10)
    {
        m_view_size_state = SizeState::FollowingPlayer; //! by default start Following Player if neede can be overriden in callback
        m_on_reaching_target_callback(*this);
        view_speed = 0.;
        m_view_velocity = {0};
    }

    m_view_velocity = dr_to_target / dist_to_target * view_speed;
    utils::truncate(m_view_velocity, m_max_view_speed);

    m_view.setSize(m_view.getSize() + m_view_velocity * dt);
}

bool Camera::moveToTarget(float dt)
{
    m_move_view_time += dt;

    utils::Vector2f dr_to_target = m_view_target - m_view.getCenter();
    float dist_to_target = utils::norm(dr_to_target);
    float speed = m_max_view_speed;
    if(dist_to_target < 20)
    {
        m_view_velocity = {0};
        return true;
    }
    
    m_view_velocity = dr_to_target / dist_to_target * m_max_view_speed;
    utils::truncate(m_view_velocity, m_max_view_speed);
    m_view.setCenter(m_view.getCenter() + m_view_velocity * dt);
    return false;
}

void Camera::update(float dt, PlayerEntity *player)
{
    if (m_move_state == MoveState::FollowingPlayer && m_view_size_state == SizeState::FollowingPlayer)
    {
        followPlayer(dt, player);
    }
    else if (m_move_state == MoveState::MovingToPosition)
    {
        if(moveToTarget(dt))
        {
            m_move_state = MoveState::FollowingPlayer; //! by default start Following Player if neede can be overriden in callback
            m_on_reaching_target_callback(*this);
        }
    }
    else if (m_move_state == MoveState::FollowingPath)
    {
        followPath(dt);
    }
    if (m_view_size_state == SizeState::Resizing)
    {
        resizeToTarget(dt);
    }

    if (!m_view.contains(player->getPosition()))
    {
        m_messanger.send(EntityLeftViewEvent{player->getId(), m_view});
    }
}
View Camera::getView() const
{
    return m_view;
}

void Camera::followPath(float dt)
{
    
    bool reached_next_spot = moveToTarget(dt);
    if(reached_next_spot) //! reached target spot and there is more
    {
        if(m_path.empty())
        {
            m_move_state = MoveState::FollowingPlayer;
            m_on_reaching_target_callback(*this);
            return;
        }
        m_view_target = m_path.front();
        m_path.pop_front();
    }
}

void Camera::followPlayer(float dt, PlayerEntity *m_player)
{
    const utils::Vector2f view_size = m_view.getSize();

    //! look from higher distance when boosting
    float booster_ratio = m_player->speed / m_player->max_speed;
    m_view.setSize(m_default_view.getSize() * (1.f + booster_ratio));

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

    m_view_acc *= 200;
    m_view_velocity += m_view_acc * dt;
    utils::truncate(m_view_velocity, m_max_view_speed);
    m_view.setCenter(m_view.getCenter() + m_view_velocity * dt);
}
