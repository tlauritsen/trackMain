
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

volatile extern TRACKINGPAR Pars;

/*----------------------------------------------------------------------------*/

int
get_GEB_Type_str (int type, char str[])
{
//   printf("got type %i\n", type);
  if (type == GEB_TYPE_DECOMP)
    sprintf (str, "GEB_TYPE_DECOMP      ");
  else if (type == GEB_TYPE_BGS)
    sprintf (str, "GEB_TYPE_BGS         ");
  else if (type == GEB_TYPE_RAW)
    sprintf (str, "GEB_TYPE_RAW         ");
  else if (type == GEB_TYPE_TRACK)
    sprintf (str, "GEB_TYPE_TRACK       ");
  else if (type == GEB_TYPE_S800_RAW)
    sprintf (str, "GEB_TYPE_S800_RAW    ");
  else if (type == GEB_TYPE_NSCLnonevent)
    sprintf (str, "GEB_TYPE_NSCLnonevent");
  else if (type == GEB_TYPE_GT_SCALER)
    sprintf (str, "GEB_TYPE_GT_SCALER   ");
  else if (type == GEB_TYPE_GT_MOD29)
    sprintf (str, "GEB_TYPE_GT_MOD29    ");
  else if (type == GEB_TYPE_S800PHYSDATA)
    sprintf (str, "GEB_TYPE_S800PHYSDATA");
  else if (type == GEB_TYPE_G4SIM)
    sprintf (str, "GEB_TYPE_G4SIM       ");
  else if (type == GEB_TYPE_CHICO)
    sprintf (str, "GEB_TYPE_CHICO       ");
  else if (type == GEB_TYPE_DGS)
    sprintf (str, "GEB_TYPE_DGS         ");
  else
    sprintf (str, "unknown");
//      printf("type: %s\n",str);

};

/*----------------------------------------------------------------------------*/

int
print_S800PHYSDATA (FILE * fp, S800_PHYSICSDATA * dirk)
{

  /* list all Dirks stuff */

  fprintf (fp, " crdc1_x    = %10.5f ", dirk->crdc1_x);
  fprintf (fp, " crdc1_y    = %10.5f \n", dirk->crdc1_y);
  fprintf (fp, " crdc2_x    = %10.5f ", dirk->crdc2_x);
  fprintf (fp, " crdc2_y    = %10.5f \n", dirk->crdc2_y);
  fprintf (fp, " ic_sum     = %10.5f \n", dirk->ic_sum);
  fprintf (fp, " tof_xfp    = %10.5f ", dirk->tof_xfp);
  fprintf (fp, " tof_obj    = %10.5f \n", dirk->tof_obj);
  fprintf (fp, " rf         = %10.5f \n", dirk->rf);
  fprintf (fp, " trigger    = %i10 \n", dirk->trigger);
  fprintf (fp, " tof_xfpe1  = %10.5f ", dirk->tof_xfpe1);
  fprintf (fp, " tof_rfe1   = %10.5f \n", dirk->tof_rfe1);
  fprintf (fp, " tof_obje1  = %10.2f ", dirk->tof_obje1);
  fprintf (fp, " ic_de      = %10.3f \n", dirk->ic_de);
  fprintf (fp, " ata        = %10.5f ", dirk->ata);
  fprintf (fp, " bta        = %10.5f \n", dirk->bta);
  fprintf (fp, " dta        = %10.5f ", dirk->dta);
  fprintf (fp, " yta        = %10.5f \n", dirk->yta);

  /* done */

  return (0);

}

/*----------------------------------------------------------------------------*/

