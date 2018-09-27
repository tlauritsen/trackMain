
#include <stdlib.h>
#include <stdio.h>

#include "ctk.h"

volatile extern TRACKINGPAR Pars;

int
ctk_pr_trackPar (void)
{

  /* declarations */

#if(0)

  /* list all parameters */

  fprintf (stderr,"\n");
  fprintf (stderr,"--------------------------\n");
  fprintf (stderr,"tracking parameters (incomplete list):\n");
  fprintf (stderr,"\n");

  fprintf (stderr,"alpha=   %f\n", Pars.alpha);
  fprintf (stderr,"data=   %s\n", Pars.data);
  fprintf (stderr,"gamma=  %f\n", Pars.gamma);


  fprintf (stderr,"fomJump= %f\n", Pars.fomJump);
  fprintf (stderr,"fomGoodenough= %f\n", Pars.fomGoodenough);


  fprintf (stderr,"nprint=  %i\n", Pars.nprint);

  fprintf (stderr,"--------------------------\n");
  fprintf (stderr,"\n");
#endif

  return (0);

}
