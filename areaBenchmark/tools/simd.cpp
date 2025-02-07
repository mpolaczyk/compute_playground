
#include <immintrin.h>    // https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html
#include <assert.h>

#include "tools/cpuid.h"
#include "tools/shapes.h"
#include "setup.h"

namespace SIMD
{
  float areaSSE(const shapes::vectorized& shapes, int from = 0, int to = 0)
  {
    assert(shapes.num % INCREMENT_SSE == 0);
    assert(from % INCREMENT_SSE == 0);
    assert(to % INCREMENT_SSE == 0);
    to = to != 0 ? to : shapes.num;
    assert(from < to);
    if (!InstructionSet::SSE()) return 0.0f;

    __m128 sum = _mm_setzero_ps();
    int n = 0;
    for (int i = from; i < to; i += INCREMENT_SSE)
    {
      __m128 coeffVec = _mm_load_ps(&shapes.vcoeff[i]);
      __m128 aVec = _mm_load_ps(&shapes.va[i]);
      __m128 bVec = _mm_load_ps(&shapes.vb[i]);
      __m128 areaVec = _mm_mul_ps(coeffVec, _mm_mul_ps(aVec, bVec));
      sum = _mm_add_ps(sum, areaVec);
      n++;
    }
    return sum.m128_f32[0] + sum.m128_f32[1] + sum.m128_f32[2] + sum.m128_f32[3];   // No horizontal add because I'm lazy
  }

  float areaAVX(const shapes::vectorized& shapes, int from = 0, int to = 0)
  {
    assert(shapes.num % INCREMENT_AVX == 0);
    assert(from % INCREMENT_AVX == 0);
    assert(to % INCREMENT_AVX == 0);
    to = to != 0 ? to : shapes.num;
    assert(from < to);
    if (!InstructionSet::AVX()) return 0.0f;

    __m256 sum = _mm256_setzero_ps();
    for (int i = from; i < to; i += INCREMENT_AVX)
    {
      __m256 coeffVec = _mm256_load_ps(&shapes.vcoeff[i]);
      __m256 aVec = _mm256_load_ps(&shapes.va[i]);
      __m256 bVec = _mm256_load_ps(&shapes.vb[i]);
      __m256 areaVec = _mm256_mul_ps(coeffVec, _mm256_mul_ps(aVec, bVec));
      sum = _mm256_add_ps(sum, areaVec);
    }
    return sum.m256_f32[0] + sum.m256_f32[1] + sum.m256_f32[2] + sum.m256_f32[3]    // No horizontal add because I'm lazy
      + sum.m256_f32[4] + sum.m256_f32[5] + sum.m256_f32[6] + sum.m256_f32[7];
  }

  float areaAVX512(const shapes::vectorized& shapes, int from = 0, int to = 0)
  {
    assert(shapes.num % INCREMENT_AVX512 == 0);
    assert(from % INCREMENT_AVX512 == 0);
    assert(to % INCREMENT_AVX512 == 0);
    to = to != 0 ? to : shapes.num;
    assert(from < to);
    if (!InstructionSet::AVX512F()) return 0.0f;

    __m512 sum = _mm512_setzero_ps();
    for (int i = from; i < to; i += INCREMENT_AVX512)
    {
      __m512 coeffVec = _mm512_load_ps(&shapes.vcoeff[i]);
      __m512 aVec = _mm512_load_ps(&shapes.va[i]);
      __m512 bVec = _mm512_load_ps(&shapes.vb[i]);
      __m512 areaVec = _mm512_mul_ps(coeffVec, _mm512_mul_ps(aVec, bVec));
      sum = _mm512_add_ps(sum, areaVec);
    }
    return sum.m512_f32[0] + sum.m512_f32[1] + sum.m512_f32[2] + sum.m512_f32[3]    // No horizontal add because I'm lazy
      + sum.m512_f32[4] + sum.m512_f32[5] + sum.m512_f32[6] + sum.m512_f32[7]
      + sum.m512_f32[8] + sum.m512_f32[9] + sum.m512_f32[10] + sum.m512_f32[11]
      + sum.m512_f32[12] + sum.m512_f32[13] + sum.m512_f32[14] + sum.m512_f32[15];
  }
}