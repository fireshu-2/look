#include "../math_utils.hpp"
#include <iostream>

int main() {
    float i = iou(0.0f, 0.0f, 10.0f, 10.0f, 5.0f, 5.0f, 10.0f, 10.0f);
    std::cout << "IOU: " << i << std::endl;
    return 0;
}
