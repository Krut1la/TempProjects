#ifndef MATHUTILS_H
#define MATHUTILS_H

#include <math.h>

#define M_PI       3.14159265358979323846
#define M_PI_2     1.57079632679489661923
#define M_PI_4     0.785398163397448309616

namespace General
{

RWORD Factorial (RWORD Value)
{
   if (Value == 1)
      return 1;

   return Factorial(Value-1)*Value;
}

// Result M!/(N!*(M-N)!)
RWORD Combinations (RWORD M, RWORD N)
{
   RWORD Result = 1;

   for (RWORD iIteration = 0; iIteration < N; iIteration++)
      Result = Result*(M - iIteration)/(iIteration + 1);

   return Result;
}

}

#endif // MATHUTILS_H