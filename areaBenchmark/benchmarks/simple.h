#pragma once

#include <numbers>


namespace objectOriented
{
  class shape
  {
  public:
    shape()
    {
      randomize();
    };
    virtual float area() const = 0;
    virtual void randomize() {};
  };

  class square : public shape
  {
  public:
    square() : shape() {};
    square(float side) : side(side) {}
    virtual float area() const override
    {
      return side * side;
    }
    virtual void randomize() override
    {
      side = static_cast<float>(rand() % 100);
    }
  private:
    float side = 1.0f;
  };

  class rectangle : public shape
  {
  public:
    rectangle() : shape() {};
    rectangle(float width, float height) : width(width), height(height) {}
    virtual float area() const override
    {
      return width * height;
    }
    virtual void randomize() override
    {
      width = static_cast<float>(rand() % 100);
      height = static_cast<float>(rand() % 100);
    }
  private:
    float width = 1.0f;
    float height = 1.0f;
  };

  class triangle : public shape
  {
  public:
    triangle() : shape() {};
    triangle(float base, float height) : base(base), height(height) {}
    virtual float area() const override
    {
      return 0.5f * base * height;
    }
    virtual void randomize() override
    {
      base = static_cast<float>(rand() % 100);
      height = static_cast<float>(rand() % 100);
    }
  private:
    float base = 1.0f;
    float height = 1.0f;
  };

  class circle : public shape
  {
  public:
    circle() : shape() {};
    circle(float radius) : radius(radius) {}
    virtual float area() const override
    {
      return static_cast<float>(std::numbers::pi_v<float> * radius * radius);
    }
    virtual void randomize() override
    {
      radius = static_cast<float>(rand() * 100);
    }
  private:
    float radius = 1.0f;
  };
}


namespace switchStruct
{
  enum class shapeType : int
  {
    circle,
    rectangle,
    square,
    triangle
  };

  struct shape
  {
    shape(shapeType type) : type(type)
    {
      a = static_cast<float>(rand() % 100);
      b = static_cast<float>(rand() % 100);
    }

    shapeType type;
    float a = 1.0f;
    float b = 1.0f;

    float area() const
    {
      switch (type)
      {
      case shapeType::circle:
        return static_cast<float>(std::numbers::pi_v<float> * a * a);
      case shapeType::rectangle:
        return a * b;
      case shapeType::square:
        return a * a;
      case shapeType::triangle:
        return 0.5f * a * b;
      default:
        return 0.0f;
      }
    }
  };
}


namespace coeffArray
{
  enum class shapeType : int
  {
    circle,
    rectangle,
    square,
    triangle
  };
  float const shapeCoeff[4] = { std::numbers::pi_v<float>, 1.0f, 1.0f, 0.5f };

  struct shape
  {
    shape(shapeType type) : type(type)
    {
      a = static_cast<float>(rand() % 100);
      b = (type == shapeType::circle || type == shapeType::square) ? a : static_cast<float>(rand() % 100);
    }

    shapeType type;
    float a = 1.0f;
    float b = 1.0f;

    float area() const
    {
      return shapeCoeff[static_cast<int>(type)] * a * b;
    }
  };
}
