
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

#define SPECIALFIXDOUBLEHITS 0

#include "ctk.h"

/*---------*/
/* globals */
/*---------*/

extern TRACKINGPAR Pars;        /* tracking parameters */

/*----------------------------------------------------------------------------*/

trapbad (char *sid, TRACK_STRUCT * track)
{
  PAYLOAD *ptinp;
  GEBDATA *ptgd;
  int checksum, *iptr;
  int i, j;
  char str[180];

  ptgd = track->gd;
  ptinp = track->payload;
  for (i = 0; i < track->n; i++)
    {
      if (ptgd->timestamp == 5411216)
        {
          fprintf (stderr,"PAYLOAD=%i\n", sizeof (PAYLOAD));
          checksum = 0;
          iptr = (int *) ptinp;
          for (j = 0; j < (ptgd->length / sizeof (int)); j++)
            {
              checksum += *iptr;
              if (j >= 110 && j <= 120)
                fprintf (stderr,"%4i> %15i\n", j, *iptr);
              iptr++;
            };

          if (checksum != -63966280)
            {

              fprintf (stderr,"in %s::trapbad hit! ptgd->timestamp==5411216\n", sid);
              fprintf (stderr,"ptgd->length=%i\n", ptgd->length);
              fprintf (stderr,"ptgd->type=%i\n", ptgd->type);
              fprintf (stderr,"ptgd->timestamp=%lli\n", ptgd->timestamp);

              fprintf (stderr,"^^^--- ");
              get_GEB_Type_str (ptgd->type, str);
              fprintf (stderr,"%s", str);
              fprintf (stderr,"TS=%20lli ", ptgd->timestamp);
              fprintf (stderr,"checksum=%15i, =! -63966280 ", checksum);
              fprintf (stderr,"\nQUIT\n");
              fflush (stdout);

              exit (0);
            };
        };

      ptgd++;
      ptinp++;

    };
}

/*----------------------------------------------------------------------------*/

int
findDetectorNumbers (TRACK_STRUCT * track)
{

  /* declarations */

  PAYLOAD *ptinp;
  GEBDATA *ptgd;
  int i, crystalno, moduleno, i1;
  CRYS_INTPTS *gtinp;

  /* mod data */

  ptinp = track->payload;
  ptgd = track->gd;

  for (i = 0; i < track->n; i++)
    {
      if (ptgd->type == GEB_TYPE_DECOMP)
        {

          gtinp = (CRYS_INTPTS *) ptinp;

          /* manipulate pad if necessary */

//          i1=gtinp->pad&0xffffff00;
//          i1=i1>>8;
//          *G4_evno=i1;
//          gtinp->pad&=0x000000ff;
//         if (gtinp->pad>=128) gtinp->pad-=128;

          if ( (gtinp->pad & 0x000000ff) == 0)
            {
              crystalno = (gtinp->crystal_id & 0x0003);
              moduleno = (gtinp->crystal_id & 0xfffc) >> 2;
              Pars.detNo[i] = moduleno * 4 + crystalno;
            }
          else
            {
              if (Pars.nprint > 0)
                fprintf (stderr,"findDetectorNumbers: found pad=%i, return %i\n", gtinp->pad, BADPAD);
              return (BADPAD);
            };

       
       if (Pars.detNo[i]<0 || Pars.detNo[i]> MAXDETNO)
         {
         printf("bad detector number: %i, set to 1\n", Pars.detNo[i]);
         Pars.detNo[i]=1;
         };

        };
      ptinp++;
      ptgd++;
    };

  /* done */

  return (0);

}

/*--------------------------------------------------------*/

int
transferClusters (SHELLHIT * shellhit, CLUSTER_INTPTS Clstr[MAXCLUSTERHITS], int *nClusters)
{

  /* declarations */

  int i, j, ClstrNum, insertPos;

  /* init ALL cluster counters and sum energy */
  /* and valid flags -- the rest we just overwrite! */

  for (i = 0; i < MAXCLUSTERHITS; i++)
    {
      Clstr[i].ndet = 0;
      Clstr[i].esum = 0;
      Clstr[i].valid = 0;
      Clstr[i].tracked = 0;
      Clstr[i].fom = MAXFOM;
      for (j = 0; j < MAX_NDET; j++)
        Clstr[i].intpts[j].order = -1;
    };

  /* loop over all shellhits and use the */
  /* cluster number to fill the cluster array */

  for (i = 0; i < shellhit->nhit; i++)
    {

      /* transfer to proper cluster array (index: ClstrNum) */

      ClstrNum = shellhit->ClusterNumber[i];

      /* and right position in this cluster (index: insertPos) */
      /* (the cluster may already have some members) */

      insertPos = Clstr[ClstrNum].ndet;
      if (insertPos == 0)
        Clstr[ClstrNum].valid = 1;

      /* check we are not building up to large a hit pattern */

      if (insertPos >= MAX_NDET)
        {
          fprintf (stderr,"too may hits...\n");
          return (1);
        };

      assert (insertPos < MAX_NDET);

      Clstr[ClstrNum].trackno = ClstrNum;
      Clstr[ClstrNum].intpts[insertPos].shellHitPos = i;        /* need later */
      Clstr[ClstrNum].intpts[insertPos].xx = shellhit->XX[i];
      Clstr[ClstrNum].intpts[insertPos].yy = shellhit->YY[i];
      Clstr[ClstrNum].intpts[insertPos].zz = shellhit->ZZ[i];
      Clstr[ClstrNum].intpts[insertPos].edet = shellhit->edet[i];
      Clstr[ClstrNum].intpts[insertPos].timestamp = shellhit->timestamp[i];
      Clstr[ClstrNum].intpts[insertPos].detno = shellhit->detno[i];

      /* for now add up track energies to */
      /* fill .esum. We will reassign later */

      Clstr[ClstrNum].esum += shellhit->edet[i];

      /* update cluster hit counter in this cluster */

      Clstr[ClstrNum].ndet++;

      /* check that no cluster has not grown too big to handle */

      if (Clstr[ClstrNum].ndet >= MAX_NDET)
        return (TOOMANYHITS);

    };

  /* done */

  return (0);


}

/*----------------------------------------------------------------------------*/

int
modData (TRACK_STRUCT * track, STAT * ctkStat)
{

  /* declarations */

  float sum, segsum[MAX_SEG], ff;
  int j, i;
  int crystalNumber, holeNum, i1;
  PAYLOAD *ptinp;
  GEBDATA *ptgd;
  CRYS_INTPTS *gtinp;
  static int nn = 0;

  /* mod data */

  ptinp = track->payload;
  ptgd = track->gd;
  for (i = 0; i < track->n; i++)
    {
      if (ptgd->type == GEB_TYPE_DECOMP)
        {

          gtinp = (CRYS_INTPTS *) ptinp;

          holeNum = ((gtinp->crystal_id & 0xfffc) >> 2);
          if (Pars.AGATA_data == 0)
            {
              crystalNumber = (gtinp->crystal_id & 0x0003);
              i1 = holeNum * 4 + crystalNumber;
            }
          else if (Pars.AGATA_data == 1)
            {
              crystalNumber = (gtinp->crystal_id & 0x0003);
              i1 = holeNum * 3 + crystalNumber;
            };

          /* calibrate the CC energy */

//          printf("det %i, e=%f , %f %f, ",i1,gtinp->tot_e,Pars.CCcal_offset[i1],Pars.CCcal_gain[i1]);
          gtinp->tot_e = gtinp->tot_e * Pars.CCcal_gain[i1] + Pars.CCcal_offset[i1];
//          printf(" ---> e=%f\n", gtinp->tot_e);

          /* calibrate segment energies */

          for (j = 0; j < gtinp->num; j++)
            {
//            printf("%3i,%3i: e=%7.1f --> ", i1,gtinp->intpts[j].seg,gtinp->intpts[j].e);
              gtinp->intpts[j].e = gtinp->intpts[j].e * Pars.SEGcal_gain[i1][gtinp->intpts[j].seg]
                + Pars.SEGcal_offset[i1][gtinp->intpts[j].seg];
//            printf("%7.1f (%7.3f,%7.3f)\n",gtinp->intpts[j].e , Pars.SEGcal_gain[i1][gtinp->intpts[j].seg],Pars.SEGcal_offset[i1][gtinp->intpts[j].seg]);
            }

          if (Pars.useSegEnergy)
            {

              /* find segment sum energies for proper scaling */
              /* Note: no 'dino' effect correction yet, just simple scaling */
              /* avoid negative seg energies */

              for (j = 0; j < MAX_SEG; j++)
                segsum[j] = 0;

              for (j = 0; j < gtinp->num; j++)
                if (gtinp->intpts[j].e > 0)
                  {
                    segsum[gtinp->intpts[j].seg] += gtinp->intpts[j].e;
                  };

              /* replace interation energy from decomp with segment energies */
              /* scaled according to hits and sume energy etc... */

              for (j = 0; j < gtinp->num; j++)
                {
                  ff = gtinp->intpts[j].e / segsum[gtinp->intpts[j].seg];
                  gtinp->intpts[j].e = gtinp->intpts[j].seg_ener * ff;
                }

            }

          if (Pars.useCCEnergy)
            {

              /* replace interation energies bases on CC energy */
              /* only works for low countrates */

              sum = 0;
              for (j = 0; j < gtinp->num; j++)
                if (gtinp->intpts[j].e > 0)
                  sum += gtinp->intpts[j].e;

              for (j = 0; j < gtinp->num; j++)
                if (gtinp->intpts[j].e > 0)
                  gtinp->intpts[j].e = gtinp->tot_e * (gtinp->intpts[j].e / sum);
            }


        };
      ptinp++;
      ptgd++;
    };

  /* done */

  return (0);

}

/*--------------------------------------------------------*/

int
assignClusteresum (CLUSTER_INTPTS Clstr[MAXCLUSTERHITS], int *nClusters)
{


  /* NOTE: this task needs a lot more thinking... */

  /* declarations */

  /* loop through the clusters */

  /* TBD */

  /* done */

  return (0);

}


