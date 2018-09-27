
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
combineCluster (int focusCluster, int evno, STAT * ctkStat, CLUSTER_INTPTS Clstr[MAXCLUSTERHITS],
                int *nClusters, float target_pos[3])
{

  /* declarations */

  int i, j, i1;
  float p1, p2, p3;
  float q1, q2, q3;
  float dist, bestFomSoFar = MAXFLOAT, val, esum;
  int nCandidates = 0;
  int nbase, best, try, bestCombSoFar = -1;


  if (Pars.nprint > 0)
    {
      fprintf (stderr,"\n entered combineCluster: combine cluster # %i in event # %i (nClusters=%i)>>\n", focusCluster, evno,
              *nClusters);
      printCluster (focusCluster, stdout, Clstr);
      fflush (stdout);
    }

  assert (focusCluster < *nClusters);


  /* setup the base of the trial cluster */

  best = *nClusters;            /* best combined cluster will end up here */
  Clstr[best].valid = 0;
  try = best + 1;               /* trial cluster position */

#if (DEBUG)
  fprintf (stderr,"best=%i, try=%i\n", best, try);
  fflush (stdout);
#endif

  /* copy our focus cluster into the trial cluster */

  memcpy ((void *) &Clstr[try], (const char *) &Clstr[focusCluster], sizeof (struct CLUSTER_INTPTS));
  nbase = Clstr[focusCluster].ndet;
  esum = Clstr[focusCluster].esum;

  /* find the mean position of the cluster we want */
  /* to combine from */

  p1 = 0;
  p2 = 0;
  p3 = 0;
  for (i = 0; i < Clstr[focusCluster].ndet; i++)
    {
      p1 += Clstr[focusCluster].intpts[i].xx;
      p2 += Clstr[focusCluster].intpts[i].yy;
      p3 += Clstr[focusCluster].intpts[i].zz;
    };
  p1 /= Clstr[focusCluster].ndet;
  p2 /= Clstr[focusCluster].ndet;
  p3 /= Clstr[focusCluster].ndet;
#if (DEBUG)
  fprintf (stderr,"p1,p2,p3=(%f,%f,%f)\n", p1, p2, p3);
  fflush (stdout);
#endif

  /* Go through and find clusters to include. */
  /* Single hits are always candidates, since FOM */
  /* is alwas zero for them. For the others, we are */
  /* looking for clusters that also have a FOM */
  /* over a treshold wr have specified in chat file */

  for (i = 0; i < *nClusters; i++)
    if (i != focusCluster)
      if (Clstr[i].valid)
        if (Clstr[i].fom > Pars.combineclusters_thresh || Clstr[i].ndet == 1)
          {

            /* find mean position of this candidate cluster */

            q1 = 0;
            q2 = 0;
            q3 = 0;
            for (j = 0; j < Clstr[i].ndet; j++)
              {
                q1 += Clstr[i].intpts[j].xx;
                q2 += Clstr[i].intpts[j].yy;
                q3 += Clstr[i].intpts[j].zz;
              };
            q1 /= Clstr[i].ndet;
            q2 /= Clstr[i].ndet;
            q3 /= Clstr[i].ndet;

            /* Pars.combineMaxDist is already squared */
            /* dist = sqrtf (dist); */

            /* find distance (remove sqrt in final version!) */

            dist = (q1 - p1) * (q1 - p1) + (q2 - p2) * (q2 - p2) + (q3 - p3) * (q3 - p3);
#if (DEBUG)
            fprintf (stderr,"trying cluster %i at distance %f (%f,%f,%f)\n", i, dist, q1, q2, q3);
            fflush (stdout);
#endif

            /* is it a candidate? */

            if (dist < Pars.combineMaxDist)
              {
                if (Pars.nprint > 0)
                  {
                    fprintf (stderr,"__found candidate at distance %5.2f cm:\n", dist);
                    printCluster (i, stdout, Clstr);
                  };

                /* add this candicate combination cluster to the base trial */
                /* cluster (the original cluster we are trying to add to) */

                Clstr[try].esum = esum;
                for (j = 0; j < Clstr[i].ndet; j++)
                  {
                    Clstr[try].ndet++;
                    Clstr[try].esum += Clstr[i].intpts[j].edet;
                    Clstr[try].intpts[j + nbase].edet = Clstr[i].intpts[j].edet;
                    Clstr[try].intpts[j + nbase].xx = Clstr[i].intpts[j].xx;
                    Clstr[try].intpts[j + nbase].yy = Clstr[i].intpts[j].yy;
                    Clstr[try].intpts[j + nbase].zz = Clstr[i].intpts[j].zz;
                    Clstr[try].intpts[j + nbase].timestamp = Clstr[i].intpts[j].timestamp;
                    Clstr[try].intpts[j + nbase].detno = Clstr[i].intpts[j].detno;
#if SIMULATED
                    Clstr[try].intpts[j + nbase].origPos = Clstr[i].intpts[j].origPos;
                    Clstr[try].intpts[j + nbase].esumOrig = Clstr[i].intpts[j].esumOrig;
#endif
                  };

                /* initialize trial cluster for tracking */

                for (j = 0; j < Clstr[try].ndet; j++)
                  Clstr[try].intpts[j].order = -1;
                Clstr[try].valid = 1;
                Clstr[try].fom = MAXFOM;
                Clstr[try].tracked = 0;

#if (DEBUG)
                fprintf (stderr,"combined trial cluster: Clstr[try].ndet=%i\n", Clstr[try].ndet);
                printCluster (try, stdout);
#endif

                /* now track this trial cluster */

                if (Clstr[try].ndet < MAXNOSEG)
                  {
#if (DEBUG)
                    fprintf (stderr,"tracking...\n");
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

                  }
                else
                  {
#if (DEBUG)
                    fprintf (stderr,"could not track..\n");
#endif
                    Clstr[try].tracked = 0;
                  };

                if (Pars.nprint > 0)
                  {
                    fprintf (stderr,"combineCluster: tracked cluster candidate #%i\n", nCandidates);
                    printCluster (try, stdout, Clstr);
                  };

                /* store as best combination so far? */

                if (Clstr[try].fom < bestFomSoFar)
                  {
                    if (Pars.nprint > 0)
                      fprintf (stderr,"best candicate so far, with FOM=%f\n", Clstr[try].fom);
                    nCandidates++;
                    bestFomSoFar = Clstr[try].fom;
                    bestCombSoFar = i;
                    assert (*nClusters < MAXCLUSTERHITS);
                    assert (bestCombSoFar < *nClusters);
                    memcpy ((void *) &Clstr[best], (const char *) &Clstr[try], sizeof (struct CLUSTER_INTPTS));
                  }

              };


          };

#if(1)
  /* we have tried all combinations guided */
  /* by our parameters. Now we have to decide */
  /* whether we accept the best combination */
  /* we found in the process and execute the */
  /* split or not. */

  if (nCandidates > 0 && bestCombSoFar >= 0)
    {

      /* find  the unsplit 'combined' FOM */

#if(0)
      fprintf (stderr,"focusCluster=%i\n", focusCluster);
      fprintf (stderr,"bestCombSoFar=%i\n", bestCombSoFar);
      fprintf (stderr,"Clstr[focusCluster].fom=%f\n", Clstr[focusCluster].fom);
      fprintf (stderr,"Clstr[focusCluster].ndet=%f/n", Clstr[focusCluster].ndet);
      fprintf (stderr,"Clstr[bestCombSoFar].fom=%f\n", Clstr[bestCombSoFar].fom);
      fprintf (stderr,"Clstr[bestCombSoFar].ndet=%f\n", Clstr[bestCombSoFar].ndet);
      fflush (stdout);
#endif
      val = Clstr[focusCluster].fom * Clstr[focusCluster].ndet;
      val += (Clstr[bestCombSoFar].fom * Clstr[bestCombSoFar].ndet);
      i1 = Clstr[focusCluster].ndet + Clstr[bestCombSoFar].ndet;
      val /= i1;

      /* are we better off making the combination? */

      if (Clstr[best].fom < val)
        {

          /* validate the new combined cluster */
          /* careful, nClusters is a pointer!! */

          Clstr[best].valid = 1;
          (*nClusters)++;

          /* invalidate the two clusters we combined from */

          Clstr[focusCluster].valid = 0;
          Clstr[bestCombSoFar].valid = 0;

          /* return success */

          if (Pars.nprint > 0)
            {
              fprintf (stderr,"...CMB: success! made the combination (%f < %f)\n", Clstr[best].fom, val);
              fprintf (stderr,"   combined %i with %i to %i\n", focusCluster, bestCombSoFar, best);
            };

#if(DEBUG)
          nn++;
          if (nn == 2000000000)
            return (DEBUGSTOP);
#endif

          return (0);

        }
      else
        {
          if (Pars.nprint > 0)
            {
              fprintf (stderr,"...CMB: found no combination(s) that improved the overall FOM\n");
              fprintf (stderr,"... Clstr[best].fom=%f, val=%f\n", Clstr[best].fom, val);
            }
          return (1);

        };
    }
  else
    {
      if (Pars.nprint > 0)
        fprintf (stderr,"...CMB: found no nearby combination partner to try\n");
      return (2);

    };
#endif

  /* done, should not get here... */

  return (0);

}
