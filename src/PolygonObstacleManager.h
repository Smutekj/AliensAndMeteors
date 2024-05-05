#pragma once

#include "core.h"
#include "Polygon.h"
#include "Utils/RandomTools.h"
#include "SFML/Graphics/ConvexShape.hpp"
#include "BVH.h"
#include "Utils/GayVector.h"

#include <chrono>
#include <limits>
#include <iostream>
#include <memory>

struct CollisionData
{
  sf::Vector2f separation_axis;
  float minimum_translation = -1;
  bool belongs_to_a = true;
};

int inline furthestVertex(sf::Vector2f separation_axis, const std::vector<sf::Vector2f> &points)
{
  float max_dist = -std::numeric_limits<float>::max();
  int index = -1;
  for (int i = 0; i < points.size(); ++i)
  {
    auto dist = dot(points[i], separation_axis);
    if (dist > max_dist)
    {
      index = i;
      max_dist = dist;
    }
  }

  return index;
}

struct CollisionFeature
{
  sf::Vector2f best_vertex;
  Edgef edge;
};

CollisionData inline calcCollisionData(const std::vector<sf::Vector2f> &points1,
                                       const std::vector<sf::Vector2f> &points2)
{
  CollisionData collision_result;

  int next = 1;
  const auto n_points1 = points1.size();
  const auto n_points2 = points2.size();

  Edgef contact_edge;

  float min_overlap = std::numeric_limits<float>::max();
  sf::Vector2f &min_axis = collision_result.separation_axis;
  for (int curr = 0; curr < n_points1; ++curr)
  {

    auto t1 = points1[next] - points1[curr]; //! line perpendicular to current polygon edge
    sf::Vector2f n1 = {t1.y, -t1.x};
    n1 /= norm(n1);
    auto proj1 = projectOnAxis(n1, points1);
    auto proj2 = projectOnAxis(n1, points2);

    if (!overlap1D(proj1, proj2))
    {
      collision_result.minimum_translation = -1;
      return collision_result;
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
  next = 1;
  for (int curr = 0; curr < n_points2; ++curr)
  {

    auto t1 = points2[next] - points2[curr]; //! line perpendicular to current polygon edge
    sf::Vector2f n1 = {t1.y, -t1.x};
    n1 /= norm(n1);
    auto proj1 = projectOnAxis(n1, points1);
    auto proj2 = projectOnAxis(n1, points2);

    if (!overlap1D(proj1, proj2))
    {
      collision_result.minimum_translation = -1;
      return collision_result;
    }
    else
    {
      auto overlap = calcOverlap(proj1, proj2);
      if (overlap < min_overlap)
      {
        min_overlap = overlap;
        min_axis = n1;
        collision_result.belongs_to_a = false;
      }
    }

    next++;
    if (next == n_points2)
    {
      next = 0;
    }
  }

  collision_result.minimum_translation = min_overlap;
  return collision_result;
}

CollisionFeature inline obtainFeatures(const sf::Vector2f axis, std::vector<sf::Vector2f> &points)
{

  const auto n_points = points.size();
  auto furthest_v_ind1 = furthestVertex(axis, points);

  auto v1 = points[furthest_v_ind1];
  auto v1_next = points[(furthest_v_ind1 + 1) % n_points];
  auto v1_prev = points[(furthest_v_ind1 - 1 + n_points) % n_points];

  auto from_next = v1 - v1_next;
  auto from_prev = v1 - v1_prev;
  from_next /= norm(from_next);
  from_prev /= norm(from_prev);
  Edgef best_edge;
  if (dot(from_prev, axis) <= dot(from_next, axis))
  {
    best_edge = Edgef(v1_prev, v1);
  }
  else
  {
    best_edge = Edgef(v1, v1_next);
  }
  CollisionFeature feature = {v1, best_edge};
  return feature;
}

std::vector<sf::Vector2f> inline clip(sf::Vector2f v1, sf::Vector2f v2, sf::Vector2f n, float overlap)
{

  std::vector<sf::Vector2f> cp;
  float d1 = dot(v1, n) - overlap;
  float d2 = dot(v2, n) - overlap;
  // if either point is past o along n
  // then we can keep the point
  if (d1 >= 0.0)
    cp.push_back(v1);
  if (d2 >= 0.0)
    cp.push_back(v2);
  // finally we need to check if they
  // are on opposing sides so that we can
  // compute the correct point
  if (d1 * d2 < 0.0)
  {
    // if they are on different sides of the
    // offset, d1 and d2 will be a (+) * (-)
    // and will yield a (-) and therefore be
    // less than zero
    // get the vector for the edge we are clipping
    sf::Vector2f e = v2 - v1;
    // compute the location along e
    float u = d1 / (d1 - d2);
    e *= u;
    e += v1;
    // add the point
    cp.push_back(e);
  }
  return cp;
}

std::pair<std::vector<sf::Vector2f>, bool> inline clipEdges(CollisionFeature &ref_features, CollisionFeature &inc_features, sf::Vector2f n)
{

  auto &ref_edge = ref_features.edge;
  auto &inc_edge = inc_features.edge;

  bool flip = false;
  auto wtf_ref = std::abs(dot(ref_edge.t, n));
  auto wtf_inc = std::abs(dot(inc_edge.t, n));
  if (wtf_ref <= wtf_inc)
  {
  }
  else
  {
    std::swap(ref_features, inc_features);
    flip = true;
  }

  sf::Vector2f ref_v = ref_edge.t;

  double o1 = dot(ref_v, ref_edge.from);
  // clip the incident edge by the first
  // vertex of the reference edge
  auto cp = clip(inc_edge.from, inc_edge.to(), ref_v, o1);
  auto cp_new = cp;
  // if we dont have 2 points left then fail
  bool fucked = false;
  if (cp.size() < 2)
  {
    return {};
  }

  // clip whats left of the incident edge by the
  // second vertex of the reference edge
  // but we need to clip in the opposite direction
  // so we flip the direction and offset
  double o2 = dot(ref_v, ref_edge.to());
  cp = clip(cp[0], cp[1], -ref_v, -o2);
  // if we dont have 2 points left then fail
  if (cp.size() < 2)
    return {};

  // get the reference edge normal
  sf::Vector2f refNorm = {-ref_v.y, ref_v.x};
  refNorm /= norm(refNorm);
  // if we had to flip the incident and reference edges
  // then we need to flip the reference edge normal to
  // clip properly
  // if (flip)
  // refNorm *= -1.f;
  // get the largest depth
  double max = dot(refNorm, ref_features.best_vertex);
  // make sure the final points are not past this maximum

  std::vector<float> depths(2);
  depths[0] = dot(refNorm, cp.at(0)) - max;
  depths[1] = dot(refNorm, cp.at(1)) - max;
  // if (depths[0] < 0.0f && depths[1] < 0.f){
  //   return {};
  // }
  if (depths[0] < 0.0f)
  {
    cp.erase(cp.begin());
  }
  if (depths[1] < 0.0f)
  {
    cp.pop_back();
  }
  // return the valid points
  return {cp, flip};
}

void inline collidePolygons(Polygon &pa, Polygon &pb)
{

  auto points_a = pa.getPointsInWorld();
  auto points_b = pb.getPointsInWorld();
  auto c_data = calcCollisionData(points_a, points_b);
  if (c_data.minimum_translation < 0) //! no intersection
  {
    return;
  }
  sf::Vector2f com_a = {0, 0};
  for (auto &point : points_a)
  {
    com_a += point;
  }
  com_a /= (float)pa.points.size();

  sf::Vector2f com_b = {0, 0};
  for (auto &point : points_b)
  {
    com_b += point;
  }
  com_b /= (float)pb.points.size();

  c_data.belongs_to_a = dot((com_b - com_a), c_data.separation_axis) > 0;
  if (!c_data.belongs_to_a)
  {
    c_data.separation_axis *= -1.f;
  }

  auto col_feats1 = obtainFeatures(c_data.separation_axis, points_a);
  auto col_feats2 = obtainFeatures(-c_data.separation_axis, points_b);

  auto [clipped_edge, flipped] = clipEdges(col_feats1, col_feats2, c_data.separation_axis);
  if (clipped_edge.size() == 0)
  {
    return;
  }

  sf::Vector2f cont_point = {0, 0};
  for (auto ce : clipped_edge)
  {
    cont_point += ce;
  }
  if (clipped_edge.size() == 0)
  {
    cont_point = (com_a + com_b) / 2.f;
  }
  else
  {
    cont_point /= (float)clipped_edge.size();
  }

  auto n = c_data.separation_axis;
  //
  pa.move(-c_data.separation_axis * c_data.minimum_translation * 1.05f);
  //
  if (c_data.belongs_to_a)
  {
    // pb.move(c_data.separation_axis * c_data.minimum_translation * 1.05f);
  }
  else
  {
    // pa.move(c_data.separation_axis * c_data.minimum_translation * 1.05f);
  }
  auto v_rel = pa.vel - pb.vel;
  auto v_reln = dot(v_rel, n);

  if (v_reln > 0)
  {
    // pa.move(-c_data.separation_axis * c_data.minimum_translation * 1.05f);
  }
  else
  {
    // pa.move(c_data.separation_axis * c_data.minimum_translation * 1.05f);
  }

  points_a = pa.getPointsInWorld();
  points_b = pb.getPointsInWorld();
  c_data = calcCollisionData(points_a, points_b);
  // assert(c_data.minimum_translation < 0);

  float e = 1;
  float u_ab = 1. / pa.mass + 1. / pb.mass;

  auto r_cont_coma = cont_point - com_a;
  auto r_cont_comb = cont_point - com_b;

  sf::Vector2f r_cont_coma_perp = {r_cont_coma.y, -r_cont_coma.x};
  sf::Vector2f r_cont_comb_perp = {r_cont_comb.y, -r_cont_comb.x};

  float ran = dot(r_cont_coma_perp, n);
  float rbn = dot(r_cont_comb_perp, n);

  float u_ab_rot = ran * ran / pa.inertia + rbn * rbn / pb.inertia;

  float j_factor = -(1 + e) * v_reln / (u_ab + u_ab_rot);

  pa.angle_vel += ran * j_factor / pa.inertia;
  pb.angle_vel -= rbn * j_factor / pb.inertia;
  // // float j_factor = -(1 + e) * v_reln / (u_ab);
  pa.vel += j_factor / pa.mass * n;
  pb.vel -= j_factor / pb.mass * n;
}

Polygon inline generateRandomConvexPolygon(int n)
{
  // Generate two lists of random X and Y coordinates
  std::vector<float> xPool(0);
  std::vector<float> yPool(0);

  for (int i = 0; i < n; i++)
  {
    xPool.push_back(randf(-1, 1));
    yPool.push_back(randf(-1, 1));
  }

  // Sort them
  std::sort(xPool.begin(), xPool.end());
  std::sort(yPool.begin(), yPool.end());

  // Isolate the extreme points
  auto minX = xPool.at(0);
  auto maxX = xPool.at(n - 1);
  auto minY = yPool.at(0);
  auto maxY = yPool.at(n - 1);

  // Divide the interior points into two chains & Extract the vector components
  std::vector<float> xVec(0);
  std::vector<float> yVec(0);

  float lastTop = minX, lastBot = minX;

  for (int i = 1; i < n - 1; i++)
  {
    double x = xPool.at(i);

    if (rand() % 2)
    {
      xVec.push_back(x - lastTop);
      lastTop = x;
    }
    else
    {
      xVec.push_back(lastBot - x);
      lastBot = x;
    }
  }

  xVec.push_back(maxX - lastTop);
  xVec.push_back(lastBot - maxX);

  float lastLeft = minY, lastRight = minY;

  for (int i = 1; i < n - 1; i++)
  {
    double y = yPool.at(i);

    if (rand() % 2)
    {
      yVec.push_back(y - lastLeft);
      lastLeft = y;
    }
    else
    {
      yVec.push_back(lastRight - y);
      lastRight = y;
    }
  }

  yVec.push_back(maxY - lastLeft);
  yVec.push_back(lastRight - maxY);

  // Randomly pair up the X- and Y-components
  std::random_shuffle(yVec.begin(), yVec.end());

  // Combine the paired up components into vectors
  std::vector<sf::Vector2f> vec;

  for (int i = 0; i < n; i++)
  {
    vec.emplace_back(xVec.at(i), yVec.at(i));
  }

  // Sort the vectors by angle
  std::sort(vec.begin(), vec.end(), [](const auto &p1, const auto &p2)
            { return std::atan2(p1.y, p1.x) < std::atan2(p2.y, p2.x); });

  // Lay them end-to-end
  double x = 0, y = 0;
  double minPolygonX = 0;
  double minPolygonY = 0;
  std::vector<sf::Vector2f> points;

  for (int i = 0; i < n; i++)
  {
    points.push_back({x, y});

    x += vec.at(i).x;
    y += vec.at(i).y;

    minPolygonX = std::min(minPolygonX, x);
    minPolygonY = std::min(minPolygonY, y);
  }

  // Move the polygon to the original min and max coordinates
  double xShift = minX - minPolygonX;
  double yShift = minY - minPolygonY;

  for (int i = 0; i < n; i++)
  {
    auto p = points.at(i);
    points.at(i) += sf::Vector2f{xShift, yShift};
  }
  Polygon p;
  p.points = points;

  return p;
}

struct Collidable
{
  AABB bounding_rect;
  int entity_ind;

  virtual void update(float dt) = 0;
};

struct Meteor : public Collidable
{
  Polygon shape;

  Meteor(int entity_ind, sf::Vector2f at, float radius, std::vector<sf::Vector2f> &points)
  {
    shape.points = points;
    shape.setPosition(at);
    shape.radius = radius;
    shape.setScale({radius, radius});
  }

  void update(float dt)
  {
    shape.update(dt);

    bounding_rect = shape.getBoundingRect(); //! this could be done once in couple of frames if we set max vel
  }
};

// struct Coin : public Collidable{

//   void update(float dt){
//     shape.update(dt);

//     bounding_rect = shape.getBoundingRect(); //! this could be done once in couple of frames if we set max vel
//   }
// };


struct EnviromentEntity {
    Polygon shape;
    enum class Type{
      METEOR,
      COIN,
      FUEL,
      SPEED,
      STATION,
      TELEPORT,
    };
    Type type;
    int entity_ind;
    bool is_static = false;
};

constexpr int N_MAX_METEORS = 5000;

struct PolygonObstacleManager
{

  typedef std::shared_ptr<EnviromentEntity> EntityPtr;

  ObjectPool<std::shared_ptr<EnviromentEntity>, 5000> actors;

  ObjectPool<std::unique_ptr<Collidable>, 10000> collidables;

  BoundingVolumeTree collision_tree;

  ObjectPool<Polygon, N_MAX_METEORS> meteors;

  ObjectPool<sf::ConvexShape, N_MAX_METEORS> drawables;

  std::vector<std::pair<int, int>> collisions;
  std::vector<std::pair<EntityPtr, EntityPtr>> collisions2;

struct pair_hash
    {
        inline std::size_t operator()(const std::pair<int, int> &v) const
        {
            return v.first * 31 + v.second;
        }
    };
    std::unordered_set<std::pair<int, int>, pair_hash> collided;

  PolygonObstacleManager(int n_meteors = 10);

  void update(float dt);
  void update2(float dt)
  {
    for (auto entity_ind : actors.active_inds)
    {
      auto &c1 = actors.at(entity_ind);
      if(!c1->is_static){
        c1->shape.update(dt);
      }

      auto possible_colliders = collision_tree.findIntersectingLeaves(c1->shape.getBoundingRect());
      for (auto entity_ind_2 : possible_colliders)
      {
        if (entity_ind == entity_ind_2)
        {
          continue;
        }

         std::pair<int, int> collision_inds = {std::min(entity_ind_2, entity_ind), std::max(entity_ind_2, entity_ind)};
         auto& c2 = actors.at(entity_ind_2);
            if (collided.count(collision_inds) == 0)
            {
                // collideEntities(*c1, *c2);
                collidePolygons(c1->shape, c2->shape);
                collided.insert(collision_inds);
            }

        collisions.push_back({entity_ind, entity_ind_2});
      }
    }

    for(auto& [e1, e2] : collided){
      
    }

    collided.clear();
  }

  // void collideEntities(EnviromentEntity& e1, EnviromentEntity& e2){
  //   if(e1.type == EnviromentEntity::Type::METEOR){
  //     switch(e2.type){
  //       case EnviromentEntity::Type::METEOR:
  //         break;
  //       case EnviromentEntity::Type::COIN:
  //         break;
  //       case EnviromentEntity::Type:::
  //         break;
  //       case EnviromentEntity::Type::METEOR:
  //         break;
  //     }
  //   }
    
  // }

  void draw(sf::RenderWindow &window);

  std::vector<Polygon *> getNearestMeteors(sf::Vector2f r, float radius);

  std::vector<int> getNearestMeteorInds(sf::Vector2f lower_left, sf::Vector2f upper_right);

  Polygon createRandomMeteor();
  void addRandomMeteorAt(sf::Vector2f position);

  void addMeteor(Polygon &new_meteor);

  void destroyMeteor(int entity_ind);
};

struct CollisionHandler
{

  PolygonObstacleManager *p_cs;

  void update()
  {
    auto& broad_collisions = p_cs->collisions2;
    for(auto [e1, e2] : broad_collisions){
      
    }
  }
};
