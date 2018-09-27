
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

int
findAngle (float n1[3], float n2[3], float *th)
{

  /* declarations */

  float dotProd = 0;
  int i;

  /* find dot product */

  dotProd = 0;
  for (i = 0; i < 3; i++)
    dotProd += (n1[i] * n2[i]);

  /* find angle */

  if (dotProd > 1)
    dotProd = 1;
  if (dotProd < -1)
    dotProd = -1;

  *th = (float) acosf (dotProd);

  /* done */

  return (0);

}
