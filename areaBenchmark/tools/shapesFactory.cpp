
#include <vector>
#include <map>
#include <thread>

#include "setup.h"
#include "shapesFactory.h"

shapesFactory::shapesFactory()
{
  init(RANGE_MAX);
}

shapesFactory::~shapesFactory()
{
  for (auto& shape : objectOrientedShapes)
  {
    delete shape;
  }
}

void shapesFactory::init(int num)
{
  using namespace shapes;

  cache = vectorized(num);
  for (int i = 0; i < num; i += 4)
  {
    cache.initAndRandomize(i, shapeType::circle);
    cache.initAndRandomize(i + 1, shapeType::rectangle);
    cache.initAndRandomize(i + 2, shapeType::square);
    cache.initAndRandomize(i + 3, shapeType::triangle);
  }
}

const std::vector<shapes::objectOriented::shape*>& shapesFactory::getObjectOriented()
{
  using namespace shapes;
  using namespace shapes::objectOriented;

  if (objectOrientedShapes.size() == 0)
  {
    for (auto i = 0; i < RANGE_MAX; i++)
    {
      switch (cache.vtype[i])
      {
      case shapeType::circle:
        objectOrientedShapes.push_back(new circle(cache.va[i]));
        break;
      case shapeType::rectangle:
        objectOrientedShapes.push_back(new rectangle(cache.va[i], cache.vb[i]));
        break;
      case shapeType::square:
        objectOrientedShapes.push_back(new square(cache.va[i]));
        break;
      case shapeType::triangle:
        objectOrientedShapes.push_back(new triangle(cache.va[i], cache.vb[i]));
        break;
      default:
        break;
      }
    }
  }
  return objectOrientedShapes;
}

const std::vector<shapes::switchStruct::shape>& shapesFactory::getSwitchStruct()
{
  using namespace shapes;
  using namespace shapes::switchStruct;

  if (switchStructShapes.size() == 0)
  {
    switchStructShapes.reserve(RANGE_MAX);
    for (int i = 0; i < RANGE_MAX; i++)
    {
      switchStructShapes.emplace_back(shape(cache.vtype[i], cache.va[i], cache.vb[i]));
    }
  }
  return switchStructShapes;
}

const std::vector<shapes::coeffArray::shape>& shapesFactory::getCoeffArray()
{
  using namespace shapes;
  using namespace shapes::coeffArray;

  if (coeffArrayShapes.size() == 0)
  {
    coeffArrayShapes.reserve(RANGE_MAX);
    for (int i = 0; i < RANGE_MAX; i++)
    {
      coeffArrayShapes.emplace_back(shape(cache.vtype[i], cache.va[i], cache.vb[i]));
    }

  }
  return coeffArrayShapes;
}

bool shapesFactory::validateResult(int num, float area)
{
  float expected = 0.0f;
  if (results.find(num) == results.end())
  {
    auto shapes = getObjectOriented();  // Use oop shapes as the base
    for (auto i = 0; i < num; i++)
    {
      expected += shapes[i]->area();
    }
    results[num] = expected;
  }
  else
  {
    expected = results[num];
  }
  float diff = std::abs(area - expected);
  float diffPercent = diff / expected * 100.0f;
  if (diffPercent > 0.002f)   // floating point issues? math breaks for biggest sets. Additional work needed to sort data properly before aggregating, but this is not a purpose of this exercise.
  {
    std::printf("Invalid sum [%d]. Expected: %f Given: %f ErrorPercentage: %f \n", num, expected, area, diffPercent);
    return false;
  }
  return true;
}