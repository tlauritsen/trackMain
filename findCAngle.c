
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

float
findCAngle (float eg, float ee, float *thc)
{

  /* eg: incoming gamma ray energy */
  /* ee: interaction energy (energy deposited in crystal) */

  /* declarations */

  float egp, cosa;

  /* calc compton angle */

  egp = eg - ee;
  cosa = 1. + 0.511 / eg - 0.511 / egp;

  if (cosa > 1)
    {
    *thc = acosf (1.0);
//    printf("findCAngle returns %f\n", cosa-1.0);
    return(cosa-1.0);
    }
  if (cosa < -1)
    {
    *thc = acosf (-1.0);    
//    printf("findCAngle returns %f\n", -1.0-cosa);
    return(-1.0-cosa);
    }
  else
    {
    *thc = acosf (cosa);
    return (0);
    };

#if(0)
  fprintf (stderr,"findCAngle: eg %f, ee %f, *thc %f\n", eg, ee, *thc);
#endif

  /* done */

  return (0);

}
