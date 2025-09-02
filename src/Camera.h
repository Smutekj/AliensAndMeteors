#pragma once

#include "View.h"

#include "Utils/Vector2.h"
#include <queue>

struct PlayerEntity;
class PostOffice;

struct Camera
{
  enum class MoveState
  {
    FollowingPlayer,
    Fixed,
    MovingToPosition,
    FollowingPath,
  };
  enum class SizeState
  {
    FollowingPlayer,
    Fixed,
    Resizing,
  };

  Camera(utils::Vector2f center, utils::Vector2f size, PostOffice &messanger);

  void update(float dt, PlayerEntity *player);
  void startMovingTo(utils::Vector2f target, float duration, std::function<void(Camera &)> callback = [](Camera &) {});
  void setPostition(utils::Vector2f pos);
  void setSize(utils::Vector2f size);
  void setSpeed(float speed);
  void startFollowingPath(std::deque<utils::Vector2f> path, float duration, std::function<void(Camera&)> callback = [](Camera&){});
  void followPath(float dt);
  void startChangingSize(utils::Vector2f size, float duration, std::function<void(Camera &)> callback = [](Camera &) {});
  View getView() const;

private:
  bool moveToTarget(float dt);
  void resizeToTarget(float dt);
  void followPlayer(float dt, PlayerEntity *p_player);

public:
  MoveState m_move_state = MoveState::FollowingPlayer;
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

  PostOffice& m_messanger;

  std::function<void(Camera &)> m_on_reaching_target_callback = [](Camera &) {};


  std::deque<utils::Vector2f> m_path;
};
