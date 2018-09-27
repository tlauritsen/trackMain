
#include <stdlib.h>
#include <stdio.h>

#include "ctk.h"

int
ctksort (int iCluster, CLUSTER_INTPTS Clstr[MAXCLUSTERHITS], int *nClusters)
{
  /* declarations */

  int i, j;
  float tmp;


  /* Geant simulations show that one the */
  /* average we loose half the energy for */
  /* every interaction and that the Photo */
  /* absorption mostly happen at an energy of */
  /* 200 keV.  As a matter of fact, of the */
  /* last interaction energy is not near 200 */
  /* keV the sequence is probably wrong. */

  /* bubble sort on segment energies */

  for (i = 0; i < Clstr[iCluster].ndet; i++)
    for (j = i + 1; j < Clstr[iCluster].ndet; j++)
      if (Clstr[iCluster].intpts[i].edet < Clstr[iCluster].intpts[j].edet)
        {
          tmp = Clstr[iCluster].intpts[i].xx;
          Clstr[iCluster].intpts[i].xx = Clstr[iCluster].intpts[j].xx;
          Clstr[iCluster].intpts[j].xx = tmp;

          tmp = Clstr[iCluster].intpts[i].yy;
          Clstr[iCluster].intpts[i].yy = Clstr[iCluster].intpts[j].yy;
          Clstr[iCluster].intpts[j].yy = tmp;

          tmp = Clstr[iCluster].intpts[i].zz;
          Clstr[iCluster].intpts[i].zz = Clstr[iCluster].intpts[j].zz;
          Clstr[iCluster].intpts[j].zz = tmp;

          tmp = Clstr[iCluster].intpts[i].edet;
          Clstr[iCluster].intpts[i].edet = Clstr[iCluster].intpts[j].edet;
          Clstr[iCluster].intpts[j].edet = tmp;

#if SIMULATED
          tmp = Clstr[iCluster].intpts[i].origPos;
          Clstr[iCluster].intpts[i].origPos = Clstr[iCluster].intpts[j].origPos;
          Clstr[iCluster].intpts[j].origPos = tmp;
#endif

          tmp = Clstr[iCluster].intpts[i].shellHitPos;
          Clstr[iCluster].intpts[i].shellHitPos = Clstr[iCluster].intpts[j].shellHitPos;
          Clstr[iCluster].intpts[j].shellHitPos = tmp;

          tmp = Clstr[iCluster].intpts[i].order;
          Clstr[iCluster].intpts[i].order = Clstr[iCluster].intpts[j].order;
          Clstr[iCluster].intpts[j].order = tmp;

        };

  /* done */

  return (0);

}
