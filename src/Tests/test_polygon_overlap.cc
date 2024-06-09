#include <gtest/gtest.h>
#include "../CollisionSystem.h"

TEST(TestPolygonOverlap, BasicAssertions)
{
    using namespace Collisions;

    Polygon p1(4);
    p1.setScale(10.f, 10.f);
    Polygon p2(4);
    p2.setScale(10.f, 10.f);
    p2.move({7, 0});

    Polygon p3 = p1;
    p3.move({15, 0});

    Polygon p4(3);
    p4.setScale(2, 2);

    auto c_data = calcCollisionData(p1.getPointsInWorld(), p2.getPointsInWorld());
    auto c_data2 = calcCollisionData(p1.getPointsInWorld(), p3.getPointsInWorld());
    auto c_data3 = calcCollisionData(p1.getPointsInWorld(), p4.getPointsInWorld());

    EXPECT_TRUE(c_data.minimum_translation > 0); //! Overlapping
    EXPECT_TRUE(c_data2.minimum_translation < 0); //! no overlap
    EXPECT_FLOAT_EQ(c_data.separation_axis.x, -1 ); //! x axis is the separating axis
    EXPECT_FLOAT_EQ(c_data.separation_axis.y, 0.f);

    EXPECT_TRUE(c_data3.minimum_translation > 0); //! small inside big
}

inline sf::Vector2f calcCom(const Polygon &p1)
{
    sf::Vector2f com;
    for (auto &point : p1.points)
    {
        com += point;
    }
    com /= (float)p1.points.size();
    return com;
}

TEST(TestPolygonClipping, BasicAssertions)
{

    using namespace Collisions;

}
