#pragma once
#include <gtest/gtest.h>
#include "../Utils/ObjectPool.h"

TEST(TestObjectPool, BasicAssertions) {

    DynamicObjectPool<float, 10> pool;
    
    auto& inds = pool.getObjects();
    EXPECT_TRUE(inds.empty());

    auto e1_ind = pool.addObject(1.0f);
    auto e2_ind = pool.addObject(1.5f);
    
    EXPECT_EQ(pool.getObjects().size(), 2);
    EXPECT_FLOAT_EQ(pool.at(e1_ind), 1.0f);
    EXPECT_FLOAT_EQ(pool.at(e2_ind), 1.5f);

    pool.remove(e1_ind);
    EXPECT_EQ(pool.getObjects().size(), 1);
    EXPECT_FLOAT_EQ(pool.getObjects()[0], 1.5);

    auto e3_ind = pool.addObject(2.0f);
    EXPECT_EQ(pool.getObjects().size(), 2);
    EXPECT_FLOAT_EQ(pool.at(e3_ind), 2.0f);
    EXPECT_FLOAT_EQ(pool.at(e2_ind), 1.5f);

    pool.clear();
    EXPECT_EQ(pool.getObjects().size(), 0);
    
}


