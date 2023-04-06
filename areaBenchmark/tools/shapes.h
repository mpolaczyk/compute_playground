#pragma once

#include <numbers>
#include <vector>
#include <random>
#include <assert.h>

namespace shapes
{
  enum class shapeType : int
  {
    circle,
    rectangle,
    square,
    triangle
  };

  float const coeff[4] = { std::numbers::pi_v<float>, 1.0f, 1.0f, 0.5f };

  // Structure of Arrays
  struct vectorized
  {
    vectorized() {}
    vectorized& operator=(const vectorized& other)
    {
      if (this != &other)
      {
        vtype = other.vtype;
        vcoeff = other.vcoeff;
        va = other.va;
        vb = other.vb;
        num = other.num;
      }
      return *this;
    }
    vectorized(int num) : num(num)
    {
      assert(num > 0);
      vtype.reserve(num);
      vcoeff.reserve(num);
      va.reserve(num);
      vb.reserve(num);
    }

    void initAndRandomize(int n, shapeType type)
    {
      assert(n < num);

      static std::default_random_engine e;
      static std::uniform_real_distribution<> dis(0, 1); // rage 0 - 1

      vtype.insert(vtype.begin() + n, type);
      vcoeff.insert(vcoeff.begin() + n, shapes::coeff[static_cast<int>(type)]);
      va.insert(va.begin() + n, static_cast<float>(1.0f + dis(e) / 2.0f));
      vb.insert(vb.begin() + n, (type == shapeType::circle || type == shapeType::square) ? va[n] : static_cast<float>(1.0f + dis(e) / 2.0f));
    }

    std::vector<shapeType> vtype;
    std::vector<float> vcoeff;
    std::vector<float> va;
    std::vector<float> vb;
    int num = 0;

  };

  // Object Oriented
  namespace objectOriented
  {
    class shape
    {
    public:
      shape() { }
      virtual float area() const = 0;
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
        return static_cast<float>(std::numbers::pi_v<float> *radius * radius);
      }
    private:
      float radius = 1.0f;
    };
  }

  // Struct with switch by type
  namespace switchStruct
  {
    struct shape
    {
      shape(shapeType type, float a, float b) : type(type), a(a), b(b) { }

      shapeType type;
      float a = 1.0f;
      float b = 1.0f;

      float area() const
      {
        switch (type)
        {
        case shapeType::circle:
          return static_cast<float>(std::numbers::pi_v<float> *a * a);
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

  // Array of coefficients
  namespace coeffArray
  {
    struct shape
    {
      shape(shapeType type, float a, float b) : type(type), a(a), b(b)
      {
        if (type == shapeType::circle || type == shapeType::square)
        {
          b = a;
        }
      }

      shapeType type;
      float a = 1.0f;
      float b = 1.0f;

      float area() const
      {
        return coeff[static_cast<int>(type)] * a * b;
      }
    };
  }
}
