
export uniform float getArea(uniform float a[], uniform float b[], uniform float coeff[], uniform int from, uniform int to)
{
  varying float varea = 0;
  foreach (index = from ... to)
  {
      varea = varea + coeff[index] * a[index] * b[index];
  }
  uniform float area = reduce_add(varea);
  return area;
}