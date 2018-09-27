
/* main clustering and tracking program */
/* ~simulating Carl's event builder */

#include <stdlib.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/resource.h>
#include <sys/times.h>
#include <assert.h>

#include "ctk.h"
#define DEBUGGETEVENT 0

TRACKINGPAR Pars;

/*--------------------------------------------------*/

/*-----------------------------------------------------------*/

int
find_target_pos (float target_pos[3])
{

  /* declarations */

  int i;

  /* find target position from external data */
  /* [for not just use 0,0,0] */

  for (i = 0; i < 3; i++)
    target_pos[i] = 0;

  /* add the offsets from chatfile */

  target_pos[0] -= Pars.target_x;
  target_pos[1] -= Pars.target_y;
  target_pos[2] -= Pars.target_z;


  /* done */

  return (0);

}

/*-----------------------------------------------------------*/
int
main (int argc, char **argv)
{

  /* declarations */

  FILE *inData;
  int ncalls = 0, haveTrack = 0, trackStatus = 1, i, i1, j;
  int returnCodes[1000];
  int ndecomp;
  char *p;

  /* the input track structure */

  TRACK_STRUCT track;

  /* collection of former global variables */
  /* which we now keep in the stack so */
  /* we can run independent threads in the cores */

  struct tms timesThen;
  STAT ctkStat;
  SHELLHIT shellhit;
  int nClusters;
  CLUSTER_INTPTS Clstr[MAXCLUSTERHITS];
  float target_pos[3];

  /*------*/
  /* help */
  /*------*/

  if (argc != 3 && argc != 4)
    {
      fprintf (stderr, "use: %s chatfile datafile [outputfile]\n", argv[0]);
      fprintf (stderr, "\n");
      fprintf (stderr, "almost everything that has to do\n");
      fprintf (stderr, "with specifying parameters for how\n");
      fprintf (stderr, "to do the tracking is specified in the\n");
      fprintf (stderr, "chat file. This file is ~selfdocumenting.\n");
      fprintf (stderr, "\n");

      exit (0);
    };

  /*------------*/
  /* initialize */
  /*------------*/

  fprintf (stderr, "started tracking at ");
  time_stamp (stderr);

  for (i = 0; i < 1000; i++)
    returnCodes[i] = 0;

  /* allocate space for track structure */

  track.gd = (GEBDATA *) calloc (MAXTRACK, sizeof (GEBDATA));
  track.payload = (PAYLOAD *) calloc (MAXTRACK, sizeof (PAYLOAD));

  memset ((void *) &ctkStat, 0, sizeof (STAT));

  /* open data file */

  /*                        + name of data file */
  /*                        |                   */

  if ((p = strstr (argv[2], "STDIN")) != NULL)
    inData = stdin;
  else
#if(ISMAC==1) 
    inData = fopen ((char *) argv[2], "r");
#else
    inData = fopen64 ((char *) argv[2], "r");
#endif
  if (inData == NULL)
    {
      fprintf (stderr, "could not open input data file %s, quit!\n", argv[2]);
      exit (1);
    };
  fprintf (stderr, "input data file %s is open\n", argv[2]);
//  printf("inData=0x%x\n", inData);

//  Pars.trackDataStream = -1;
  if (argc == 4)
    {
      /* open the output file */

//      Pars.trackDataStream = open ((char *) argv[3], O_WRONLY | O_CREAT | O_TRUNC, PMODE);

      if ((p = strstr (argv[3], "STDOUT")) != NULL)
        Pars.trackDataStream = stdout;
      else
#if(ISMAC==1) 
        Pars.trackDataStream = fopen ((char *) argv[3], "w");
#else
        Pars.trackDataStream = fopen64 ((char *) argv[3], "w");
#endif

      if (Pars.trackDataStream == NULL)
        {
          fprintf (stderr, "could not open output file %s\n", argv[3]);
          return (1);
        }
      else
        fprintf (stderr, "tracked output file \"%s\" is open\n", argv[3]);
    }


  /* do all necessary initializations */
  /* for the clustering/tracking code */

  setupTrack (&timesThen, &ctkStat, &shellhit);

  /* read chat script for tracking parameters */
  /* this input format could be changed in the future */
  /* but for now this is how we input the */
  /* parameters we need for tracking */
  /* setupTrack must be called first */
  /*              + name of chat file */
  /*              |                   */
  readChatFile (argv[1]);

  /* read in the rotational/translational matrices */

  setupTrack_crmat (&timesThen, &ctkStat, &shellhit);

  /* for debug purpuses only:: here we */
  /* skip through a number of events */
  /* to get to an interesting one in */
  /* a hurry; not for production version */

#if(0)
  i1 = 109320;
  Pars.nprint = 0;
  for (i = 0; i < i1; i++)
    getEvent_mode2 (&track, &ctkStat);
  fprintf (stderr, "trackMain: skipped %i events\n", i1);
  Pars.nprint = 100;
#endif

  /*---------------------------------------------*/
  /* loop: get event, cluster&track, write event */
  /*---------------------------------------------*/

  haveTrack = 0;
  fprintf (stderr, "start...\n");
  fflush (stdout);
  while (haveTrack == 0 && ctkStat.nTrackCalled <= Pars.maxevents)
    {

#if(1)
      /* zero out (should not be necessary) */

      for (j = 0; j < MAXCLUSTERHITS; j++)
        memset ((void *) &Clstr[j], 0, sizeof (CLUSTER_INTPTS));
#endif

      /* get an event to pass on */
      /* to the tracking code */

#if(0)
      haveTrack = getEvent (inData, &track);
#else
      haveTrack = getEvent_mode2 (inData, &track, &ctkStat);
#endif


//        printf("haveTrack = %i\n", haveTrack);

//      trapbad ("trackMain",&track);

      if (haveTrack == 0)
        {
          ncalls++;
//          fprintf (stderr,"trackMain: ncalls %i\n", ncalls);
//          fflush (stdout);
          find_target_pos (target_pos);

          /* echo the original data to the output */
          /* unmolested by the tracking code, with */
          /* original units and all. This is the clean */
          /* way to do this */

          if (!Pars.nodupmode2)
            writeTrack_repeat (&track);

          /* send it to the tracking code */

//          fprintf (stderr,"trackMain: trackEvent\n");
//          fflush (stdout);

          trackStatus = trackEvent (target_pos, &track, &ctkStat, &shellhit, Clstr, &nClusters);

//          printf("trackStatus = %i\n", trackStatus);
//          fprintf (stderr,"ev# %i: ", ctkStat.nEvents);
//          fflush (stdout);

          returnCodes[trackStatus]++;

          /* log every so often where we are */

          if (ncalls % 100000 == 0)
            {
              fprintf (stderr, "trackMain: ncalls= %i\n", ncalls);
              fflush (stdout);
            };

          /* add the tracked data to the data stream */

          writeTrack_addtrack (trackStatus, &track, Clstr, &nClusters, &ctkStat);

          if (Pars.nprint > 0 && trackStatus > 0)
            {
              fprintf (stderr, "trackEvent failed at ev# %i: ", ctkStat.nTrackCalled);
              switch (trackStatus)
                {
                case NOHITS:
                  fprintf (stderr, "returned: no ge hits in event\n");
                  break;
                case MODULENOOUTOFRANGE:
                  fprintf (stderr, "returned: Detector number out of range\n");
                  break;
                case CRYSTALNOOUTOFRANGE:
                  fprintf (stderr, "returned: crystal number out of range\n");
                  break;
                case TOOMANYHITS:
                  fprintf (stderr, "returned: too many hits\n");
                  break;
                case BADPAD:
                  fprintf (stderr, "returned: bad decomp PAD value\n");
                  break;
                default:
                  fprintf (stderr, "returned: unknown error = %i\n", trackStatus);
                  break;
                };
              fflush (stderr);
            };

        };

    };

done:

  fprintf (stderr, "\n");
  fprintf (stderr, "read %i hits\n", ncalls);
  fprintf (stderr, "\n");


  /* tracking statistics and closeout */

  ctkStats (&timesThen, &ctkStat);

  system ("ulimit -a | grep stack > /dev/stderr");


  fclose (Pars.trackDataStream);

  fprintf (stderr, "done...\n");

  for (i = 0; i < 1000; i++)
    if (returnCodes[i] > 0)
      fprintf (stderr, "returnCodes %3i: %i\n", i, returnCodes[i]);

  fprintf (stderr, "tracking done at ");
  time_stamp (stderr);

  /* done */

  fflush (stderr);
  fflush (stdout);

  exit (0);

}
