#include "PolygonObstacleManager.h"
#include "Player.h"
#include "Game.h"

PolygonObstacleManager::PolygonObstacleManager(int n_meteors, Game* game)
: p_game(game)
{
  obstacle_textures.load(Textures::ID::Heart, "../Resources/Heart.png");
  for (int i = 0; i < n_meteors; ++i)
  {
    auto meteor = createRandomMeteor();
    addMeteor(meteor);
  }
}

std::vector<Polygon *> PolygonObstacleManager::getNearestMeteors(sf::Vector2f r, float radius)
{
  std::vector<Polygon *> nearest_meteors;
  auto entity_inds = collision_tree.findIntersectingLeaves({r - sf::Vector2f{radius, radius},
                                                            r + sf::Vector2f{radius, radius}});

  for (auto ind : entity_inds)
  {
    if (obstacles.at(ind).type == ObstacleType::METEOR)
    {
      nearest_meteors.push_back(&obstacles.at(ind));
    }
  }
  return nearest_meteors;
}

std::vector<Obstacle *> PolygonObstacleManager::getNearestObstacles(ObstacleType type, sf::Vector2f r, float radius)
{
  std::vector<Obstacle *> nearest_meteors;
  auto entity_inds = collision_tree.findIntersectingLeaves({r - sf::Vector2f{radius, radius},
                                                            r + sf::Vector2f{radius, radius}});

  for (auto ind : entity_inds)
  {
    if (obstacles.at(ind).type == type || type == ObstacleType::ALL)
    {
      nearest_meteors.push_back(&obstacles.at(ind));
    }
  }
  return nearest_meteors;
}

std::vector<int> PolygonObstacleManager::getNearestMeteorInds(sf::Vector2f lower_left, sf::Vector2f upper_right)
{
  return getNearestObstacleInds(ObstacleType::METEOR, lower_left, upper_right);
}

std::vector<int> PolygonObstacleManager::getNearestObstacleInds(ObstacleType type, sf::Vector2f lower_left, sf::Vector2f upper_right)
{
  sf::Vector2f ll = lower_left;
  sf::Vector2f ur = upper_right;

  ll.x = std::min(lower_left.x, upper_right.x);
  ll.y = std::min(lower_left.y, upper_right.y);

  ur.x = std::max(lower_left.x, upper_right.x);
  ur.y = std::max(lower_left.y, upper_right.y);

  std::vector<int> obstacle_inds;
  auto broad_entity_inds = collision_tree.findIntersectingLeaves({ll, ur});

  for (auto ind : broad_entity_inds)
  {
    if (obstacles.at(ind).type == type)
    {
      obstacle_inds.push_back(ind);
    }
  }

  return obstacle_inds;
}

Obstacle PolygonObstacleManager::createRandomMeteor()
{
  auto polygon = generateRandomConvexPolygon(12 + rand() % 3);
  auto radius = randf(5, 20);
  polygon.radius = radius;
  polygon.setScale({radius, radius});
  polygon.setPosition(randomPosInBox());
  polygon.vel = {randf(-6, 6), randf(-6, 6)};
  polygon.angle_vel = randf(-0.02, 0.02);
  polygon.mass = radius * radius;
  polygon.inertia = std::pow(polygon.radius, 2) * polygon.mass;
  return {ObstacleType::METEOR, polygon};
}

void PolygonObstacleManager::addRandomMeteorAt(sf::Vector2f position, sf::Vector2f scale)
{
  auto new_meteor = createRandomMeteor();
  new_meteor.setPosition(position);
  new_meteor.radius /= new_meteor.radius / scale.x;
  new_meteor.setScale(scale);
  addMeteor(new_meteor);
}

