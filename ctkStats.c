
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

#include "ctk.h"

extern TRACKINGPAR Pars;        /* tracking parameters */

/*---------*/
/* globals */
/*---------*/

long __sysconf (int);

#ifdef SOLARIS
#include <strings.h>
#include <string.h>
#define CLK_TCK ((clock_t)_sysconf(3))
#endif

#ifdef LINUX
#include <string.h>
#define CLK_TCK ((__clock_t) __sysconf (2))
#endif

int
ctkStats (struct tms *timesThen, STAT * ctkStat)
{

  /* declarations */

  FILE *fp;
  int i, j, k, i1;
  float r1, r2;
  double d1, d2, d3, d4, totalTime, FM;
  struct rusage r_usage;
  struct tms timesNow;
  long long int il1;
  float rr[1000];
  char str[100];

  int wr_spe (char *, int *, float *);

  /* figure out what cputime we used (first) */

  times ((struct tms *) &timesNow);

  fprintf (stderr,"\n");
  fprintf (stderr,"-------------------\n");
  fprintf (stderr,"done processing data\n");
  fprintf (stderr,"-------------------\n");
  fprintf (stderr,"\n");

  /* close output file */

  fclose (Pars.trackDataStream);
  fprintf (stderr,"\n");
  fprintf (stderr,"trackDataStream output file closed\n");
        
  fprintf (stderr,"\n");
  fprintf (stderr,"bytes read=%lli, %f MB\n", ctkStat->numbytes,(double)ctkStat->numbytes/1024.0/1024.0 );
  fprintf (stderr,"\n");

  fprintf (stderr,"\n");
  fprintf (stderr,"permutation selection statistics\n");
  fprintf (stderr,"--> \"permutation_selection.log\"\n");

  fp = fopen ("permutation_selection.log", "w");

  for (i = 1; i < MAXNOSEG; i++)
    {
      fprintf (fp, "-------ndet=%i--------, jump strategy: ", i);

      /* print jump nomenclature */

      for (j = 0; j < i; j++)
        if (j < Pars.jmpGrpLen[i])
          fprintf (fp, "g");
        else
          fprintf (fp, "t");
      for (j = i; j < MAXNOSEG; j++)
        fprintf (fp, " ");
      fprintf (fp, "\n");

#if(0)
      r1 = 0;
      for (j = 0; j < Pars.nperm[i]; j++)
        r1 += ctkStat->permHit[i][j];
      r1 /= 100;

      if (r1 > 0)
        for (j = 0; j < Pars.nperm[i]; j++)
          if (ctkStat->permHit[i][j] > 0)
            {
              fprintf (fp, "perm# %3i: { ", j);
              for (k = 0; k < i; k++)
                fprintf (fp, "%i ", Pars.permlkup[i][j][k]);
              fprintf (fp, "} ");

              fprintf (fp, " selected ");
              fprintf (fp, "%5.1f%% (%10i)\n", (float) ctkStat->permHit[i][j] / r1, ctkStat->permHit[i][j]);
            };
#endif

    };

  /* track selected, statistics  */

  d1 = 100.0;
  fprintf (stderr,"ctkStat->trackGetCalls     = %12li (%6.2f%%)\n", ctkStat->trackGetCalls, (float) d1);

  d1 = 100.0 * (double) ctkStat->trackPassCount / (double) ctkStat->trackGetCalls;
  fprintf (stderr,"ctkStat->trackPassCount    = %12li (%6.2f%%) [basic conditions]\n", ctkStat->trackPassCount, (float) d1);

  d1 = 100.0 * (double) ctkStat->trackFMok / (double) ctkStat->trackGetCalls;
  fprintf (stderr,"ctkStat->trackFMok         = %12li (%6.2f%%) \n", ctkStat->trackFMok, (float) d1);
  fprintf (stderr,"average permutations/track: %9.3f\n", d1);

  /* tracking stategies */

  fprintf (stderr,"\n");
  fprintf (stderr,"tracking stategies:\n");
  fprintf (stderr,"\n");

  d1 = (double) ctkStat->nperm / (double) ctkStat->trackFMok;
  for (i = 0; i < MAXNOSEG; i++)
    {
      fprintf (stderr,"tracking option for ndet=%i, %i == ", i, Pars.trackOps[i]);
      ctkTrackOpt (Pars.trackOps[i]);
    };
  fprintf (stderr,"\n");
  fprintf (stderr,"\n");

  /* list tracking options */

  fprintf (stderr,"\n");
  fprintf (stderr,"tracking options available\n");
  fprintf (stderr,"\n");

  k = 0;
  for (i = 0; i < 20 && k == 0; i++)
    {
      fprintf (stderr,"%2i: ", i);
      k = ctkTrackOpt (i);
    };

  getrusage (RUSAGE_SELF, &r_usage);

  fprintf (stderr,"\n");
  fprintf (stderr,"maximum resident set size, ru_maxrss.............. %li\n", r_usage.ru_maxrss);
  fprintf (stderr,"integral resident set size, ru_idrss.............. %li\n", r_usage.ru_idrss);
  fprintf (stderr,"page faults not requiring physical I/O, ru_minflt. %li\n", r_usage.ru_minflt);
  fprintf (stderr,"page faults requiring physical I/O, ru_majflt .... %li\n", r_usage.ru_majflt);
  fprintf (stderr,"swaps ru_nswap.................................... %li\n", r_usage.ru_nswap);
  fprintf (stderr,"block input operations, ru_inblock................ %li\n", r_usage.ru_inblock);
  fprintf (stderr,"block output operations, ru_oublock............... %li\n", r_usage.ru_oublock);
  fprintf (stderr,"messages sent, ru_msgsnd.......................... %li\n", r_usage.ru_msgsnd);
  fprintf (stderr,"messages received, ru_msgrcv...................... %li\n", r_usage.ru_msgrcv);
  fprintf (stderr,"signals receivedru_nvcsw, ru_nsignals, ru_nsignals %li\n", r_usage.ru_nsignals);
  fprintf (stderr,"voluntary context switches, ru_nvcsw.............. %li\n", r_usage.ru_nvcsw);
  fprintf (stderr,"involuntary context switches, ru_nivcsw........... %li\n", r_usage.ru_nivcsw);

  /* write out various things */

  fprintf (stderr,"\n");
  fprintf (stderr,"\nCPU usage:\n\n");

  r1 = timesNow.tms_utime - timesThen->tms_utime;
  totalTime = r1;
  fprintf (stderr,"user   CPU time: %9.3f sec\n", r1 / CLK_TCK);
  r1 = timesNow.tms_stime - timesThen->tms_stime;
  totalTime += r1;
  fprintf (stderr,"system CPU time: %9.3f sec\n", r1 / CLK_TCK);
  totalTime /= CLK_TCK;
  fprintf (stderr,"Total  CPU time: %9.3f sec\n", totalTime);




  /* statistics */

  fprintf (stderr,"\n");
  fprintf (stderr,"read %li tracks\n", ctkStat->trackGetCalls);
  fprintf (stderr,"\n");

  fprintf (stderr,"sectors hit statistics\n");

  i1 = 0;
  for (i = 0; i < 64; i++)
    i1 += ctkStat->nhit[i];

  fprintf (stderr,"total sector hits..........: %12i \n", i1);
  fprintf (stderr,"sector hits with neg energy: %12i ", ctkStat->negIntEnergy);
  r1 = (double) ctkStat->negIntEnergy / i1;
  fprintf (stderr," or %8.4f%%\n", r1 * 100);

  d1 = 0;
  d2 = 0;
  for (i = 0; i < 64; i++)
    if (ctkStat->nhit[i] > 0)
      {
        r1 = 100.0 * (float) ctkStat->nhit[i] / (float) i1;
        fprintf (stderr,"%2i > %10i (%5.1f%%) ", i, ctkStat->nhit[i], r1);
        d1 += i * (float) ctkStat->nhit[i] / (float) i1;
        d2 += Pars.alpha[i] * (float) ctkStat->nhit[i] / (float) i1;
        switch (i)
          {
          case 1:
            fprintf (stderr,"   single absorption    ; alpha=%5.2f deg\n", Pars.alpha[i] * 180.0 / M_PI);
            break;
          case 2:
            fprintf (stderr," 1 comption interaction ; alpha=%5.2f deg\n", Pars.alpha[i] * 180.0 / M_PI);
            break;
          default:
            fprintf (stderr,"%2i comption interactions; alpha=%5.2f deg\n", i - 1, Pars.alpha[i] * 180.0 / M_PI);
            break;
          };
      };
  fprintf (stderr,"__ mean number of sectors that fired: %6.2f\n", d1);
  fprintf (stderr,"__ mean alpha angle: %5.2f deg\n", d2 * 180.0 / M_PI);

  system ("ulimit -a > /dev/stderr");

  /* reminder to myself:: */
  /* pthread_attr_setstacksize */
  /* setrlimit(RLIMIT_STACK,.... */

  fprintf (stderr,"\n");
  ctk_pr_trackPar ();
  fprintf (stderr,"\n");
  fprintf (stderr,"size of struct TRACKINGPAR Pars,  %f MB\n", (float) sizeof (struct TRACKINGPAR) / 1e6);
  fprintf (stderr,"MAXNOSEG= %i; ", MAXNOSEG);
  fprintf (stderr,"MAXPERM= %i\n", MAXPERM);
  fprintf (stderr,"\n");

  /* ctkStat->nTrackCalled    :: triggers in GRETINA */
  /* ctkStat->nClusters  :: resolved gamma ray hits  */
  /*                       (with multiple hits per gamma ray) */
  /* ctkStat->nHits      :: total # interactions in GRETA */
  /* The goal is 20,000 gamma rays/sec */

  fprintf (stderr,"-------------------\n");
  fprintf (stderr,"read         %10i events (== triggered events in GRETA) [ctkStat->nTrackCalled]\n", ctkStat->nTrackCalled);
  fprintf (stderr,"constructed  %10i gamma-rays( aka clusters)             [ctkStat->nClusters]\n", ctkStat->nClusters);
  fprintf (stderr,"write out    %10i requests                              [ctkStat->gammasForWriteout]\n",
          ctkStat->gammasForWriteout);
  fprintf (stderr,"write out    %10i requests that are OK1                 [ctkStat->gammasForWriteout_OK1]\n",
          ctkStat->gammasForWriteout_OK1);
  fprintf (stderr,"write out    %10i requests that are OK2                 [ctkStat->gammasForWriteout_OK2]\n",
          ctkStat->gammasForWriteout_OK2);
  fprintf (stderr,"wrote out    %10i gammarays                             [ctkStat->gammasWritten]\n", ctkStat->gammasWritten);
  fprintf (stderr,"wrote out    %10i GEBheader/Payloads                    [ctkStat->trackedEventsWritten]\n",
          ctkStat->trackedEventsWritten);
  fprintf (stderr,"mean event multiplicity        : %6.3f based on tracking\n",
          (float) ctkStat->nClusters / (float) ctkStat->nTrackCalled);
  fprintf (stderr,"-------------------\n");
//  fprintf (stderr,"clustering angle [alpha] %5.2f degrees or %3.3f rad\n", Pars.alpha / RAD2DEG, Pars.alpha);
  fprintf (stderr,"-------------------\n");

  r1 = ctkStat->nTrackCalled / totalTime;
  fprintf (stderr,"processed %8.1f triggered events/sec\n", r1);
  r1 = ctkStat->nClusters / totalTime;
  fprintf (stderr,"processed %8.1f gamma rays/sec (aka clusters)(goal==20,000)\n", r1);
  r1 = 20000 / r1;
  fprintf (stderr,"I.e., we need %i threads, or %i (8 core) nodes, to keep up\n", (int) r1 + 1, (int) (r1 / 8) + 1);
  r1 = ctkStat->nClusters / totalTime;
  fprintf (stderr,"processed %8.1f interactions/sec\n", r1);
  fprintf (stderr,"-------------------\n");
  fprintf (stderr,"\n");

#if(0)
  fprintf (stderr,"\n");
  fprintf (stderr,"tracking return errors\n");
  fprintf (stderr,"\n");
  fprintf (stderr,"ctktk0_errors\n");
  fprintf (stderr,"\n");
  for (i = 0; i < NRETURNCODES; i++)
    if (ctkStat->ctktk0_errors[i] > 0)
      fprintf (stderr,"%2i: %10i\n", i, ctkStat->ctktk0_errors[i]);
  fprintf (stderr,"\n");
  fprintf (stderr,"ctktk1_errors\n");
  fprintf (stderr,"\n");
  for (i = 0; i < NRETURNCODES; i++)
    if (ctkStat->ctktk1_errors[i] > 0)
      fprintf (stderr,"%2i: %10i\n", i, ctkStat->ctktk1_errors[i]);
  fprintf (stderr,"\n");
  fprintf (stderr,"ctktk3_errors\n");
  fprintf (stderr,"\n");
  for (i = 0; i < NRETURNCODES; i++)
    if (ctkStat->ctktk3_errors[i] > 0)
      fprintf (stderr,"%2i: %10i\n", i, ctkStat->ctktk3_errors[i]);
  fprintf (stderr,"\n");
  fprintf (stderr,"ctktk4_errors\n");
  fprintf (stderr,"\n");
  for (i = 0; i < NRETURNCODES; i++)
    if (ctkStat->ctktk4_errors[i] > 0)
      fprintf (stderr,"%2i: %10i\n", i, ctkStat->ctktk4_errors[i]);
  fprintf (stderr,"\n");
  fprintf (stderr,"ctktk5_errors\n");
  fprintf (stderr,"\n");
  for (i = 0; i < NRETURNCODES; i++)
    if (ctkStat->ctktk5_errors[i] > 0)
      fprintf (stderr,"%2i: %10i\n", i, ctkStat->ctktk5_errors[i]);
#endif

  fprintf (stderr,"\n");
  fprintf (stderr,"*first trackings after initial clustering........:");
  fprintf (stderr,"                                %10i %6.1f%%\n", ctkStat->firstClusterTrackCalls,
          100.0 * ctkStat->firstClusterTrackCalls / ctkStat->TrackingCalls);

  if (Pars.recluster1)
    {

      fprintf (stderr,"\n");
      fprintf (stderr,"*reCluster return codes for UNTRACKED (monster) clusters::\n");
      r2 = 0;
      for (i = 0; i < NRETURNCODES; i++)
        r2 += ctkStat->reClusterReturns1[i];

      fprintf (stderr,"total calls.....................................: ");
      fprintf (stderr,"%10i %6.1f%%\n", (int) r2, 100 * r2 / r2);
      for (i = 0; i < NRETURNCODES; i++)
        if (ctkStat->reClusterReturns1[i] > 0)
          {
            fprintf (stderr,"[%2i] ", i);
            switch (i)
              {
              case 0:
                fprintf (stderr,"successful kickout reclustering.............: ");
                break;
              case 1:
                fprintf (stderr,"at least one non trivial is better..........: ");
                break;
              case 8:
                fprintf (stderr,"failed reclustering.........................: ");
                break;
              default:
                fprintf (stderr,"unknown error code..........................: ");
                break;
              };
            fprintf (stderr,"%10i %6.1f%%; ", ctkStat->reClusterReturns1[i], 100 * ctkStat->reClusterReturns1[i] / r2);
            fprintf (stderr,"trackings: %10i %6.1f%%\n", ctkStat->reClusterTrackCalls1[i],
                    100.0 * ctkStat->reClusterTrackCalls1[i] / ctkStat->TrackingCalls);
          };

    }
  else
    fprintf (stderr,"\ndid not attempt to recluster for UNTRACKED (monster) clusters\n");

  if (Pars.splitclusters1)
    {

      fprintf (stderr,"\n");
      fprintf (stderr,"*splitClusters return codes for UNTRACKED (monster) clusters::\n");
      r2 = 0;
      for (i = 0; i < NRETURNCODES; i++)
        r2 += ctkStat->splitClusterReturns1[i];

      fprintf (stderr,"total calls......................................: ");
      fprintf (stderr,"%10i %6.1f%%\n", (int) r2, 100 * r2 / r2);
      for (i = 0; i < NRETURNCODES; i++)
        if (ctkStat->splitClusterReturns1[i] > 0)
          {
            fprintf (stderr,"[%2i] ", i);
            switch (i)
              {
              case 0:
                fprintf (stderr,"successful cluster splits...................: ");
                break;
              case 1:
                fprintf (stderr,"success/best-we-have-return after max tries.: ");
                break;
              case 2:
                fprintf (stderr,"success, hard fom limit kickout.  ..........: ");
                break;
              case 3:
                fprintf (stderr,"success, fraction fom limit kickout.........: ");
                break;
              case 5:
                fprintf (stderr,"cluster too big to handle...................: ");
                break;
              case 6:
                fprintf (stderr,"failed/nosplit after max tries..............: ");
                break;
              case 8:
                fprintf (stderr,"no best split was found.....................: ");
                break;
              default:
                fprintf (stderr,"unknown error code..........................: ");
                break;
              };
            fprintf (stderr,"%10i %6.1f%%; ", ctkStat->splitClusterReturns1[i], 100 * ctkStat->splitClusterReturns1[i] / r2);
            fprintf (stderr,"trackings: %10i %6.1f%%\n", ctkStat->splitClusterTrackCalls1[i],
                    100.0 * ctkStat->splitClusterTrackCalls1[i] / ctkStat->TrackingCalls);
          };

    }
  else
    fprintf (stderr,"\ndid not attempt to split any clusters for UNTRACKED (monster) clusters\n");


  if (Pars.recluster2)
    {

      fprintf (stderr,"\n");
      fprintf (stderr,"*reCluster return codes for TRACKED clusters::\n");
      r2 = 0;
      for (i = 0; i < NRETURNCODES; i++)
        r2 += ctkStat->reClusterReturns2[i];

      fprintf (stderr,"total calls......................................: ");
      fprintf (stderr,"%10i %6.1f%%\n", (int) r2, 100 * r2 / r2);
      for (i = 0; i < NRETURNCODES; i++)
        if (ctkStat->reClusterReturns2[i] > 0)
          {
            fprintf (stderr,"[%2i] ", i);
            switch (i)
              {
              case 0:
                fprintf (stderr,"successful kickout reclustering.............: ");
                break;
              case 1:
                fprintf (stderr,"at least one non trivial is better..........: ");
                break;
              case 8:
                fprintf (stderr,"failed reclustering.........................: ");
                break;
              default:
                fprintf (stderr,"unknown error code..........................: ");
                break;
              };
            fprintf (stderr,"%10i %6.1f%%; ", ctkStat->reClusterReturns2[i], 100 * ctkStat->reClusterReturns2[i] / r2);
            fprintf (stderr,"trackings: %10i %6.1f%%\n", ctkStat->reClusterTrackCalls2[i],
                    100.0 * ctkStat->reClusterTrackCalls2[i] / ctkStat->TrackingCalls);
          };

    }
  else
    fprintf (stderr,"\ndid not attempt to split recluster for TRACKED clusters\n");


  if (Pars.splitclusters2)
    {

      fprintf (stderr,"\n");
      fprintf (stderr,"*splitClusters return codes for TRACKED clusters::\n");
      r2 = 0;
      for (i = 0; i < NRETURNCODES; i++)
        r2 += ctkStat->splitClusterReturns2[i];

      fprintf (stderr,"total calls......................................: ");
      fprintf (stderr,"%10i %6.1f%%\n", (int) r2, 100 * r2 / r2);
      for (i = 0; i < NRETURNCODES; i++)
        if (ctkStat->splitClusterReturns2[i] > 0)
          {
            fprintf (stderr,"[%2i] ", i);
            switch (i)
              {
              case 0:
                fprintf (stderr,"regular successful cluster splits...........: ");
                break;
              case 1:
                fprintf (stderr,"success/best-we-have-return after max tries.: ");
                break;
              case 2:
                fprintf (stderr,"success, hard fom limit kickout.  ..........: ");
                break;
              case 3:
                fprintf (stderr,"success, fraction fom limit kickout.........: ");
                break;
              case 5:
                fprintf (stderr,"cluster too big to handle...................: ");
                break;
              case 6:
                fprintf (stderr,"failed/nosplit after max tries..............: ");
                break;
              case 8:
                fprintf (stderr,"no best split was found.....................: ");
                break;
              default:
                fprintf (stderr,"unknown error code..........................: ");
                break;
              };
            fprintf (stderr,"%10i %6.1f%%; ", ctkStat->splitClusterReturns2[i], 100 * ctkStat->splitClusterReturns2[i] / r2);
            fprintf (stderr,"trackings: %10i %6.1f%%\n", ctkStat->splitClusterTrackCalls2[i],
                    100.0 * ctkStat->splitClusterTrackCalls2[i] / ctkStat->TrackingCalls);
          };

    }
  else
    fprintf (stderr,"\ndid not attempt to split any clusters for TRACKED clusters\n");

  if (Pars.combineclusters)
    {

      fprintf (stderr,"\n");
      fprintf (stderr,"*CombineClusters return code breakdown::\n");
      r2 = 0;
      for (i = 0; i < NRETURNCODES; i++)
        r2 += ctkStat->combineClusterReturns[i];

      fprintf (stderr,"total calls......................................: ");
      fprintf (stderr,"%10i %6.1f%%\n", (int) r2, 100 * r2 / r2);
      for (i = 0; i < NRETURNCODES; i++)
        if (ctkStat->combineClusterReturns[i] > 0)
          {
            fprintf (stderr,"[%2i] ", i);
            switch (i)
              {
              case 0:
                fprintf (stderr,"success combinations........................: ");
                break;
              case 1:
                fprintf (stderr,"unsuccessful/no improve.....................: ");
                break;
              case 2:
                fprintf (stderr,"no nearby candidates........................: ");
                break;
              default:
                fprintf (stderr,"unknown error code..........................: ");
                break;
              };
            fprintf (stderr,"%10i %6.1f%%; ", ctkStat->combineClusterReturns[i], 100 * ctkStat->combineClusterReturns[i] / r2);
            fprintf (stderr,"trackings: %10i %6.1f%%\n", ctkStat->combineClusterTrackCalls[i],
                    100.0 * ctkStat->combineClusterTrackCalls[i] / ctkStat->TrackingCalls);


          }
        else
          fprintf (stderr,"\ndid not attempt to combine any clusters\n");
      fprintf (stderr,"-------------------\n");
    }

      if (Pars.matchmaker)
        {

          fprintf (stderr,"\n");
          fprintf (stderr,"*matchMaker return code breakdown::\n");
          r2 = 0;
          for (i = 0; i < NRETURNCODES; i++)
            r2 += ctkStat->matchMakerReturns[i];

          fprintf (stderr,"total calls......................................: ");
          fprintf (stderr,"%10i %6.1f%%\n", (int) r2, 100 * r2 / r2);
          for (i = 0; i < NRETURNCODES; i++)
            if (ctkStat->matchMakerReturns[i] > 0)
              {
                fprintf (stderr,"[%2i] ", i);
                switch (i)
                  {
                  case 0:
                    fprintf (stderr,"success combinations........................: ");
                    break;
                  case 1:
                    fprintf (stderr,"not close enough............................: ");
                    break;
                  case 2:
                    fprintf (stderr,"FOM over threshold..........................: ");
                    break;
                  default:
                    fprintf (stderr,"unknown error code....................................: ");
                    break;
                  };
                fprintf (stderr,"%10i %6.1f%%; ", ctkStat->matchMakerReturns[i], 100 * ctkStat->matchMakerReturns[i] / r2);
                fprintf (stderr,"trackings: %10i %6.1f%%\n", ctkStat->matchMakerTrackCalls[i],
                        100.0 * ctkStat->matchMakerTrackCalls[i] / ctkStat->TrackingCalls);
              };

        }
      else
        fprintf (stderr,"\ndid not attempt to combine any single hits\n");
      fprintf (stderr,"-------------------\n");


#if(0)
  d1 =
    ctkStat->simpleTrackCalls + ctkStat->splitTrackCalls + ctkStat->combineTrackCalls + ctkStat->matchMakerTrackCalls;
  r1 = 100 * (double) ctkStat->simpleTrackCalls / d1;
  fprintf (stderr,"simpleTrackCalls....: %12i %6.1f%%\n", ctkStat->simpleTrackCalls, r1);
  r1 = 100 * (double) ctkStat->splitTrackCalls / d1;
  fprintf (stderr,"splitTrackCalls.....: %12i %6.1f%%\n", ctkStat->splitTrackCalls, r1);
  r1 = 100 * (double) ctkStat->combineTrackCalls / d1;
  fprintf (stderr,"combineTrackCalls...: %12i %6.1f%%\n", ctkStat->combineTrackCalls, r1);
  r1 = 100 * (double) ctkStat->matchMakerTrackCalls / d1;
  fprintf (stderr,"matchMakerTrackCalls: %12i %6.1f%%\n", ctkStat->matchMakerTrackCalls, r1);
#endif

  /* success rate */

  fprintf (stderr,"\n");

  d2 = (double) ctkStat->nClusters / (double) ctkStat->nTrackedGammas;
  fprintf (stderr,"mean multiplicity of tracked data....: %6.2f\n", (float) d2);

#if(SIMULATED)
  d1 = (double) ctkStat->noTrueClusters / (double) ctkStat->nTrackCalled;
  fprintf (stderr,"mean multiplicity of input data..: %6.2f (for simulated data)\n", (float) d1);
  FM = (d1 - d2) / d1;
  if (FM < 0)
    FM *= -1;
  FM += 1;
  fprintf (stderr,"deduced FM on multiplicity.......: %6.2f\n", (float) FM);


  d3 = ctkStat->goodtrak + ctkStat->badtrack;
  d3 = 100. * (double) ctkStat->goodtrak / d3;
  fprintf (stderr,"** good tracks vs total..........: %5.1f%%\n", d3);
  d4 = 100. * (double) ctkStat->goodtrak / (double) ctkStat->noTrueClusters;
  fprintf (stderr,"** first interaction points......: %5.1f%% (FOM for simulated data)\n", d4);
  fprintf (stderr,"\n");

  d1 = ctkStat->goodtrak + ctkStat->badtrack;
  FM = 100.0 * ctkStat->goodtrak / d1;
  fprintf (stderr,"*** overall FM %6.1f ***\n", FM);
  fprintf (stderr,"\n");

  fprintf (stderr,"full energy vs compton loss for input data: ");
  il1 = ctkStat->fullEnergyEv + ctkStat->comptonLossEnergyEv;
  r1 = (float) ctkStat->fullEnergyEv / (float) il1;
  fprintf (stderr,"%5.1f%%\n", r1 * 100);
#endif

  d1 = 100.0 * (double) ctkStat->notTracked / (double) ctkStat->trackGetCalls;
  fprintf (stderr,"number of Clusters we could not track: %10li, %6.1f%%\n", ctkStat->notTracked, d1);
  d1 = 100.0 * ((double) ctkStat->nTrackCalled - (double) ctkStat->notTracked) / (double) ctkStat->nTrackCalled;
  fprintf (stderr,"number of Clusters we did track......: %10li, %6.1f%%\n", ctkStat->nTrackCalled - ctkStat->notTracked, d1);
  d1 = 100.0 * ((double) ctkStat->badpad) / (double) ctkStat->nTrackCalled;
  fprintf (stderr,"badpad...............................: %10li, %6.1f%%\n", ctkStat->badpad, d1);

  fprintf (stderr,"SegHits..............................: %10li\n", ctkStat->SegHits);
  d1 = 100.0 * ((double) ctkStat->doubleSegHits) / (double) ctkStat->SegHits;
  fprintf (stderr,"doubleSegHits........................: %10li, %6.1f%%\n", ctkStat->doubleSegHits, d1);


#if(1)
  fprintf (stderr,"ctkStat->nTrackCalled=    %10li\n", ctkStat->nTrackCalled);
  fprintf (stderr,"ctkStat->nTrackedGammas=  %10li\n", ctkStat->nTrackedGammas);
  fprintf (stderr,"ctkStat->nClusters=       %10i\n", ctkStat->nClusters);
  fprintf (stderr,"tkStat.notTracked=        %10li\n", ctkStat->notTracked);
  fprintf (stderr,"tkStat.goodtrak=          %10li\n", ctkStat->goodtrak);
  fprintf (stderr,"tkStat.badtrack=          %10li\n", ctkStat->badtrack);
#endif

  fprintf (stderr,"ctkStat->singlehitoutofrange= %lli\n", ctkStat->singlehitoutofrange);
  fprintf (stderr,"ctkStat->badndetElim........= %lli\n", ctkStat->badndetElim);

  /* mean figure of merit */

  d1 = ctkStat->sfom / ctkStat->nClusters;
  fprintf (stderr,"mean figure of merit per event.......: %9.3f\n", (float) (d1));
  d1 = ctkStat->sfom_nt / ctkStat->nsfom_nt;
  fprintf (stderr,"mean FOM/event non-trivial (ndet>1)..: %9.3f\n", (float) (d1));

  /* done */

  fprintf (stderr,"Total  CPU time: %9.3f sec\n", totalTime);
  fprintf (stderr,"ctkStats... done\n");

  for (j = 0; j < 8; j++)
    {
      sprintf (str, "max_alpha%i.spe", j);
      for (i = 0; i < 1000; i++)
        rr[i] = ctkStat->sp_max_ang[i][j];
      i1 = 1000;
      wr_spe (str, &i1, rr);
    };

  /* done */

  return (0);

}