/*------------------------------------------------------------------*/
int
trackUnpack (TRACK_STRUCT * track, SHELLHIT * shellhit, STAT * ctkStat)
{

  /* Here we transfer the track data to the shellhit */
  /* structure which is what is internally used in the */
  /* tracking task. This array has an enty for each hit  */
  /* (interaction point) and carries the common stuff */
  /* with each hit so that it is easy to  */
  /* cluster/track/reCluster */

  /* declarations */

  int i, j, k, l, Ineg, i1, crystalNumber, holeNum;
  int ndecomp;
  PAYLOAD *ptinp;
  GEBDATA *ptgd;
  CRYS_INTPTS *gtinp;
  double TS_event[MAX_GAMMA_RAYS];
  float r1;

  /* find the T0 corrected TSs before we do anything else */
  /* so we have them available in the ensuing binning */

  ptinp = track->payload;
  ptgd = track->gd;
  ndecomp = 0;
  for (i = 0; i < track->n; i++)
    {
      if (ptgd->type == GEB_TYPE_DECOMP)
        {
          /* cast */

          gtinp = (CRYS_INTPTS *) ptinp;

          /* store T0 corrected and uncorrected TS  */

          TS_event[ndecomp] = (double) gtinp->timestamp + (double) gtinp->t0;
          ndecomp++;

        };

      /* next */

      ptinp++;
      ptgd++;
    };


  shellhit->knownNumClusters = track->n;
  shellhit->nhit = 0;
  ptinp = track->payload;
  ptgd = track->gd;
  k = 0;
  ndecomp = 0;
  for (i = 0; i < track->n; i++)
    {
      if (ptgd->type == GEB_TYPE_DECOMP)
        {
          gtinp = (CRYS_INTPTS *) ptinp;

          /* only add if withing the fine T0 */
          /* corrected time time gate */


          /* find det ID */

          holeNum = ((gtinp->crystal_id & 0xfffc) >> 2);
          if (Pars.AGATA_data == 0)
            {
              crystalNumber = (gtinp->crystal_id & 0x0003);
              i1 = holeNum * 4 + crystalNumber;
            }
          else if (Pars.AGATA_data == 1)
            {
              crystalNumber = (gtinp->crystal_id & 0x0003);
              i1 = holeNum * 3 + crystalNumber;
            };

#if(0)
          fprintf (stderr,"...pad=%i\n", gtinp->pad);

#endif

          if (Pars.enabled[i1])
            {

              /* check for negative energy */

              Ineg = 0;
              for (j = 0; j < gtinp->num; j++)
                {
                  if (gtinp->intpts[j].e <= 0)
                    {
                      Ineg++;
                    };
                };
              ctkStat->negIntEnergy += Ineg;

              if (gtinp->tot_e > 0 && Ineg == 0)
                {
                  for (j = 0; j < gtinp->num; j++)
                    {

                      /* find the radius^2 of the hit */

                      r1 = gtinp->intpts[j].x * gtinp->intpts[j].x
                        + gtinp->intpts[j].y * gtinp->intpts[j].y + gtinp->intpts[j].z * gtinp->intpts[j].z;

                      if (r1 >= Pars.min_radius && r1 <= Pars.max_radius)
                        {

                          shellhit->knownClusterNumber[k] = i;
                          shellhit->t0[k] = gtinp->tot_e;
                          shellhit->chisq[k] = gtinp->chisq;
                          shellhit->norm_chisq[k] = gtinp->norm_chisq;
                          shellhit->timestamp[k] = gtinp->timestamp;
                          shellhit->crystal_id[k] = gtinp->crystal_id;
                          shellhit->XX[k] = gtinp->intpts[j].x;
                          shellhit->YY[k] = gtinp->intpts[j].y;
                          shellhit->ZZ[k] = gtinp->intpts[j].z;

                          /* in the tracking we use MeV and not keV */

                          shellhit->edet[k] = gtinp->intpts[j].e / 1000.;
                          shellhit->esum[k] = gtinp->tot_e / 1000.;

                          shellhit->origPos[k] = j;

                          /* unpact the crystal_id for quick use later */

                          shellhit->crystaltype[k] = (gtinp->crystal_id & 0x0003);
                          shellhit->module[k] = (gtinp->crystal_id & 0xfffc) >> 2;
                          shellhit->detno[k] = 4 * shellhit->module[k] + shellhit->crystaltype[k];

#if(0)
                          fprintf (stderr,"trackUnpack: k=%i\n", k);
                          fprintf (stderr,"gtinp->crystal_id=%i\n", gtinp->crystal_id);
                          fprintf (stderr,"shellhit->module[k]=%i\n", shellhit->module[k]);
                          fprintf (stderr,"shellhit->crystaltype[k]=%i\n", shellhit->crystaltype[k]);
                          fprintf (stderr,"shellhit->detno[k]=%i\n", shellhit->detno[k]);

                          fprintf (stderr,"+++%i,%i\n", Pars.detNo[i], shellhit->detno[k]);
                          if (shellhit->detno[k] != Pars.detNo[i])
                            for (l = 0; l < track->n; l++)
                              fprintf (stderr,"i=%i,k=%i, Pars.detNo=%i,shellhit->detno[k]=%i\n", i, l, Pars.detNo[l],
                                      shellhit->detno[k]);
                          fflush (stdout);
                          assert (shellhit->detno[k] == Pars.detNo[i]);
#endif

                          /* next index for shellhit structure */

                          k++;

                        };

                    };
                };
            };

          ndecomp++;

        };

      shellhit->nhit = k;

      /* next */

      ptinp++;
      ptgd++;

    };


  /* done */

  if (shellhit->nhit > 0)
    {
      return (0);
    }
  else
    {
      if (Pars.nprint > 0)
        fprintf (stderr,"oooops, shellhit->nhit=%i, problem?\n", shellhit->nhit);
      return (NOHITS);
    };

}

/*--------------------------------------------------------*/

int
findClusters (SHELLHIT * shellhit)
{

  /* We actually just assign cluster indexes here */
  /* and do the actual 'clustering' in 'transferClusters' */

  /* declarations */

  int i, j, k, i1, i2;

  /* init */

  for (k = 0; k < shellhit->nhit; k++)
    shellhit->ClusterNumber[k] = -1;

  shellhit->NumClusters = 0;

  /* hunt for multihit clusters (can loops be simplified???) */

  for (i = 0; i < shellhit->nhit; i++)
    for (j = 0; j < shellhit->nhit; j++)
      if (i != j)
        {

          /* single out hits that are close */

          if (shellhit->relAng[j][i] < Pars.alpha[shellhit->nhit])
            {

#if(0)
              fprintf (stderr,"%p:: %2i %2i; %9.6f, %9.6f\n", shellhit, i, j, shellhit->relAng[i][j], Pars.alpha);
#endif
              /* we have two interactions that are close */

              if (shellhit->ClusterNumber[i] < 0 && shellhit->ClusterNumber[j] < 0)
                {

                  /* none of them have been clustered before, */
                  /* so make a new cluster then */

                  shellhit->ClusterNumber[i] = shellhit->NumClusters;
                  shellhit->ClusterNumber[j] = shellhit->NumClusters;
                  shellhit->NumClusters++;

                }
              else if (shellhit->ClusterNumber[i] >= 0 && shellhit->ClusterNumber[j] < 0)
                {

                  /* [j] joins [i]'s cluster */

                  shellhit->ClusterNumber[j] = shellhit->ClusterNumber[i];

                }
              else if (shellhit->ClusterNumber[j] >= 0 && shellhit->ClusterNumber[i] < 0)
                {

                  /* [i] joins [j]'s cluster */

                  shellhit->ClusterNumber[i] = shellhit->ClusterNumber[j];

                }
              else if (shellhit->ClusterNumber[i] >= 0 && shellhit->ClusterNumber[j] >= 0
                       && shellhit->ClusterNumber[i] != shellhit->ClusterNumber[j])
                {

                  /* ooops, colliding clusters - put them together */
                  /* i1=common cluster index, i2= will be moved to i1 */

                  if (shellhit->ClusterNumber[i] > shellhit->ClusterNumber[j])
                    {
                      i1 = shellhit->ClusterNumber[j];
                      i2 = shellhit->ClusterNumber[i];
                    }
                  else
                    {
                      i1 = shellhit->ClusterNumber[i];
                      i2 = shellhit->ClusterNumber[j];
                    }

                  /* merge high numbered cluster */
                  /* to the low numbered cluster */

                  for (k = 0; k < shellhit->nhit; k++)
                    if (shellhit->ClusterNumber[k] == i2)
                      shellhit->ClusterNumber[k] = i1;

                  /* bump everyone above i2 down one */
                  /* in cluster index */

                  for (k = 0; k < shellhit->nhit; k++)
                    if (shellhit->ClusterNumber[k] > i2)
                      shellhit->ClusterNumber[k]--;

                  /* mark we lost one in the counting of */
                  /* number of clusters we have so far */

                  shellhit->NumClusters--;

                };
            };
        };

  /* assign single hit clusters to the rest */
  /* since they never found any partner */

  for (i = 0; i < shellhit->nhit; i++)
    if (shellhit->ClusterNumber[i] < 0)
      {
        shellhit->ClusterNumber[i] = shellhit->NumClusters;

        /* since this was a new cluster,  */
        /* we must increment the cluster counter */

        shellhit->NumClusters++;

      };

#if(0)
  /* sanity checks */

  for (i = 0; i < shellhit->nhit; i++)
    {
      assert (shellhit->ClusterNumber[i] >= 0);
      assert (shellhit->ClusterNumber[i] < shellhit->NumClusters);
    };
#endif

  /* done */

  return (0);

}

/*--------------------------------------------------------*/

