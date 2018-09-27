
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

#include "ctk.h"
#include "gdecomp.h"

#define DEBUG 0

/* globals */

volatile extern TRACKINGPAR Pars;


/*----------------------------------------------------------------------------*/

int
writeTrack_repeat (TRACK_STRUCT * track)
{

  /* this function first writes the original data to the */
  /* output data sream, then it writes the tracked data. */
  /* this function is for offline use. online the  serializeTrack */
  /* is used */

  /* declarations */

  static int nwritten = 0;
  int siz, iCluster, i1, ng, nmTS;
  float rn, rn1;
  unsigned int grnum = 0;
  char str[128];
  long long int mTS[1000];
  int wo[1000];
#if(DEBUG)
  int nobytes = 0;
#endif
  PAYLOAD *ptinp;
  GEBDATA *ptgd;
  CRYS_INTPTS *gtinp;

  GEBDATA trackGeb;
  char *payLoad;
  TRACKED_GAMMA_RAY tgr;

  long long int dTS;
  static long long int oldTS;
  static int nn = 0;
  int checksum, *iptr;

  int i, j;


  if (Pars.trackDataStream > 0)
    {

      /*-------------------------*/
      /* write the original data */
      /*-------------------------*/


      /* simple echo of input data to the tracking task. */
      /* It may contain external data for all we know */
      /* o we will not echo any old tracked data */
      /* o we do not pass on the multiplicity information */

      ptgd = track->gd;
      ptinp = track->payload;
      for (i = 0; i < track->n; i++)
        {

          /* only repeat non tracked data */

          if (ptgd->type != GEB_TYPE_TRACK)
            {
            assert(Pars.trackDataStream!=NULL);
//            siz = write (Pars.trackDataStream, (char *) ptgd, sizeof (GEBDATA));
            siz = fwrite((char *) ptgd,1,sizeof (GEBDATA),Pars.trackDataStream);
//            printf("wrote gebheader, length=%i, %i\n",sizeof (GEBDATA) , siz); fflush(stdout);
//            assert (siz == sizeof (GEBDATA));
//            siz = write (Pars.trackDataStream, (char *) ptinp, ptgd->length);
            siz = fwrite ((char *) ptinp, 1, ptgd->length, Pars.trackDataStream);
//            printf("wrote data of type %i, length=%i, %i\n", ptgd->type, ptgd->length, siz); fflush(stdout);
            assert (siz == ptgd->length);
            };

          ptgd++;
          ptinp++;
        };
//      fprintf (stderr,"done\n");
//      fflush (stdout);



//      fprintf (stderr,"wrote event # %i out\n", nwritten);
//      nwritten++;

      return (0);

    };

  return (0);
}

/*----------------------------------------------------------------------------*/



















































































#if(0)


/*--------------------------------------------------------*/
/* Here is Carls new  CRYS_INTPTS *gtinp;
 fancy data output routine. Even..... */
/* though I don't use it directly, we keep it here so.... */
/* it follows my updates. Wed 12 Jan 2011 02:43:28 PM CST */
/*--------------------------------------------------------*/

/* this function is out of date!!!!! */

void *
serializeTrack (TRACK_STRUCT * track, CLUSTER_INTPTS Clstr[MAXCLUSTERHITS], int *nClusters, int *bufferlen)
{

  /* this produces an output buffer identical to what writeTrack would write */

  /* declarations */

  int iCluster, i1, nValidClusters, ninteractions;
  char *retval, *bptr;
  int retsize = 0;
  CRYS_INTPTS *ptinp;
  GEBDATA *ptgd;
  int i, j;


  retsize = track->n * (sizeof (CRYS_INTPTS) + sizeof (GEBDATA));
  retsize += sizeof (int);


  /* find number of valid clusters */

  nValidClusters = 0;
  for (iCluster = 0; iCluster < *nClusters; iCluster++)
    if (Clstr[iCluster].valid)
      nValidClusters++;

  /* calculate how much buffer space we need */
  /* for the clusters */

  retsize = retsize + nValidClusters * sizeof (CLUSTER_INTPTS) + sizeof (int);

  /* and allocate space */

  retval = (char *) malloc (retsize);
  if (!retval)
    {
      *bufferlen = 0;
      return 0;
    }
  bptr = retval;



  /*----------*/
  /* raw data */
  /*----------*/

  /* first write multiplicity of event */

  memcpy (bptr, (char *) &track->n, sizeof (int));
  bptr += sizeof (int);

  /* then write the rest of the event */


  ptgd = track->gd;
  ptinp = track->inp;
  for (i = 0; i < track->n; i++)
    {
      memcpy (bptr, (char *) ptgd, sizeof (GEBDATA));
      bptr += sizeof (GEBDATA);
      memcpy (bptr, (char *) ptinp, ptgd->length);
      bptr += ptgd->length;
      ptgd++;
      ptinp++;
    };



  /*--------------*/
  /* tracked data */
  /*--------------*/


  /* first write multiplicity of event */
  /* we of course only write valid clusters */
  /* out, skipping discarded ones */

  /* nValidClusters already determined above */

  memcpy (bptr, (char *) &nValidClusters, sizeof (int));
  bptr += sizeof (int);

  /* now write the data */

  for (iCluster = 0; iCluster < *nClusters; iCluster++)
    if (Clstr[iCluster].valid)
      {
        memcpy (bptr, (char *) &Clstr[iCluster], sizeof (CLUSTER_INTPTS));
        bptr += sizeof (CLUSTER_INTPTS);
      }


  *bufferlen = retsize;
  return retval;
}

#endif
