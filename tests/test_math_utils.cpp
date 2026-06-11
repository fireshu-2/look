#include <iostream>
#include <cassert>
#include <cmath>
#include "../math_utils.hpp"

void test_iou() {
    // Exact match
    assert(std::abs(iou(0, 0, 10, 10, 0, 0, 10, 10) - 1.0f) < 1e-5);

    // No overlap
    assert(std::abs(iou(0, 0, 10, 10, 20, 20, 10, 10) - 0.0f) < 1e-5);

    // Partial overlap: A is [0,0,10,10], B is [5,5,10,10].
    // Intersection: [5,5,10,10] which has w=5, h=5, area=25.
    // Union: 100 + 100 - 25 = 175.
    // IoU: 25 / 175 = 1/7 ~= 0.142857
    assert(std::abs(iou(0, 0, 10, 10, 5, 5, 10, 10) - 0.142857f) < 1e-5);

    std::cout << "All IOU tests passed!" << std::endl;
}

int main() {
    test_iou();
    return 0;
}