void PolygonObstacleManager::addMeteor(Obstacle &new_meteor)
{

  auto new_entity_ind = obstacles.addObject(new_meteor);
  auto &new_obstacle = obstacles.at(new_entity_ind);
  collision_tree.addRect(new_obstacle.getBoundingRect().inflate(1.5f), new_entity_ind);

  auto draw_shape = std::make_unique<sf::ConvexShape>(new_obstacle.points.size());

  for (int i = 0; i < new_obstacle.points.size(); ++i)
  {
    draw_shape->setPoint(i, new_obstacle.points.at(i));
  }

  draw_shape->setPosition(new_obstacle.getPosition());
  draw_shape->setRotation(new_obstacle.getRotation());
  draw_shape->setScale(new_obstacle.getScale());
  draw_shape->setFillColor(sf::Color(rand() % 256, 255, 0, 255));

  new_obstacle.draw_shape = std::move(draw_shape);
  // auto new_entity_ind2 = drawables.addObject(wtf);
  // assert(new_entity_ind == new_entity_ind2);
}

void PolygonObstacleManager::destroyMeteor(int entity_ind)
{
  // meteors.remove(entity_ind);
  obstacles.remove(entity_ind);
  collision_tree.removeObject(entity_ind);
}

void PolygonObstacleManager::bounceFromWall(Polygon &meteor)
{
  if (meteor.getPosition().x < 0)
  {
    meteor.vel.x = std::abs(meteor.vel.x);
  }
  if (meteor.getPosition().y < 0)
  {
    meteor.vel.y = std::abs(meteor.vel.y);
  }
  if (meteor.getPosition().x >= Geometry::BOX[0])
  {
    meteor.vel.x = -std::abs(meteor.vel.x);
  }
  if (meteor.getPosition().y >= Geometry::BOX[1])
  {
    meteor.vel.y = -std::abs(meteor.vel.y);
  }
}

void PolygonObstacleManager::update(float dt)
{
  for (auto k : obstacles.active_inds)
  {
    auto &meteor = obstacles.at(k);
    meteor.update(dt);
    bounceFromWall(meteor);
    truncate(meteor.vel, 5.0f);

    meteor.draw_shape->setPosition(meteor.getPosition());
    meteor.draw_shape->setRotation(meteor.getRotation());
    meteor.draw_shape->setScale(meteor.getScale());

    auto fitting_rect = meteor.getBoundingRect();
    auto big_bounding_rect = collision_tree.getObjectRect(k);

    //! if object moved in a way that rect in the collision tree does not fully contain it
    if (makeUnion(fitting_rect, big_bounding_rect).volume() > big_bounding_rect.volume())
    {
      collision_tree.removeObject(k);
      collision_tree.addRect(fitting_rect.inflate(1.2f), k);
    }
  }

  struct pair_hash
  {
    inline std::size_t operator()(const std::pair<int, int> &v) const
    {
      return v.first * 31 + v.second;
    }
  };
  std::unordered_set<std::pair<int, int>, pair_hash> collided;

  // collision_tree.clear();

  // for (auto i : meteors.active_inds)
  // {
  //   collision_tree.addRect(meteors.at(i).getBoundingRect(), i);
  // }

  auto tic = std::chrono::high_resolution_clock::now();
  for (auto i : obstacles.active_inds)
  {

    auto nearest_inds = collision_tree.findIntersectingLeaves(obstacles.at(i).getBoundingRect());
    for (int meteor_ind : nearest_inds)
    {
      if (i == meteor_ind)
      {
        continue;
      }
      std::pair<int, int> collision_inds = {std::min(i, meteor_ind), std::max(i, meteor_ind)};
      if (collided.count(collision_inds) == 0)
      {
        if (intersects(obstacles.at(meteor_ind).getBoundingRect(), obstacles.at(i).getBoundingRect()))
        {
          collidePolygons(obstacles.at(meteor_ind), obstacles.at(i));
        }
        collided.insert(collision_inds);
      }
    }
  }
  auto delta_time = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::high_resolution_clock::now() - tic);
  std::cout << "collisions took: " << delta_time.count() << " ms"
            << "\n";
}

void PolygonObstacleManager::draw(sf::RenderTarget &window)
{
  for (auto i : obstacles.active_inds)
  {
    window.draw(*obstacles.at(i).draw_shape);
  }
}

