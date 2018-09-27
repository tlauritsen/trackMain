
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>

#include "ctk.h"
volatile extern TRACKINGPAR Pars;

#define DEBUG 1

/* ----------------------------------------------------------------------*/

int
#ifdef FULL
ctktk0 (int iCluster, float target_pos[3], STAT * ctkStat, CLUSTER_INTPTS Clstr[MAXCLUSTERHITS], int *nClusters)
#endif
#ifdef MAXE
ctktk1 (int iCluster, float target_pos[3], STAT * ctkStat, CLUSTER_INTPTS Clstr[MAXCLUSTERHITS], int *nClusters)
#endif
#ifdef KICKOUT
ctktk3 (int iCluster, float target_pos[3], STAT * ctkStat, CLUSTER_INTPTS Clstr[MAXCLUSTERHITS], int *nClusters)
#endif
#ifdef GOODENOUGH
ctktk4 (int iCluster, float target_pos[3], STAT * ctkStat, CLUSTER_INTPTS Clstr[MAXCLUSTERHITS], int *nClusters)
#endif
#ifdef JUMP
ctktk5 (int iCluster, float target_pos[3], STAT * ctkStat, CLUSTER_INTPTS Clstr[MAXCLUSTERHITS], int *nClusters)
#endif
{
  /* declarations */

  float fom, minfom, r0;
  int curPerm, k, ii;
  int indx[MAXNOSEG];
  float eg, th, thc, v1[3], v2[3];
  int j;
  float pfac;

#if (DEBUG)
  FILE *fp;
  char str[132];
  float r1;
#endif

#ifdef KICKOUT
  float oldfom = 0;
#endif

#ifdef GOODENOUGH
  float FOMcutGoodenogh;
#endif

#ifdef JUMP
  int distNextGrp, basejump;
  float FOMcutJump;
#endif

#if(DEBUG)
  if (Pars.nprint > 0)
    {
//      if (fp != NULL) fclose(fp);
      sprintf (str, "F,track_%3.3i.list", ctkStat->nTrackCalled);
      fp = fopen (str, "w");
      assert (fp != NULL);
      fprintf (stderr,"%s is open\n", str);
    };
#endif

  /* count how many times we are called */

  ctkStat->TrackingCalls++;

  /* info */

#if(DEBUG)
  if (Pars.nprint > 0)
    {
      fprintf (fp, "\n");
      fprintf (fp, "--------------------------------------------------------------\n");
      fprintf (fp, "\n");
#ifdef FULL
      fprintf (fp, "entered ctktk0/FULL\n");
#endif
#ifdef MAXE
      fprintf (fp, "entered ctktk0/MAXE\n");
#endif
#ifdef KICKOUT
      fprintf (fp, "entered ctktk0/KICKOUT\n");
#endif
#ifdef GOODENOUGH
      fprintf (fp, "entered ctktk0/GOODENOUGH\n");
#endif
#ifdef JUMP
      fprintf (fp, "entered ctktk0/JUMP\n");
#endif
    };
#endif

  /* init */

  Clstr[iCluster].fom = 0;

  /* trap single hits, which are trivial. */
  /* But we have to mark them tracked anyway */

  if (Clstr[iCluster].ndet == 1)
    {
      Clstr[iCluster].tracked = 1;
      Clstr[iCluster].fom = Pars.snglsFom;
      Clstr[iCluster].bestPermutation = 0;
      Clstr[iCluster].intpts[0].order = 0;
#if(DEBUG)
      if (Pars.nprint > 0)
        fprintf (fp, "single interaction, return\n");
#endif
      return (1);
    };

  /* trap if ndet is higher than we can handle */

  if (Clstr[iCluster].ndet >= MAXNOSEG)
    {
      Clstr[iCluster].tracked = 0;
      Clstr[iCluster].fom = MAXFOM;
      return (2);
    }
  else
    Clstr[iCluster].tracked = 1;
  assert (Clstr[iCluster].tracked == 1);

#if(DEBUG)
  if (Pars.nprint > 0)
    {
      fprintf (fp, "iCluster=%2i, # sectors (ndet)= %2i, ", iCluster, Clstr[iCluster].ndet);
      fprintf (fp, "# perm=%10i; ", Pars.nperm[Clstr[iCluster].ndet]);
      fprintf (fp, "target= %5.2f %5.2f %5.2f\n", target_pos[0], target_pos[1], target_pos[2]);
    };
#endif

  assert (Clstr[iCluster].ndet < MAXNOSEG);

#ifdef GOODENOUGH
  FOMcutGoodenogh = Clstr[iCluster].ndet * Pars.fomGoodenough;
#endif

#ifdef MAXE

  /* we just return the order we have as we */
  /* sorted according to energy already */

  j = 0;
  for (k = 0; k < Clstr[iCluster].ndet; k++)
    {
      Clstr[iCluster].intpts[k].order = j;
      j++;
    };

  /* mark as tracked */

  Clstr[iCluster].tracked = 1;

  /* assign the proper fom for it */

  findVector (target_pos[0], target_pos[1], target_pos[2],
              Clstr[iCluster].intpts[0].xx, Clstr[iCluster].intpts[0].yy,
              Clstr[iCluster].intpts[0].zz, &v1[0], &v1[1], &v1[2]);
  eg = Clstr[iCluster].esum;
  fom = 0;
  for (k = 0; k < (Clstr[iCluster].ndet - 1); k++)
    {

      /* find norm vector to next interaction point */

      findVector (Clstr[iCluster].intpts[k].xx, Clstr[iCluster].intpts[k].yy,
                  Clstr[iCluster].intpts[k].zz, Clstr[iCluster].intpts[k + 1].xx,
                  Clstr[iCluster].intpts[k + 1].yy, Clstr[iCluster].intpts[k + 1].zz, &v2[0], &v2[1], &v2[2]);

//      fprintf (stderr,"k  =%i, %f %f %f\n", k, Clstr[iCluster].intpts[k].xx, Clstr[iCluster].intpts[k].yy, Clstr[iCluster].intpts[k].zz);
//      fprintf (stderr,"k+1=%i, %f %f %f\n", k + 1, Clstr[iCluster].intpts[k + 1].xx, Clstr[iCluster].intpts[k + 1].yy, Clstr[iCluster].intpts[k + 1].zz);


      /* find scattering angle */

      findAngle (v1, v2, &th);
//    fprintf (stderr,"vectors:\n");
//    fprintf (stderr,"v1:  (%9.3f,%9.3f,%9.3f)\n", v1[0], v1[1], v1[2]);
//    fprintf (stderr,"v2:  (%9.3f,%9.3f,%9.3f)\n", v2[0], v2[1], v2[2]);
//    fprintf (stderr,"th=%9.4f\n", th);

      /* check for sanity, invalidate so noone else will see it */

      if (th < 0)
        {
          Clstr[iCluster].tracked = 0;
          Clstr[iCluster].valid = 0;
          return (5);
        };
      if (th > PI)
        {
          Clstr[iCluster].tracked = 0;
          Clstr[iCluster].valid = 0;
          return (6);
        };
      if (isnan (th))
        {
          Clstr[iCluster].tracked = 0;
          Clstr[iCluster].valid = 0;
          return (7);
        };

      /* find compton scattering angle */

      pfac = findCAngle (eg, Clstr[iCluster].intpts[k].edet, &thc);

      /* add to Figure of Merit (FOM) */

      fom += (thc - th) * (thc - th);

      /* add penalty if energies makes no sense in */
      /* the compton scattering formula and it returns */
      /* a non zero pfac value */

      fom += (pfac * Pars.badTheoAngPenalty);
//      if (pfac*Pars.badTheoAngPenalty>0)
//        printf("a: added penalty %f\n", pfac*Pars.badTheoAngPenalty);

//      fprintf (stderr," _%i, %f\n", k, fom);


      /* shift norm vector */

      v1[0] = v2[0];
      v1[1] = v2[1];
      v1[2] = v2[2];

      /* subtract energy lost in this scatter */

      eg = eg - Clstr[iCluster].intpts[k].edet;

    };                          /* loop over interaction points */

  fom /= (Clstr[iCluster].ndet - 1);
  Clstr[iCluster].fom = fom;

  /* we are done */

//  fprintf (stderr,"MAXE case fom=%f, ndet=%i\n", Clstr[iCluster].fom, Clstr[iCluster].ndet);
  return (8);

#endif


#ifdef JUMP

  FOMcutJump = Clstr[iCluster].ndet * Pars.fomJump;

  /* distance to next group */
  /* determinde by size of tail */

  basejump = Pars.fac[Clstr[iCluster].ndet - Pars.jmpGrpLen[Clstr[iCluster].ndet]];
  distNextGrp = basejump;

#if(0)
  fprintf (fp, "iCluster=%i\n", iCluster);
  fprintf (fp, "Clstr[iCluster].ndet=%i\n", Clstr[iCluster].ndet);
  fprintf (fp, "Pars.jmpGrpLen[Clstr[iCluster].ndet]=%i\n", Pars.jmpGrpLen[Clstr[iCluster].ndet]);
  fprintf (fp, "basejump=%i\n", basejump);

  for (i = 0; i < MAXNOSEG; i++)
    fprintf (fp, "%i, Pars.jmpGrpLen[i]=%5i\n", i, Pars.jmpGrpLen[i]);

  exit (0);
#endif


#if(DEBUG)
  if (Pars.nprint > 0)
    {
      fprintf (fp, "JUMP init: ndet=%i, basejump=%i in strategy ", Clstr[iCluster].ndet, basejump);

      /* print jump nomenclature */

      for (j = 0; j < Clstr[iCluster].ndet; j++)
        if (j < Pars.jmpGrpLen[Clstr[iCluster].ndet])
          fprintf (fp, "g");
        else
          fprintf (fp, "t");
      for (j = Clstr[iCluster].ndet; j < MAXNOSEG; j++)
        fprintf (fp, " ");
      fprintf (fp, "\n");
    }
#endif
#endif

  /* find sum of interaction energies */

  Clstr[iCluster].esum = 0;
  for (k = 0; k < Clstr[iCluster].ndet; k++)
    Clstr[iCluster].esum += Clstr[iCluster].intpts[k].edet;

  /*-----------------------------*/
  /* loop over ALL permutations  */
  /* full search/not tree search */
  /*-----------------------------*/

  minfom = MAXFLOAT;
  for (curPerm = 0; curPerm < Pars.nperm[Clstr[iCluster].ndet]; curPerm++)
    {

#ifdef KICKOUT
      /* save previous FOM */

      if (curPerm > 0)
        oldfom = minfom;
#endif

      /* count permutation we make */

      ctkStat->nperm++;

      eg = Clstr[iCluster].esum;

      /* find index for lookup */

      for (k = 0; k < Clstr[iCluster].ndet; k++)
        indx[k] = Pars.permlkup[Clstr[iCluster].ndet][curPerm][k];

      /* the lookup is then: */
      /* XX Clstr[iCluster].intpts[i].xx[indx[i]] */
      /* YY Clstr[iCluster].intpts[i].yy[indx[i]] */
      /* ZZ Clstr[iCluster].intpts[i].zz[indx[i]] */
      /* EE Clstr[iCluster].edet[indx[i]] */

#if(DEBUG)
      if (Pars.nprint > 0)
        {
#ifdef JUMP
          if (distNextGrp == basejump)
            fprintf (fp, "\n\n...start of new group\n");
#endif
          fprintf (fp, "**permutation # %3i :: ", curPerm);
          for (k = 0; k < Clstr[iCluster].ndet; k++)
            fprintf (fp, "%i ", indx[k]);
          fprintf (fp, " start eg=%5.3f\n", eg);
        };
#endif

      /* find norm vector to first interaction point */

      findVector (target_pos[0], target_pos[1], target_pos[2],
                  Clstr[iCluster].intpts[indx[0]].xx, Clstr[iCluster].intpts[indx[0]].yy,
                  Clstr[iCluster].intpts[indx[0]].zz, &v1[0], &v1[1], &v1[2]);

      fom = 0;
      for (k = 0; k < (Clstr[iCluster].ndet - 1); k++)
        {

          /* find norm vector to next interaction point */

          findVector (Clstr[iCluster].intpts[indx[k]].xx, Clstr[iCluster].intpts[indx[k]].yy,
                      Clstr[iCluster].intpts[indx[k]].zz, Clstr[iCluster].intpts[indx[k + 1]].xx,
                      Clstr[iCluster].intpts[indx[k + 1]].yy, Clstr[iCluster].intpts[indx[k + 1]].zz, &v2[0],
                      &v2[1], &v2[2]);


          /* find scattering angle */

          findAngle (v1, v2, &th);

#if(DEBUG &&0)
          if (Pars.nprint > 0)
            {
              fprintf (fp, "vectors:");
              fprintf (fp, "v1:  (%9.3f,%9.3f,%9.3f)\n ", v1[0], v1[1], v1[2]);
              fprintf (fp, "v2:  (%9.3f,%9.3f,%9.3f)\n ", v2[0], v2[1], v2[2]);
              fprintf (fp, "th=%9.4f\n", th);
            };
#endif

          /* check for sanity, invalidate so noone else will see it */

          if (th < 0)
            {
              Clstr[iCluster].tracked = 0;
              Clstr[iCluster].valid = 0;
              return (5);
            };
          if (th > PI)
            {
              Clstr[iCluster].tracked = 0;
              Clstr[iCluster].valid = 0;
              return (6);
            };
          if (isnan (th))
            {
              Clstr[iCluster].tracked = 0;
              Clstr[iCluster].valid = 0;
              /*printf("oooopsie, th=%f, isnan=%i at event no # %i\n", th, isnan(th), ctkStat->nEvents); */
              return (7);
            };

          /* find compton scattering angle */

          pfac = findCAngle (eg, Clstr[iCluster].intpts[indx[k]].edet, &thc);

          /* add to Figure of Merit (FOM) */

          fom += (thc - th) * (thc - th);

          /* add penalty if energies makes no sense in */
          /* the compton scattering formula */

          fom += (pfac * Pars.badTheoAngPenalty);
//      if (pfac*Pars.badTheoAngPenalty > 0 ||1)
//        printf("b: added penalty %f, %f, %f\n", pfac,Pars.badTheoAngPenalty, pfac*Pars.badTheoAngPenalty);

          /* shift norm vector */

          v1[0] = v2[0];
          v1[1] = v2[1];
          v1[2] = v2[2];

          /* subtract energy lost in this scatter */

          eg = eg - Clstr[iCluster].intpts[indx[k]].edet;

#if(DEBUG)
          if (Pars.nprint > 0)
            {
              fprintf (fp, "%6.2f %6.2f %6.2f; ", Clstr[iCluster].intpts[indx[k]].xx,
                       Clstr[iCluster].intpts[indx[k]].yy, Clstr[iCluster].intpts[indx[k]].zz);
              fprintf (fp, "(%5.3f); ", Clstr[iCluster].intpts[indx[k]].edet);
              fprintf (fp, "th/cth= %6.4f/%6.4f; ", th, thc);
              r1 = sqrtf (fom) / (Clstr[iCluster].ndet - 1);
              fprintf (fp, "sum square FOM now %9.6f (%9.6f), ", fom, r1);
              fprintf (fp, "eg now %5.3f\n", eg);
            };
#endif

        };                      /* loop over interaction points */

#if(DEBUG)
      if (Pars.nprint > 0)
        {
          k = Clstr[iCluster].ndet - 1;
          fprintf (fp, "%6.2f %6.2f %6.2f; ", Clstr[iCluster].intpts[indx[k]].xx,
                   Clstr[iCluster].intpts[indx[k]].yy, Clstr[iCluster].intpts[indx[k]].zz);
          fprintf (fp, "(%5.3f); ", Clstr[iCluster].intpts[indx[k]].edet);
          fprintf (fp, "(absorption)\n");
        };
#endif

      /* apply last interaction energy out of range penalty */

      if (Pars.checkElast)
        {
          k = Clstr[iCluster].ndet - 1;
          if (Pars.nprint > 0)
            {
              fprintf (fp, "checkElast:\n");
              fprintf (fp, "  fom=%f for permutation %i\n", fom, curPerm);
              fprintf (fp, "  last interaction energy: %f, ", Clstr[iCluster].intpts[indx[k]].edet);
              fprintf (fp, "range %f to %f\n", Pars.checkElast_lo, Pars.checkElast_hi);
            }
          if (Clstr[iCluster].intpts[indx[k]].edet <= Pars.checkElast_lo
              || Clstr[iCluster].intpts[indx[k]].edet >= Pars.checkElast_hi)
            {
              fom *= Pars.checkElast_penalty;
              if (Pars.nprint > 0)
                fprintf (fp, "  elast: apply penalty, new fom=%f\n", fom);
            }

        };

      /* apply max energy if not first interaction penalty */

      if (Pars.checkEfirst)
        if (Clstr[iCluster].esum>Pars.checkEfirst_lo)
        {
          k = Clstr[iCluster].ndet - 1;
          if (Pars.nprint > 0)
            fprintf (fp, "  efirst: first point interaction order: %i\n", Pars.permlkup[Clstr[iCluster].ndet][curPerm][0]);
          if (Pars.permlkup[Clstr[iCluster].ndet][curPerm][0] != 0)
            {
              /* progressive penalty at higher energies */

#if(0)
              if (Clstr[iCluster].esum<1.0)
                fom *= Pars.checkEfirst_penalty;
              else
                fom *= (Pars.checkEfirst_penalty*Clstr[iCluster].esum);
#endif

                fom *= (Pars.checkEfirst_penalty*(Clstr[iCluster].esum-Pars.checkEfirst_lo));

              if (Pars.nprint > 0)
                fprintf (fp, "  efirst: apply penalty,  fom now = %f\n", fom);
            }

        };

      /*--------------------------------------------*/
      /* keep record of the best permutation so far */
      /*--------------------------------------------*/

      if (fom < minfom)
        {
          minfom = fom;
          Clstr[iCluster].bestPermutation = curPerm;
          Clstr[iCluster].fom = fom;

          /* store the permutation order, This is trickier */
          /* than you might think */

          for (k = 0; k < Clstr[iCluster].ndet; k++)
            Clstr[iCluster].intpts[indx[k]].order = k;
#if(DEBUG)
          if (Pars.nprint > 0)
            {
              fprintf (fp, "...best permutation so far\n");
              fprintf (fp, "...Clstr[iCluster].bestPermutation= %i, \n", Clstr[iCluster].bestPermutation);
              fprintf (fp, "...Clstr[iCluster].fom=%f\n", Clstr[iCluster].fom);
            };
#endif
        };





#ifdef KICKOUT

      /* if we are doing worse than last */
      /* permutation we tried, kickout here */

      if (curPerm > 0)
        if (fom > oldfom)
          {

            /* normalize FOM, debug print, return minfom */

            minfom /= (Clstr[iCluster].ndet - 1);
            Clstr[iCluster].fom = minfom;

#if(DEBUG)
            if (Pars.nprint > 0)
              fprintf (fp, "best (kickout) FM= %f for permutation # %i\n", minfom, Clstr[iCluster].bestPermutation);
#endif

            return (3);
          };

#endif






#ifdef GOODENOUGH

      /* is this permutation FOM below */
      /* cut? I.e., Goodenough */

      if (fom < FOMcutGoodenogh)
        {

          /* normalize FOM, debug print, return minfom */

          minfom /= (Clstr[iCluster].ndet - 1);
          Clstr[iCluster].fom = minfom;

#if(DEBUG)
          if (Pars.nprint > 0)
            fprintf (fp, "best (goodenough) FM= %f for permutation # %i\n", minfom, Clstr[iCluster].bestPermutation);
#endif

          return (4);
        };
#endif








#ifdef JUMP

#if(DEBUG)

      if (Pars.nprint > 0)
        fprintf (fp, "...distNextGrp=%i\n", distNextGrp);
#endif

      /* jump ahead if this fom is over the fom cut */

      if (fom > Pars.fomJump)
        {
#if(DEBUG)
          if (Pars.nprint > 0)
            {
              fprintf (fp, "JUMP: fom>Pars.fomJump: %f > %f, jump %i [+=%i] ", fom, Pars.fomJump, distNextGrp,
                       distNextGrp - 1);
              fprintf (fp, "(basejump=%i)\n", basejump);
            };
#endif

          /* forward curPerm, remember we gain one anyway */

          curPerm += (distNextGrp - 1);

          /* reset distNextGrp */

          distNextGrp = basejump;

        }
      else
        {

          /* find distance to next group */

          distNextGrp--;
          if (distNextGrp <= 0)
            distNextGrp = basejump;

        };


#endif





    };                          /* loop over permutations */

  /* normalize FOM, debug print, return minfom */

#if(0)
  /* old way I had it, could argue OK, but... */
  /* below I take the root and normalize */
  minfom /= (Clstr[iCluster].ndet - 1);
#endif
  minfom = sqrtf (minfom) / (Clstr[iCluster].ndet - 1);
  Clstr[iCluster].fom = minfom;




#if(DEBUG)
  if (Pars.nprint > 0)
    {
      fprintf (fp, "\n");
      fprintf (fp, "tracking, final result:\n");
      fprintf (fp, "best FOM is %10.6f for permutation # %i\n\n", minfom, Clstr[iCluster].bestPermutation);
      for (k = 0; k < Clstr[iCluster].ndet; k++)
        {
          fprintf (fp, "%6.2f %6.2f %6.2f; ",
                   Clstr[iCluster].intpts[k].xx, Clstr[iCluster].intpts[k].yy, Clstr[iCluster].intpts[k].zz);
          fprintf (fp, "(%5.3f); ", Clstr[iCluster].intpts[k].edet);
          fprintf (fp, "interaction order: %i\n", Clstr[iCluster].intpts[k].order);
        };
      fprintf (fp, "reported FOM is: %10.6f\n", Clstr[iCluster].fom);
    };
#endif

  /* done */

#if(DEBUG)
  if (Pars.nprint > 0)
    {
      fprintf (fp, "\n");
      fclose (fp);
    };
#endif

  return (0);

}
