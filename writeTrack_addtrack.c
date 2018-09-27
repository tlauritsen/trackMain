
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
writeTrack_addtrack (int trackStatus, TRACK_STRUCT * track, CLUSTER_INTPTS Clstr[MAXCLUSTERHITS], int *nClusters,
                     STAT * ctkStat)
{

  /* this function writes the tracked data. */
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

//  printf("entered writeTrack_addtrack\n");

  /* write the tracked data to tracked data stream */

  /* note: the tracked data come in an array with *nClusters elements. */
  /* However, not all of them are necessarely valid (the gamma rays */
  /* could have be split or combined). Thus, the Clstr[iCluster].valid */
  /* flag has to always be inspected before we write anything out */

  if (Pars.trackDataStream > 0)
    {

      /* assign mean gamma ray time stamps */
      /* and check things */

      ng = 0;
//      fprintf (stderr,"\n\n---------\n");


      for (iCluster = 0; iCluster < *nClusters; iCluster++)
        {
          /* per default we do not write it out. */
          /* The clusters have to qualify first */

          wo[iCluster] = 0;
          mTS[iCluster] = (long long int) 0;
          nmTS = 0;

          /* now loop and find good gamma rays */

          if (Clstr[iCluster].valid)
            {

              /* keep record of how many raw gamma arays */
              /* we actually have for writeout */

              ctkStat->gammasForWriteout++;

              if (Clstr[iCluster].tracked && Clstr[iCluster].ndet > 0 && Clstr[iCluster].ndet < MAX_NDET)
                {

                  ctkStat->gammasForWriteout_OK1++;

                  /* find the mean time for this gamma ray */

                  for (j = 0; j < Clstr[iCluster].ndet; j++)
                    {
                      mTS[iCluster] += Clstr[iCluster].intpts[j].timestamp;
                      nmTS++;
//                  printf("Clstr[%i].intpts[%i].timestamp: %lli\n",iCluster,j,Clstr[iCluster].intpts[j].timestamp);
                    };
                  if (nmTS > 1)
                    mTS[iCluster] /= (long long int) nmTS;

                  /* mark for writeout if it qualifies */

                  if (nmTS > 0 && mTS[iCluster] > (long long int) 0)
                    {
                      wo[iCluster] = 1;
                      ng++;
                      ctkStat->gammasForWriteout_OK2++;
                    };
//              fprintf (stderr,"nmTS=%i, mTS[iCluster=%i]=%lli,ng=%i, wo=%i\n", nmTS, iCluster, mTS[iCluster], ng, wo[iCluster]);
                };
            };
        };


      /* if there was a problem with the tracking */
      /* we suppress the writeout of tracked data */

      if (trackStatus != 0)
        ng = 0;

      /* write tracked data out */

      if (ng > 0)
        {

          /*--------------------*/
          /* write tracked data */
          /*--------------------*/

          trackGeb.type = GEB_TYPE_TRACK;
          trackGeb.length = 0;
          trackGeb.timestamp = 0;

          /* use the average time stamp for */
          /* the GEB header */

          for (iCluster = 0; iCluster < *nClusters; iCluster++)
            if (wo[iCluster])
              trackGeb.timestamp += mTS[iCluster];
          trackGeb.timestamp /= (long long int) ng;
//        fprintf (stderr,"--> trackGeb.timestamp=%lli\n", trackGeb.timestamp);

          /* first write multiplicity of event */
          /* we of course only write valid clusters */
          /* out, skipping discarded ones */

          /* allocate space for payLoad */

          payLoad = (char *) calloc (1, ng * sizeof (TRACKED_GAMMA_RAY) + 2 * sizeof (int));
          assert (payLoad != NULL);

          /* write number of gamma rays following */

          memcpy (payLoad + trackGeb.length, (char *) &ng, sizeof (int));
          trackGeb.length += sizeof (int);

          /* pad with extra word for 64 bit machines, make it a serial # just because */

          memcpy (payLoad + trackGeb.length, (char *) &grnum, sizeof (int));
          trackGeb.length += sizeof (int);
          grnum++;

          /* now write the gamma rays in units of mm and keV */

          for (iCluster = 0; iCluster < *nClusters; iCluster++)
            if (wo[iCluster])
              {

                bzero ((char *) &tgr, sizeof (TRACKED_GAMMA_RAY));

                tgr.esum = Clstr[iCluster].esum * 1000;
                tgr.ndet = Clstr[iCluster].ndet;
                tgr.fom = Clstr[iCluster].fom;
                tgr.tracked = Clstr[iCluster].tracked;



#if(0)
                fprintf (stderr,"sizeof (TRACKED_GAMMA_RAY)=%i bytes\n", sizeof (TRACKED_GAMMA_RAY));
                fprintf (stderr,"sizeof(int)=%i\n", sizeof (int));
                fprintf (stderr,"sizeof(float)=%i\n", sizeof (float));
                fprintf (stderr,"sizeof(long long int)=%i\n", sizeof (long long int));
                fprintf (stderr,"sizeof(char)=%i\n", sizeof (char));
                fprintf (stderr,"tgr.esum=%f ;", tgr.esum);
                fprintf (stderr,"tgr.ndet=%i;", tgr.ndet);
                fprintf (stderr,"tgr.fom=%f ;", tgr.fom);
                fprintf (stderr,"tgr.tracked=%i ;", tgr.tracked);
                fprintf (stderr,"\n");
#endif

                /* find and extract first and second interaction points */
                /* write in mm and keV from cm and keV */

                for (j = 0; j < Clstr[iCluster].ndet; j++)
                  {

                    if (Clstr[iCluster].intpts[j].order == 0)
                      {

                        tgr.x0 = Clstr[iCluster].intpts[j].xx * 10;
                        tgr.y0 = Clstr[iCluster].intpts[j].yy * 10;
                        tgr.z0 = Clstr[iCluster].intpts[j].zz * 10;
                        tgr.e0 = Clstr[iCluster].intpts[j].edet * 1000;
                        tgr.timestamp = Clstr[iCluster].intpts[j].timestamp;
#if (TRACK2==1)
                        tgr.fhcrID=Clstr[iCluster].intpts[j].detno;
#endif

                      }

                    if (Clstr[iCluster].intpts[j].order == 1)
                      {
                        tgr.x1 = Clstr[iCluster].intpts[j].xx * 10;
                        tgr.y1 = Clstr[iCluster].intpts[j].yy * 10;
                        tgr.z1 = Clstr[iCluster].intpts[j].zz * 10;
                        tgr.e1 = Clstr[iCluster].intpts[j].edet * 1000;
                      }

                  };

                /* store in payload */

                memcpy (payLoad + trackGeb.length, (char *) &tgr, sizeof (TRACKED_GAMMA_RAY));
                trackGeb.length += sizeof (TRACKED_GAMMA_RAY);

                /* keep record of how many gamma arays */
                /* we actually write out */

                ctkStat->gammasWritten++;

              };


          /* write to disk */

//          siz = write (Pars.trackDataStream, (char *) &trackGeb, sizeof (GEBDATA));
          siz = fwrite ((char *) &trackGeb, 1, sizeof (GEBDATA),Pars.trackDataStream);
//        printf("wrote type %i data, length=%i; ", trackGeb.type, trackGeb.length);;

//          siz = write (Pars.trackDataStream, (char *) payLoad, trackGeb.length);
          siz = fwrite((char *) payLoad, 1, trackGeb.length, Pars.trackDataStream);
//        printf("actual siz=%i\n",siz);

          ctkStat->trackedEventsWritten++;

          free (payLoad);

        };

//      fprintf (stderr,"wrote event # %i out\n", nwritten);
//      nwritten++;

      return (0);

    };

  return (0);
}






























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
