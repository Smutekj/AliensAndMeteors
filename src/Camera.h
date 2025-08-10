#pragma once

#include "View.h"

#include "Utils/Vector2.h"

struct PlayerEntity;

struct Camera
{
  enum class MoveState
  {
    FOLLOWING_PLAYER,
    FIXED,
    MOVING_TO_POSITION,
  };
  enum class SizeState
  {
    FollowingPlayer,
    Fixed,
    Resizing,
  };

  Camera(utils::Vector2f center, utils::Vector2f size)
  {
    m_view.setCenter(center);
    m_view.setSize(size);
    m_default_view = m_view;
  }

  void update(float dt, PlayerEntity *player);
  void startMovingTo(utils::Vector2f target, float duration, std::function<void(Camera &)> callback = [](Camera &) {});
  void setPostition(utils::Vector2f pos);
  void setSize(utils::Vector2f size);
  void startChangingSize(utils::Vector2f size, float duration, std::function<void(Camera &)> callback = [](Camera &) {});
  View getView() const;

private:
  void moveToTarget(float dt);
  void resizeToTarget(float dt);
  void followPlayer(float dt, PlayerEntity *p_player);

public:
  MoveState m_view_state = MoveState::FOLLOWING_PLAYER;
  SizeState m_view_size_state = SizeState::FollowingPlayer;

private:
  float m_move_view_time = 0.;
  float m_move_view_duration = 5.;
  float m_max_view_speed = 150.;
  float m_size_change_speed = 1.;
  float m_resize_view_duration;
  float m_resize_view_time = 0.f;

  utils::Vector2f m_view_target_size;
  utils::Vector2f m_view_target;
  utils::Vector2f m_view_velocity = {0};

  View m_default_view;
  View m_view;

  std::function<void(Camera &)> m_on_reaching_target_callback = [](Camera &) {};
};
