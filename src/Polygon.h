#pragma once
#include "core.h"

struct Projection1D
{
  float min = std::numeric_limits<float>::max();
  float max = -std::numeric_limits<float>::max();
};

bool inline overlap1D(const Projection1D &p1, const Projection1D &p2)
{
  return p1.min <= p2.max && p2.min <= p1.max;
}

float inline calcOverlap(const Projection1D &p1, const Projection1D &p2)
{
  p1.min <= p2.max &&p2.min <= p1.max;
  float a = p1.max - p2.min;
  float b = p2.max - p1.min;
  assert(overlap1D(p1, p2));
  return std::min(p1.max, p2.max) - std::max(p1.min, p2.min);
  // return std::min(a, b);
}

Projection1D inline projectOnAxis(sf::Vector2f t, const std::vector<sf::Vector2f> &points)
{

  Projection1D projection;
  for (auto &point : points)
  {
    auto proj = dot(t, point);
    projection.min = std::min(projection.min, proj);
    projection.max = std::max(projection.max, proj);
  }
  return projection;
}

struct Polygon : sf::Transformable
{
  std::vector<sf::Vector2f> points;
  sf::Vector2f center;

  sf::Vector2f vel;
  float angle_vel = 0;

  float mass = 1;
  float inertia = 1;
  float radius;

  Polygon(int n_points = 3, sf::Vector2f at = {0, 0}, float radius = 10.f);

  AABB getBoundingRect() const
  {
    auto r = getPosition();
    return {r - 2.5f*getScale(), r + 2.5f*getScale()};
  }

  sf::Vector2f getCenter();

  std::vector<sf::Vector2f> getPointsInWorld() const;
  void move(sf::Vector2f by);
  void rotate(float by);
  void update(float dt);

  sf::Vector2f getMVTOfSphere(sf::Vector2f center, float radius)
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
      Projection1D proj2(proj_sphere - radius, proj_sphere + radius);

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

  void makeShapeFromPolygon(sf::ConvexShape &shape)
  {
    shape.setPointCount(points.size());
    for (int i = 0; i < points.size(); ++i)
    {
      shape.setPoint(i, points[i]);
    }
    shape.setFillColor(sf::Color::Green);
    shape.setScale(getScale());
    shape.setPosition(getPosition());
    shape.setRotation(getRotation());
  }

  void draw(sf::RenderWindow &window)
  {
    sf::ConvexShape shape;
    makeShapeFromPolygon(shape);
    window.draw(shape);
  }
};