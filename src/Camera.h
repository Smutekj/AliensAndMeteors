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

  Camera(utils::Vector2f center, utils::Vector2f size)
  {
    m_view.setCenter(center);
    m_view.setSize(size);
    m_default_view = m_view;
  }

  void update(float dt, PlayerEntity *player);
  void startMovingTo(utils::Vector2f target, float duration);
  View getView() const;

private:
  void moveToTarget(float dt);
  void followPlayer(float dt, PlayerEntity *p_player);

private:
  float m_move_view_time = 0.;
  float m_move_view_duration = 5.;
  float m_max_view_speed = 50.;

  utils::Vector2f m_view_target;
  utils::Vector2f m_view_velocity = {0};

  View m_default_view;
  View m_view;

  MoveState m_view_state = MoveState::FOLLOWING_PLAYER;
};
