#include <vector>

#include "benchmark/benchmark.h"

#include "simple.h"



static std::vector<objectOriented::shape*> shapesFactoryVTBL(int numShapes)
{
  using namespace objectOriented;
  std::vector<shape*> ans;
  ans.reserve(numShapes);
  for (int i = 0; i < numShapes / 4; i++)
  {
    circle* c = new circle();
    rectangle* r = new rectangle();
    square* s = new square();
    triangle* t = new triangle();
    ans.push_back(c);
    ans.push_back(r);
    ans.push_back(s);
    ans.push_back(t);
  }
  return ans;
}
void BM_getAreaVTBL(benchmark::State& state)
{
  int numShapes = static_cast<int>(state.range(0));
  std::vector<objectOriented::shape*> shapes = shapesFactoryVTBL(numShapes);

  for (auto _ : state)   // Alternative: while (state.KeepRunning())
  {
    float sum = 0;
    benchmark::DoNotOptimize(sum);     // force result to be stored in either memory or register
    for (objectOriented::shape* shape : shapes)
    {
      // Performance overhead: 
      // - Vector of pointers to structs, spread across wide memory area, cache misses
      // - Indirection to vtable with offset based on the function name, then indirection to a function, than call
      // - No inline
      // - SSE used to calculate single float value
      sum += shape->area();
    }
  }
  state.SetComplexityN(state.range(0));
}


static std::vector<switchStruct::shape> shapesFactorySwitchStruct(int numShapes)
{
  using namespace switchStruct;
  std::vector<shape> ans;
  ans.reserve(numShapes);
  for (int i = 0; i < numShapes / 4; i++)
  {
    ans.push_back(shape(shapeType::circle));
    ans.push_back(shape(shapeType::rectangle));
    ans.push_back(shape(shapeType::square));
    ans.push_back(shape(shapeType::triangle));
  }
  return ans;
}
void BM_getAreaSwitchStruct(benchmark::State& state)
{
  int numShapes = static_cast<int>(state.range(0));
  std::vector<switchStruct::shape> shapes = shapesFactorySwitchStruct(numShapes);

  for (auto _ : state)
  {
    float sum = 0;
    benchmark::DoNotOptimize(sum);
    for (const switchStruct::shape& shape : shapes)
    {
      // Performance:
      // + No vtable, no indirection
      // + Size of any struct is known, vector is a continuous block of memory. Better despite sizeof(struct) being bigger.
      // + area() inlined
      // - Switch statement costs but less than vtable indirection
      // - SSE used to calculate single float values
      sum += shape.area();
    }
  }
  state.SetComplexityN(state.range(0));
}


static std::vector<coeffArray::shape> shapesFactoryCoeffArray(int numShapes)
{
  using namespace coeffArray;
  std::vector<shape> ans;
  ans.reserve(numShapes);
  for (int i = 0; i < numShapes / 4; i++)
  {
    ans.push_back(shape(shapeType::circle));
    ans.push_back(shape(shapeType::rectangle));
    ans.push_back(shape(shapeType::square));
    ans.push_back(shape(shapeType::triangle));
  }
  return ans;
}
void BM_getAreaCoeffArray(benchmark::State& state)
{
  int numShapes = static_cast<int>(state.range(0));
  std::vector<coeffArray::shape> shapes = shapesFactoryCoeffArray(numShapes);

  for (auto _ : state)
  {
    float sum = 0;
    benchmark::DoNotOptimize(sum);
    for (const coeffArray::shape& shape : shapes)
    {
      // Performance:
      // + No switch statement costs but less than vtable indirection
      // - SSE used to calculate single float values
      sum += shape.area();
    }
  }
  state.SetComplexityN(state.range(0));
}