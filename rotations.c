

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

extern TRACKINGPAR Pars;        /* tracking parameters */

/*------------------------------------------------------------------*/

int
findGlobalHitPositions_GT (TRACK_STRUCT * track, STAT * ctkStat)
{
  /* use the rmatrices to put the hits specified by the */
  /* decomposition into the actual positions in the */
  /* GRETINA shell based on holeNum number and crystal type */

  /* declarations */

  int i, j, holeNum, crystalNumber, k, l;
  float xx, yy, zz;
  PAYLOAD *ptinp;
  GEBDATA *ptgd;
  CRYS_INTPTS *gtinp;

  /* rotate and translate */

  ptinp = track->payload;
  ptgd = track->gd;
  for (i = 0; i < track->n; i++)
    {
      if (ptgd->type == GEB_TYPE_DECOMP)
        {

          gtinp = (CRYS_INTPTS *) ptinp;

          /* common for these hits */

          holeNum = ((gtinp->crystal_id & 0xfffc) >> 2);
          crystalNumber = (gtinp->crystal_id & 0x0003);

//          holeNum = (int) gtinp->crystal_id / 4 + 1;
//          crystalNumber = gtinp->crystal_id % 4;


          assert (holeNum >= 0);

          if (Pars.nprint > 0)
            {
              fprintf (stderr, "\n");
              fprintf (stderr, "entered:: findGlobalHitPositions_GT at event # %i\n", ctkStat->nTrackCalled);
              fprintf (stderr, "holeNum =%i ; ", holeNum);
              fprintf (stderr, "crystalNumber =%i, --> detno: %i\n", crystalNumber, holeNum * 4 + crystalNumber);
            };

          /* check for conditions that would prevent */
          /* us from finding the world coordinates */

          if (holeNum > MAXDETPOS)
            return (MODULENOOUTOFRANGE);

          if (crystalNumber > MAXCRYSTALNO)
            return (CRYSTALNOOUTOFRANGE);

          /* rotate/translate the individual hits */

          for (j = 0; j < gtinp->num; j++)
            {

              xx = gtinp->intpts[j].x;
              yy = gtinp->intpts[j].y;
              zz = gtinp->intpts[j].z;

              if (Pars.nprint > 0)
                {
                  fprintf (stderr, "crmat: hit no (j): %i of %i\n", j, gtinp->num);
                  fprintf (stderr, "crystal coords: %9.4f %9.4f %9.4f \n", xx, yy, zz);
                };

              gtinp->intpts[j].x = Pars.crmat[holeNum][crystalNumber][0][0] * xx
                + Pars.crmat[holeNum][crystalNumber][0][1] * yy
                + Pars.crmat[holeNum][crystalNumber][0][2] * zz + Pars.crmat[holeNum][crystalNumber][0][3];

              gtinp->intpts[j].y = Pars.crmat[holeNum][crystalNumber][1][0] * xx
                + Pars.crmat[holeNum][crystalNumber][1][1] * yy
                + Pars.crmat[holeNum][crystalNumber][1][2] * zz + Pars.crmat[holeNum][crystalNumber][1][3];

              gtinp->intpts[j].z = Pars.crmat[holeNum][crystalNumber][2][0] * xx
                + Pars.crmat[holeNum][crystalNumber][2][1] * yy
                + Pars.crmat[holeNum][crystalNumber][2][2] * zz + Pars.crmat[holeNum][crystalNumber][2][3];

              /* apply xyzoffsets [are in cm] */

              gtinp->intpts[j].x += Pars.xyzoffset[holeNum * 4 + crystalNumber][0];
              gtinp->intpts[j].y += Pars.xyzoffset[holeNum * 4 + crystalNumber][1];
              gtinp->intpts[j].z += Pars.xyzoffset[holeNum * 4 + crystalNumber][2];


              if (Pars.nprint > 0)
                {
                  /* debug printout */

                  for (k = 0; k < 4; k++)
                    {
                      for (l = 0; l < 4; l++)
                        fprintf (stderr, "%8.4f ", Pars.crmat[holeNum][crystalNumber][k][l]);
                      fprintf (stderr, "\n");
                    };
                  fprintf (stderr, "world   coords: x,y,z=%9.4f,%9.4f,%9.4f\n", gtinp->intpts[j].x, gtinp->intpts[j].y,
                           gtinp->intpts[j].z);
                }
            };

        };

      /* next */

      ptinp++;
      ptgd++;

    };

  /* done */

  return (0);
}

/*------------------------------------------------------------------*/


