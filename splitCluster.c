
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

#ifdef SOLARIS
#include <strings.h>
#include <string.h>
#endif

#ifdef LINUX
#include <string.h>
#endif

#include "ctk.h"

#define MAXPNUM 10000

/*---------*/
/* globals */
/*---------*/

#define DEBUG1 0
#define DEBUG2 0

volatile extern TRACKINGPAR Pars;       /* tracking parameters */

/*--------------------------------------------------------*/

int
print_this_pm (int pnum, PM pm[MAXPNUM])
{

  /* declarations */

  int j, k;

  /* list */

  for (j = 0; j < pnum; j++)
    {

      fprintf (stderr,"PTP__ %4i ( ", j);

      for (k = 0; k < pm[j].np1; k++)
        fprintf (stderr,"%i ", pm[j].p1[k]);

      fprintf (stderr,") : ( ");

      for (k = 0; k < pm[j].np2; k++)
        fprintf (stderr,"%i ", pm[j].p2[k]);
      fprintf (stderr," ) ");

      fprintf (stderr,"L(%9.2f, ", pm[j].likely1);
      fprintf (stderr,"%9.2f) ", pm[j].likely2);

      fprintf (stderr,"F(%9.3f, ", pm[j].fom1);
      fprintf (stderr,"%9.3f) ", pm[j].fom2);

      fprintf (stderr,"\n");

    };

  /* done */

  return (0);


}

/*--------------------------------------------------------*/

