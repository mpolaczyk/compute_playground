
export uniform float getArea(uniform float a[], uniform float b[], uniform float coeff[], uniform int count)
{
  uniform float area = 0.0f;
  varying float areav = 0;
  foreach (index = 0 ... count)
  {
      areav = coeff[index] * a[index] * b[index];
  }
  area += reduce_add(areav);
  return area;
}