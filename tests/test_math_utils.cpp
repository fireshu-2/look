#include <iostream>
#include <cmath>
#include "../math_utils.hpp"

#define ASSERT_APPROX_EQUAL(a, b) \
    if (std::abs((a) - (b)) > 1e-5) { \
        std::cerr << "Assertion failed: " << #a << " (" << (a) << ") != " << #b << " (" << (b) << ")" << std::endl; \
        return 1; \
    }

int main() {
    // Exact overlap
    float iou1 = iou(0, 0, 10, 10, 0, 0, 10, 10);
    ASSERT_APPROX_EQUAL(iou1, 1.0f);

    // No overlap
    float iou2 = iou(0, 0, 10, 10, 20, 20, 10, 10);
    ASSERT_APPROX_EQUAL(iou2, 0.0f);

    // Partial overlap (half)
    // Box 1: 0,0 to 10,10 (area 100)
    // Box 2: 5,0 to 15,10 (area 100)
    // Intersection: 5,0 to 10,10 (width 5, height 10, area 50)
    // Union: 100 + 100 - 50 = 150
    // IOU: 50 / 150 = 1/3
    float iou3 = iou(0, 0, 10, 10, 5, 0, 10, 10);
    ASSERT_APPROX_EQUAL(iou3, 1.0f / 3.0f);

    std::cout << "All tests passed!" << std::endl;
    return 0;
}
