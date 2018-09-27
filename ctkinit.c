
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#ifdef SOLARIS
#include <strings.h>
#endif
#ifdef LINUX
#include <string.h>
#endif

#include "ctk.h"

volatile extern TRACKINGPAR Pars;

int
ctkinit (STAT * ctkStat)
{
  /* declarations */

  int i, j, i1, i2, i3, i4, i5, i6, i7, i8;
  int ndet, nn, k;
  FILE *fp;

  Pars.badTheoAngPenalty=0.0;
  Pars.nodupmode2=0;
  Pars.min_radius=0;
  Pars.max_radius=50*50;

  for (i = 0; i < MAXDETNO; i++)
    Pars.dirk_rcf[i] = 1.0;

  for (i = 0; i < MAXDETNO; i++)
    {
    Pars.CCcal_gain[i]=1.0;
    Pars.CCcal_offset[i]=0.0;
    }

  for (i=0;i<=MAXDETNO; i++)
    for (j=0;j<=MAXCRYSTALNO;j++)
      {
      Pars.SEGcal_gain[i][j]=1.0;
      Pars.SEGcal_offset[i][j]=0.0;
      };

  for (i = 0; i < MAXNOSEG; i++)
    {
      Pars.ndetElim_lo[i] = 0;
      Pars.ndetElim_hi[i] = 1000.0;
    };

    

  /* precalculate # permutations possible */

  for (j = 1; j < MAXNOSEG; j++)
    {
      i1 = 1;
      for (i = 1; i <= j; i++)
        i1 *= i;
      Pars.nperm[j] = i1;
    };
  for (j = 1; j < MAXNOSEG; j++)
    fprintf (stderr,"ndet=%2i, # permutations= %10i\n", j, Pars.nperm[j]);


  /* ------------------------------- */
  /* make permutation lookup tables */
  /* ------------------------------- */

  /* calculate the permutation lookup table */
  /* lexicographic ordered ; haven't found an */
  /* elegant way of doing this, but since we */
  /* only do it once, it does not matter */

  /* we may want to order the lookups acoording to */
  /* likeliness at some point */

  /* 1: ndet, selects the order */
  /* 2: nperm, permutation number */
  /* 3: actual permutation for this permutation number */


#include "permtable1.h"
#include "permtable2.h"
#include "permtable3.h"
#include "permtable4.h"
#include "permtable5.h"
#include "permtable6.h"
#include "permtable7.h"
#include "permtable8.h"
  /* #include "permtable9.h" too much, lookup blows up */





  /* --------------------------------- */
  /* print permutation lookup tables.. */
  /* --------------------------------- */

  fp = fopen ("permutation.table", "w");
  fprintf (fp, "ctkinit");
  fprintf (fp, " ");
  fprintf (fp, "list of generated permutation tables ");
  fprintf (fp, " ");

  for (ndet = 1; ndet < 8; ndet++)
    {
      fprintf (fp, "\n\nOrder, ndet= %i -------------\n\n", ndet);
      for (j = 0; j < Pars.nperm[ndet]; j++)
        {
          fprintf (fp, "{%2i,%4i}:: ", ndet, j + 1);
          for (k = 0; k < ndet; k++)
            fprintf (fp, "%i ", Pars.permlkup[ndet][j][k]);
          fprintf (fp, "\n");
        };
    };

  fprintf (fp, " ");
  fprintf (fp, "... done");
  fprintf (fp, " ");
  fprintf (stderr,"wrote \"permutation.table\"\n");

  for (i=0;i<1000;i++)
    for (j=0;j<8;j++)
      ctkStat->sp_max_ang[i][j]=0;

  return (0);

}