int
printCRYS_INTPTS (FILE * fp, CRYS_INTPTS * TT, GEBDATA * DG)
{

  /* declarations */

  int j, i1, i2;
  float sum, r1;
  char str[100];

  /* printf container header */

  get_GEB_Type_str (DG->type, str);
  fprintf (fp, "data container: type=%i(%s), ", DG->type, str);
  fprintf (fp, "length=%i, ", DG->length);
  fprintf (fp, "TS=%lli\n", DG->timestamp);

  /* data print header */

  fprintf (fp, "__num ID=(crystalno/detpos) tot_e t0 chisq norm_chisq timestamp          \n");
  fprintf (fp, "%2i ", TT->num);
  i1 = (TT->crystal_id & 0x0003);
  i2 = ((TT->crystal_id & 0xfffc) >> 2);
  fprintf (fp, "[id=0x%4.4x] ", TT->crystal_id);
  fprintf (fp, "ID=(%2.2i/", i2);       /* Crystal number */
  fprintf (fp, "%2.2i)", i1);   /* Detector position */
  fprintf (fp, "[detno=%3.3i] ", i2 * 4 + i1);
  fprintf (fp, "e= %5.3f ", TT->tot_e);
  fprintf (fp, "%5.3f ", TT->t0);
  fprintf (fp, "%5.3f ", TT->chisq);
  fprintf (fp, "%5.3f ", TT->norm_chisq);
  fprintf (fp, "%10lli ", TT->timestamp);
  fprintf (fp, "\n");

  /* print interactions */

  sum = 0;
  for (j = 0; j < TT->num; j++)
    {
      fprintf (fp, "  [%i] ", j);
      fprintf (fp, "(%8.4f ", TT->intpts[j].x);
      fprintf (fp, "%8.4f ", TT->intpts[j].y);
      fprintf (fp, "%8.4f); ", TT->intpts[j].z);
      fprintf (fp, "%5.0f ", TT->intpts[j].e);
      sum += TT->intpts[j].e;
      fprintf (fp, "{sum %5.0f/%5.0f} ", sum, TT->tot_e);
      r1 = TT->intpts[j].x * TT->intpts[j].x;
      r1 += TT->intpts[j].y * TT->intpts[j].y;
      r1 += TT->intpts[j].z * TT->intpts[j].z;
      r1 = sqrtf (r1);
      fprintf (fp, "r=%9.2f ", r1);
      fprintf (fp, "\n");
    };




  /* done */

  return (0);

}

/*-----------------------------------------------------*/

int
print_tracked_gamma_rays (FILE * fp, TRACKED_GAMMA_HIT * grh)
{

/* declarations */

  int j;

  fprintf (fp, "number of gamma rays: %i\n", grh->ngam);
  for (j = 0; j < grh->ngam; j++)
    {
      fprintf (fp, "esum=%6.3f, ", grh->gr[j].esum);
      fprintf (fp, "ndet (#interactions)=%2i, ", grh->gr[j].ndet);
      fprintf (fp, "fom=%6.3f, ", grh->gr[j].fom);
      fprintf (fp, "tracked=%i\n", grh->gr[j].tracked);
      fprintf (fp, "1'th hit: (%6.3f, %6.3f, %6.3f), e= %6.3f\n", grh->gr[j].x0, grh->gr[j].y0, grh->gr[j].z0,
               grh->gr[j].e0);
      fprintf (fp, "2'nd hit: (%6.3f, %6.3f, %6.3f), e= %6.3f\n", grh->gr[j].x1, grh->gr[j].y1, grh->gr[j].z1,
               grh->gr[j].e1);
    };

  /* done */

  return (0);

};


/*-----------------------------------------------------*/

