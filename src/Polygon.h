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

  sf::Vector2f vel;
  float angle_vel = 0;

  float mass = 1;
  float inertia = 1;
  float radius;

  Polygon(int n_points = 3, sf::Vector2f at = {0, 0}, float radius = 10.f);

  AABB getBoundingRect() const
  {
    auto r = getPosition();
    return {r - getScale(), r + getScale()};
  }

  sf::Vector2f getCenter();

  std::vector<sf::Vector2f> getPointsInWorld() const;
  void move(sf::Vector2f by);
  void rotate(float by);
  void update(float dt);

  bool isCircle()const{
    return points.size() < 3;
  }

  sf::Vector2f getMVTOfSphere(sf::Vector2f center, float radius);

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

