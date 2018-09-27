
#include <stdlib.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <assert.h>

#ifdef SOLARIS
#include <strings.h>
#include <string.h>
#endif

#ifdef LINUX
#include <string.h>
#endif

#include "ctk.h"

/*---------*/
/* globals */
/*---------*/

#define DEBUG 0

volatile extern TRACKINGPAR Pars;       /* tracking parameters */

/*--------------------------------------------------------*/

int
pairProd (int evno, STAT * ctkStat, CLUSTER_INTPTS Clstr[MAXCLUSTERHITS],
            int *nClusters, float target_pos[3])
{
  /* declarations */

  float d1, dist;
  int try, j;
  static int nn=0;

  nn++;

  if (Pars.nprint > 0 || DEBUG)
    {
      fprintf (stderr,"** pairProd called at event no %i, ", evno);
    };

  /* done */

  return (0);

};
