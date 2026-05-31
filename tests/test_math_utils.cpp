#include <iostream>
#include <cassert>
#include <cmath>
#include "../math_utils.hpp"

// Utility function to compare floats
bool is_close(float a, float b, float epsilon = 1e-5) {
    return std::abs(a - b) < epsilon;
}

int main() {
    // Test 1: Identical boxes (IoU = 1.0)
    float iou1 = iou(0, 0, 10, 10, 0, 0, 10, 10);
    assert(is_close(iou1, 1.0f));

    // Test 2: Non-overlapping boxes (IoU = 0.0)
    float iou2 = iou(0, 0, 10, 10, 20, 20, 10, 10);
    assert(is_close(iou2, 0.0f));

    // Test 3: Partially overlapping boxes
    // Box 1: (0,0) to (10,10) - Area = 100
    // Box 2: (5,5) to (15,15) - Area = 100
    // Intersection: (5,5) to (10,10) - Area = 25
    // Union: 100 + 100 - 25 = 175
    // Expected IoU = 25 / 175 = 1/7 ~= 0.142857
    float iou3 = iou(0, 0, 10, 10, 5, 5, 10, 10);
    assert(is_close(iou3, 0.142857f));

    // Test 4: One box completely inside another
    // Box 1: (0,0) to (20,20) - Area = 400
    // Box 2: (5,5) to (10,10) - Area = 25
    // Intersection: Area = 25
    // Union: Area = 400
    // Expected IoU = 25 / 400 = 1/16 = 0.0625
    float iou4 = iou(0, 0, 20, 20, 5, 5, 5, 5);
    assert(is_close(iou4, 0.0625f));

    // Test 5: Boxes that touch edges but don't overlap
    float iou5 = iou(0, 0, 10, 10, 10, 0, 10, 10);
    assert(is_close(iou5, 0.0f));

    std::cout << "All math_utils.hpp tests passed!" << std::endl;
    return 0;
}