int
findGlobalHitPositions_AGATA (TRACK_STRUCT * track, STAT * ctkStat)
{
  /* use the rmatrices to put the hits specified by the */
  /* decomposition into the actual positions in the */
  /* GRETINA shell based on holeNum number and crystal type */

  /* declarations */

  int i, j, holeNum, crystalNumber, k, l, crystalID;
  float xx, yy, zz;
  float r1, r2;
  PAYLOAD *ptinp;
  GEBDATA *ptgd;
  CRYS_INTPTS *gtinp;

  /* rotate and translate */

  ptinp = track->payload;
  ptgd = track->gd;
  for (i = 0; i < track->n; i++)
    {
      if (ptgd->type == GEB_TYPE_DECOMP)
        {

          gtinp = (CRYS_INTPTS *) ptinp;

          /* common for these hits */

          holeNum = ((gtinp->crystal_id & 0xfffc) >> 2);
          crystalNumber = (gtinp->crystal_id & 0x0003);
          crystalID = holeNum * 3 + crystalNumber;


          if (crystalID < 0 || crystalID > MAXAGID)
            return (BADCRYSTID);

          if (Pars.nprint > 0)
            {
              fprintf (stderr, "\n");
              fprintf (stderr, "entered:: findGlobalHitPositions_AGATA at event # %i\n", ctkStat->nTrackCalled);
              fprintf (stderr, "holeNum =%i ; ", holeNum);
              fprintf (stderr, "[1]crystalNumber =%i, --> detno: %i\n", crystalNumber, crystalID);
            };

          if (Pars.nprint > 0)
            {
              fprintf (stderr, "** %2i:\n", crystalID);
              fprintf (stderr, "AG::Tr?: %10.5f %10.5f %10.5f\n", Pars.TrX[crystalID], Pars.TrX[crystalID],
                       Pars.TrZ[crystalID]);
              fprintf (stderr, "AG::rx?: %10.5f %10.5f %10.5f\n", Pars.rotxx[crystalID], Pars.rotxy[crystalID],
                       Pars.rotxz[crystalID]);
              fprintf (stderr, "AG::ry?: %10.5f %10.5f %10.5f\n", Pars.rotyx[crystalID], Pars.rotyy[crystalID],
                       Pars.rotyz[crystalID]);
              fprintf (stderr, "AG::rz?: %10.5f %10.5f %10.5f\n", Pars.rotzx[crystalID], Pars.rotzy[crystalID],
                       Pars.rotzz[crystalID]);
            };

          /* rotate/translate the individual hits */

          for (j = 0; j < gtinp->num; j++)
            {

              xx = gtinp->intpts[j].x * 10;
              yy = gtinp->intpts[j].y * 10;
              zz = gtinp->intpts[j].z * 10;

              gtinp->intpts[j].x =
                Pars.rotxx[crystalID] * xx
                + Pars.rotxy[crystalID] * yy + Pars.rotxz[crystalID] * zz + Pars.TrX[crystalID];
              gtinp->intpts[j].y =
                Pars.rotyx[crystalID] * xx
                + Pars.rotyy[crystalID] * yy + Pars.rotyz[crystalID] * zz + Pars.TrY[crystalID];;
              gtinp->intpts[j].z =
                Pars.rotzx[crystalID] * xx
                + Pars.rotzy[crystalID] * yy + Pars.rotzz[crystalID] * zz + Pars.TrZ[crystalID];;

              gtinp->intpts[j].x /= 10;
              gtinp->intpts[j].y /= 10;
              gtinp->intpts[j].z /= 10;

              /* apply y-offsets [are in cm] */

              gtinp->intpts[j].x += Pars.xyzoffset[crystalID][0];
              gtinp->intpts[j].y += Pars.xyzoffset[crystalID][1];
              gtinp->intpts[j].z += Pars.xyzoffset[crystalID][2];

              if (Pars.nprint > 0)
                {
                  fprintf (stderr, "AG::x: %9.2f --> %9.2f\n", xx, gtinp->intpts[j].x);
                  fprintf (stderr, "AG::y: %9.2f --> %9.2f\n", yy, gtinp->intpts[j].y);
                  fprintf (stderr, "AG::z: %9.2f --> %9.2f\n", zz, gtinp->intpts[j].z);
                  r1 = xx * xx + yy * yy + zz * zz;
                  r1 = sqrtf (r1);
                  r2 = gtinp->intpts[j].x * gtinp->intpts[j].x
                    + gtinp->intpts[j].y * gtinp->intpts[j].y + gtinp->intpts[j].z * gtinp->intpts[j].z;
                  r2 = sqrtf (r2);
                  fprintf (stderr, "AG::radius %f --> %f\n", r1, r2);
                }

            };
        };

      /* next */

      ptinp++;
      ptgd++;

    };

  /* done */

//  if (Pars.nprint > 3)exit(0);
  return (0);
}