int
findAbsAngles (SHELLHIT * shellhit)
{

  /* declarations */

  int j;

  /* find absolute polar and azimuth angles */

  for (j = 0; j < shellhit->nhit; j++)
    {

      /* find azimuth angle */
      /* use atan2 to include quadrant information */

      shellhit->aziAng[j] = atan2f (shellhit->nYY[j], shellhit->nXX[j]);

      /* find polar angle */

      shellhit->polAng[j] = acosf (shellhit->nZZ[j]);

    };

  /* done */

  return (0);

}

/*--------------------------------------------------------*/

int
findRelativeAngles (SHELLHIT * shellhit, STAT * ctkStat)
{

  /* declarations */

  int i, j, i1;
  float f1;
  float max;

  /* find all relative polar and azimuth */
  /* angles between interaction points */

  max = 0;
  for (i = 0; i < shellhit->nhit; i++)
    for (j = i + 1; j < shellhit->nhit; j++)
      {

        f1 = shellhit->nXX[i] * shellhit->nXX[j];
        f1 += shellhit->nYY[i] * shellhit->nYY[j];
        f1 += shellhit->nZZ[i] * shellhit->nZZ[j];

        /* for some reason this happens */
        /* so make sure we pass argument */
        /* in range for acosf to handle */

#if(0)
        if (f1 > 1.0 || f1 < -1.0)
          {
            fprintf (stderr,"oopsi...f1=%f\n", f1);
            fprintf (stderr,"shellhit->nXX[%i]=%f\n", i, shellhit->nXX[i]);
            fprintf (stderr,"shellhit->nYY[%i]=%f\n", i, shellhit->nYY[i]);
            fprintf (stderr,"shellhit->nZZ[%i]=%f\n", i, shellhit->nZZ[i]);
            fprintf (stderr,"shellhit->nXX[%i]=%f\n", j, shellhit->nXX[j]);
            fprintf (stderr,"shellhit->nYY[%i]=%f\n", j, shellhit->nYY[j]);
            fprintf (stderr,"shellhit->nZZ[%i]=%f\n", j, shellhit->nZZ[j]);
            fflush (stdout);
          };
#endif

        if (f1 > 1.0)
          f1 = 1.0;
        if (f1 < -1.0)
          f1 = -1.0;


        assert (i < MAXSHELLHITS);
        assert (j < MAXSHELLHITS);

        shellhit->relAng[i][j] = acosf (f1);
        shellhit->relAng[j][i] = shellhit->relAng[i][j];

        i1 = (int) (10 * 57.2958 * shellhit->relAng[i][j] + 0.5);
        if (shellhit->nhit < 8)
          if (i1 > 0 && i1 < 1000)
            ctkStat->sp_max_ang[i1][shellhit->nhit]++;

//        if (shellhit->relAng[i][j]> max)
//          max=shellhit->relAng[i][j];

      };

  /* bin maxangle */

//  i1=(int) ( 10*57.2958*max+0.5);
//  if (shellhit->nhit < 8)
//  if (i1>0 && i1<1000)
//    ctkStat->sp_max_ang[i1][shellhit->nhit]++;

  /* done */

  return (0);

}

/*--------------------------------------------------------*/

int
findNormVectors (SHELLHIT * shellhit)
{

  /* declarations */

  float d1;
  int j;

  /* find normalization */

  for (j = 0; j < shellhit->nhit; j++)
    {
      d1 = shellhit->XX[j] * shellhit->XX[j];
      d1 += shellhit->YY[j] * shellhit->YY[j];
      d1 += shellhit->ZZ[j] * shellhit->ZZ[j];
      d1 = sqrt ((double) d1);
      shellhit->rLen[j] = d1;
      shellhit->nXX[j] = shellhit->XX[j] / d1;
      shellhit->nYY[j] = shellhit->YY[j] / d1;
      shellhit->nZZ[j] = shellhit->ZZ[j] / d1;

    };

  /* done */

  return (0);

}

/*--------------------------------------------------------*/

void
CheckNoArgs (int required, int actual, char *str)
{

  if (required < actual)
    {
      fprintf (stderr,"argument problem with chat option\n");
      fprintf (stderr,"--> %s\n", str);
      fprintf (stderr,"required # arguments: %i\n", required - 1);
      fprintf (stderr,"actual   # arguments: %i\n", actual - 1);
      fprintf (stderr,"Please fix and try again, quitting...\n");
      exit (1);
    };

}

/*--------------------------------------------------------*/

