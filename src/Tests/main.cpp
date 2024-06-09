
#include <gtest/gtest.h>

#include "test_polygon_overlap.cc"
#include "test_object_pool.cc"
#include "test_bvh.cc"

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}