int
printEvent (FILE * fp, int evno, TRACK_STRUCT * track)
{

  /* function to print the raw event that we receive */

  /* declarations */

  int i, j, ncomptonloss = 0, i1, i2, eventnumber;
  PAYLOAD *ptinp;
  GEBDATA *ptgd;
  CRYS_INTPTS *gtinp;
  float sum = 0, diff, r1;

  /* print an event out in human readable form */

  fprintf (fp, "\n");

  fprintf (fp, "Event Builder event # %i, #payloads=%i\n", evno, track->n);
  fprintf (fp, "||||\n");


  ptinp = track->payload;
  ptgd = track->gd;

  for (i = 0; i < track->n; i++)
    {

//      fprintf (fp, "ptgd->type = %i, ptgd->length = %i\n", ptgd->type, ptgd->length);
      fprintf (fp, "----%2i> ", i);
      fprintf (fp, "[ptgd->type = %i]\n", ptgd->type);

      if (ptgd->type == GEB_TYPE_DECOMP)
        {

          gtinp = (CRYS_INTPTS *) ptinp;

          fprintf (fp, "GEB_TYPE_DECOMP with timestamp %lli\n", ptgd->timestamp);
          fprintf (fp, "  num (crystalno/detpos) tot_e t0 chisq norm_chisq timestamp          \n");
          fprintf (fp, "#interactions=%2i ", gtinp->num);
#if(0)
          fprintf (fp, "ID=(%2.2i/", (gtinp->crystal_id & 0x0030) >> 4);        /* Digitizer number */
          fprintf (fp, "%2.2i/", gtinp->crystal_id & 0x000f);   /* Digitizer Channel ID */
          fprintf (fp, "%2.2i/", (gtinp->crystal_id & 0x00c0) >> 6);    /* Crystal number */
          fprintf (fp, "%2.2i) ", (gtinp->crystal_id & 0x1f00) >> 8);   /* Detector position */
          fprintf (fp, "[id=0x%4.4x] ", gtinp->crystal_id);
#endif
          i1 = (gtinp->crystal_id & 0x0003);
          i2 = ((gtinp->crystal_id & 0xfffc) >> 2);
          fprintf (fp, "ID=(%2.2i/", i1);       /* Crystal number */
          fprintf (fp, "%2.2i) ", i2);  /* Detector position */
          fprintf (fp, "[id=0x%4.4x(%i)] ", gtinp->crystal_id, gtinp->crystal_id);
          fprintf (fp, "[detno=%3.3i] ", i2 * 4 + i1);


          fprintf (fp, "%5.3f ", gtinp->tot_e);
          fprintf (fp, "%5.3f ", gtinp->t0);
          fprintf (fp, "%5.3f ", gtinp->chisq);
          fprintf (fp, "%5.3f ", gtinp->norm_chisq);
          fprintf (fp, "%10lli ", gtinp->timestamp);
          fprintf (fp, "\n");
          fprintf (fp, "  raw pad=%2i", gtinp->pad);
          eventnumber=gtinp->pad&0xffffff00;
          eventnumber=eventnumber>>8;
          gtinp->pad&=0x000000ff;
          fprintf (fp, " real pad=%2i", gtinp->pad);
          fprintf (fp, " G4 event #=%2i", eventnumber);
          fprintf (fp, "\n");
          fflush (fp);

          /* print positions */

#if(0)
          assert (gtinp->num > 0);
          assert (gtinp->num < MAX_INTPTS);
#endif
          if (gtinp->num <= 0)
            fprintf (fp, "     ....wierd, no interaction points...\n");
          sum = 0;
          for (j = 0; j < gtinp->num; j++)
            {
              fprintf (fp, "  [%i] ", j);
              fprintf (fp, "(%8.4f ", gtinp->intpts[j].x);
              fprintf (fp, "%8.4f ", gtinp->intpts[j].y);
              fprintf (fp, "%8.4f); ", gtinp->intpts[j].z);
              fprintf (fp, "seg=%2i; ", gtinp->intpts[j].seg);
              r1 = gtinp->intpts[j].x * gtinp->intpts[j].x
                + gtinp->intpts[j].y * gtinp->intpts[j].y + gtinp->intpts[j].z * gtinp->intpts[j].z;
              r1 = sqrtf (r1);
              fprintf (fp, " r=%5.1fcm ", r1);
              fprintf (fp, "%5.0f ", gtinp->intpts[j].e);
              sum += gtinp->intpts[j].e;
              fprintf (fp, "{sum %5.0f/%5.0f} ", sum, gtinp->tot_e);

#if(0)
              if (j == (gtinp->num - 1))
                {
                  diff = sum - gtinp->tot_e;
                  if (diff < 0)
                    diff = -diff;
                  if (diff < 5)
                    fprintf (fp, "full energy");
                  else
                    {
                      fprintf (fp, "COMPTON LOSS=%5.0f", diff);
                      ncomptonloss++;
                    };
                }
#endif

              fprintf (fp, "\n");
            };
        }
      else if (ptgd->type == GEB_TYPE_RAW)
        {
          fprintf (fp, "GEB_TYPE_RAW  with timestamp %lli\n", ptgd->timestamp);
          fprintf (fp, "[no interpretation here]\n");
        }
      else if (ptgd->type == GEB_TYPE_TRACK)
        {
          fprintf (fp, "GEB_TYPE_TRACK with timestamp %lli\n", ptgd->timestamp);
          print_tracked_gamma_rays (fp, (TRACKED_GAMMA_HIT *) ptinp);

        }
      else if (ptgd->type == GEB_TYPE_BGS)
        {
          fprintf (fp, "GEB_TYPE_BGS with timestamp %lli\n", ptgd->timestamp);
          fprintf (fp, "[no interpretation here]\n");
        }
      else if (ptgd->type == GEB_TYPE_S800_RAW)
        {
          fprintf (fp, "GEB_TYPE_S800_RAW with timestamp %lli\n", ptgd->timestamp);
          fprintf (fp, "[no interpretation here]\n");
        }
      else if (ptgd->type == GEB_TYPE_NSCLnonevent)
        {
          fprintf (fp, "GEB_TYPE_NSCLnonevent with timestamp %lli\n", ptgd->timestamp);
          fprintf (fp, "[no interpretation here]\n");
        }
      else if (ptgd->type == GEB_TYPE_GT_SCALER)
        {
          fprintf (fp, "GEB_TYPE_GT_SCALER with timestamp %lli\n", ptgd->timestamp);
          fprintf (fp, "[no interpretation here]\n");
        }
      else if (ptgd->type == GEB_TYPE_GT_MOD29)
        {
          fprintf (fp, "GEB_TYPE_GT_MOD29 with timestamp %lli\n", ptgd->timestamp);
          fprintf (fp, "[no interpretation here]\n");
        }
      else if (ptgd->type == GEB_TYPE_S800PHYSDATA)
        {
          fprintf (fp, "GEB_TYPE_S800PHYSDATA with timestamp %lli\n", ptgd->timestamp);
          print_S800PHYSDATA (fp, (S800_PHYSICSDATA *) ptinp);
        }
      else if (ptgd->type == GEB_TYPE_G4SIM)
        {
          fprintf (fp, "GEB_TYPE_G4SIM with timestamp %lli\n", ptgd->timestamp);
          fprintf (fp, "[no interpretation here]\n");
        }
      else if (ptgd->type == GEB_TYPE_CHICO)
        {
          fprintf (fp, "GEB_TYPE_CHICO with timestamp %lli\n", ptgd->timestamp);
          fprintf (fp, "[no interpretation here]\n");
        }
      else if (ptgd->type == GEB_TYPE_DGS)
        {
          fprintf (fp, "GEB_TYPE_DGS with timestamp %lli\n", ptgd->timestamp);
          fprintf (fp, "[no interpretation here]\n");
        }
      else if (ptgd->type == GEB_TYPE_DGSTRIG)
        {
          fprintf (fp, "GEB_TYPE_DGSTRIG with timestamp %lli\n", ptgd->timestamp);
          fprintf (fp, "[no interpretation here]\n");
        }
      else if (ptgd->type == GEB_TYPE_DFMA)
        {
          fprintf (fp, "GEB_TYPE_DFMA with timestamp %lli\n", ptgd->timestamp);
          fprintf (fp, "[no interpretation here]\n");
        }
      else if (ptgd->type == GEB_TYPE_PHOSWICH)
        {
          fprintf (fp, "GEB_TYPE_PHOSWICH with timestamp %lli\n", ptgd->timestamp);
          fprintf (fp, "[no interpretation here]\n");
        }
      else if (ptgd->type == GEB_TYPE_PHOSWICHAUX)
        {
          fprintf (fp, "GEB_TYPE_PHOSWICHAUX with timestamp %lli\n", ptgd->timestamp);
          fprintf (fp, "[no interpretation here]\n");
        }
      else
        {
          fprintf (fp, "data with type %i and timestamp %lli\n", ptgd->type, ptgd->timestamp);
          fprintf (fp, "[no interpretation here]\n");
        }

      /* next */

      ptinp++;
      ptgd++;

    };

  fprintf (fp, "\n");
  fprintf (fp, "\n");

#if(0)
  r1 = (float) (track->n - ncomptonloss) / (float) track->n;
  fprintf (fp, "\n");
  fprintf (fp, "max reconstruction possible: %5.1f %%\n", r1 * 100);
  fprintf (fp, "\n");
#endif

  /* done */

  return (0);

}


