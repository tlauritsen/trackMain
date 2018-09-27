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

extern off_t inData;            /* inpugetEvent.ct file */
TRACKINGPAR Pars;

/*----------------------------------------------------------------------------*/
#if(0)

int
getEvent (FILE *inData, TRACK_STRUCT * track)
{

  /* temporarely get/read an event from a file rather */
  /* than getting it from Carl's global event builder */

  /* declarations */

  static int nread = 0;
  int siz, i1, i, j, k1;
  int ii;
  PAYLOAD *ptinp;
  GEBDATA *ptgd;
  int checksum, *iptr;
  static int nchecksum = 0;
  char str[128];

#if (DEBUGGETEVENT)
  fprintf (stderr,"getEvent called\n");
  fflush (stdout);
#endif

  /* read the time coincidence event length; final format like this */

#if (DEBUGGETEVENT)
  fprintf (stderr,"inData=0x%x\n", inData);
#endif
  siz = read (inData, (char *) &track->n, sizeof (int));
  if (siz != sizeof (int))
    {
      fprintf (stderr,"getEvent: return 1, siz=%i\n", siz);
      return (1);
    };
#if (DEBUGGETEVENT)
  fprintf (stderr,"track->n=%i\n", track->n);
  fflush (stdout);
#endif

  ptgd = track->gd;
  ptinp = track->payload;
#if (DEBUGGETEVENT)
  fprintf (stderr,"ptgd=%p,ptinp=%p\n", ptgd, ptinp);
#endif

  for (i = 0; i < track->n; i++)
    {

      /* now read the data container info */

      siz = read (inData, (char *) ptgd, sizeof (GEBDATA));
#if (DEBUGGETEVENT)
      fprintf (stderr,"siz=%i\n", siz);
      fflush (stdout);
      fprintf (stderr,"ptgd->length=%i\n", ptgd->length);
      fprintf (stderr,"ptgd->type=%i\n", ptgd->type);
      fprintf (stderr,"ptgd->timestamp=%lli\n", ptgd->timestamp);
#endif


      /* sanity check */

      if (ptgd->type < 1 || ptgd->type > MAX_GEB_TYPE)
        {
          fprintf (stderr,"\nptgd->length=%i\n", ptgd->length);
          fprintf (stderr,"ptgd->type=%i\n", ptgd->type);
          fprintf (stderr,"ptgd->timestamp=%lli\n", ptgd->timestamp);
          fprintf (stderr,"....geb type = %i is outside range of 1 to %i\n", ptgd->type, MAX_GEB_TYPE);
          fprintf (stderr,"....maybe you are not reading mode 2 or mode 1 data?, return code 3\n");
          return (3);
        };

      if (siz != sizeof (GEBDATA))
        {
          fprintf (stderr,"cannot read GEBDATA, got siz=%i, not %i as expected\n", siz, sizeof (GEBDATA));
          return (2);
        };

      /* now read the real data */

      memset ((void *) ptinp, 0, ptgd->length);
      siz = read (inData, (char *) ptinp, ptgd->length);
#if (DEBUGGETEVENT)
      fprintf (stderr,"siz=%i\n", siz);
      fflush (stdout);
#endif
      if (siz != ptgd->length)
        {
          fprintf (stderr,"getEvent: return 2\n");
          return (2);
        }


      ptgd++;
      ptinp++;

    };

//  fprintf (stderr,"read event # %i in\n", nread);
//  nread++;
#if (DEBUGGETEVENT)
  printEvent (stdout, 0, track);
#endif

//trapbad ("getEvent",track);

  /* done */

  return (0);

}
#endif

/*----------------------------------------------------------------------------*/

