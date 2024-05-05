
#include <gtest/gtest.h>

#include "test_gay_vector.cc"
#include "test_polygon_overlap.cc"

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}