int
readChatFile (char *name)
{

  /* declarations */

  FILE *fp, *fp1;
  char *p, *pc, str[STRLEN] = { '0' }, str1[STRLEN] =
  {
  '0'};
  char str2[STRLEN] = { '0' };
  int echo = 0, nret = 0, nni = 0, nn = 0, i, i1, i2, st;
  double d1;
  int ng, nt, ok, ndet, j;
  float r1, r2, r3, aa, bb, distToCrystal;
  int str_decomp (char *, int, int *, int);

  /* open chat file */

  if ((fp = fopen (name, "r")) == NULL)
    {
      fprintf (stderr,"error: could not open chat file: <%s>\n", name);
      exit (0);
    };
  fprintf (stderr,"chat file: <%s> open\n", name);
  fprintf (stderr,"\n");
  fflush (stdout);

  /* read content and act */

  pc = fgets (str, STRLEN, fp);

  while (pc != NULL)
    {
      if (echo)
        fprintf (stderr,"chat->%s", str);
      fflush (stdout);

      /* attemp to interpret the line */

      if ((p = strstr (str, "echo")) != NULL)
        {
          echo = 1;
          if (echo)
            fprintf (stderr,"will echo command lines\n");
        }
      else if (str[0] == 35)
        {
          /* '#' comment line, do nothing */
          nni--;                /* don't count as instruction */

        }
      else if (str[0] == 59)
        {
          /* ';' comment line, do nothing */
          nni--;                /* don't count as instruction */

        }
      else if (str[0] == 10)
        {
          /* empty line, do nothing */
          nni--;                /* don't count as instruction */

        }
      else if ((p = strstr (str, "trackingstrategy")) != NULL)
        {
          nret = sscanf (str, "%s %i %i %s", str1, &ndet, &i2, str2);
          fprintf (stderr,"nret=%i\n", nret);



          if (nret == 3)
            {
              /* set the option */

              fprintf (stderr,"nret == 3, simple\n");
              Pars.trackOps[ndet] = i2;
              i1 = ctkTrackOpt (Pars.trackOps[ndet]);
              if (i1 != 0)
                {
                  fprintf (stderr,"bad tracking option, quit...\n");
                  exit (1);
                };

            }
          else if (nret == 4)
            {

              fprintf (stderr,"nret == 4, complex\n");
              fprintf (stderr,"5: jumping option\n");
              if (i2 != 5)
                exit (-1);

              /* set the option */

              Pars.trackOps[ndet] = i2;
              i1 = ctkTrackOpt (Pars.trackOps[ndet]);
              if (i1 != 0)
                {
                  fprintf (stderr,"bad tracking option, quit...\n");
                  exit (1);
                };

              /* count g's and t's */

              ng = 0;
              nt = 0;
              ok = 1;
              i = 0;
              while (ok && i < 10)
                {
                  switch (str2[i])
                    {
                    case 103:
                      ng++;     /* it is a g */
                      break;
                    case 116:
                      nt++;     /* it is a t */
                      break;
                    default:
                      ok = 0;
                      break;
                    };
                  i++;
                };
              fprintf (stderr,"ng=%i, nt=%i\n", ng, nt);

              /* check */

              if (ndet != (ng + nt))
                {
                  fprintf (stderr,"bad jump strategy string length, quit\n");
                  exit (1);
                };

              /* set group length */

              Pars.jmpGrpLen[ndet] = ng;

            }
          else
            {
              fprintf (stderr,"bad trackingstrategy input\n\n");
              exit (1);
            };

        }
      else if ((p = strstr (str, "fixdoublehits")) != NULL)
        {
          nret = sscanf (str, "%s %i", str1, &Pars.fixdoublehitsCode);
          CheckNoArgs (nret, 2, str);
          Pars.fixdoublehits = 1;
        }
      else if ((p = strstr (str, "nodupmode2")) != NULL)
        {
          nret = sscanf (str, "%s", str1);
          CheckNoArgs (nret, 1, str);
          Pars.nodupmode2 = 1;
        }
      else if ((p = strstr (str, "radiuslimits")) != NULL)
        {
          nret = sscanf (str, "%s %f %f", str1, &Pars.min_radius, &Pars.max_radius);
          CheckNoArgs (nret, 3, str);
          Pars.min_radius *= Pars.min_radius;
          Pars.max_radius *= Pars.max_radius;
        }
      else if ((p = strstr (str, "AGATA_data")) != NULL)
        {
          nret = sscanf (str, "%s %s", str1, Pars.AGATA_data_fn);
          CheckNoArgs (nret, 2, str);
          Pars.AGATA_data = 1;
          fprintf (stderr,"Pars.AGATA_data=%i, process AGATA geometry\n", Pars.AGATA_data);
          fprintf (stderr,"crmat file: %s\n", Pars.AGATA_data_fn);
        }
      else if ((p = strstr (str, "CCcal")) != NULL)
        {
          nret = sscanf (str, "%s %s", str1, str2);
          CheckNoArgs (nret, 2, str);
          fp1 = fopen (str2, "r");
          if (fp1 == NULL)
            {
              fprintf (stderr,"WARNING: could not open CCcal file \"%s\", quit\n", str2);
            }
          else
            {
              fprintf (stderr,"reading cal file \"%s\"\n", str2);
              nn = 0;
              st = fscanf (fp1, "%i %f %f", &i1, &r1, &r2);
              while (st == 3)
                {
                  nn++;
                  Pars.CCcal_offset[i1] = r1;
                  Pars.CCcal_gain[i1] = r2;
                  fprintf (stderr,"CC det/offset/gain %3i %6.3f %9.6f\n", i1, Pars.CCcal_offset[i1], Pars.CCcal_gain[i1]);
                  st = fscanf (fp1, "%i %f %f", &i1, &r1, &r2);
                };
              fprintf (stderr,"done, read %i calibration \n", nn);
              fclose (fp1);
            };
        }
      else if ((p = strstr (str, "fomjump")) != NULL)
        {
          nret = sscanf (str, "%s %f", str1, &Pars.fomJump);
          CheckNoArgs (nret, 2, str);
        }
      else if ((p = strstr (str, "nocrystaltoworldrot")) != NULL)
        {
          nret = sscanf (str, "%s", str1);
          Pars.nocrystaltoworldrot = 1;
          CheckNoArgs (nret, 1, str);
        }
      else if ((p = strstr (str, "dtwin")) != NULL)
        {
          nret = sscanf (str, "%s %lli", str1, &Pars.dtwin);
          CheckNoArgs (nret, 2, str);
        }
      else if ((p = strstr (str, "badTheoAngPenalty")) != NULL)
        {
          nret = sscanf (str, "%s %f", str1, &Pars.badTheoAngPenalty);
          CheckNoArgs (nret, 2, str);
        }
      else if ((p = strstr (str, "ndetElim")) != NULL)
        {
          nret = sscanf (str, "%s %i %f %f %f", str1, &i1, &r1, &r2, &r3);
          CheckNoArgs (nret, 5, str);
          Pars.ndetElim_lo[i1] = r1;
          Pars.ndetElim_hi[i1] = r2;
          Pars.ndetElim_fom[i1] = r3;
          fprintf (stderr,"will set fom to %5.2f for ndet==%i if outside %9.3f MeV to %9.3f MeV\n", Pars.ndetElim_fom[i1], i1,
                  Pars.ndetElim_lo[i1], Pars.ndetElim_hi[i1]);
        }
      else if ((p = strstr (str, "useCCEnergy")) != NULL)
        {
          CheckNoArgs (nret, 1, str);
          Pars.useCCEnergy = 1;
          fprintf (stderr,"set Pars.useCCEnergy=%i\n", Pars.useCCEnergy);
        }
      else if ((p = strstr (str, "SEGcal")) != NULL)
        {
          nret = sscanf (str, "%s %s", str1, str2);
          CheckNoArgs (nret, 2, str);
          fp1 = fopen (str2, "r");
          if (fp1 == NULL)
            {
              fprintf (stderr,"WARNING: could not open SEGcal file \"%s\", quit\n", str2);
            }
          else
            {
              fprintf (stderr,"reading cal file \"%s\"\n", str2);
              nn = 0;
              st = fscanf (fp1, "%i %i %f %f", &i1, &i2, &r1, &r2);
              while (st == 4)
                {
                  nn++;
                  Pars.SEGcal_offset[i1][i2] = r1;
                  Pars.SEGcal_gain[i1][i2] = r2;
                  fprintf (stderr,"SEG det/seg/offset/gain %3i %2i %6.3f %9.6f\n", i1, i2,
                          Pars.SEGcal_offset[i1][i2], Pars.SEGcal_gain[i1][i2]);
                  st = fscanf (fp1, "%i %i %f %f", &i1, &i2, &r1, &r2);
                };
              fprintf (stderr,"done, read %i calibrations \n", nn);
              fclose (fp1);
            };
        }
      else if ((p = strstr (str, "useSegEnergy")) != NULL)
        {
          CheckNoArgs (nret, 1, str);
          Pars.useSegEnergy = 1;
          fprintf (stderr,"set Pars.useSegEnergy=%i\n", Pars.useSegEnergy);
        }
      else if ((p = strstr (str, "enabled")) != NULL)
        {
          nret = sscanf (str, "%s %s", str1, str2);
          CheckNoArgs (nret, 2, str);
          str_decomp (str2, MAXDETNO + 1, Pars.enabled, 1);
          for (j = 0; j < MAXDETNO; j++)
            if (!Pars.enabled[j])
              fprintf (stderr,"detector %3i is disabled\n", j);
            else
              fprintf (stderr,"detector %3i is enabled\n", j);
        }
      else if ((p = strstr (str, "singlesfom")) != NULL)
        {
          nret = sscanf (str, "%s %f", str1, &Pars.snglsFom);
          CheckNoArgs (nret, 2, str);
        }
      else if ((p = strstr (str, "fomgoodenough")) != NULL)
        {
          nret = sscanf (str, "%s %f", str1, &Pars.fomGoodenough);
          CheckNoArgs (nret, 2, str);
        }
      else if ((p = strstr (str, "target_x")) != NULL)
        {
          nret = sscanf (str, "%s %f", str1, &Pars.target_x);
          CheckNoArgs (nret, 2, str);
          Pars.target_x /= 10;
        }
      else if ((p = strstr (str, "target_y")) != NULL)
        {
          nret = sscanf (str, "%s %f", str1, &Pars.target_y);
          CheckNoArgs (nret, 2, str);
          Pars.target_y /= 10;
        }
      else if ((p = strstr (str, "target_z")) != NULL)
        {
          nret = sscanf (str, "%s %f", str1, &Pars.target_z);
          CheckNoArgs (nret, 2, str);
          Pars.target_z /= 10;
        }
      else if ((p = strstr (str, "xyzoffset")) != NULL)
        {
          nret = sscanf (str, "%s %s", str1, Pars.xyzoffsetfn);
          CheckNoArgs (nret, 2, str);
          fprintf(stderr,"will use xyzoffset file \"%s\" in the tracking\n", Pars.xyzoffsetfn);
          Pars.havexyzoffset = 1;
        }
      else if ((p = strstr (str, "singlehitmaxdepth")) != NULL)
        {
          nret = sscanf (str, "%s %i %f %f %f", str1, &i1, &Pars.singlehitrangeFOM, &distToCrystal, &r3);
          CheckNoArgs (nret, 5, str);

          Pars.marksinglehitrange = 1;
          fprintf (stderr,"Pars.marksinglehitrange=%i\n", Pars.marksinglehitrange);
          fprintf (stderr,"singlehits outside range will be assigned fom of %f\n", Pars.singlehitrangeFOM);
          fprintf (stderr,"distToCrystal= %f (will be added)\n", distToCrystal);


          fprintf (stderr,"will read %i values\n", i1);
          for (i = 0; i < i1; i++)
            {
              fscanf (fp, "%f %f\n", &r1, &r2);
              i2 = (int) (r1 * 1000 + 0.5);     /* now keV */
              fprintf (stderr,"max range at %6i keV ", i2);
              fflush (stdout);
              Pars.singlehitrange[i2] = r3 * r2 + distToCrystal;
              fprintf (stderr,"is %4.2f cm\n", Pars.singlehitrange[i2]);
              fflush (stdout);
            };

          /* interpolate for the rest */

          for (i = 0; i < LONGLEN; i++)
            if (Pars.singlehitrange[i] == 0)
              {

                /* find next up that is filled */

                j = i + 1;
                while (Pars.singlehitrange[j] == 0 && j < LONGLEN)
                  j++;


                /* interpolate */

                bb = (Pars.singlehitrange[i - 1] - Pars.singlehitrange[j]) / (i - 1 - j);
                aa = Pars.singlehitrange[j] - bb * j;
                Pars.singlehitrange[i] = aa + bb * i;

              };

          /* smooth the function */

          /* TBD */

          /* dump the function to file */

          fp1 = fopen ("singlehitrange.xy", "w");
          for (i = 0; i < LONGLEN; i++)
            fprintf (fp1, "%i, %f\n", i, Pars.singlehitrange[i]);
          fclose (fp1);

          /* quietly make the value the square range */
          /* so we do not have to take square roots later */

          for (i = 0; i < LONGLEN; i++)
            Pars.singlehitrange[i] *= Pars.singlehitrange[i];

        }
      else if ((p = strstr (str, "splitclusters1")) != NULL)
        {
          nret = sscanf (str, "%s %f %f %i %i %i", str2, &Pars.splitclusters_kickoutfom1, &Pars.splitclusters_thresh1,
                         &Pars.splitclusters_minn1, &Pars.splitclusters_maxn1, &Pars.splitclusters_maxtry1);
          CheckNoArgs (nret, 6, str);
          Pars.splitclusters1 = 1;
        }
      else if ((p = strstr (str, "splitclusters2")) != NULL)
        {
          nret =
            sscanf (str, "%s %f %f %i %i %i %f", str2, &Pars.splitclusters_kickoutfom2, &Pars.splitclusters_thresh2,
                    &Pars.splitclusters_minn2, &Pars.splitclusters_maxn2, &Pars.splitclusters_maxtry2,
                    &Pars.goodImproveFraction);
          CheckNoArgs (nret, 7, str);
          Pars.splitclusters2 = 1;
        }
      else if ((p = strstr (str, "recluster1")) != NULL)
        {
          nret =
            sscanf (str, "%s %f %f %i %i %f", str2, &Pars.recluster_kickoutfom1, &Pars.recluster_thresh1,
                    &Pars.recluster_minn1, &Pars.recluster_maxtry1, &Pars.recluster_reduxfactor1);
          CheckNoArgs (nret, 6, str);
          Pars.recluster1 = 1;
        }
      else if ((p = strstr (str, "recluster2")) != NULL)
        {
          nret =
            sscanf (str, "%s %f %f %i %i %f", str2, &Pars.recluster_kickoutfom2, &Pars.recluster_thresh2,
                    &Pars.recluster_minn2, &Pars.recluster_maxtry2, &Pars.recluster_reduxfactor2);
          CheckNoArgs (nret, 6, str);
          Pars.recluster2 = 1;
        }
      else if ((p = strstr (str, "combineclusters")) != NULL)
        {
          nret = sscanf (str, "%s %f %f %i %f", str2, &Pars.combineclusters_kickoutfom, &Pars.combineclusters_thresh,
                         &Pars.combineclusters_maxn, &Pars.combineMaxDist);
          CheckNoArgs (nret, 5, str);
          Pars.combineclusters = 1;

          /* quietly make this the square so we don't */
          /* have to do squareroots later when we use it */

          Pars.combineMaxDist = Pars.combineMaxDist * Pars.combineMaxDist;

        }
      else if ((p = strstr (str, "matchmaker")) != NULL)
        {
          nret = sscanf (str, "%s %f %f", str2, &Pars.matchmaker_kickoutfom, &Pars.matchmakerMaxDist);
          CheckNoArgs (nret, 3, str);
          Pars.matchmaker = 1;

          /* quietly make this the square so we don't */
          /* have to do squareroots later when we use it */

          Pars.matchmakerMaxDist = Pars.matchmakerMaxDist * Pars.matchmakerMaxDist;

        }
      else if ((p = strstr (str, "pairproduction")) != NULL)
        {
          nret = sscanf (str, "%s", str2);
          CheckNoArgs (nret, 1, str);
          Pars.pairProd = 1;

        }
      else if ((p = strstr (str, "elast")) != NULL)
        {
          nret = sscanf (str, "%s %f %f %f", str2, &Pars.checkElast_lo, &Pars.checkElast_hi, &Pars.checkElast_penalty);
          CheckNoArgs (nret, 4, str);
          Pars.checkElast = 1;
          fprintf (stderr,"any cluster where the last interaction energy\n");
          fprintf (stderr,"is not in the range of %f to %f will \n", Pars.checkElast_lo, Pars.checkElast_hi);
          fprintf (stderr,"have a penalty of %f applied\n", Pars.checkElast_penalty);
        }
      else if ((p = strstr (str, "efirst")) != NULL)
        {
          nret = sscanf (str, "%s %f %f", str2, &Pars.checkEfirst_lo, &Pars.checkEfirst_penalty);
          CheckNoArgs (nret, 3, str);
          Pars.checkEfirst = 1;
          fprintf (stderr,"any cluster where the first interaction energy\n");
          fprintf (stderr,"is the greatest if the gamma ray energy is above %f will \n", Pars.checkElast_lo);
          fprintf (stderr,"have a penalty of %f applied\n", Pars.checkElast_penalty);
        }
      else if ((p = strstr (str, "itterations")) != NULL)
        {
          nret = sscanf (str, "%s %i", str1, &Pars.itterations);
          CheckNoArgs (nret, 2, str);
        }
      else if ((p = strstr (str, "removeinvalidentries")) != NULL)
        {
          CheckNoArgs (nret, 1, str);
          fprintf (stderr,"option removed, nolonger needed\n");
        }
      else if ((p = strstr (str, "maxevents")) != NULL)
        {
          nret = sscanf (str, "%s %i", str1, &Pars.maxevents);
          CheckNoArgs (nret, 2, str);
        }
      else if ((p = strstr (str, "untracked_fom_kickout")) != NULL)
        {
          nret = sscanf (str, "%s %f", str1, &Pars.untracked_fom_kickout);
          CheckNoArgs (nret, 2, str);
        }
      else if ((p = strstr (str, "trackDataStream")) != NULL)
        {
          nret = sscanf (str, "%s %s", str1, str2);
          CheckNoArgs (nret, 2, str);

#if(0)
          /* this is done in trackMain, why do we have it here? */

          if (Pars.trackDataStream < 0)
            {
              /* open the output file */
              Pars.trackDataStream = open (str2, O_WRONLY | O_CREAT | O_TRUNC, PMODE);

              if (Pars.trackDataStream <= 0)
                {
                  fprintf (stderr,"could not open output file %s\n", str2);
                  return (1);
                }
              else
                fprintf (stderr,"tracked output file \"%s\" is open\n", str2);
            }
          else
            {
              printf
                ("it appears that the tracked output file is already open. ignoring the trackDataStream %s line in the chat file\n",
                 str2);
            }
#endif
        }
      else if ((p = strstr (str, "nprint")) != NULL)
        {
          nret = sscanf (str, "%s %i", str1, &Pars.nprint);
          CheckNoArgs (nret, 2, str);
        }
      else if ((p = strstr (str, "radialcorrectionfile")) != NULL)
        {
          nret = sscanf (str, "%s %s", str1, str2);
          CheckNoArgs (nret, 2, str);
          fprintf (stderr,"attempting to open radialcorrectionfile \"%s\"... ", str2);
          if ((fp1 = fopen (str2, "r")) != NULL)
            {
              fprintf (stderr,"OK\n");
              j = fscanf (fp1, "%i %f\n", &i1, &r2);
              nn = 0;
              while (j == 2)
                {
                  fprintf (stderr,"%i %f\n", i1, r2);
                  fflush (stdout);
                  Pars.dirk_rcf[i1] = r2;
                  fprintf (stderr,"%i %f\n", i1, Pars.dirk_rcf[i1]);
                  fflush (stdout);
                  nn++;
                  j = fscanf (fp1, "%i %f\n", &i1, &r2);
                };

              fclose (fp1);
              fprintf (stderr,"read %i factors\n", nn);
              fflush (stdout);
            }
          else
            {
              fprintf (stderr,"failed, QUIT!!\n\n");
              exit (1);
            };

        }
      else if ((p = strstr (str, "clusterangle")) != NULL)
        {
          nret = sscanf (str, "%s %i %f", str1, &i1, &r1);
          CheckNoArgs (nret, 3, str);
          if (i1 >= MAXSHELLHITS)
            {
              fprintf (stderr,"error: max number if interaction points are %i [MAXSHELLHITS in ctk.h]\n", MAXSHELLHITS);

              exit (1);
            };
          Pars.alpha[i1] = r1;
          Pars.alpha[i1] *= RAD2DEG;
//          fprintf (stderr,"Pars.alpha=%f rad %f deg\n", Pars.alpha, Pars.alpha / RAD2DEG);
          //         fprintf (stderr,"RAD2DEG=%f\n", RAD2DEG);
        }
      else
        {

      /*-----------------------------*/
      /* chatscript read error point */
      /*-----------------------------*/

          fprintf (stderr,"line %2.2i in chat script, option :%s \n__not understood\n", nn, str);
          fprintf (stderr,"%i\n", str[0]);
          fprintf (stderr,"aborting\n");
          fflush (stdout);
          exit (0);
        };

      /* read next line in chat script */

      nn++;                     /* line counter */
      nni++;                    /* instruction counter */
      pc = fgets (str, STRLEN, fp);

    };

  /* done */

  fclose (fp);
  fprintf (stderr,"\n");
  fprintf (stderr,"chat file: <%s> closed\n", name);
  fprintf (stderr,"__processed %i sort instructions and %i lines\n", nni, nn);
  fprintf (stderr,"\n");

  /* check the alpha values */

  if (Pars.alpha[1] <= 0)
    {
      fprintf (stderr,"you must at least specify the clustereing angle for %i interaction points, clusterangle %i xxx.xxx\n", 1,
              1);
      exit (1);
    };
  if (Pars.alpha[MAXSHELLHITS - 1] <= 0)
    {
      /* go hunt for the last nonzero one */

      for (i = (MAXSHELLHITS - 1); i >= 1; i--)
        if (Pars.alpha[i] > 0)
          {
            Pars.alpha[MAXSHELLHITS - 1] = Pars.alpha[i];
            break;
          };
    };

  /* process the anpha clustering angle (interpolate) */

  for (i = 1; i < MAXSHELLHITS; i++)
    if (Pars.alpha[i] == 0)
      {

        /* find next up that is filled */

        j = i + 1;
        while (Pars.alpha[j] == 0 && j < MAXSHELLHITS)
          j++;


        /* interpolate */

        bb = (Pars.alpha[i - 1] - Pars.alpha[j]) / (i - 1 - j);
        aa = Pars.alpha[j] - bb * j;
        Pars.alpha[i] = aa + bb * i;

      };

  fprintf (stderr,"\n");
  fprintf (stderr,"list of clustering angles that will be used\n");
  fprintf (stderr,"__as function of the number of interaction points\n");
  fprintf (stderr,"__seen in the entire array:\n");
  fprintf (stderr,"\n");
  for (i = 1; i < MAXSHELLHITS; i++)
    if (Pars.alpha[i] != Pars.alpha[i - 1] || i == 1 || i == (MAXSHELLHITS - 1))
      fprintf (stderr,"nhits=%3i will use alpha=%9.2fdeg\n", i, Pars.alpha[i] / RAD2DEG);
  fprintf (stderr,"(like entries are suppressed in this listing)\n");

  fprintf (stderr,"\n");
  fprintf (stderr,"done reading the tracking chat file\n");
  fprintf (stderr,"\n");
  fflush (stdout);
  return (0);

}


