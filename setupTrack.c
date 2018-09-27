
#include <stdlib.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/resource.h>
#include <sys/times.h>
#include <assert.h>
#ifdef SOLARIS
#include <strings.h>
#endif
#ifdef LINUX
#include <string.h>
#endif

#include "ctk.h"

/*---------*/
/* globals */
/*---------*/

volatile extern TRACKINGPAR Pars;

/*-----------------------------------------------------------------------*/

int
setupTrack (struct tms *timesThen, STAT * ctkStat, SHELLHIT * shellhit)
{

  /* declarations */

  int i, in, siz, j, zero = 0, i1, i2;
  float r1;
  char str[132], buffer[512];
  FILE *fp0;
  int ir[NUMAGATAPOS], dummy_i[NUMAGATAPOS];

  /* initialize */

  fprintf (stderr, "pi=%f\n", PI);
  fprintf (stderr, "rad2deg=%f\n", RAD2DEG);

  /* zap the statistics structure */

  bzero ((void *) ctkStat, sizeof (STAT));

  ctkinit (ctkStat);

  /* faculty function lookup */

  fprintf (stderr, "\n");
  fprintf (stderr, "faculty function\n");
  fprintf (stderr, "\n");
  Pars.fac[0] = 1;
  Pars.fac[1] = 1;
  for (i = 2; i < MAXFACULTY; i++)
    Pars.fac[i] = (long long) i *Pars.fac[i - 1];
  for (i = 1; i < MAXFACULTY; i++)
    {
      fprintf (stderr, "fac[%2i]=%20lld   ", i, Pars.fac[i]);
      fprintf (stderr, "check %i\n", (int) (Pars.fac[i] / Pars.fac[i - 1]));
    };

  fprintf (stderr, "sizeof (long long) = %li bytes\n", sizeof (long long));
  fprintf (stderr, "sizeof (long) = %li bytes\n", sizeof (long));
  fprintf (stderr, "sizeof (int) = %li bytes\n", sizeof (int));
  fprintf (stderr, "\n");

  /* sanity checks */

  if (Pars.useCCEnergy == 1 && Pars.useSegEnergy == 1)
    {
      fprintf (stderr, "error: only one of Pars.useCCEnergy and Pars.useSegEnergy can be true\n");
      exit (1);
    };

  /* print tracking parameters that will be used */

  ctk_pr_trackPar ();

  /* start timer */

  times ((struct tms *) timesThen);

  /* done */

  fprintf (stderr, "\n");
  return (0);

}