Polygon PolygonObstacleManager::generateRandomConvexPolygon(int n) const
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
    auto x = xPool.at(i);

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
    auto y = yPool.at(i);

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

  std::random_device rd;
  std::mt19937 g(rd());

  // Randomly pair up the X- and Y-components
  std::shuffle(yVec.begin(), yVec.end(), g);

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
  float x = 0, y = 0;
  float minPolygonX = 0;
  float minPolygonY = 0;
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
  auto xShift = minX - minPolygonX;
  auto yShift = minY - minPolygonY;

  for (int i = 0; i < n; i++)
  {
    auto p = points.at(i);
    points.at(i) += sf::Vector2f{xShift, yShift};
  }
  Polygon p;
  p.points = points;

  return p;
}

void PolygonObstacleManager::collidePolygons(Polygon &pa, Polygon &pb)
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
  cont_point /= (float)clipped_edge.size();

  auto n = c_data.separation_axis;
  pa.move(-c_data.separation_axis * c_data.minimum_translation * 1.05f);

  auto v_rel = pa.vel - pb.vel;
  auto v_reln = dot(v_rel, n);

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
  pa.vel += j_factor / pa.mass * n;
  pb.vel -= j_factor / pb.mass * n;
}

sf::Vector2f PolygonObstacleManager::findClosestIntesection(sf::Vector2f at, sf::Vector2f dir, float length)
{
  sf::Vector2f closest_intersection = at + dir * length;
  float min_dist = 200.f;
  auto inters = collision_tree.rayCast(at, dir, length);
  for (auto ent_idn : inters)
  {
    auto points = obstacles.at(ent_idn).getPointsInWorld();
    int next = 1;

    for (int i = 0; i < points.size(); ++i)
    {
      sf::Vector2f r1 = points.at(i);
      sf::Vector2f r2 = points.at(next);

      auto intersection = getIntersection(r1, r2, at, at + dir * length);
      if (intersection.x > 0 & intersection.y > 0)
      {
        auto new_dist = dist(intersection, at);
        if (new_dist < min_dist)
        {
          closest_intersection = intersection;
          min_dist = new_dist;
        }
      }
      next++;
      if (next == points.size())
      {
        next = 0;
      }
    }
  }
  return closest_intersection;
}

  void PolygonObstacleManager::collideWithPlayer(Player &player)
  {

    auto player_vel = player.speed * angle2dir(player.angle);
    auto meteors = getNearestMeteors(player.pos, player.radius);
    for (auto *meteor : meteors)
    {
      auto mvt = meteor->getMVTOfSphere(player.pos, player.radius);
      auto vel_mvt_dir = dot(mvt, player_vel);
      if (norm(mvt) > 0.01f && vel_mvt_dir < 0)
      {
        player_vel -= 2.f * vel_mvt_dir * mvt;
        player.angle = std::atan2(player_vel.y, player_vel.x) * 180.f / M_PIf;
        player.health -= 1;
        p_game->score--;
      }
    }
    auto power_ups = getNearestObstacleInds(ObstacleType::POWERUP, player.pos , {player.radius, player.radius});
    for (auto ind : power_ups)
    {
      auto mvt = obstacles.at(ind).getMVTOfSphere(player.pos, player.radius);
      auto vel_mvt_dir = dot(mvt, player_vel);
      if (norm(mvt) > 0.01f)
      {
        player.health += 5;
        destroyMeteor(ind);
      }
    }
    auto flags = getNearestObstacleInds(ObstacleType::FLAG, player.pos , {player.radius, player.radius});
    for (auto ind : flags)
    {
      auto mvt = obstacles.at(ind).getMVTOfSphere(player.pos, player.radius);
      auto vel_mvt_dir = dot(mvt, player_vel);
      if (norm(mvt) > 0.01f)
      {
        addRegularObstacle(ObstacleType::FLAG, randomPosInBox());
        destroyMeteor(ind);
      }
    }
  }