/*------------------------------------------------------------------*/

int
mod_coordinates (TRACK_STRUCT * track)
{

  /* the decomposition code gives coordinates in mmm */
  /* and the trackingcode need to see cm (for historical */
  /* reasons), so we fix that here */

  /* declarations */

  int i, j;
  PAYLOAD *ptinp;
  GEBDATA *ptgd;
  CRYS_INTPTS *gtinp;

//  printf("entered mod_coordinates\n");

  ptinp = track->payload;
  ptgd = track->gd;
  for (i = 0; i < track->n; i++)
    {
      if (ptgd->type == GEB_TYPE_DECOMP)
        {

          gtinp = (CRYS_INTPTS *) ptinp;

          for (j = 0; j < gtinp->num; j++)
            {

              /* inside the tracking we use cm not mm */

              gtinp->intpts[j].x /= 10;
              gtinp->intpts[j].y /= 10;
              gtinp->intpts[j].z /= 10;

//              printf("\n* %6.2f %6.2f %6.2f\n", gtinp->intpts[j].x,gtinp->intpts[j].y,gtinp->intpts[j].z);

              gtinp->intpts[j].x *= Pars.dirk_rcf[Pars.detNo[i]];
              gtinp->intpts[j].y *= Pars.dirk_rcf[Pars.detNo[i]];
              /* do not modify z */

//              printf("Pars.detNo[j]=%i; ",Pars.detNo[i] );
//              printf("Pars.dirk_rcf=%5.2f\n",Pars.dirk_rcf[Pars.detNo[i]]);
//              printf("  %6.2f %6.2f %6.2f\n", gtinp->intpts[j].x,gtinp->intpts[j].y,gtinp->intpts[j].z);


            };
        };
      ptinp++;
      ptgd++;
    };

//if(1)exit(0);
  /* done */

  return (0);

}

