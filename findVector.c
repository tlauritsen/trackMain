
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

/* find normalized vector v=x2-x1 */

int
findVector (float x1, float y1, float z1, float x2, float y2, float z2, float *v1, float *v2, float *v3)
{

  /* declarations */

  float sum = 0;
  float vv[3];


  /* subtract */

  vv[0] = x2 - x1;
  sum += (vv[0] * vv[0]);

  vv[1] = y2 - y1;
  sum += (vv[1] * vv[1]);

  vv[2] = z2 - z1;
  sum += (vv[2] * vv[2]);

  /* normalize */

  sum = sqrtf (sum);
  *v1 = vv[0] / sum;
  *v2 = vv[1] / sum;
  *v3 = vv[2] / sum;

  /* debug */

#if(0)
  fprintf (stderr,"x2,y2,z2   = %8.4f,%8.4f,%8.4f\n", x2, y2, z2);
  fprintf (stderr,"x1,y1,z1   = %8.4f,%8.4f,%8.4f\n", x1, y1, z1);
  fprintf (stderr,"vv         = %8.4f,%8.4f,%8.4f\n", vv[0], vv[1], vv[2]);
  fprintf (stderr,"sum        = %f\n", sum);
  fprintf (stderr,"*v1,*v2,*v3= %8.4f,%8.4f,%8.4f\n", *v1, *v2, *v3);
#endif

  /* done */

  return (0);


}
