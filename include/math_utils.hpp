#ifndef MATH_UTILS_HPP
#define MATH_UTILS_HPP

#include <algorithm>

static inline float iou(float x1, float y1, float w1, float h1, float x2, float y2, float w2, float h2)
{
	float xi = std::max(x1, x2);
	float yi = std::max(y1, y2);
	float wi = std::min(x1 + w1, x2 + w2) - xi;
	float hi = std::min(y1 + h1, y2 + h2) - yi;
	if (wi <= 0 || hi <= 0)
		return 0.0f;
	float area_i = wi * hi;
	float area_u = w1 * h1 + w2 * h2 - area_i;
	return area_i / area_u;
}

#endif // MATH_UTILS_HPP
