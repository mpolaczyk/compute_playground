
task void getAreaTask(uniform float a[], uniform float b[], uniform float coeff[], uniform int count, uniform float output[])
{
  uniform int from = taskIndex*count;
  uniform int to = from+count;
  varying float areav = 0;
  foreach (index = from ... to)
  {
      areav = coeff[index] * a[index] * b[index];
  }
  output[taskIndex] = reduce_add(areav); // multiple tasks writing to the same uniform, but no collision, optim?

  //print("getAreaTask: %d from: %d to: %d area: %f \n", taskIndex, from, to, output[taskIndex]);
}

export uniform float getAreaMT(uniform float a[], uniform float b[], uniform float coeff[], uniform int count, uniform int tasks)
{
  uniform int countPerTask = count / tasks;
  uniform float * uniform output = uniform new uniform float[tasks];

  launch[tasks] getAreaTask(a, b, coeff, countPerTask, output);
  sync;
  uniform float result = 0.0f;
  for(uniform int i = 0; i < tasks; i ++)
  {
    //print("getAreaMT: %d output: %d \n", i, output[i]);
    result += output[i];
  }
  return result;
}