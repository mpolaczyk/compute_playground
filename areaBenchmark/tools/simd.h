#pragma once

#include "tools/shapes.h"

namespace SIMD
{
    float areaSSE(const shapes::vectorized& shapes, int from = 0, int to = 0);

    float areaAVX(const shapes::vectorized& shapes, int from = 0, int to = 0);

    float areaAVX512(const shapes::vectorized& shapes, int from = 0, int to = 0);
}