int
getEvent_mode2 (FILE *inData, TRACK_STRUCT * track, STAT * ctkStat)
{

/* declarations */

  int siz;
  static int firsttime = 1, ncall = 0;;
  long long int curTS, dTS;
  PAYLOAD *ptinp;
  GEBDATA *ptgd;

  static GEBDATA tmp_gd;
  static PAYLOAD tmp_payload;

  if (firsttime)
    {

      fprintf (stderr,"getEvent_mode2: initial read, ncall=%i\n", ncall);
      fflush (stdout);

      firsttime = 0;

      /* first time, need to prime the system */

//      siz = read (inData, (char *) &tmp_gd, sizeof (GEBDATA));
      siz = fread((char *) &tmp_gd, 1, sizeof (GEBDATA), inData);
      if (siz != sizeof (GEBDATA))
        {
          fprintf (stderr,"getEvent_mode2 cannot read GEBDATA, got siz=%i, not %i as expected\n", siz, sizeof (GEBDATA));
          return (2);
        };
      ctkStat->numbytes+=siz;      
      fprintf (stderr,"getEvent_mode2: GEBheader TS=%lli, payload length=%i\n", tmp_gd.timestamp, tmp_gd.length);
      fflush (stdout);

//      memset ((void *) &tmp_payload, 0, tmp_gd.length);
//      siz = read (inData, (char *) &tmp_payload, tmp_gd.length);
      siz = fread((char *) &tmp_payload, 1, tmp_gd.length, inData);
      if (siz != tmp_gd.length)
        {
          fprintf (stderr,"getEvent_mode2: return 2, siz=%i but tmp_gd.length=%i\n",siz,tmp_gd.length);
          return (2);
        };
      ctkStat->numbytes+=siz;
      fprintf (stderr,"getEvent_mode2: we are primed: TS=%lli\n", tmp_gd.timestamp);
      fflush (stdout);
    };

  /* start with first member, which is */
  /* the leftover from last read */

#if (0)
  fprintf (stderr,"getEvent_mode2: tmp storage; ");
  fprintf (stderr,"length=%i; ", tmp_gd.length);
  fprintf (stderr,"type=%i; ", tmp_gd.type);
  fprintf (stderr,"timestamp=%lli\n", tmp_gd.timestamp);
  fflush (stdout);
#endif

  ptgd = track->gd;
  ptinp = track->payload;
  memcpy ((char *) ptgd, (char *) &tmp_gd, sizeof (GEBDATA));
  memcpy ((char *) ptinp, (char *) &tmp_payload, tmp_gd.length);
#if (0)
  fprintf (stderr,"getEvent_mode2: first load; ");
  fflush (stdout);
  fprintf (stderr,"ptgd->length=%i; ", ptgd->length);
  fprintf (stderr,"ptgd->type=%i; ", ptgd->type);
  fprintf (stderr,"ptgd->timestamp=%lli\n", ptgd->timestamp);
#endif
  track->n = 1;
  curTS = ptgd->timestamp;
  ncall++;

//  if (ncall>5) exit(0);

#if(1)
  if (Pars.nprint > 0)
    {
      fprintf (stderr,"getEvent_mode2: start TS=%lli\n", curTS);
      fflush (stdout);
    };
#endif

  /* next postion */

  ptgd++;
  ptinp++;

  while (1)
    {

      /* now read the next events in */

//      siz = read (inData, (char *) ptgd, sizeof (GEBDATA));
        siz=fread( (char *) ptgd, 1, sizeof (GEBDATA),inData);
#if (1)
      if (Pars.nprint > 0)
        {
          fprintf (stderr,"getEvent_mode2: siz=%i; ", siz);
          fprintf (stderr,"ptgd->length=%i; ", ptgd->length);
          fprintf (stderr,"ptgd->type=%i; ", ptgd->type);
          fprintf (stderr,"ptgd->timestamp=%lli\n", ptgd->timestamp);
          fflush (stdout);
        }
#endif
      ctkStat->numbytes+=siz;


      if (siz != sizeof (GEBDATA))
        {
          fprintf (stderr,"getEvent_mode2: cannot read GEBDATA, got siz=%i, not %i as expected\n", siz, sizeof (GEBDATA));
          return (2);
        };


      /* now read the real data */

#if(0)
      /* can't do anymore because we use fread and not read */

      if (ptgd->length>=MAXDATASIZE || ptgd->type>= MAX_GEB_TYPE)
        {
        fprintf (stderr,"getEvent_mode2: siz=%i; ", siz);
        fprintf (stderr,"ptgd->length=%i; ", ptgd->length);
        fprintf (stderr,"ptgd->type=%i; ", ptgd->type);
        fprintf (stderr,"ptgd->timestamp=%lli\n", ptgd->timestamp);
        fprintf (stderr,"nbytes=%lli, %f MB\n", ctkStat->numbytes,(double)ctkStat->numbytes/1024.0/1000.0 );
        fprintf (stderr,"size of payload= %i and MAXDATASIZE=%i\n",ptgd->length,MAXDATASIZE);
        fflush(stdout);
//        assert (ptgd->length < MAXDATASIZE);

        /* try to seek past bad payload */

        lseek(inData,ptgd->length,SEEK_CUR);
        printf("seeked forward %i bytes and return 0\n", ptgd->length);
        fflush(stdout);
        ctkStat->numbytes+=ptgd->length;
        return(0);

        };
#endif

      if (Pars.nprint > 0)
        {
          fprintf (stderr,"getEvent_mode2: ts=%lli, ptgd->length=%i, %p\n", ptgd->timestamp, ptgd->length, ptinp);
          fflush (stdout);
        };

//      siz = read (inData, (char *) ptinp, ptgd->length);
        siz = fread((char *) ptinp, 1, ptgd->length, inData);
      if (siz != ptgd->length)
        {
          fprintf (stderr,"getEvent_mode2:  return 2, siz=%i, ptgd->length=%i\n",siz, ptgd->length);
          return (2);
        };
      ctkStat->numbytes+=siz;

      /* delta TS */

      dTS = ptgd->timestamp - curTS;

      if (dTS < (long long int) 0)
        {

          /* this should not happen, but */
          /* it sometimes does, so we have */
          /* to deal with it, done below with '|| dTS < 0' */

          fprintf (stderr,"getEvent_mode2: dTS=%lli, evasive maneuvers at events no %i\n", dTS, ncall);

        };

      if (dTS > Pars.dtwin || dTS < 0)
        {

          /* we are done with this coincidence */

          /* either the just read in event is outside */
          /* the coincidence window, or we read */
          /* something in from the past. In either */
          /* case we terminate the current event, */
          /* and get ready for a new event. */

          if (Pars.nprint > 0)
            {
              fprintf (stderr,"getEvent_mode2: done with this coincidence curTS=%lli, track->n=%i\n", curTS, track->n);
              fflush (stdout);
            }

          /* transfer last read to the temporary holding area */
          /* ready to be used for the start of the next event */

          memcpy ((char *) &tmp_gd, (char *) ptgd, sizeof (GEBDATA));
          memcpy ((char *) &tmp_payload, (char *) ptinp, ptgd->length);

          /* break out */

          return (0);

        }
      else
        {

          /* we need to read the next one */

          if (Pars.nprint > 0)
            {
              fprintf (stderr,"getEvent_mode2: get ready to read the next,curTS=%lli, track->n=%i\n", curTS, track->n);
              fflush (stdout);
            }

          track->n++;
          ptgd++;
          ptinp++;


        };

    };



}
