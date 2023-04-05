#pragma once

#include <immintrin.h>    // https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html
#include <assert.h>
#include <numbers>

#include "tools/cpuid.h"

namespace SIMD
{
  enum class shapeType : int
  {
    circle,
    rectangle,
    square,
    triangle
  };
  float const shapeCoeff[4] = { std::numbers::pi_v<float>, 1.0f, 1.0f, 0.5f };

  struct shapes
  {
    shapes(int num) : num(num)
    {
      assert(num > 0);
      coeff.reserve(num);
      a.reserve(num);
      b.reserve(num);
    }

    void initAndRandomize(int n, shapeType type)
    {
      assert(n < num);
      coeff.insert(coeff.begin() + n, shapeCoeff[static_cast<int>(type)]);    // Cheating? We're moving part of computation to the init phase! No, it's int anyway
      a.insert(a.begin() + n, static_cast<float>(rand() % 100));
      b.insert(b.begin() + n, (type == shapeType::circle || type == shapeType::square) ? a[n] : static_cast<float>(rand() % 100));
    }

    const int incrementSSE = 4;
    const int incrementAVX = 8;
    const int incrementAVX512 = 16;

    std::vector<float> coeff;
    std::vector<float> a;
    std::vector<float> b;
    int num = 0;

    float areaSSE(int from = 0, int to = 0) const
    {
      assert(num % incrementSSE == 0);
      assert(from % incrementSSE == 0);
      assert(to % incrementSSE == 0);
      to = to != 0 ? to : num;    // use to if defined
      assert(from < to);
      if (!InstructionSet::SSE()) return 0.0f;

      __m128 sum = _mm_setzero_ps();
      for (int i = from; i < to; i += incrementSSE)
      {
        __m128 coeffVec = _mm_load_ps(&coeff[i]);
        __m128 aVec = _mm_load_ps(&a[i]);
        __m128 bVec = _mm_load_ps(&b[i]);
        __m128 areaVec = _mm_mul_ps(coeffVec, _mm_mul_ps(aVec, bVec));
        sum = _mm_add_ps(sum, areaVec);
      }
      return sum.m128_f32[0] + sum.m128_f32[1] + sum.m128_f32[2] + sum.m128_f32[3];   // No horizontal add because I'm lazy
    }

    float areaAVX(int from = 0, int to = 0) const
    {
      assert(num % incrementAVX == 0);
      assert(from % incrementAVX == 0);
      assert(to % incrementAVX == 0);
      to = to != 0 ? to : num;    // use to if defined
      assert(from < to);
      if (!InstructionSet::AVX()) return 0.0f;

      __m256 sum = _mm256_setzero_ps();
      for (int i = from; i < to; i += incrementAVX)
      {
        __m256 coeffVec = _mm256_load_ps(&coeff[i]);
        __m256 aVec = _mm256_load_ps(&a[i]);
        __m256 bVec = _mm256_load_ps(&b[i]);
        __m256 areaVec = _mm256_mul_ps(coeffVec, _mm256_mul_ps(aVec, bVec));
        sum = _mm256_add_ps(sum, areaVec);
      }
      return sum.m256_f32[0] + sum.m256_f32[1] + sum.m256_f32[2] + sum.m256_f32[3]    // No horizontal add because I'm lazy
        + sum.m256_f32[4] + sum.m256_f32[5] + sum.m256_f32[6] + sum.m256_f32[7];
    }

    float areaAVX512(int from = 0, int to = 0) const
    {
      assert(num % incrementAVX512 == 0);
      assert(from % incrementAVX512 == 0);
      assert(to % incrementAVX512 == 0);
      to = to != 0 ? to : num;    // use to if defined
      assert(from < to);
      if (!InstructionSet::AVX512F()) return 0.0f;

      __m512 sum = _mm512_setzero_ps();
      for (int i = from; i < to; i += incrementAVX512)
      {
        __m512 coeffVec = _mm512_load_ps(&coeff[i]);
        __m512 aVec = _mm512_load_ps(&a[i]);
        __m512 bVec = _mm512_load_ps(&b[i]);
        __m512 areaVec = _mm512_mul_ps(coeffVec, _mm512_mul_ps(aVec, bVec));
        sum = _mm512_add_ps(sum, areaVec);
      }
      return sum.m512_f32[0] + sum.m512_f32[1] + sum.m512_f32[2] + sum.m512_f32[3]    // No horizontal add because I'm lazy
        + sum.m512_f32[4] + sum.m512_f32[5] + sum.m512_f32[6] + sum.m512_f32[7]
        + sum.m512_f32[8] + sum.m512_f32[9] + sum.m512_f32[10] + sum.m512_f32[11]
        + sum.m512_f32[12] + sum.m512_f32[13] + sum.m512_f32[14] + sum.m512_f32[15];
    }
  };
}
