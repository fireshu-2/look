#include "../math_utils.hpp"
#include <iostream>
#include <cassert>
#include <cmath>

bool approx_equal(float a, float b) {
    return std::fabs(a - b) < 1e-5;
}

int main() {
    // Exact overlap
    assert(approx_equal(iou(0, 0, 10, 10, 0, 0, 10, 10), 1.0f));

    // No overlap
    assert(approx_equal(iou(0, 0, 10, 10, 20, 20, 10, 10), 0.0f));

    // Partial overlap
    // Box1: 0,0 10x10 -> Area 100
    // Box2: 5,0 10x10 -> Area 100
    // Intersection: 5,0 5x10 -> Area 50
    // Union: 100 + 100 - 50 = 150
    // IOU: 50 / 150 = 0.33333...
    assert(approx_equal(iou(0, 0, 10, 10, 5, 0, 10, 10), 0.3333333f));

    std::cout << "All IOU tests passed!" << std::endl;
    return 0;
}