/*------------------------------------------------------------------*/
int
fixDoubleHitsInSegs (TRACK_STRUCT * track, STAT * ctkStat)
{

  /* this is a function where we play with  */
  /* the data and look for double hits in  */
  /* the same segment. We may try to do  */
  /* something with these events or we may  */
  /* just want to mark them */

  /* declarations */

  int i, j, seghit[TOT_SEGS + 1], nn, nbad = 0, i1;
  PAYLOAD *ptinp;
  GEBDATA *ptgd;
  DCR_INTPTS tmpintpts[MAX_INTPTS];
  CRYS_INTPTS *gtinp;
  static int nxxx = 0;
  static int firstime = 1;
  static FILE *fp;
  float ee[100], r1;
  static int sp[100];

  if (Pars.nprint > 0)
    fprintf (stderr,"entered fixDoubleHitsInSegs\n");

#if(SPECIALFIXDOUBLEHITS)
  if (firstime == 1)
    {
      firstime = 0;
      fp = fopen ("fixDoubleHitsInSegs.log", "w");
      for (i = 0; i < 100; i++)
        sp[i] = 0;
    };
#endif

  /* loop through the decomp data */

  ptinp = track->payload;
  ptgd = track->gd;
  for (i = 0; i < track->n; i++)
    {
      if (ptgd->type == GEB_TYPE_DECOMP)
        {

          /* cast */

          gtinp = (CRYS_INTPTS *) ptinp;

          /* zero segment hit counter */

          for (i = 0; i <= TOT_SEGS; i++)
            seghit[i] = 0;

          /* debug print */

          if (Pars.nprint > 0)
            for (j = 0; j < gtinp->num; j++)
              {
                fprintf (stderr,"--> %6.2f %6.2f %6.2f, %8.2f; ", gtinp->intpts[j].x, gtinp->intpts[j].y, gtinp->intpts[j].z,
                        gtinp->intpts[j].e);
                fprintf (stderr," seg= %i", gtinp->intpts[j].seg);
                fprintf (stderr,"\n");
              }

          /* count how many hits we have in the segments */

          for (j = 0; j < gtinp->num; j++)
            {
              ctkStat->SegHits++;
              assert (gtinp->intpts[j].seg < TOT_SEGS);
              seghit[gtinp->intpts[j].seg]++;
            };

          /* some debug info about what we found */

          for (i = 0; i <= TOT_SEGS; i++)
            if (seghit[i] > 1)
              {
                if (Pars.nprint > 0)
                  fprintf (stderr,"duphit: seg %2i has %2i hits\n", i, seghit[i]);
                nbad++;
              };

          if (nbad > 0)
            {
              for (j = 0; j < gtinp->num; j++)
                {
                  if (Pars.nprint > 0 && seghit[gtinp->intpts[j].seg] > 1)
                    {
                      fprintf (stderr,"%6.2f %6.2f %6.2f, %8.2f; ", gtinp->intpts[j].x, gtinp->intpts[j].y,
                              gtinp->intpts[j].z, gtinp->intpts[j].e);
                      fprintf (stderr," seg= %i", gtinp->intpts[j].seg);
                      fprintf (stderr,"\n");
                    }
                };
            };


#if(SPECIALFIXDOUBLEHITS)
          if (nbad == 1)
            {
              nn = 0;
              for (j = 0; j < gtinp->num; j++)
                {
                  if (seghit[gtinp->intpts[j].seg] == 2 && fp != NULL)
                    {
                      fprintf (fp, "%6.2f %6.2f %6.2f, %8.2f; ", gtinp->intpts[j].x, gtinp->intpts[j].y,
                               gtinp->intpts[j].z, gtinp->intpts[j].e);
                      fprintf (fp, " seg= %i", gtinp->intpts[j].seg);
                      fprintf (fp, "\n");
                      ee[nn] = gtinp->intpts[j].e;
                      nn++;
                    }
                };
              if (ee[0] < ee[1])
                {
                  r1 = ee[0];
                  ee[0] = ee[1];
                  ee[1] = r1;
                };
              r1 = ee[1] / ee[0];
              fprintf (fp, "ratio: %6.2f/%6.2f=%6.3f\n", ee[0], ee[1], r1);
              i1 = r1 * 100;
              if (i1 >= 0 && i1 < 1000)
                sp[i1]++;
//              assert(sp[i]>=0);
            };
#endif

          /* do something about it if fixdoublehitsCode>0 */

          for (i = 0; i <= TOT_SEGS; i++)
            if (seghit[i] > 1)
              {
                ctkStat->doubleSegHits++;

                if (Pars.fixdoublehitsCode == 0)
                  {

                    if (Pars.nprint > 0)
                      fprintf (stderr,"we have double hit in segment, but do nothing\n");

                  }
                else if (Pars.fixdoublehitsCode == 1)
                  {

                    /* the simples option, simply reject */
                    /* the 'whole' event */

                    if (Pars.nprint > 0)
                      fprintf (stderr,"reject event, with error %i code\n", DOUBLEHITS);
                    return (DOUBLEHITS);

                  }
                else if (Pars.fixdoublehitsCode == 2)
                  {

                    /* second simplest thing to do: rejecting  */
                    /* the hits in the segments that had  */
                    /* double hits even though that may not  */
                    /* be a good thing to do */

                    if (Pars.nprint > 0)
                      fprintf (stderr,"reject the hits\n");

                    /* temporary store */

                    nn = gtinp->num;
                    for (j = 0; j < nn; j++)
                      {
                        memcpy ((char *) &tmpintpts[j], (char *) &gtinp->intpts[j], sizeof (DCR_INTPTS));
                      };

                    /* restore */

                    gtinp->num = 0;
                    for (j = 0; j < nn; j++)
                      {
                        if (seghit[tmpintpts[j].seg] == 1)
                          {
                            memcpy ((char *) &gtinp->intpts[gtinp->num], (char *) &tmpintpts[j], sizeof (DCR_INTPTS));
                            gtinp->num++;
                          };
                      };

                    if (Pars.nprint > 0)
                      fprintf (stderr,"reduced gtinp->num from %i to %i\n", nn, gtinp->num);

                  }
                else if (Pars.fixdoublehitsCode == 3)
                  {

                    /* consolidate the two or more hits into one */

                    if (Pars.nprint > 0)
                      fprintf (stderr,"consolidate the hits; TBD\n");

                  };
              };

        };
      ptinp++;
      ptgd++;
    };

  /* done */

#if(SPECIALFIXDOUBLEHITS)
  nxxx++;
  if (nxxx > 5000)
    {
      i1 = 0;
      for (i = 0; i < 100; i++)
        i1 += sp[i];
      for (i = 0; i < 100; i++)
        fprintf (fp, "%9.2f: [%4i] %9.2f%%\n", (float) i / 100, sp[i], (float) sp[i] / i1 * 100);
      exit (0);
    };
#endif

  if (Pars.nprint > 0)
    fprintf (stderr,"exit fixDoubleHitsInSegs\n");

  return (0);

}

/*------------------------------------------------------------------*/

