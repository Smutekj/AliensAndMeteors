#include <gtest/gtest.h>
#include "../BVH.h"

TEST(TestBoundingVolumeAddRemove, BasicAssertions)
{

    BoundingVolumeTree bvh;
    AABB rect1({0,0}, {10,10});
    AABB rect2({11,11}, {15,15});
    AABB rect3({0,0}, {2,2});
    AABB rect4({0,0}, {10,10});
    bvh.addRect(rect1, 0);
    bvh.addRect(rect2, 1);
    bvh.addRect(rect3, 2);
    bvh.addRect(rect4, 3);
    EXPECT_TRUE(bvh.findIntersectingLeaves({{30, 30}, {40,40}}).empty()); //! no intersection
    auto intersections = bvh.findIntersectingLeaves({{0,0}, {10,10}}); //! intersects with 0 2 3
    std::sort(intersections.begin(), intersections.end());
    EXPECT_EQ(intersections.size(), 3);
    EXPECT_EQ(intersections[0], 0);
    EXPECT_EQ(intersections[1], 2);
    EXPECT_EQ(intersections[2], 3);


    bvh.removeObject(0); //! remove rect 1;
    intersections = bvh.findIntersectingLeaves(bvh.getObjectRect(3)); //! should intersect with 2 and 3
    std::sort(intersections.begin(), intersections.end());
    EXPECT_EQ(intersections.size(), 2);
    EXPECT_EQ(intersections[0], 2);
    EXPECT_EQ(intersections[1], 3);

}
