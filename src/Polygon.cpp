#include "Polygon.h"


  Polygon::Polygon(int n_points, sf::Vector2f at, float radius) : points(n_points), radius(radius)
  {
    for (int i = 0; i < n_points; ++i)
    {
      sf::Vector2f new_pos;
      new_pos.x = std::cos(i * 2.f * M_PI / n_points);
      new_pos.y = std::sin(i * 2.f * M_PI / n_points);
      points[i] = new_pos;
    }

    setScale({radius, radius});
    setPosition(at);
  }

  sf::Vector2f Polygon::getCenter()
  {
    sf::Vector2f com_a = {0, 0};
    for (auto &point : points)
    {
      com_a += point;
    }
    com_a /= (float)points.size();
    return com_a;
  }

  std::vector<sf::Vector2f> Polygon::getPointsInWorld() const
  {
    const auto& trans = getTransform();

    std::vector<sf::Vector2f> world_points;
    for (int i = 0; i < points.size(); ++i)
    {
      world_points.push_back(trans.transformPoint(points[i]));
    }
    return world_points;
  }

  void Polygon::move(sf::Vector2f by)
  {
    setPosition(getPosition() + by);
  }

  void Polygon::rotate(float by)
  {
    setRotation(getRotation() + by * 180. / M_PI);
  }

  void Polygon::update(float dt)
  {

    angle_vel = std::min(angle_vel, 0.02f);
    angle_vel = std::max(angle_vel, -0.02f);
    move(vel * dt);
    rotate(angle_vel);

  }


  sf::Vector2f Polygon::getMVTOfSphere(sf::Vector2f center, float radius)
  {

    const auto &points_world = getPointsInWorld();

    int next = 1;
    const auto n_points1 = points.size();

    float min_overlap = std::numeric_limits<float>::max();
    sf::Vector2f min_axis;
    for (int curr = 0; curr < n_points1; ++curr)
    {
      auto t1 = points_world.at(next) - points_world[curr]; //! line perpendicular to current polygon edge
      sf::Vector2f n1 = {t1.y, -t1.x};
      n1 /= norm(n1);
      auto proj1 = projectOnAxis(n1, points_world);
      float proj_sphere = dot(n1, center);
      Projection1D proj2({proj_sphere - radius, proj_sphere + radius});

      if (!overlap1D(proj1, proj2))
      {
        return {0, 0};
      }
      else
      {
        auto overlap = calcOverlap(proj1, proj2);
        if (overlap < min_overlap)
        {
          min_overlap = overlap;
          min_axis = n1;
        }
      }
      next++;
      if (next == n_points1)
      {
        next = 0;
      }
    }
    return min_axis;
  }