int
trackEvent (float target_pos[3],        /* external beam interaction position */
            TRACK_STRUCT * track,       /* input track data */
            STAT * ctkStat,     /* statistics */
            SHELLHIT * shellhit,        /* internal storage */
            CLUSTER_INTPTS Clstr[MAXCLUSTERHITS], int *nClusters /* tracking outout */ )
{

  /* declarations */

  int i, j, k, st, numCurrentClusters, i1, itno;
  int iCluster, success;
  char str[132];
  FILE *fp;
  float goodEnough, r1;
  int detNo[MAXDETPOS];

//trapbad ("tracEvent",track);

  /* count */

  ctkStat->nTrackCalled++;
  Pars.nprint--;

  if (Pars.nprint > 0)
    {
      fprintf (stderr,"entered track: ****** %6i ******\n", ctkStat->nTrackCalled);
      fflush (stdout);
    };

  if (Pars.nprint > 0)
    fprintf (stderr,"\nevent # %i, detailed logging on disk\n", ctkStat->nTrackCalled);

  /* find the detector numbers as we need */
  /* them for various reasons below, stored in Pars */

  st = findDetectorNumbers (track);

  /* quit if the decomp was bad (checked in findDetectorNumbers) */
  /* we do not try to make the best of it */
  /* we just quit  if (st == BADPAD) */

  if (st == BADPAD)
    {
      ctkStat->badpad++;
      if (Pars.nprint > 0)
        fprintf (stderr,"BADPAD: %i\n", ctkStat->badpad);
      return (BADPAD);
    };

  /* replace the bad resolution tracked energies  */

  mod_coordinates (track);

  modData (track, ctkStat);

  if (0)
    {
      sprintf (str, "A,nrrawEvent_%3.3i.list", ctkStat->nTrackCalled);
      fprintf (stderr,"%s\n", str);
      fp = fopen (str, "w");
      printEvent (fp, ctkStat->nTrackCalled, track);
      fclose (fp);
      if (1)
        exit (0);
    };

  /* debug print */

  if (Pars.nprint > 0)
    {
      sprintf (str, "A,nrrawEvent_%3.3i.list", ctkStat->nTrackCalled);
      fprintf (stderr,"%s\n", str);
      fp = fopen (str, "w");
      printEvent (fp, ctkStat->nTrackCalled, track);
      fclose (fp);
    };

  /* replace bad double hits in same sector */
  /* with single hits instead, compensating */
  /* for problems in the decomposition */

  if (Pars.fixdoublehits)
    {
      st = fixDoubleHitsInSegs (track, ctkStat);
      if (st != 0)
        {
          if (Pars.nprint > 0)
            fprintf (stderr,"MARK: fixDoubleHitsInSegs returned %i\n", st);
          return (st);
        };
    };


  /* before we do anything, rotate and translate */
  /* the crystal hit positions to the actual hit positions */
  /* in the gretina shell (crystal --> Global coordinates ) */

//  printf("findGlobalHitPositions_GT\n"); fflush(stdout);

  st = 0;
  if (Pars.nocrystaltoworldrot == 0)
    {
      if (Pars.AGATA_data == 0)
        st = findGlobalHitPositions_GT (track, ctkStat);
      if (Pars.AGATA_data == 1)
        st = findGlobalHitPositions_AGATA (track, ctkStat);
    };
  if (st != 0)
    {
      if (Pars.nprint > 0)
        fprintf (stderr,"MARK: findGlobalHitPositions_xxx returned %i\n", st);
      return (st);
    };

  /* debug print */

  if (Pars.nprint > 0)
    {
      sprintf (str, "B,rawEvent_%3.3i.list", ctkStat->nTrackCalled);
      fprintf (stderr,"%s\n", str);
      fp = fopen (str, "w");
      printEvent (fp, ctkStat->nTrackCalled, track);
      fclose (fp);
    };

  /* statistics */


  /* parse the input event to the  */
  /* internal tracking format (struct SHELLHIT) */

//  printf("rackUnpack\n"); fflush(stdout);
  st = trackUnpack (track, shellhit, ctkStat);
  if (st != 0)
    {
      if (Pars.nprint > 0)
        fprintf (stderr,"MARK: trackUnpack returned %i\n", st);
      return (st);
    };

  /* find normalization vectors */

//  printf("findNormVectors\n"); fflush(stdout);
  findNormVectors (shellhit);

  /* find realtive angles */

//  printf("findRelativeAngles\n"); fflush(stdout);
  findRelativeAngles (shellhit, ctkStat);

  /* absolute angles only needed for */
  /* debug printouts */

//  printf("findAbsAngles\n"); fflush(stdout);
  if (Pars.nprint > 0)
    findAbsAngles (shellhit);

  /* perform the clustering task */
  /* relative angles determines the */
  /* clusters. Works on the shellhit array */

//  printf("findClusters\n"); fflush(stdout);
  findClusters (shellhit);

  /* debug print the shell hit/ with clustering? */

  if (Pars.nprint > 0)
    {
      sprintf (str, "C,shellhit_%3.3i.list", ctkStat->nTrackCalled);
      fprintf (stderr,"%s\n", str);
      fp = fopen (str, "w");
      printShellHit (fp, shellhit);
      fclose (fp);
    };

  /* transfer the data in the shellhit array */
  /* into the cluster array which is what */
  /* the tracking functions work on. The rest */
  /* of the code from here on will only use */
  /* the cluster array for processing */

//  printf("transferClusters\n"); fflush(stdout);
  st = transferClusters (shellhit, Clstr, nClusters);
  if (st != 0)
    {
      if (Pars.nprint > 0)
        fprintf (stderr,"MARK: transferClusters returned %i\n", st);
      return (st);
    };

  /* nhit statistics */

  /* at this point the cluster array only has */
  /* valid entries since nothing has been */
  /* rearranged yet in reclustering, so this */
  /* assignment is simple */

  *nClusters = shellhit->NumClusters;

  if (Pars.nprint > 0)
    {
      sprintf (str, "D,originalCluster_%3.3i.list", ctkStat->nTrackCalled);
      fprintf (stderr,"%s\n", str);
      printAllClusters (str, str, Clstr, nClusters);
    };

  /* sort segment energies */
  /* so we can skip groups properly */

//  printf("ctksort\n"); fflush(stdout);
  for (i = 0; i < *nClusters; i++)
    ctksort (i, Clstr, nClusters);

  if (Pars.nprint > 0)
    {
      sprintf (str, "E,originalCluster_%3.3i.list", ctkStat->nTrackCalled);
      fprintf (stderr,"%s\n", str);
      printAllClusters (str, str, Clstr, nClusters);
    };


  /*--------------------*/
  /* track each CLUSTER */
  /*--------------------*/

  fp = stdout;

#if(0)
  if (Pars.nprint > 0)
    fprintf (stderr,"start tracking %i clusters>>>\n", nCluster);
#endif
  for (iCluster = 0; iCluster < *nClusters; iCluster++)
    if (Clstr[iCluster].valid)
      {

        ctkStat->trackGetCalls++;

        /* hit statistics (before any gating) */

        ctkStat->nhit[Clstr[iCluster].ndet]++;

        /* count how many passed conditions above */

        ctkStat->trackPassCount++;

        /* find best interaction sequence */
        /* (this is really what is meant by `tracking') */

        i1 = -ctkStat->TrackingCalls;
        Clstr[iCluster].tracked = 0;
        if (Clstr[iCluster].ndet < MAXNOSEG)
          switch (Pars.trackOps[Clstr[iCluster].ndet])
            {
            case 0:
              st = ctktk0 (iCluster, target_pos, ctkStat, Clstr, nClusters);
              ctkStat->ctktk0_errors[st]++;
              break;
            case 1:
              st = ctktk1 (iCluster, target_pos, ctkStat, Clstr, nClusters);
              ctkStat->ctktk1_errors[st]++;
//              printf("case 1\n");
//              exit(0);
              break;
            case 3:
              st = ctktk3 (iCluster, target_pos, ctkStat, Clstr, nClusters);
              ctkStat->ctktk3_errors[st]++;
              break;
            case 4:
              st = ctktk4 (iCluster, target_pos, ctkStat, Clstr, nClusters);
              ctkStat->ctktk4_errors[st]++;
              break;
            case 5:
              st = ctktk5 (iCluster, target_pos, ctkStat, Clstr, nClusters);
              ctkStat->ctktk5_errors[st]++;
              break;
            default:
              fprintf (stderr,"ctk: tracking option not known!?, option=%i\n, Quit\n", Pars.trackOps[Clstr[iCluster].ndet]);
              exit (1);
            };
        i1 += ctkStat->TrackingCalls;
        ctkStat->firstClusterTrackCalls += i1;

        if (Clstr[iCluster].ndet == 1 && Clstr[iCluster].tracked == 0)
          assert (0);


        /* count good tracks with Clstr[iCluster].fom below cut */

        ctkStat->trackFMok++;

#if(0)
        /* what permutations work? */

        ctkStat->permHit[Clstr[iCluster].ndet][Clstr[iCluster].bestPermutation]++;
#endif

        /* sanity check */

#if(0)
        if (Clstr[iCluster].ndet <= MAXNOSEG)
          assert (Clstr[iCluster].tracked == 1);
#endif

        if (Clstr[iCluster].valid)
          if (Clstr[iCluster].tracked)
            if (Clstr[iCluster].fom > MAXFOM)
              {
                fprintf (stderr,"ERROR, ooops, fom make no sense = %f\n", Clstr[iCluster].fom);
                fprintf (stderr,"iCluster=%i\n", iCluster);
                fprintf (stderr,"Pars.trackOps[Clstr[iCluster].ndet=%i\n", Pars.trackOps[Clstr[iCluster].ndet]);
//                assert (Clstr[iCluster].fom < 150);
              };

      };


  /* (re)assign the cluster sum (~center contact energy) */
  /* .esum so far only contained the track sum */
  /* ... does a few more assignments things as well */

  assignClusteresum (Clstr, nClusters);

  /*------------------------------------------------*/
  /* at this point we have formed the clusters */
  /* and tracked them, now we need to split and add */
  /*------------------------------------------------*/

  /* print what we have at this point */

  if (Pars.nprint > 0)
    {
      sprintf (str, "G,firstTrack_%3.3i.list", ctkStat->nTrackCalled);
      fprintf (stderr,"%s\n", str);
      printAllClusters (str, str, Clstr, nClusters);
    };

  /*--------------------------------*/
  /* itteratively trying to improve */
  /*--------------------------------*/

  for (itno = 0; itno < Pars.itterations; itno++)
    {

      /*----------------------------------------*/
      /* look for clusters that are so big they */
      /* were not tracked and split them        */
      /*----------------------------------------*/

      if (Pars.recluster1)
        {

          if (Pars.nprint > 0)
            fprintf (stderr,"track:trying to split untracked monster clusters using reCluster at ev # %i\n", ctkStat->nEvents);

          success = 0;
          numCurrentClusters = *nClusters;
          for (i = 0; i < numCurrentClusters; i++)
            if (Clstr[i].valid)
              if (!Clstr[i].tracked)
                if (Clstr[i].ndet >= Pars.recluster_minn1)
                  {
                    i1 = -ctkStat->TrackingCalls;
                    st =
                      reCluster (i, ctkStat->nTrackCalled, ctkStat, Clstr, nClusters, target_pos, shellhit,
                                 Pars.recluster_reduxfactor1, Pars.recluster_maxtry1, Pars.recluster_kickoutfom1);
                    i1 += ctkStat->TrackingCalls;
                    ctkStat->reClusterReturns1[st]++;
                    ctkStat->reClusterTrackCalls1[st] += i1;
                    if (st != 8)
                      success += 1;
                  };

          /* print what we have at this point (if there were changes) */

          if (Pars.nprint > 0 && success > 0)
            {
              sprintf (str, "H,reclustered_%3.3i.list", ctkStat->nTrackCalled);
              fprintf (stderr,"%s\n", str);
              printAllClusters (str, str, Clstr, nClusters);
            };
        };


      if (Pars.splitclusters1)
        {

          if (Pars.nprint > 0)
            fprintf (stderr,"track: trying to split untracked monster clusters using splitCluster\n");

          success = 0;
          numCurrentClusters = *nClusters;
          for (i = 0; i < numCurrentClusters; i++)
            if (Clstr[i].valid)
              if (!Clstr[i].tracked)
                {
                  i1 = -ctkStat->TrackingCalls;
                  st =
                    splitCluster (i, ctkStat->nTrackCalled, ctkStat, Clstr, nClusters, target_pos,
                                  Pars.untracked_fom_kickout, Pars.splitclusters_maxtry1,
                                  Pars.splitclusters_kickoutfom1);
                  i1 += ctkStat->TrackingCalls;
                  ctkStat->splitClusterReturns1[st]++;
                  ctkStat->splitClusterTrackCalls1[st] += i1;
                  if (st != 18)
                    success = 1;
                };

#if(0)
          /* sanity check */

          for (i = 0; i < *nClusters; i++)
            if (Clstr[i].valid)
              {
                assert (Clstr[i].ndet > 0);
                assert (Clstr[i].ndet < MAX_NDET);
              };
#endif

          /* print what we have at this point */

          if (Pars.nprint > 0 && success)
            {
              sprintf (str, "I,splitUntracked_%3.3i.list", ctkStat->nTrackCalled);
              fprintf (stderr,"%s\n", str);
              printAllClusters (str, str, Clstr, nClusters);
            };

        };

      /*-----------------------------------------*/
      /* now find clusters that must likely are  */
      /* too small and combine them.             */
      /* notice we try single hits nomatter what */
      /* since they have no FOM                  */
      /*-----------------------------------------*/


      if (Pars.combineclusters)
        {

          if (Pars.nprint > 0)
            fprintf (stderr,"track: trying to combine clusters we think are too small\n");

          success = 0;
          numCurrentClusters = *nClusters;
          for (i = 0; i < numCurrentClusters; i++)
            if (Clstr[i].valid)
              if (Clstr[i].fom > Pars.combineclusters_thresh)
                if (Clstr[i].ndet <= Pars.combineclusters_maxn)
                  {
                    i1 = -ctkStat->TrackingCalls;
                    st = combineCluster (i, ctkStat->nTrackCalled, ctkStat, Clstr, nClusters, target_pos);
                    i1 += ctkStat->TrackingCalls;
                    ctkStat->combineClusterReturns[st]++;
                    ctkStat->combineClusterTrackCalls[st] += i1;
                    if (st == 0)
                      success = 1;
                  };

          /* print what we have at this point */

          if (Pars.nprint > 0 && success)
            {
              sprintf (str, "J,combinedEvents_%3.3i.list", ctkStat->nTrackCalled);
              fprintf (stderr,"%s\n", str);
              printAllClusters (str, str, Clstr, nClusters);
            };

        };


      /*----------------------------------------*/
      /* now find clusters that must likely are */
      /* too big and then try to split them     */
      /*----------------------------------------*/

      if (Pars.recluster2)
        {
          if (Pars.nprint > 0)
            fprintf (stderr,"trying to reCluster clusters we think are too big\n");

          success = 0;
          numCurrentClusters = *nClusters;
          for (i = 0; i < numCurrentClusters; i++)
            if (Clstr[i].valid)
              if (Clstr[i].fom > Pars.recluster_thresh2)
                if (Clstr[i].ndet >= Pars.recluster_minn2)
                  {
                    i1 = -ctkStat->TrackingCalls;
                    st = reCluster (i, ctkStat->nTrackCalled, ctkStat, Clstr, nClusters, target_pos,
                                    shellhit, Pars.recluster_reduxfactor2, Pars.recluster_maxtry2,
                                    Pars.recluster_kickoutfom2);
                    i1 += ctkStat->TrackingCalls;
                    ctkStat->reClusterReturns2[st]++;
                    ctkStat->reClusterTrackCalls2[st] += i1;
                  };

          /* print what we have at this point, if there is a change */

          if (Pars.nprint > 0 && *nClusters > numCurrentClusters)
            {
              sprintf (str, "L,reclusterTooBig_%3.3i.list", ctkStat->nTrackCalled);
              fprintf (stderr,"%s\n", str);
              printAllClusters (str, str, Clstr, nClusters);
            };

        };

      if (Pars.splitclusters2)
        {
          if (Pars.nprint > 0)
            fprintf (stderr,"trying to split clusters we think are too big\n");

          success = 0;
          numCurrentClusters = *nClusters;
          for (i = 0; i < numCurrentClusters; i++)
            if (Clstr[i].valid)
              if (Clstr[i].fom > Pars.splitclusters_thresh2)
                if (Clstr[i].ndet >= Pars.splitclusters_minn2)
                  if (Clstr[i].ndet <= Pars.splitclusters_maxn2)
                    {
                      i1 = -ctkStat->TrackingCalls;
                      goodEnough = Pars.goodImproveFraction * Clstr[i].fom;
                      st =
                        splitCluster (i, ctkStat->nTrackCalled, ctkStat, Clstr, nClusters, target_pos, goodEnough,
                                      Pars.splitclusters_maxtry2, Pars.splitclusters_kickoutfom2);
                      i1 += ctkStat->TrackingCalls;
                      ctkStat->splitClusterReturns2[st]++;
                      ctkStat->splitClusterTrackCalls2[st] += i1;
                      if (st != 8)
                        success = 1;
                    };

          /* print what we have at this point */

          if (Pars.nprint > 0 && success)
            {
              sprintf (str, "N,splitTooBig_%3.3i.list", ctkStat->nTrackCalled);
              fprintf (stderr,"%s\n", str);
              printAllClusters (str, str, Clstr, nClusters);
            };

        };

      if (Pars.matchmaker)
        {

          if (Pars.nprint > 0)
            fprintf (stderr,"track: trying to combine single hits\n");

          /* here we should try to come up with  */
          /* a way to evaluate whether we should  */
          /* try to combine some of the hits that  */
          /* are single hits. The above check will  */
          /* not catch any of those cases */

          /* look for single hits within a treshold distance */
          /* and try to combine them <-- strategy */

          /* no foam treshold for kick-in:: FOM is zero for */
          /* these hits per default */

          success = 0;
          numCurrentClusters = *nClusters;
          for (i = 0; i < numCurrentClusters; i++)
            if (Clstr[i].ndet == 1)
              if (Clstr[i].valid)
                for (j = i + 1; j < numCurrentClusters; j++)
                  if (Clstr[j].ndet == 1)
                    if (Clstr[j].valid)
                      {
                        i1 = -ctkStat->TrackingCalls;
                        st = matchMaker (i, j, ctkStat->nTrackCalled, ctkStat, Clstr, nClusters, target_pos);
                        i1 += ctkStat->TrackingCalls;
                        ctkStat->matchMakerReturns[st]++;
                        ctkStat->matchMakerTrackCalls[st] += i1;
                        if (st == 0)
                          success = 1;
                      };


          /* print what we have at this point */

          if (Pars.nprint > 0 && success)
            {
              sprintf (str, "K,matchMakerEvents_%3.3i.list", ctkStat->nTrackCalled);
              fprintf (stderr,"%s\n", str);
              printAllClusters (str, str, Clstr, nClusters);
            };

        };

      if (Pars.pairProd)
        {

          /* see if clusters with bad fom could be pair productions */

          st = pairProd (ctkStat->nTrackCalled, ctkStat, Clstr, nClusters, target_pos);

        };


#if(0)
      /* sanity check */

      for (i = 0; i < *nClusters; i++)
        if (Clstr[i].valid)
          if (Clstr[i].ndet <= 0)
            {
              sprintf (str, "Z,final_%3.3i.list", ctkStat->nTrackCalled);
              fprintf (stderr,"bad: %s\n", str);
              printAllClusters (str, str, Clstr, nClusters);
              fflush (stdout);
              exit (1);
              assert (Clstr[i].ndet > 0);
            }
#endif


    };                          /* end of itteration loop */

  /* print final */

  if (Pars.nprint > 0)
    {
      sprintf (str, "Z,final_%3.3i.list", ctkStat->nTrackCalled);
      fprintf (stderr,"%s\n", str);
      printAllClusters (str, str, Clstr, nClusters);
    };

  /*+++++++++++++++ */
  /* done tracking */
  /*+++++++++++++++ */

  /* mark single hits that are out of range */

  if (Pars.marksinglehitrange)
    {
      numCurrentClusters = *nClusters;
      for (i = 0; i < numCurrentClusters; i++)
        if (Clstr[i].valid)
          if (Clstr[i].ndet == 1)
            {

              i1 = (int) (Clstr[i].esum * 1000);
              if (i1 > 0 && i1 < LONGLEN)
                {
                  r1 = Clstr[i].intpts[0].xx * Clstr[i].intpts[0].xx
                    + Clstr[i].intpts[0].yy * Clstr[i].intpts[0].yy + Clstr[i].intpts[0].zz * Clstr[i].intpts[0].zz;
                  if (r1 > Pars.singlehitrange[i1])
                    {
                      Clstr[i].fom = Pars.singlehitrangeFOM;
                      ctkStat->singlehitoutofrange++;
                    }
                };
            };
    };

  /* mark gammarays that do not have a reasonable */
  /* number of interation points */

  numCurrentClusters = *nClusters;
  for (i = 0; i < numCurrentClusters; i++)
    if (Clstr[i].valid)
      {

        if (Clstr[i].esum < Pars.ndetElim_lo[Clstr[i].ndet])
          {
            Clstr[i].fom = Pars.ndetElim_fom[Clstr[i].ndet];
            ctkStat->badndetElim++;
          };

        if (Clstr[i].esum > Pars.ndetElim_hi[Clstr[i].ndet])
          {
            Clstr[i].fom = Pars.ndetElim_fom[Clstr[i].ndet];
            ctkStat->badndetElim++;
          };
      };

  /* more tracking statistics */

  for (i = 0; i < *nClusters; i++)
    if (Clstr[i].valid)
      {
        if (Clstr[i].tracked)
          {
            ctkStat->sfom += Clstr[i].fom;
//            assert (Clstr[i].fom < MAXFOM);
            if (Clstr[i].ndet > 1)
              {
                ctkStat->sfom_nt += Clstr[i].fom;
                ctkStat->nsfom_nt++;
              };
          };
        ctkStat->nClusters++;
        if (!Clstr[i].tracked)
          ctkStat->notTracked++;
      }

  /* count actual tracked gammas */

  i1 = 0;
  for (i = 0; i < *nClusters; i++)
    if (Clstr[i].valid)
      if (Clstr[i].tracked)
        i1++;
  if (i1 > 0)
    ctkStat->nTrackedGammas++;

  /* done */

  return (0);

}