int
splitCluster (int ii, int evno, STAT * ctkStat, CLUSTER_INTPTS Clstr[MAXCLUSTERHITS], int *nClusters,
              float target_pos[3], float goodEnough, int splitclusters_maxtry, float splitclusters_kickoutfom)
{

  /*---------------------------------*/
  /* split a cluster that is too big */
  /* into two clusters               */
  /*---------------------------------*/

  /* declarations */

  int nbaseSplits, i, j, k, l, nways, i0, i1, i2, i3, i4, i5, i6, nsplit = 0;
  float e1, e2, sumfom, bestsplitfom;
  float dst[MAX_NDET][MAX_NDET], r1, r2;
  int n1, n2, pnum, try1, try2, bestsplit = -1, best1, best2;
  int ntrackstried = 0;
  int order[MAXPNUM];
  PM pm[MAXPNUM], tmppm;

  /* init */

  if (Pars.nprint > 0)
    fprintf (stderr,"\n##--> entered splitCluster: split cluster # %i in event # %i >>\n", ii, evno);


  /* make sure we are not out of bounds */

  if (Clstr[ii].ndet >= MAXFACULTY)
    {
      fprintf (stderr,"splitCluster: ndet=%i is too big to be handled\n", Clstr[ii].ndet);

#if( DEBUG1 || DEBUG2 )
      fprintf (stderr,"return 5\n");
      fflush (stdout);
#endif
      return (5);
    };

  assert (Clstr[ii].ndet < MAXFACULTY);

#if(DEBUG1 || DEBUG2)
  fprintf (stderr,"\n");
  printCluster (ii, stdout, Clstr);
  fflush (stdout);
#endif

  /* initialize */

  bestsplitfom = MAXFLOAT;
  bestsplitfom = 100;
  bestsplit = -1;
  pnum = 0;
  for (j = 0; j < MAXPNUM; j++)
    {
      bzero ((void *) &pm[j], sizeof (struct PM));
    };

  /* find all relative distances of hits in the cluster */

  assert (Clstr[ii].ndet < MAX_NDET);

  for (i = 0; i < Clstr[ii].ndet; i++)
    for (j = i + 1; j < Clstr[ii].ndet; j++)
      {
        r1 = Clstr[ii].intpts[i].xx - Clstr[ii].intpts[j].xx;
        dst[i][j] = r1 * r1;
        r1 = Clstr[ii].intpts[i].yy - Clstr[ii].intpts[j].yy;
        dst[i][j] += r1 * r1;
        r1 = Clstr[ii].intpts[i].zz - Clstr[ii].intpts[j].zz;
        dst[i][j] += r1 * r1;
        dst[i][j] = sqrtf (dst[i][j]);  /* necessary?? */
        dst[i][j] += 0.001;     /* avoid zero */
        dst[j][i] = dst[i][j];
        /* printf("%i,%i %f\n", i,j,dst[i][j]); */
      };


  /* for now we will only consider splitting into two */
  /* note: we cannot use lookup tables, they would be */
  /* too big for large clusters */

  nbaseSplits = (int) (Clstr[ii].ndet / 2);
#if(DEBUG1)
  fprintf (stderr,"nbaseSplits= %i\n", nbaseSplits);
  fflush (stdout);
#endif
  pnum = 0;
  for (j = 1; j <= nbaseSplits; j++)
    if (j < MAXNOSEG)
      if ((Clstr[ii].ndet - j) < MAXNOSEG)
        {
          nways = (int) (Pars.fac[Clstr[ii].ndet] / Pars.fac[j] / Pars.fac[Clstr[ii].ndet - j]);

#if(DEBUG1)
          fprintf (stderr,"split:: (%2i):(%2i); ", j, Clstr[ii].ndet - j);
          fprintf (stderr," %i ways\n", nways);
          fflush (stdout);
#endif

          assert (pnum < MAXPNUM);

          switch (j)
            {
            case 1:
              for (i0 = 0; i0 < Clstr[ii].ndet; i0++)
                {
                  pm[pnum].s1 = 0;
                  n1 = 0;
                  n2 = 0;
                  pm[pnum].s2 = 0;
                  for (k = 0; k < Clstr[ii].ndet; k++)
                    if (k == i0)
                      {
                        n1++;
                        pm[pnum].s1 += Clstr[ii].intpts[k].edet;
                        pm[pnum].p1[pm[pnum].np1] = k;
                        pm[pnum].np1++;
                        assert (pm[pnum].np1 < Clstr[ii].ndet);
                      }
                    else
                      {
                        n1++;
                        pm[pnum].s2 += Clstr[ii].intpts[k].edet;
                        pm[pnum].p2[pm[pnum].np2] = k;
                        pm[pnum].np2++;
                        assert (pm[pnum].np2 < Clstr[ii].ndet);
                      }
                  pnum++;
                };
              break;
            case 2:
              for (i0 = 0; i0 < Clstr[ii].ndet; i0++)
                for (i1 = i0 + 1; i1 < Clstr[ii].ndet; i1++)
                  {
                    pm[pnum].s1 = 0;
                    n1 = 0;
                    n2 = 0;
                    pm[pnum].s2 = 0;
                    for (k = 0; k < Clstr[ii].ndet; k++)
                      if (k == i0 || k == i1)
                        {
                          e1 += Clstr[ii].intpts[k].edet;
                          n1++;
                          pm[pnum].s1 += Clstr[ii].intpts[k].edet;
                          pm[pnum].p1[pm[pnum].np1] = k;
                          pm[pnum].np1++;
                          assert (pm[pnum].np1 < Clstr[ii].ndet);
                        }
                      else
                        {
                          e2 += Clstr[ii].intpts[k].edet;
                          n1++;
                          pm[pnum].s2 += Clstr[ii].intpts[k].edet;
                          pm[pnum].p2[pm[pnum].np2] = k;
                          pm[pnum].np2++;
                          assert (pm[pnum].np2 < Clstr[ii].ndet);
                        }
                    pnum++;
                  };
              break;
            case 3:
              for (i0 = 0; i0 < Clstr[ii].ndet; i0++)
                for (i1 = i0 + 1; i1 < Clstr[ii].ndet; i1++)
                  for (i2 = i1 + 1; i2 < Clstr[ii].ndet; i2++)
                    {
                      pm[pnum].s1 = 0;
                      n1 = 0;
                      n2 = 0;
                      pm[pnum].s2 = 0;
                      for (k = 0; k < Clstr[ii].ndet; k++)
                        if (k == i0 || k == i1 || k == i2)
                          {
                            e1 += Clstr[ii].intpts[k].edet;
                            n1++;
                            pm[pnum].s1 += Clstr[ii].intpts[k].edet;
                            pm[pnum].p1[pm[pnum].np1] = k;
                            pm[pnum].np1++;
                            assert (pm[pnum].np1 < Clstr[ii].ndet);
                          }
                        else
                          {
                            e2 += Clstr[ii].intpts[k].edet;
                            n1++;
                            pm[pnum].s2 += Clstr[ii].intpts[k].edet;
                            pm[pnum].p2[pm[pnum].np2] = k;
                            pm[pnum].np2++;
                            assert (pm[pnum].np2 < Clstr[ii].ndet);
                          }
                      pnum++;
                    };
              break;
            case 4:
              for (i0 = 0; i0 < Clstr[ii].ndet; i0++)
                for (i1 = i0 + 1; i1 < Clstr[ii].ndet; i1++)
                  for (i2 = i1 + 1; i2 < Clstr[ii].ndet; i2++)
                    for (i3 = i2 + 1; i3 < Clstr[ii].ndet; i3++)
                      {
                        pm[pnum].s1 = 0;
                        n1 = 0;
                        n2 = 0;
                        pm[pnum].s2 = 0;
                        for (k = 0; k < Clstr[ii].ndet; k++)
                          if (k == i0 || k == i1 || k == i2 || k == i3)
                            {
                              e1 += Clstr[ii].intpts[k].edet;
                              n1++;
                              pm[pnum].s1 += Clstr[ii].intpts[k].edet;
                              pm[pnum].p1[pm[pnum].np1] = k;
                              pm[pnum].np1++;
                              assert (pm[pnum].np1 < Clstr[ii].ndet);
                            }
                          else
                            {
                              e2 += Clstr[ii].intpts[k].edet;
                              n1++;
                              pm[pnum].s2 += Clstr[ii].intpts[k].edet;
                              pm[pnum].p2[pm[pnum].np2] = k;
                              pm[pnum].np2++;
                              assert (pm[pnum].np2 < Clstr[ii].ndet);
                            }
                        pnum++;
                      };
              break;
            case 5:
              for (i0 = 0; i0 < Clstr[ii].ndet; i0++)
                for (i1 = i0 + 1; i1 < Clstr[ii].ndet; i1++)
                  for (i2 = i1 + 1; i2 < Clstr[ii].ndet; i2++)
                    for (i3 = i2 + 1; i3 < Clstr[ii].ndet; i3++)
                      for (i4 = i3 + 1; i4 < Clstr[ii].ndet; i4++)
                        {
                          pm[pnum].s1 = 0;
                          n1 = 0;
                          n2 = 0;
                          pm[pnum].s2 = 0;
                          for (k = 0; k < Clstr[ii].ndet; k++)
                            if (k == i0 || k == i1 || k == i2 || k == i3 || k == i4)
                              {
                                n1++;
                                pm[pnum].s1 += Clstr[ii].intpts[k].edet;
                                pm[pnum].p1[pm[pnum].np1] = k;
                                pm[pnum].np1++;
                                assert (pm[pnum].np1 < Clstr[ii].ndet);
                              }
                            else
                              {
                                n1++;
                                pm[pnum].s2 += Clstr[ii].intpts[k].edet;
                                pm[pnum].p2[pm[pnum].np2] = k;
                                pm[pnum].np2++;
                                assert (pm[pnum].np2 < Clstr[ii].ndet);
                              }
                          pnum++;
                        };
              break;
            case 6:
              for (i0 = 0; i0 < Clstr[ii].ndet; i0++)
                for (i1 = i0 + 1; i1 < Clstr[ii].ndet; i1++)
                  for (i2 = i1 + 1; i2 < Clstr[ii].ndet; i2++)
                    for (i3 = i2 + 1; i3 < Clstr[ii].ndet; i3++)
                      for (i4 = i3 + 1; i4 < Clstr[ii].ndet; i4++)
                        for (i5 = i4 + 1; i5 < Clstr[ii].ndet; i5++)
                          {
                            pm[pnum].s1 = 0;
                            n1 = 0;
                            n2 = 0;
                            pm[pnum].s2 = 0;
                            for (k = 0; k < Clstr[ii].ndet; k++)
                              if (k == i0 || k == i1 || k == i2 || k == i3 || k == i4 || k == i5)
                                {
                                  e1 += Clstr[ii].intpts[k].edet;
                                  n1++;
                                  pm[pnum].s1 += Clstr[ii].intpts[k].edet;
                                  pm[pnum].p1[pm[pnum].np1] = k;
                                  pm[pnum].np1++;
                                  assert (pm[pnum].np1 < Clstr[ii].ndet);
                                }
                              else
                                {
                                  e2 += Clstr[ii].intpts[k].edet;
                                  n1++;
                                  pm[pnum].s2 += Clstr[ii].intpts[k].edet;
                                  pm[pnum].p2[pm[pnum].np2] = k;
                                  pm[pnum].np2++;
                                  assert (pm[pnum].np2 < Clstr[ii].ndet);
                                }
                            pnum++;
                          };
              break;
            case 7:
              for (i0 = 0; i0 < Clstr[ii].ndet; i0++)
                for (i1 = i0 + 1; i1 < Clstr[ii].ndet; i1++)
                  for (i2 = i1 + 1; i2 < Clstr[ii].ndet; i2++)
                    for (i3 = i2 + 1; i3 < Clstr[ii].ndet; i3++)
                      for (i4 = i3 + 1; i4 < Clstr[ii].ndet; i4++)
                        for (i5 = i4 + 1; i5 < Clstr[ii].ndet; i5++)
                          for (i6 = i5 + 1; i6 < Clstr[ii].ndet; i6++)
                            {
                              pm[pnum].s1 = 0;
                              n1 = 0;
                              n2 = 0;
                              pm[pnum].s2 = 0;
                              for (k = 0; k < Clstr[ii].ndet; k++)
                                if (k == i0 || k == i1 || k == i2 || k == i3 || k == i4 || k == i5 || k == i6)
                                  {
                                    e1 += Clstr[ii].intpts[k].edet;
                                    n1++;
                                    pm[pnum].s1 += Clstr[ii].intpts[k].edet;
                                    pm[pnum].p1[pm[pnum].np1] = k;
                                    pm[pnum].np1++;
                                    assert (pm[pnum].np1 < Clstr[ii].ndet);
                                  }
                                else
                                  {
                                    e2 += Clstr[ii].intpts[k].edet;
                                    n1++;
                                    pm[pnum].s2 += Clstr[ii].intpts[k].edet;
                                    pm[pnum].p2[pm[pnum].np2] = k;
                                    pm[pnum].np2++;
                                    assert (pm[pnum].np2 < Clstr[ii].ndet);
                                  }
                              pnum++;
                            };
              break;
            default:
              fprintf (stderr,"splitCluster: too many base groups, ev# %i, continuing \n", ctkStat->nTrackCalled);
              break;
            }

        };


  /* go through the clusters and evaluate */
  /* which split gives the lowest overall */
  /* FOM compared to the total cluster    */

  best1 = *nClusters;           /* best 1 cluster will end up here */
  best2 = best1 + 1;            /* best 2 cluster will end up here */
  Clstr[best1].valid = 0;
  Clstr[best2].valid = 0;

  try1 = best2 + 1;             /* trial cluster 1 position */
  try2 = try1 + 1;              /* trial cluster 2 position */

#if(DEBUG1)
  fprintf (stderr,"pnum=%i; ", pnum);
  fprintf (stderr,"best1=%i; ", best1);
  fprintf (stderr,"best2=%i; ", best2);
  fprintf (stderr,"try1=%i; ", try1);
  fprintf (stderr,"try2=%i\n", try2);
#endif

  /* try to find an evaluation order */

  for (j = 0; j < pnum; j++)
    {
#if(DEBUG1)
      fprintf (stderr,"%3i/%4i: %i %i, ndet=%i\n", j, pnum, pm[j].np1, pm[j].np2, Clstr[ii].ndet);
      fflush (stdout);
#endif

      /* the more clustered the fragments are */
      /* the more likely they are (in general) */

      pm[j].likely1 = 1;

      r1 = 0.0;
      if (pm[j].np1 > 1)
        {
          for (k = 0; k < pm[j].np1; k++)
            for (l = k + 1; l < pm[j].np1; l++)
              r1 += dst[pm[j].p1[k]][pm[j].p1[l]];
          r1 /= (pm[j].np1 * (pm[j].np1 - 1.0) * 2);
        }
      else
        r1 = Clstr[ii].ndet + 1.0;

      r2 = 0.0;
      if (pm[j].np2 > 1)
        {
          for (k = 0; k < pm[j].np2; k++)
            for (l = k + 1; l < pm[j].np2; l++)
              r2 += dst[pm[j].p2[k]][pm[j].p2[l]];
          r2 /= (pm[j].np2 * (pm[j].np2 - 1.0) * 2);
        }
      else
        r2 = Clstr[ii].ndet + 1.0;

      pm[j].likely1 = 1.0 / r1 + 1.0 / r2;


      /* also evaluate if the number of hits */
      /* in the cluster is proper for the */
      /* energy in the cluster and evaluate */
      /* the likelyhood of a split based on that */

      pm[j].likely2 = 1.0;

#if (0)

      i1 = pm[j].s1 * 1000;
      if (pm[j].np1 > Pars.ndetFun_hi[i1])
        pm[j].likely2 = (1 + pm[j].np1 - Pars.ndetFun_hi[i1]);
      if (pm[j].np1 < Pars.ndetFun_lo[i1])
        pm[j].likely2 = (1 + Pars.ndetFun_lo[i1] - pm[j].np1);

      i1 = pm[j].s2 * 1000;
      if (pm[j].np2 > Pars.ndetFun_hi[i1])
        pm[j].likely2 /= (1 + pm[j].np2 - Pars.ndetFun_hi[i1]);
      if (pm[j].np2 < Pars.ndetFun_lo[i1])
        pm[j].likely2 /= (1 + Pars.ndetFun_lo[i1] - pm[j].np2);
#endif

#if(DEBUG2)
      fprintf (stderr,"pm[%i].likely1=%5.2f, r1=%5.2f, r2=%5.2f ", j, pm[j].likely1, r1, r2);
      fprintf (stderr,"pm[%i].likely2=%5.2f\n", j, pm[j].likely2);
#endif




    };

  /* sort for this order so that in the following */
  /* we try the more likely splits first and hopefully */
  /* 'kickout' early before we use too much CPU time */

  for (j = 0; j < pnum; j++)
    order[j] = j;
  for (j = 0; j < pnum; j++)
    for (k = j + 1; k < pnum; k++)
      if (pm[k].likely1 * pm[k].likely2 > pm[j].likely1 * pm[j].likely2)
        {
          memcpy (&tmppm, &pm[j], sizeof (struct PM));
          memcpy (&pm[j], &pm[k], sizeof (struct PM));
          memcpy (&pm[k], &tmppm, sizeof (struct PM));
          i1 = order[j];
          order[j] = order[k];
          order[k] = i1;
        };

#if(0)
  for (j = 0; j < pnum; j++)
    fprintf (stderr,"%3i, %9.3f, %9.3f, orig pos: %3i\n", j, pm[j].likely1, pm[j].likely2, order[j]);


  if (nn == 3)
    exit (0);
#endif

  /*----------------------*/
  /* go though the splits */
  /*----------------------*/

  for (j = 0; j < pnum; j++)
    {

      /* give up going on? */

      if (j > splitclusters_maxtry)
        {

          /* we have reached the max tries */
          /* so we have to evaluate what to */
          /* do and return to calling function */

          /* this should not really be necessary, but just in case... */

          Clstr[try1].valid = 0;
          Clstr[try2].valid = 0;
          Clstr[best1].valid = 0;
          Clstr[best2].valid = 0;

          if (Pars.nprint > 0)
            fprintf (stderr,"__too many tries ( > %i), return what we have or give up...\n", splitclusters_maxtry);


          if (nsplit > 0)
            {
              if (!Clstr[ii].tracked || (bestsplitfom < Clstr[ii].fom) || bestsplitfom < goodEnough)
                {

                  /* note: for now, we do not care that we waste */
                  /* some space invalidating like this */

                  /* Increase cluster index by two. */
                  /* The new clusters are already there */
                  /* and marked valid */

                  *nClusters += 2;
                  Clstr[best1].valid = 1;
                  Clstr[best2].valid = 1;

                  /* invalidate the old cluster */
                  /* (i.e., virtually nolonger there) */

                  Clstr[ii].valid = 0;

#if( DEBUG1 || DEBUG2 )
                  fprintf (stderr,"return 1\n");
                  fflush (stdout);
#endif
                  return (1);

                }
            };

          /* here if we failed any attempt */

#if( DEBUG1 || DEBUG2 )
          fprintf (stderr,"return 6\n");
          fflush (stdout);
#endif
          return (6);

        };






#if(DEBUG1 || DEBUG2)
      fprintf (stderr,"\n__ %4i ( ", j);
      fflush (stdout);
#endif

      if (pm[j].np1 < MAXNOSEG && pm[j].np2 < MAXNOSEG)
        {

          /* initialize trial clusters */

          Clstr[try1].ndet = 0;
          Clstr[try1].esum = 0;
          Clstr[try1].tracked = 0;
          Clstr[try1].valid = 1;
          Clstr[try1].fom = 0;
          for (k = 0; k < MAX_NDET; k++)
            Clstr[try1].intpts[k].order = -1;

          Clstr[try2].ndet = 0;
          Clstr[try2].esum = 0;
          Clstr[try2].tracked = 0;
          Clstr[try2].valid = 1;
          Clstr[try2].fom = 0;
          for (k = 0; k < MAX_NDET; k++)
            Clstr[try2].intpts[k].order = -1;


          for (k = 0; k < pm[j].np1; k++)
            {

#if(DEBUG1 || DEBUG2)
              fprintf (stderr,"%i ", pm[j].p1[k]);
              fflush (stdout);
#endif

              /* fill trial cluster 1 */

              Clstr[try1].esum += Clstr[ii].intpts[pm[j].p1[k]].edet;
              Clstr[try1].intpts[Clstr[try1].ndet].xx = Clstr[ii].intpts[pm[j].p1[k]].xx;
              Clstr[try1].intpts[Clstr[try1].ndet].yy = Clstr[ii].intpts[pm[j].p1[k]].yy;
              Clstr[try1].intpts[Clstr[try1].ndet].zz = Clstr[ii].intpts[pm[j].p1[k]].zz;
              Clstr[try1].intpts[Clstr[try1].ndet].edet = Clstr[ii].intpts[pm[j].p1[k]].edet;
              Clstr[try1].intpts[Clstr[try1].ndet].timestamp = Clstr[ii].intpts[pm[j].p1[k]].timestamp;
              Clstr[try1].intpts[Clstr[try1].ndet].detno = Clstr[ii].intpts[pm[j].p1[k]].detno;
              Clstr[try1].intpts[Clstr[try1].ndet].shellHitPos = Clstr[ii].intpts[pm[j].p1[k]].shellHitPos;
              Clstr[try1].ndet++;
            }

#if(DEBUG1 || DEBUG2)
          fprintf (stderr,") : (");
#endif

          for (k = 0; k < pm[j].np2; k++)
            {
              /* fill trial cluster 2 */
#if(DEBUG1 || DEBUG2)
              fprintf (stderr,"%i ", pm[j].p2[k]);
#endif
              Clstr[try2].esum += Clstr[ii].intpts[pm[j].p2[k]].edet;
              Clstr[try2].intpts[Clstr[try2].ndet].xx = Clstr[ii].intpts[pm[j].p2[k]].xx;
              Clstr[try2].intpts[Clstr[try2].ndet].yy = Clstr[ii].intpts[pm[j].p2[k]].yy;
              Clstr[try2].intpts[Clstr[try2].ndet].zz = Clstr[ii].intpts[pm[j].p2[k]].zz;
              Clstr[try2].intpts[Clstr[try2].ndet].edet = Clstr[ii].intpts[pm[j].p2[k]].edet;
              Clstr[try2].intpts[Clstr[try2].ndet].timestamp = Clstr[ii].intpts[pm[j].p2[k]].timestamp;
              Clstr[try2].intpts[Clstr[try2].ndet].detno = Clstr[ii].intpts[pm[j].p2[k]].detno;
              Clstr[try2].intpts[Clstr[try2].ndet].shellHitPos = Clstr[ii].intpts[pm[j].p2[k]].shellHitPos;
              Clstr[try2].ndet++;
            };

#if(DEBUG1 || DEBUG2)
          fprintf (stderr," ) ");
#endif

          /* sort for energy */

          ctksort (try1, Clstr, nClusters);
          ctksort (try2, Clstr, nClusters);

          /* track try1 clusters */

          ntrackstried++;

#if(DEBUG1)
          fprintf (stderr,"Clstr[try1].ndet=%i\n", Clstr[try1].ndet);
          fprintf (stderr,"Pars.trackOps[Clstr[try1].ndet]=%i\n", Pars.trackOps[Clstr[try1].ndet]);
#endif
          pm[j].fom1 = -1;
          Clstr[try1].tracked = 0;
          if (Clstr[try1].ndet < MAXNOSEG)
            switch (Pars.trackOps[Clstr[try1].ndet])
              {
              case 0:
                ctktk0 (try1, target_pos, ctkStat, Clstr, nClusters);
                break;
              case 1:
                ctktk1 (try1, target_pos, ctkStat, Clstr, nClusters);
                break;
              case 3:
                ctktk3 (try1, target_pos, ctkStat, Clstr, nClusters);
                break;
              case 4:
                ctktk4 (try1, target_pos, ctkStat, Clstr, nClusters);
                break;
              case 5:
                ctktk5 (try1, target_pos, ctkStat, Clstr, nClusters);
                break;
              default:
                fprintf (stderr,"ctk: tracking option not known!?\n, Quit");
                exit (1);
              };

          pm[j].fom1 = Clstr[try1].fom;

          /* track try2 clusters, -- if it makes sense to try */

          pm[j].fom2 = -1;
          if (Clstr[try1].fom < bestsplitfom && Clstr[try1].fom < Clstr[ii].fom)
            {

              ntrackstried++;
#if(DEBUG1)
              fprintf (stderr,"Clstr[try2].ndet=%i\n", Clstr[try2].ndet);
              fprintf (stderr,"Pars.trackOps[Clstr[try2].ndet]=%i\n", Pars.trackOps[Clstr[try2].ndet]);
#endif
              Clstr[try2].tracked = 0;
              if (Clstr[try2].ndet < MAXNOSEG)
                switch (Pars.trackOps[Clstr[try2].ndet])
                  {
                  case 0:
                    ctktk0 (try2, target_pos, ctkStat, Clstr, nClusters);
                    break;
                  case 1:
                    ctktk1 (try2, target_pos, ctkStat, Clstr, nClusters);
                    break;
                  case 3:
                    ctktk3 (try2, target_pos, ctkStat, Clstr, nClusters);
                    break;
                  case 4:
                    ctktk4 (try2, target_pos, ctkStat, Clstr, nClusters);
                    break;
                  case 5:
                    ctktk5 (try2, target_pos, ctkStat, Clstr, nClusters);
                    break;
                  default:
                    fprintf (stderr,"ctk: tracking option not known!?\n, Quit");
                    exit (1);
                  };

            }
          else
            Clstr[try2].tracked = 0;

          pm[j].fom2 = Clstr[try2].fom;

          /* at this point we have the two trial clusters tracked */
          /* and we can evaluate whether the split is better than */
          /* the best split we have seen so far */

          if (Clstr[try1].tracked && Clstr[try2].tracked)
            {
              /* combined FOM of split event */

              sumfom = Clstr[try1].fom + Clstr[try2].fom;
#if(DEBUG1)
              fprintf (stderr,"succesfully tracked ");
#endif

#if(0)
              if (nn < 1000)
                {
                  nn++;
                  fprintf (stderr,"%5i:: ", j);
                  fprintf (stderr,"%9.2f ", Clstr[ii].fom);
                  fprintf (stderr,"%2i,%6.3f:      ", Clstr[ii].ndet, Clstr[ii].esum);
                  fprintf (stderr,"{%2i,%2i}", Clstr[try2].ndet, Clstr[try2].ndet);
                  fprintf (stderr,"%9.2f", sumfom);
                  fprintf (stderr,"[%9.2f,", Clstr[try1].fom);
                  fprintf (stderr,"%9.2f] ", Clstr[try2].fom);
                  fprintf (stderr,"[%9.2f,", Clstr[try1].esum);
                  fprintf (stderr,"%9.2f] ", Clstr[try2].esum);
                  fprintf (stderr,"(%5.2f ", Clstr[ii].ndet / Clstr[ii].esum);
                  fprintf (stderr,"%5.2f ", Clstr[try1].ndet / Clstr[try1].esum);
                  fprintf (stderr,"%5.2f) ", Clstr[try2].ndet / Clstr[try2].esum);
                  fprintf (stderr,"\n");
                };
#endif

              if (sumfom < bestsplitfom)
                {
                  nsplit++;
#if(DEBUG1)
                  fprintf (stderr,"** best split so far ");
#endif

                  bestsplitfom = sumfom;
                  bestsplit = j;

                  /* transfer to best1 and best2 position */

#if(DEBUG1)
                  fprintf (stderr," transfer\n");
#endif
                  memcpy ((void *) &Clstr[best1], (const char *) &Clstr[try1], sizeof (struct CLUSTER_INTPTS));
                  Clstr[best1].valid = 1;       /* should not be necessary */
                  memcpy ((void *) &Clstr[best2], (const char *) &Clstr[try2], sizeof (struct CLUSTER_INTPTS));
                  Clstr[best2].valid = 1;       /* should not be necessary */

#if(DEBUG1)
                  fprintf (stderr,"best------------\n");
                  printCluster (best1, stdout, Clstr);
                  printCluster (best2, stdout, Clstr);
                  fprintf (stderr,"try------------\n");
                  printCluster (try1, stdout, Clstr);
                  printCluster (try2, stdout, Clstr);

#endif
                  /* if this result is better than the 'split cluster */
                  /* kickout FOM', then we return at this point with */
                  /* what we have */

                  if (bestsplitfom < splitclusters_kickoutfom)
                    {
                      *nClusters += 2;
                      Clstr[ii].valid = 0;
                      if (Pars.nprint > 0)
                        {
#if(DEBUG1 || DEBUG2)
                          fprintf (stderr,"(%9.4f ", pm[j].likely1);
                          fprintf (stderr,"%9.4f) ", pm[j].likely2);
                          fprintf (stderr,"%9.4f ", pm[j].likely1 * pm[j].likely2);
                          fprintf (stderr,"sumfom=%f", sumfom);
                          if (sumfom == bestsplitfom)
                            fprintf (stderr," best\n");
                          fflush (stdout);
#endif
                          fprintf (stderr,"kickout,bestsplitfom = %f\n", bestsplitfom);
                          fprintf (stderr,"__ntrackstried=%i\n", ntrackstried);
                        };

#if( DEBUG1 || DEBUG2 )
                      fprintf (stderr,"return 2\n");
                      fflush (stdout);
#endif
                      return (2);
                    };

                  /* if this result is better than the 'split cluster */
                  /* 'goodenough fraction' kickout FOM', then we return at this */
                  /* point with what we have */

                  if (bestsplitfom < goodEnough)
                    {
                      *nClusters += 2;
                      Clstr[ii].valid = 0;
                      if (Pars.nprint > 0)
                        {
#if(DEBUG1 || DEBUG2)
                          fprintf (stderr,"(%9.4f ", pm[j].likely1);
                          fprintf (stderr,"%9.4f) ", pm[j].likely2);
                          fprintf (stderr,"%9.4f ", pm[j].likely1 * pm[j].likely2);
                          fprintf (stderr,"sumfom=%f", sumfom);
                          if (sumfom == bestsplitfom)
                            fprintf (stderr," best\n");
                          fflush (stdout);
#endif
                          fprintf (stderr,"kickout,bestsplitfom = %f\n", bestsplitfom);
                          fprintf (stderr,"__ntrackstried=%i\n", ntrackstried);
                        };

#if( DEBUG1 || DEBUG2 )
                      fprintf (stderr,"return 3\n");
                      fflush (stdout);
#endif
                      return (3);
                    };


                }
#if(DEBUG1)
              else
                fprintf (stderr," not best split \n");
#endif


            }
#if(DEBUG1)
          else
            {
              fprintf (stderr," could not track; ");
              if (Clstr[try1].tracked == 0)
                fprintf (stderr,"no tracking of try1");
              if (Clstr[try2].tracked == 0)
                fprintf (stderr,"no tracking of try2");
              fprintf (stderr,"\n");
            };

#endif

        };

#if(DEBUG1 || DEBUG2)
      fprintf (stderr,"(%9.4f ", pm[j].likely1);
      fprintf (stderr,"%9.4f) ", pm[j].likely2);
      fprintf (stderr,"%9.4f ", pm[j].likely1 * pm[j].likely2);
      fprintf (stderr,"sumfom=%f", sumfom);
      if (sumfom == bestsplitfom)
        fprintf (stderr," best ");
      fflush (stdout);
#endif

    };

  /* In case we did not manage to split */
  /* the event, we clean up and return */

  if (bestsplit < 0)
    {

      Clstr[try1].valid = 0;
      Clstr[try2].valid = 0;
      Clstr[best1].valid = 0;
      Clstr[best2].valid = 0;

      if (Pars.nprint > 0)
        fprintf (stderr,"__ NO best split was found, quit nothing done\n");

#if( DEBUG1 || DEBUG2 )
      fprintf (stderr,"return 8\n");
      fflush (stdout);
#endif
      return (8);

    };

#if(DEBUG1)
  fprintf (stderr,"\n\nbest split was %i with a summed FOM of %f\n", bestsplit, bestsplitfom);

  printCluster (best1, stdout, Clstr);
  printCluster (best2, stdout, Clstr);


#endif

  /* execute the split of the original event */
  /* simply wasn't tracked at all or if we */
  /* do better in a split */

  if (nsplit > 0)
    {
      if (!Clstr[ii].tracked || (bestsplitfom < Clstr[ii].fom || bestsplitfom < goodEnough))
        {

          /* note: for now, we do not care that we waste */
          /* some space invalidating like this */

          /* Increase cluster index by two. */
          /* The new clusters are already there */
          /* and marked valid */

          *nClusters += 2;

          /* invalidate the old cluster */
          /* (i.e., virtually nolonger there) */

          Clstr[ii].valid = 0;

          if (Pars.nprint > 0)
            {
              fprintf (stderr,"\n");
              fprintf (stderr,"__succesfully split cluster :-)\n");

              fprintf (stderr,"old fom=%9.3f, ", Clstr[ii].fom);
              fprintf (stderr,"(%9.3f, ", Clstr[best1].fom);
              fprintf (stderr,"%9.3f) ", Clstr[best2].fom);
              fprintf (stderr," new combined fom = %9.3f; ", bestsplitfom);
              fprintf (stderr,"__ntrackstried=%i\n", ntrackstried);

              fprintf (stderr,"split cluster % i into %i and %i\n", ii, best1, best2);

            };

          /* succesful return */

#if( DEBUG1 || DEBUG2 )
          fprintf (stderr,"return 0\n");
          fflush (stdout);
#endif
          return (0);

        }
      else
        {

          /* this should not really be necessary, but just in case... */

          Clstr[try1].valid = 0;
          Clstr[try2].valid = 0;
          Clstr[best1].valid = 0;
          Clstr[best2].valid = 0;

          if (Pars.nprint > 0)
            {
              fprintf (stderr,"__could not find better split of cluster; ");
              fprintf (stderr,"  old fom=%9.3f, ", Clstr[ii].fom);
              fprintf (stderr,"(%9.3f, ", Clstr[best1].fom);
              fprintf (stderr,"%9.3f) ", Clstr[best2].fom);
              fprintf (stderr," best fom= %9.3f\n", bestsplitfom);
              fprintf (stderr,"__ntrackstried=%i\n", ntrackstried);
            };

#if( DEBUG1 || DEBUG2 )
          fprintf (stderr,"return 8\n");
          fflush (stdout);
#endif
          return (8);

        };
    };

  /* done, we should not get here!! */

  assert (1 == 2);
  return (0);
}
