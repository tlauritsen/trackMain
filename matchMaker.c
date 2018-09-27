
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
matchMaker (int ii, int jj, int evno, STAT * ctkStat, CLUSTER_INTPTS Clstr[MAXCLUSTERHITS],
            int *nClusters, float target_pos[3])
{
  /* declarations */

  float d1, dist;
  int try, j;
  static int nn=0;

  nn++;

  if (Pars.nprint > 0 || DEBUG)
    {
      fprintf (stderr,"** matchMaker called at event no %i, ", evno);
      fprintf (stderr,"combine cluster %i and %i?\n", ii, jj);
    };

  /* find distance beween them */
  /* return if they are too far from one another */
  /* Pars.matchmakerMaxDist is already squared */
  /* dist = sqrtf (dist); */
  /* kickout as soon as we can */

  dist = 0;

  d1 = Clstr[ii].intpts[0].xx - Clstr[jj].intpts[0].xx;
  dist += d1 * d1;
  if (Pars.nprint > 0 || DEBUG)
    printf("1,dist=%f, Pars.matchmakerMaxDist=%f\n", dist,Pars.matchmakerMaxDist);
  if (dist > Pars.matchmakerMaxDist)
    return (1);

  d1 = Clstr[ii].intpts[0].yy - Clstr[jj].intpts[0].yy;
  dist += d1 * d1;
  if (Pars.nprint > 0 || DEBUG)
    printf("2,dist=%f, Pars.matchmakerMaxDist=%f\n", dist,Pars.matchmakerMaxDist);
  if (dist > Pars.matchmakerMaxDist)
    return (1);

  d1 = Clstr[ii].intpts[0].zz - Clstr[jj].intpts[0].zz;
  dist += d1 * d1;
  if (Pars.nprint > 0 || DEBUG)
    printf("3,dist=%f, Pars.matchmakerMaxDist=%f\n", dist,Pars.matchmakerMaxDist);
  if (dist > Pars.matchmakerMaxDist)
    return (1);

  /* if we get here, the two single hits */
  /* are close enought to warrent making */
  /* a trial cluster and track it */

  try = (*nClusters);

  Clstr[try].ndet = 2;
  Clstr[try].esum = Clstr[ii].esum + Clstr[jj].esum;
//printf("esum: %f %f --> %f\n", Clstr[ii].esum, Clstr[jj].esum, Clstr[try].esum);

  Clstr[try].intpts[0].shellHitPos = Clstr[ii].intpts[0].shellHitPos;
  Clstr[try].intpts[0].xx = Clstr[ii].intpts[0].xx;
  Clstr[try].intpts[0].yy = Clstr[ii].intpts[0].yy;
  Clstr[try].intpts[0].zz = Clstr[ii].intpts[0].zz;
  Clstr[try].intpts[0].edet = Clstr[ii].intpts[0].edet;
  Clstr[try].intpts[0].timestamp = Clstr[ii].intpts[0].timestamp;
  Clstr[try].intpts[0].detno = Clstr[ii].intpts[0].detno;

  Clstr[try].intpts[1].shellHitPos = Clstr[ii].intpts[0].shellHitPos;
  Clstr[try].intpts[1].xx = Clstr[jj].intpts[0].xx;
  Clstr[try].intpts[1].yy = Clstr[jj].intpts[0].yy;
  Clstr[try].intpts[1].zz = Clstr[jj].intpts[0].zz;
  Clstr[try].intpts[1].edet = Clstr[jj].intpts[0].edet;
  Clstr[try].intpts[1].timestamp = Clstr[jj].intpts[0].timestamp;
  Clstr[try].intpts[1].detno = Clstr[jj].intpts[0].detno;

  /* initialize trial cluster for tracking */

  for (j = 0; j < Clstr[try].ndet; j++)
    Clstr[try].intpts[j].order = -1;
  Clstr[try].valid = 1;
  Clstr[try].fom = MAXFOM;
  Clstr[try].tracked = 0;

#if (DEBUG)
  fprintf (stderr,"combined trial cluster: Clstr[try].ndet=%i\n", Clstr[try].ndet);
  printCluster (try, stdout, Clstr);
#endif

  /* track the cluster */

#if (DEBUG)
  fprintf (stderr,"tracking...Pars.trackOps[Clstr[try].ndet=%i\n",Pars.trackOps[Clstr[try].ndet]);
  fflush (stdout);
#endif
  ctksort (try, Clstr, nClusters);
  switch (Pars.trackOps[Clstr[try].ndet])
    {
    case 0:
      ctktk0 (try, target_pos, ctkStat, Clstr, nClusters);
      break;
    case 1:
      ctktk1 (try, target_pos, ctkStat, Clstr, nClusters);
      break;
    case 3:
      ctktk3 (try, target_pos, ctkStat, Clstr, nClusters);
      break;
    case 4:
      ctktk4 (try, target_pos, ctkStat, Clstr, nClusters);
      break;
    case 5:
      ctktk5 (try, target_pos, ctkStat, Clstr, nClusters);
      break;
    default:
      fprintf (stderr,"ctk: tracking option not known!?\n, Quit");
      exit (1);
    };

  /* if OK, promote to valid cluster */

  if (Clstr[try].fom < Pars.matchmaker_kickoutfom)
    {
      /* validate the new combined cluster */

      Clstr[try].valid = 1;
      (*nClusters)++;

      /* invalidate the two clusters we combined from */

      Clstr[ii].valid = 0;
      Clstr[jj].valid = 0;

      if (Pars.nprint > 0 || DEBUG)
        {
          fprintf (stderr,"...matchMaker: success! made the combination> \n");
          fflush (stdout);
          printCluster (try, stdout, Clstr);
        };

      return (0);
    }
  else
    {
      Clstr[try].valid = 0;

      if (Pars.nprint > 0 || DEBUG)
        {
          fprintf (stderr,"...matchMaker: combination not OK, FOM=%f \n", Clstr[try].fom);
          fflush (stdout);
        };

      return (2);

    };



  if (1)
    exit (0);

  /* done */

  return (0);

};
