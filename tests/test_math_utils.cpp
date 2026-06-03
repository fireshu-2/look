#include <iostream>
#include <cmath>
#include "../math_utils.hpp"

#define ASSERT_ALMOST_EQUAL(a, b, epsilon) \
    if (std::abs((a) - (b)) > (epsilon)) { \
        std::cerr << "Assertion failed: " << #a << " != " << #b << " (diff: " << std::abs((a) - (b)) << ")" << std::endl; \
        return 1; \
    }

int main() {
    float iou_val;

    // Test Case 1: Identical boxes
    iou_val = iou(0, 0, 10, 10, 0, 0, 10, 10);
    ASSERT_ALMOST_EQUAL(iou_val, 1.0f, 1e-5f);

    // Test Case 2: Non-overlapping boxes
    iou_val = iou(0, 0, 10, 10, 20, 20, 10, 10);
    ASSERT_ALMOST_EQUAL(iou_val, 0.0f, 1e-5f);

    // Test Case 3: Partially overlapping boxes
    // Box 1: (0, 0, 10, 10) -> Area 100
    // Box 2: (5, 5, 10, 10) -> Area 100
    // Intersection: (5, 5) to (10, 10) -> width 5, height 5 -> Area 25
    // Union: 100 + 100 - 25 = 175
    // IOU: 25 / 175 = 1 / 7
    iou_val = iou(0, 0, 10, 10, 5, 5, 10, 10);
    ASSERT_ALMOST_EQUAL(iou_val, 1.0f / 7.0f, 1e-5f);

    std::cout << "All IOU tests passed!" << std::endl;
    return 0;
}