/*--------------------------------------------------------------------*/

int
printShellHit (FILE * fp, SHELLHIT * shellhit)
{

  /* declarations */

  int j, oldClusterNumber = -1;

  fprintf (fp, "shell hit print\n");
  fprintf (fp, "%i shell hits in %4i clusters;", shellhit->nhit, shellhit->NumClusters);
  fprintf (fp, " simulated/true:[%i]\n", shellhit->knownNumClusters);
  fprintf (fp, "\n");

  oldClusterNumber = shellhit->ClusterNumber[0];
  for (j = 0; j < shellhit->nhit; j++)
    {
      if (shellhit->ClusterNumber[j] != oldClusterNumber)
        fprintf (fp, "\n");

      fprintf (fp, "%2i:: %2i/%1i xyz:", j, shellhit->module[j], shellhit->crystaltype[j]);
      fprintf (fp, "(%6.2f ", shellhit->XX[j]);
      fprintf (fp, "%6.2f ", shellhit->YY[j]);
      fprintf (fp, "%6.2f ) ", shellhit->ZZ[j]);
      fprintf (fp, "e/esum[%7.4f  ", shellhit->edet[j]);
      fprintf (fp, "%7.4f] ", shellhit->esum[j]);
      fprintf (fp, "ts %8i ", (int) shellhit->timestamp[j]);
      fprintf (fp, "cluster: %2i ;", shellhit->ClusterNumber[j]);
      fprintf (fp, "(%2i) ", shellhit->knownClusterNumber[j]);
      fprintf (fp, "origPos: %2i", shellhit->origPos[j]);

      oldClusterNumber = shellhit->ClusterNumber[j];
      fprintf (fp, "\n");
    };

  fprintf (fp, "\n");
  fprintf (fp, "\n");
  oldClusterNumber = shellhit->ClusterNumber[0];
  for (j = 0; j < shellhit->nhit; j++)
    {
      if (shellhit->ClusterNumber[j] != oldClusterNumber)
        fprintf (fp, "\n");

      fprintf (fp, "%2i:: nxyz:", j);
      fprintf (fp, "(%11.6f ", shellhit->nXX[j]);
      fprintf (fp, "%11.6f ", shellhit->nYY[j]);
      fprintf (fp, "%11.6f ) r/pol/azi ", shellhit->nZZ[j]);
      fprintf (fp, "( %5.2f  ", shellhit->rLen[j]);
      fprintf (fp, "%5.1f  ", shellhit->polAng[j] / RAD2DEG);
      fprintf (fp, "%6.1f )  ", shellhit->aziAng[j] / RAD2DEG);
      fprintf (fp, "cluster: %2i ;", shellhit->ClusterNumber[j]);
      fprintf (fp, "origPos: %2i ;", shellhit->origPos[j]);

      oldClusterNumber = shellhit->ClusterNumber[j];
      fprintf (fp, "\n");


    };

  /* done */

  return (0);

}

