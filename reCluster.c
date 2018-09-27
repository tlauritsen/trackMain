
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
#include <limits.h>
#include <string.h>

#include "ctk.h"

/*---------*/
/* globals */
/*---------*/

extern TRACKINGPAR Pars;        /* tracking parameters */

#define DEBUG 0

/*--------------------------------------------------------*/

int
reCluster (int ii, int evno, STAT * ctkStat, CLUSTER_INTPTS Clstr[MAXCLUSTERHITS], int *nClusters, float target_pos[3],
           SHELLHIT * shellhit, float reductFactor, int ntimes, float kickoutFOM)
{

/* this function is a little like splitCluster but based on doing */
/* a recluster with a smaller alpha value than was originally done. */
/* For some clusters we ought to be able to get away with that and  */
/* it will be faster than the heavy machinery in splitCluster.  */
/* However, if the two/three clusters that are assumed to make up this */
/* bigger cluster are too near one another, it will not work */
  /* already know the relative angles */
  /* k=Clstr[ii].intpts[i].shellHitPos */

  int nn, i, j, k, l, opi, opj, i1, i2;
  int clusterPos[MAXSHELLHITS];
  float curAlpha, sumNewFom, smallestnontrivial = 0, biggestnontrivial = 0;
  int trialPos[MAXSHELLHITS], oldtrialPos[MAXSHELLHITS], ndiffs = 0;
  int nTrialClusters = 0;
  int trackrtn[MAXSHELLHITS];

#if(DEBUG)
  fprintf (stderr,"\n");
  fprintf (stderr,"reCluster called: evno %10i, cluster %3i\n", evno, ii);
  fprintf (stderr,"reductFactor=%5.2f, ", reductFactor);
  fprintf (stderr,"ntimes=%2i, ", ntimes);
  fprintf (stderr,"kickoutFOM=%6.4f, ", kickoutFOM);
  fprintf (stderr,"*nClusters=%i\n", *nClusters);
  fprintf (stderr,"\n");
  fprintf (stderr,"Cluster we will try to split:\n");
  printCluster (ii, stdout, Clstr);
#endif

  /* initialize */

  for (i = 0; i < Clstr[ii].ndet; i++)
    oldtrialPos[i] = -1;

  curAlpha = Pars.alpha[shellhit->nhit];

  /* loop through smaller cluster angles */
  /* and find a split that makes sense */

  for (nn = 0; nn < ntimes; nn++)
    {

      /* init */

      nTrialClusters = 0;
      for (i = 0; i < Clstr[ii].ndet; i++)
        {
          clusterPos[i] = -1;   /* mark as not found */
          oldtrialPos[i] = 0;   /* trivial old position */
        };

      /* cluster angle for this attempt */

      curAlpha *= reductFactor;
#if(DEBUG)
      fprintf (stderr,"reCluster: try alpha= %5.4f\n", curAlpha / RAD2DEG);
#endif

      /* work on multi-member clusters */

      for (i = 0; i < Clstr[ii].ndet; i++)
        for (j = 0; j < Clstr[ii].ndet; j++)
          if (i != j)
            {

              /* shell hit positions, for lookup */

              opi = Clstr[ii].intpts[i].shellHitPos;
              opj = Clstr[ii].intpts[j].shellHitPos;
#if(0)
              fprintf (stderr,"__%p; %2i %2i; %9.6f, %9.6f\n", shellhit, i, j, shellhit->relAng[opi][opj], Pars.alpha);
#endif
              fflush (stdout);

              if (shellhit->relAng[opi][opj] < curAlpha)
                {

                  /* new cluster */

                  if (clusterPos[i] < 0 && clusterPos[j] < 0)
                    {
                      clusterPos[i] = nTrialClusters;
                      clusterPos[j] = nTrialClusters;
                      nTrialClusters++;
#if(0)
                      fprintf (stderr,"new cluster\n");
#endif
                    };

                  /* join already established cluster */

                  if (clusterPos[i] >= 0 && clusterPos[j] < 0)
                    {
                      clusterPos[j] = clusterPos[i];
#if(0)
                      fprintf (stderr,"join i\n");
#endif
                    };

                  /* join already established cluster */

                  if (clusterPos[i] < 0 && clusterPos[j] > 0)
                    {
                      clusterPos[i] = clusterPos[j];
#if(0)
                      fprintf (stderr,"join j\n");
#endif
                    };

                  /* clusters collide */

                  if (clusterPos[j] >= 0 && clusterPos[i] >= 0 && clusterPos[j] != clusterPos[i])
                    {
#if(0)
                      fprintf (stderr,"collide\n");
#endif
                      /* i1=common cluster index, i2= will be moved to i1 */

                      if (clusterPos[i] > clusterPos[j])
                        {
                          i1 = clusterPos[j];
                          i2 = clusterPos[i];
                        }
                      else
                        {
                          i1 = clusterPos[i];
                          i2 = clusterPos[j];
                        };

                      /* merge high numbered cluster */
                      /* to the low numbered cluster */

                      for (k = 0; k < Clstr[ii].ndet; k++)
                        if (clusterPos[k] == i2)
                          clusterPos[k] = i1;

                      /* bump everyone above i2 down one */
                      /* in cluster index */

                      for (k = 0; k < Clstr[ii].ndet; k++)
                        if (clusterPos[k] > i2)
                          clusterPos[k]--;

                      /* mark we lost one in the counting of */
                      /* number of clusters we have so far */

                      nTrialClusters--;

                    };

                };


            };

      /* assign single hit clusters to the rest */
      /* since they never found any partner */

      for (k = 0; k < Clstr[ii].ndet; k++)
        if (clusterPos[k] < 0)
          {
            clusterPos[k] = nTrialClusters;

            /* since this was a new cluster,  */
            /* we must increment the cluster counter */

            nTrialClusters++;

          };
#if(DEBUG)
      fprintf (stderr,"nn=%i, nTrialClusters=%i for alpha=%9.2f\n", nn, nTrialClusters, curAlpha / RAD2DEG);
#endif

      /* fail if we only found single clusters since they will */
      /* always have a FOM of zero <=== THIS IS PROBLEMATIC! */

      if (nTrialClusters == Clstr[ii].ndet)
        return (8);

      assert (nTrialClusters >= 1);

      /* see if this clustering is different from the previous one */


      ndiffs = 0;
      for (i = 0; i < Clstr[ii].ndet; i++)
        if (clusterPos[i] != oldtrialPos[i])
          ndiffs++;

      /* do we have more clusters that are different!? */

      if (ndiffs > 0 && nTrialClusters > 0)
        {

          for (i = 0; i < Clstr[ii].ndet; i++)
            oldtrialPos[i] = clusterPos[i];

#if(DEBUG)
          fprintf (stderr,"recluster{%i}: %i clusters, alpha=%5.2f\n", nn, nTrialClusters, curAlpha / RAD2DEG);
          for (k = 0; k < Clstr[ii].ndet; k++)
            fprintf (stderr,"%i ", clusterPos[k]);
          fprintf (stderr,") fom: ");
#endif

          /*-------------------------*/
          /* make the trial clusters */
          /*-------------------------*/

          /* initialize the trial clusters */

          for (k = 0; k < nTrialClusters; k++)
            {
              trialPos[k] = k + (*nClusters);
#if(DEBUG)
              fprintf (stderr,"trialPos[%i]=%i, (*nClusters)=%i\n", k, trialPos[k], (*nClusters));
#endif

              Clstr[trialPos[k]].ndet = 0;
              Clstr[trialPos[k]].esum = 0;
              Clstr[trialPos[k]].tracked = 0;
              Clstr[trialPos[k]].valid = 1;
              Clstr[trialPos[k]].fom = 98;
              for (l = 0; l < MAX_NDET; l++)
                Clstr[trialPos[k]].intpts[l].order = -1;
            }

          /* now fill trial clusters */

          for (l = 0; l < Clstr[ii].ndet; l++)
            {
              k = clusterPos[l];

              Clstr[trialPos[k]].esum += Clstr[ii].intpts[l].edet;
              Clstr[trialPos[k]].intpts[Clstr[trialPos[k]].ndet].xx = Clstr[ii].intpts[l].xx;
              Clstr[trialPos[k]].intpts[Clstr[trialPos[k]].ndet].yy = Clstr[ii].intpts[l].yy;
              Clstr[trialPos[k]].intpts[Clstr[trialPos[k]].ndet].zz = Clstr[ii].intpts[l].zz;
              Clstr[trialPos[k]].intpts[Clstr[trialPos[k]].ndet].edet = Clstr[ii].intpts[l].edet;
              Clstr[trialPos[k]].intpts[Clstr[trialPos[k]].ndet].timestamp = Clstr[ii].intpts[l].timestamp;
              Clstr[trialPos[k]].intpts[Clstr[trialPos[k]].ndet].detno = Clstr[ii].intpts[l].detno;
              Clstr[trialPos[k]].intpts[Clstr[trialPos[k]].ndet].shellHitPos = Clstr[ii].intpts[l].shellHitPos;
              Clstr[trialPos[k]].ndet++;

            };

          /* track the trial clusters */

          for (l = 0; l < nTrialClusters; l++)
            {
              ctksort (trialPos[l], Clstr, nClusters + nTrialClusters);
              trackrtn[l] = (-1);
              k = clusterPos[l];
              Clstr[trialPos[l]].fom = MAXFOM;
              Clstr[trialPos[l]].tracked = 0;
//              fprintf (stderr,"Pars.trackOps[Clstr[k].ndet]=%i\n", Pars.trackOps[Clstr[k].ndet]);
              if (Clstr[trialPos[l]].ndet < MAXNOSEG)
                {
                  switch (Pars.trackOps[Clstr[trialPos[l]].ndet])
                    {
                    case 0:
                      trackrtn[l] = ctktk0 (trialPos[l], target_pos, ctkStat, Clstr, nClusters + nTrialClusters);
                      break;
                    case 1:
                      trackrtn[l] = ctktk1 (trialPos[l], target_pos, ctkStat, Clstr, nClusters + nTrialClusters);
                      break;
                    case 3:
                      trackrtn[l] = ctktk3 (trialPos[l], target_pos, ctkStat, Clstr, nClusters + nTrialClusters);
                      break;
                    case 4:
                      trackrtn[l] = ctktk4 (trialPos[l], target_pos, ctkStat, Clstr, nClusters + nTrialClusters);
                      break;
                    case 5:
                      trackrtn[l] = ctktk5 (trialPos[l], target_pos, ctkStat, Clstr, nClusters + nTrialClusters);
                      break;
                    default:
                      fprintf (stderr,"recluster:ctk: tracking option not known!?, Quit\n");
                      fprintf (stderr,"trialPos[l]=%i\n", trialPos[l]);
                      fprintf (stderr,"Clstr[trialPos[l]].ndet=%i\n", Clstr[trialPos[l]].ndet);
                      fprintf (stderr,"Pars.trackOps[Clstr[trialPos[l]].ndet]=%i\n", Pars.trackOps[Clstr[trialPos[l]].ndet]);
                      exit (1);
                    }
                }
              else
                {
                  Clstr[trialPos[l]].fom = MAXFOM;
                  Clstr[trialPos[l]].tracked = 0;
                };
#if(DEBUG)
              fprintf (stderr,"(trial cluster=%i (at pos %i) assigned FOM=%5.3f, tracked=%i)\n", l, trialPos[l],
                      Clstr[trialPos[l]].fom, Clstr[trialPos[l]].tracked);
              printCluster (trialPos[l], stdout, Clstr);
#endif
            };
#if(DEBUG)
          fprintf (stderr,"\n");
#endif

          /* are we doing better!? several checks below... */
          /* a problem in this code is to kick out the right time!! */

          sumNewFom = 0;
          smallestnontrivial = 999;
          biggestnontrivial = 0;
          for (l = 0; l < nTrialClusters; l++)
            {
              i1 = trialPos[l];
              sumNewFom += Clstr[i1].fom;
              if (Clstr[i1].ndet > 1 && Clstr[i1].fom < smallestnontrivial)
                smallestnontrivial = Clstr[i1].fom;
              if (Clstr[i1].ndet > 1 && Clstr[i1].fom > biggestnontrivial)
                biggestnontrivial = Clstr[i1].fom;
            };

          if (sumNewFom < Clstr[ii].fom)
            {

              /* always require sum of new FOMs is better than it was */
              /* but that is not all, see below... */

              /* comment: may need more options to declare success... */

              if (sumNewFom < kickoutFOM)
                {

                  /* kickout success. Just update nClusters since */
                  /* the trial clusters are already valid and in */
                  /* the right position */

                  (*nClusters) += nTrialClusters;

                  /* invalidate the old cluster */

                  Clstr[ii].valid = 0;
#if(DEBUG)
                  fprintf (stderr,"Clstr[ii].fom=%f\n", Clstr[ii].fom);
                  fprintf (stderr,"sumNewFom=%f\n", sumNewFom);
                  fprintf (stderr,"reCluster: invalidated cluster %i\n", ii);
                  printAllClusters ("x.list", "local print", Clstr, nClusters);
#endif

                  /* return with error code */

#if(DEBUG)
                  fprintf (stderr,"reCluster: absolute kickout FOM success, return 0 nn=%i\n\n", nn);
#endif
                  return (0);

                }
              else if ((smallestnontrivial < kickoutFOM))
                {

                  /* at least one non trivial cluster */
                  /* is now good, so we declare victory */

                  (*nClusters) += nTrialClusters;

                  /* invalidate the old cluster */

                  Clstr[ii].valid = 0;

                  /* return with error code */

#if(DEBUG)
                  fprintf (stderr,"reCluster: smallestnontrivial kickout FOM success, return 1, nn=%i\n\n", nn);
#endif
                  return (1);

                }
              else
                {

                  /* invalidate the trial clusters */
                  /* and continue */

                  for (l = 0; l < nTrialClusters; l++)
                    Clstr[trialPos[l]].valid = 0;
#if(DEBUG)
                  fprintf (stderr,"reCluster: no good keep trying..nn=%i\n\n", nn);
                  fflush (stdout);
#endif

                };

            }
          else
            {

#if(0)
              /* not necessary */
              /* invalidate the trial clusters */
              /* and continue */

              for (l = 0; l < nTrialClusters; l++)
                Clstr[trialPos[l]].valid = 0;
#endif
#if(DEBUG)
              fprintf (stderr,"reCluster: no good keep trying..nn=%i\n\n", nn);
              fflush (stdout);
#endif
            };
        };

    };

  /* done */

  /* if we get here, we failed to recluster */
  /* but we might be doing better so...well, not so easy */

  /* invalidate all trial clusters */

#if(0)
  /* not necessary */
  for (l = 0; l < nTrialClusters; l++)
    {
      fprintf (stderr,"invalidate trial pos %i\n", trialPos[l]);
      Clstr[trialPos[l]].valid = 0;
    };
#endif

#if(DEBUG)
  fprintf (stderr,"reCluster: failed, return 8\n");
  fflush (stdout);
#endif
  return (8);

}
