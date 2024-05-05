#pragma once
#include <gtest/gtest.h>
#include "../Game.h"

TEST(TestPolygonOverlap, BasicAssertions)
{

    Polygon p1(3, {10, 10}, 5);
    p1.points[2].x += 1;
    Polygon p2(5, {20, 20}, 3);
    Polygon p3(4);
    Polygon p4(3);
    p3.points[0] = {26, 4};
    p3.points[1] = {26, 16};
    p3.points[2] = {14, 16};
    p3.points[3] = {14, 4};

    p4.points[0] = {14, 16};
    p4.points[1] = {26, 16};
    p4.points[2] = {14, 4};

    // auto c_data = calcCollisionData(p1, p2);
    // auto c_data2 = calcCollisionData(p1, p3);
    // auto c_data3 = calcCollisionData(p1, p4);

    // EXPECT_TRUE(c_data.minimum_translation < 0);
    // EXPECT_TRUE(c_data2.minimum_translation > 0);
    // EXPECT_FLOAT_EQ(c_data2.separation_axis.x, -1 );
    // EXPECT_FLOAT_EQ(c_data2.separation_axis.y, 0.f);
}

sf::Vector2f calcCom(const Polygon &p1)
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

    Polygon p1(4);
    Polygon p2(4);
    p2.points[0] = {4, 2};
    p2.points[1] = {12, 2};
    p2.points[2] = {12, 5};
    p2.points[3] = {4, 5};

    auto com_b = calcCom(p2);


    p1.points[0] = {8, 4};
    p1.points[1] = {14, 4};
    p1.points[2] = {14, 9};
    p1.points[3] = {8, 9};

    auto c_data = calcCollisionData(p1.points, p2.points);
    auto col_feats1 = obtainFeatures(c_data.separation_axis, p1.points);
    auto col_feats2 = obtainFeatures(-c_data.separation_axis, p2.points);

    auto clipped_edge = clipEdges(col_feats1, col_feats2, c_data.separation_axis);

    p1.points[0] = {6, 4};
    p1.points[1] = {9, 7};
    p1.points[2] = {5, 11};
    p1.points[3] = {2, 8};

    auto com_a = calcCom(p1);

    c_data = calcCollisionData(p1.points, p2.points);
    auto wtf = dot((com_b - com_a), c_data.separation_axis);
    
    col_feats1 = obtainFeatures(c_data.separation_axis, p1.points);
    col_feats2 = obtainFeatures(-c_data.separation_axis, p2.points);

    clipped_edge = clipEdges(col_feats1, col_feats2, c_data.separation_axis);

    p1.points[0] = {9, 4};
    p1.points[1] = {13, 3};
    p1.points[2] = {14, 7};
    p1.points[3] = {10, 8};

    com_a = calcCom(p1);
    
    c_data = calcCollisionData(p1.points, p2.points);
    wtf = dot((com_b - com_a), c_data.separation_axis);

    col_feats1 = obtainFeatures(c_data.separation_axis, p1.points);
    col_feats2 = obtainFeatures(-c_data.separation_axis, p2.points);

    clipped_edge = clipEdges(col_feats1, col_feats2, c_data.separation_axis);

    std::cout << "hi";
}
