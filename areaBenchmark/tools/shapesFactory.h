#pragma once

#include "tools/simd.h"
#include "tools/shapes.h"

using namespace shapes;

class shapesFactory
{
public:
  static shapesFactory& instance()
  {
    static shapesFactory s;
    return s;
  }

  shapesFactory();
  shapesFactory(shapesFactory const&) = delete;
  ~shapesFactory();

  void operator=(shapesFactory const&) = delete;
    
  const vectorized& getCache()
  {
    return cache;
  }

  const std::vector<objectOriented::shape*>& getObjectOriented();
  const std::vector<switchStruct::shape>& getSwitchStruct();
  const std::vector<coeffArray::shape>& getCoeffArray();

  bool validateResult(int num, float area);

private:

  void init(int num);

  vectorized cache;

  std::vector<objectOriented::shape*> objectOrientedShapes;
  std::vector<switchStruct::shape> switchStructShapes;
  std::vector<coeffArray::shape> coeffArrayShapes;


  std::map<int, float> results;   // int - number of shapes, float - total area
};