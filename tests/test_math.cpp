#include "../include/math_utils.hpp"
#include <iostream>
#include <cassert>
#include <cmath>

bool almost_equal(float a, float b, float epsilon = 1e-5) {
    return std::abs(a - b) < epsilon;
}

int main() {
    // 1. Identical boxes (IoU should be 1.0)
    float iou1 = iou(0, 0, 10, 10, 0, 0, 10, 10);
    assert(almost_equal(iou1, 1.0f));

    // 2. Non-overlapping boxes (IoU should be 0.0)
    float iou2 = iou(0, 0, 10, 10, 20, 20, 10, 10);
    assert(almost_equal(iou2, 0.0f));

    // 3. Partially overlapping boxes
    // Box 1: (0,0) width 10, height 10. Area = 100
    // Box 2: (5,5) width 10, height 10. Area = 100
    // Intersection: (5,5) width 5, height 5. Area = 25
    // Union = 100 + 100 - 25 = 175
    // IoU = 25 / 175 = 1 / 7
    float iou3 = iou(0, 0, 10, 10, 5, 5, 10, 10);
    assert(almost_equal(iou3, 1.0f / 7.0f));

    std::cout << "All tests passed successfully!" << std::endl;
    return 0;
}