/*----------------------------------------------------------------------------------------------*/

int
printCluster (int iCluster, FILE * fp, CLUSTER_INTPTS Clstr[MAXCLUSTERHITS])
{

  /* declarations */

  int i;
  float sum, rr;

  /* explain */

  if (iCluster == 0)
    {
      fprintf (fp, "\n");
      /*fprintf (fp, "printed/sorted according to tracking order\n"); */
      fprintf (fp, "**Tracked data\n");
      fprintf (fp, "[n/m] n: tracked pos, m: original pos\n");
      fprintf (fp, "\n");
      fflush (fp);
    }

  /* print */

  sum = 0;
  fprintf (fp, "%2i: ", iCluster);
  fflush (fp);
  if (Clstr[iCluster].valid)
    {
      fprintf (fp, " (valid) ndet=%2i ", Clstr[iCluster].ndet);
      fflush (fp);

      fprintf (fp, "esum=%7.4f", Clstr[iCluster].esum);
      if (Clstr[iCluster].tracked == 1)
        {
          fprintf (fp, "/bestperm=%5.5i/", Clstr[iCluster].bestPermutation);
          fprintf (fp, "FOM=%7.4f; (tracked)\n", Clstr[iCluster].fom);
        }
      else
        {
          fprintf (fp, " --- WARNING:: not tracked!!!\n");
//          return (0);
        };
      fflush (fp);

      for (i = 0; i < Clstr[iCluster].ndet; i++)
        {

          if (Clstr[iCluster].intpts[i].order == 0)
            {
              fprintf (fp, "# IP-> ");
              fflush (fp);
            }
          else
            {
              fprintf (fp, "# ..   ");
              fflush (fp);
            }

          fprintf (fp, "( %6.2f %6.2f %6.2f ) ", Clstr[iCluster].intpts[i].xx, Clstr[iCluster].intpts[i].yy,
                   Clstr[iCluster].intpts[i].zz);
          fprintf (fp, "order: %1i ; ", Clstr[iCluster].intpts[i].order);

          rr = Clstr[iCluster].intpts[i].xx * Clstr[iCluster].intpts[i].xx
            + Clstr[iCluster].intpts[i].yy * Clstr[iCluster].intpts[i].yy
            + Clstr[iCluster].intpts[i].zz * Clstr[iCluster].intpts[i].zz;
          rr = (float) sqrt ((double) rr);
          fprintf (fp, "r=%6.2f cm", rr);
//          fprintf (fp, " {%2i}; ", Clstr[iCluster].intpts[i].shellHitPos);
          sum += Clstr[iCluster].intpts[i].edet;
          fprintf (fp, "e=%7.3f/s=%7.3f; ", Clstr[iCluster].intpts[i].edet, sum);
          fprintf (fp, "%2i", Clstr[iCluster].intpts[i].order);
          fprintf (fp, ", ts=%lli; ", Clstr[iCluster].intpts[i].timestamp);


          fprintf (fp, "\n");

        }
#if(0)

      /* only trackMain etc has this part */

      fprintf (fp, "     ndetFun (lo/mean/hi)= %3.1f/%3.1f/%3.1f; ", Pars.ndetFun_lo[(int) (Clstr[i].esum * 1000)],
               Pars.ndetFun[(int) (Clstr[i].esum * 1000)], Pars.ndetFun_hi[(int) (Clstr[i].esum * 1000)]);
      fflush (fp);

      if (Clstr[iCluster].ndet <= Pars.ndetFun_hi[(int) (Clstr[i].esum * 1000)]
          && Clstr[iCluster].ndet >= Pars.ndetFun_lo[(int) (Clstr[i].esum * 1000)])
        fprintf (fp, " ** ndet in range\n");
      else if (Clstr[iCluster].ndet > Pars.ndetFun_hi[(int) (Clstr[i].esum * 1000)])
        fprintf (fp, " ** ndet high\n");
      else if (Clstr[iCluster].ndet < Pars.ndetFun_lo[(int) (Clstr[i].esum * 1000)])
        fprintf (fp, " ** ndet low\n");
#endif

    }
  else
    {
      fprintf (fp, " removed/invalid...\n");
      fflush (fp);
    };


  /* done */

  fflush (fp);
  return (0);

}

/*-------------------------------------------------------------------*/

int
printAllClusters (char *fn, char *str, CLUSTER_INTPTS Clstr[MAXCLUSTERHITS], int *nClusters)
{

  /* declarations */

  int i, i0 = 0, j, KK=0;
  FILE *fp;
  float HH=0;

  fp = fopen (fn, "w");
  fprintf (fp, "%s", str);

  assert (fp != NULL);
  fprintf (stderr,"%s, %i\n", str, *nClusters);

  for (i = 0; i < *nClusters; i++)
    {
      /* print this cluster (we print/mark invalid clusters!) */

      printCluster (i, fp, Clstr);

      /* find statistics */

      if (Clstr[i].valid)
        {
          i0++;

          /* find first interaction point */

          j = 0;
          while (Clstr[i].intpts[j].order != 0)
            j++;

          HH+=Clstr[i].esum;
          KK++;

        };
    };
  fprintf (fp, "\n");
  fprintf (fp, "We found %2i (valid) gamma rays; ", i0);
  fprintf(fp," KK= %i, HH= %6.3f, mean E=%6.3f", KK,HH, HH/KK);
  fprintf (fp, "\n");

  /* done */

  fclose (fp);
  return (0);
}
