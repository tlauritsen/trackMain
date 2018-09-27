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

#include "ag2mode2.h"

FILE *fp;

/*-----------------------------------------------------------------------------*/

type_identity (unsigned int eventNo)
{
  if (eventNo == 0xfa010311)
    fprintf (fp, "algo:ccrystal");
  else if (eventNo == 0xfa010301)
    fprintf (fp, "algo:crystal");
  else if (eventNo == 0xfa010303)
    fprintf (fp, "algo:eb");
  else if (eventNo == 0xfa010304)
    fprintf (fp, "algo:merger");
  else if (eventNo == 0xfa010302)
    fprintf (fp, "algo:psa");
  else if (eventNo == 0xfa010305)
    fprintf (fp, "algo:tracked");
  else if (eventNo == 0xfa010201)
    fprintf (fp, "conf:crystal");
  else if (eventNo == 0xfa010203)
    fprintf (fp, "conf:eb");
  else if (eventNo == 0xfa000200)
    fprintf (fp, "conf:global");
  else if (eventNo == 0xfa010204)
    fprintf (fp, "conf:merger");
  else if (eventNo == 0xfa010202)
    fprintf (fp, "conf:psa");
  else if (eventNo == 0xfa0202a0)
    fprintf (fp, "conf:ranc0");
  else if (eventNo == 0xfa0202a1)
    fprintf (fp, "conf:ranc1");
  else if (eventNo == 0xfa0202a2)
    fprintf (fp, "conf:ranc2");
  else if (eventNo == 0xfa010205)
    fprintf (fp, "conf:tracked");
  else if (eventNo == 0xfa010111)
    fprintf (fp, "data:ccrystal");
  else if (eventNo == 0xfa010101)
    fprintf (fp, "data:crystal");
  else if (eventNo == 0xfa010103)
    fprintf (fp, "data:eb");
  else if (eventNo == 0xfa010104)
    fprintf (fp, "data:merger");
  else if (eventNo == 0xfa010102)
    fprintf (fp, "data:psa");
  else if (eventNo == 0xfa0201a0)
    fprintf (fp, "data:ranc0");
  else if (eventNo == 0xfa0201a1)
    fprintf (fp, "data:ranc1");
  else if (eventNo == 0xfa0201a2)
    fprintf (fp, "data:ranc2");
  else if (eventNo == 0xfa010105)
    fprintf (fp, "data:tracked");
  else if (eventNo == 0xca000100)
    fprintf (fp, "event:data");
  else if (eventNo == 0xca010101)
    fprintf (fp, "event:data:crystal");
  else if (eventNo == 0xca010102)
    fprintf (fp, "event:data:psa");
  else if (eventNo == 0xca010103)
    fprintf (fp, "event:data:tracked");
  else if (eventNo == 0xfa001100)
    fprintf (fp, "meta:eof");
  else if (eventNo == 0xfa011101)
    fprintf (fp, "meta:sync");
  else if (eventNo == 0xfa011102)
    fprintf (fp, "meta:vertex");
}

/*-----------------------------------------------------------------------------*/

int
main (int argc, char **argv)
{
  /* declarations */

  off_t inData;                 /* input file */
  off_t outData;                /* output file */
  int i, eventNo = 0, siz, ssiz = 0, i1, j, offset, ntry;
  KEY *key;
  char *frame, str[256];
  GLOBAL *global;
  PSAHIT1 *psahit1[100];
  PSAHIT2 *psahit2[100];
  unsigned int *iframe;
  unsigned short int *ui2;
  float *fframe, *fframe1, f1, r1;
  unsigned int tryval, tryvalold;
  int badHeaders = 0, nevents = 0;
  GEBDATA *gd;
  CRYS_INTPTS3 *payload3;
  unsigned int moduleno, crystalno;
  long long int T0, DTS;
  float dtsp[2048];
  int minev, maxev;
  int pp1, pp2;
  static int npinfo = 0;
  int have_ca;

  /* type counters */

  long long int n_composite_ca010102 = 0;
  long long int n_psa_fa010102 = 0;
  long long int n_config_fa000200 = 0;
  long long int n_tracked_fa010105 = 0;
  long long int n_vamos_fa0201a0 = 0;
  long long int n_unknown_ca000100 = 0;
  long long int n_unknown_fa011101 = 0;

  int maxPrint;

  /* get command line arguments */

  if (argc != 6)
    {
      printf ("use: %s infile outfile firstev lastev nprint\n", argv[0]);
      exit (0);
    };
  minev = atoi (argv[3]);
  maxev = atoi (argv[4]);
  printf ("will write %i --> %i events to mode2 file\n", minev, maxev);
  maxPrint = atoi (argv[5]);
  printf ("will debug write %i events\n", maxPrint);

  pp1 = minev;
  pp2 = pp1 + maxPrint;

  /* init */

  for (i = 0; i < 2048; i++)
    dtsp[i] = 0;

  /* allocate space */

  key = (KEY *) calloc (1, sizeof (KEY));
  global = (GLOBAL *) calloc (1, sizeof (GLOBAL));
  frame = (char *) calloc (1, 10000);
  for (i = 0; i < 100; i++)
    {
      psahit1[i] = (PSAHIT1 *) calloc (1, sizeof (PSAHIT1));
      psahit2[i] = (PSAHIT2 *) calloc (1, sizeof (PSAHIT2));
    };
  gd = (GEBDATA *) calloc (1, sizeof (GEBDATA));
  payload3 = (CRYS_INTPTS3 *) calloc (1, MAXDATASIZE);

  /* open input file */

  inData = open ((char *) argv[1], O_RDONLY);
  if (inData == 0)
    {
      printf ("could not open input data file %s, quit!\n", argv[1]);
      exit (1);
    }
  else
    printf ("input data file %s is open\n", argv[1]);
fflush(stdout);

  /* open output file */

  outData = 0;
  outData = open ((char *) argv[2], O_WRONLY | O_CREAT | O_TRUNC, PMODE);
  if (outData == 0)
    {
      printf ("could not open output data file %s, quit!\n", argv[2]);
      exit (1);
    };
fflush(stdout);

  printf ("at position %i char, %i 16-bit words, %i 32-bit words\n", ssiz, ssiz / sizeof (short int),
          ssiz / sizeof (int));
fflush(stdout);

  eventNo = 0;
  while (eventNo < maxev)
//    while (1)
    {

      /* open data dump file if in range */

      if ((eventNo >= pp1 && eventNo <= pp2) || eventNo == 0)
        {
          sprintf (str, "ag2mode2_%4.4i.agevent", eventNo);
          fp = fopen (str, "w");
        };

      if (fp != NULL)
        {
          printf ("dumping Agata event in \"%s\"\n", str);
          fprintf (fp, "sizeof(KEY)=    %2i bytes, reading %2i\n", sizeof (KEY), KEYLEN);
          fprintf (fp, "sizeof(GLOBAL)= %2i bytes\n", sizeof (GLOBAL));
          fprintf (fp, "sizeof(PSAHIT1)=%2i bytes\n", sizeof (PSAHIT1));
          fprintf (fp, "sizeof(PSAHIT2)=%2i bytes\n", sizeof (PSAHIT2));
          r1 = ((float) sizeof (PSAHIT1) + (float) sizeof (PSAHIT2)) / sizeof (int);
          fprintf (fp, "the PSAHIT is %4.1f 32-bit words long, I.e., does not align on a 32 bit boundary\n", r1);
        };

      eventNo++;

      /* read key with fixed keylen, not sizeof */
      /* to make it work on 64 bit machines */

      if (fp != NULL)
        {
          fprintf (fp, "------\n");
          fprintf (fp, "event #%i: at position %i char, %i 16 bit words, %i 32 bit words\n", eventNo, ssiz,
                   ssiz / sizeof (short int), ssiz / sizeof (int));
        };
      siz = read (inData, (char *) &key->FrameLength, 3 * sizeof (int));
      if (siz != (3 * sizeof (float)))
        {
        printf("asked for %i bytes, but got %i, goto done\n",(3 * sizeof (float)), siz);
        goto done;
        };
      ssiz += siz;
      siz = read (inData, (char *) &key->TS, sizeof (unsigned long long int));
      if (siz != sizeof (unsigned long long int))
        goto done;
      ssiz += siz;

      /* key or composite key dump */

      if (fp != NULL)
        {
          fprintf (fp, "uint, ushortint float dump of key\n");
          iframe = (unsigned int *) key;
          fframe = (float *) key;
          for (j = 0; j < KEYLEN / sizeof (int); j++)
            {
              fprintf (fp, "%2i[%3i], 0x%8.8x  ", j, j * sizeof (int), *(iframe + j));
              fprintf (fp, "0x%4.4x ", (0x0000ffff & *(iframe + j)));
              fprintf (fp, "0x%4.4x ", (0xffff0000 & *(iframe + j)) >> 16);
              if (*(fframe + j) < 3000 && *(fframe + j) > -3000)
                fprintf (fp, " %10.2f [%3i] ", *(fframe + j), *(iframe + j));
              type_identity (*(iframe + j));
              fprintf (fp, "\n");
            };
          fprintf (fp, "\n");
        };

      /* print the KEY (which by now is valid) */

      if (fp != NULL)
        {
          fprintf (fp, "Read (valid) key\n");
          fprintf (fp, "FrameLength=%u bytes, 0x%x; ", key->FrameLength, key->FrameLength);
          fprintf (fp, "or %i 16 bit-words or %i 32-bit words\n", key->FrameLength / sizeof (short int),
                   key->FrameLength / sizeof (unsigned int));
          fprintf (fp, "type=0x%x == ", key->type);
          type_identity (key->type);
          fprintf (fp, "\n");
          fprintf (fp, "sub-Detector_Number= 0x%2.2x, ", (key->type & 0x00ff0000) >> 16);
          fprintf (fp, "Frame Type= 0x%2.2x, ", (key->type & 0x0000ff00) >> 8);
          fprintf (fp, "Producer Id= 0x%2.2x\n", (key->type & 0x000000ff));
          fprintf (fp, "eventNumber=%i; ", key->eventNumber);
          fprintf (fp, "next key at %i 32 bit words\n", (key->FrameLength + ssiz) / sizeof (unsigned int) + KEYLEN);
          fprintf (fp, "TS= %25lli, 0x%16llx, ", (unsigned long long int) key->TS, (unsigned long long int) key->TS);
        };


      if (eventNo == 1)
        {
          T0 = key->TS;
        }
      else
        {
          DTS = key->TS - T0;
          T0 = key->TS;

          if (fp != NULL)
            fprintf (fp, "DTS= %25lli\n", DTS);

          /* only update in minev...maxev range */

          if (eventNo >= minev)
            {
              i1 = (int) (DTS);
              if (i1 >= 0 && i1 < 2048)
                dtsp[i1]++;
            };
        };

      /* If this is a composite frame, read the key frame */
      /* right away. */
      /* We overwrite the composite frame, but that is OK */
      /* we think!? */

      have_ca=0;
      if (key->type == 0xca010102)
        {
          have_ca=1;
          n_composite_ca010102++;

          siz = read (inData, (char *) &key->FrameLength, 3 * sizeof (int));
          if (siz != 3 * sizeof (float))
            goto done;
          ssiz += siz;
          siz = read (inData, (char *) &key->TS, sizeof (unsigned long long int));
          if (siz != sizeof (unsigned long long int))
            goto done;
          ssiz += siz;

          /* print the KEY (which by now is valid) */

          if (fp != NULL)
            {
              fprintf (fp, "Read (valid) key\n");
              fprintf (fp, "FrameLength=%u bytes, 0x%x; ", key->FrameLength, key->FrameLength);
              fprintf (fp, "or %i 16 bit-words or %i 32-bit words\n", key->FrameLength / sizeof (short int),
                       key->FrameLength / sizeof (unsigned int));
              fprintf (fp, "type=0x%x == ", key->type);
              type_identity (key->type);
              fprintf (fp, "\n");
              fprintf (fp, "sub-Detector_Number= 0x%2.2x, ", (key->type & 0x00ff0000) >> 16);
              fprintf (fp, "Frame Type= 0x%2.2x, ", (key->type & 0x0000ff00) >> 8);
              fprintf (fp, "Producer Id= 0x%2.2x\n", (key->type & 0x000000ff));
              fprintf (fp, "eventNumber=%i; ", key->eventNumber);
              fprintf (fp, "next key at %i 32 bit words\n", (key->FrameLength + ssiz) / sizeof (unsigned int) + KEYLEN);
              fprintf (fp, "TS= %25lli, 0x%16llx, ", (unsigned long long int) key->TS,
                       (unsigned long long int) key->TS);
              fprintf (fp, "\n");
            };

        };

      /* read frame */

      siz = read (inData, (char *) frame, (int) key->FrameLength - KEYLEN);
      if (siz <= 0)
        goto done;
      if (fp != NULL)
        fprintf (fp, "read Frame of length %i\n", siz);
      ssiz += siz;
      nevents++;

      if (fp != NULL)
        {
          fprintf (fp, "new position %i char, %i 16 bit words, %i 32 bit words\n", ssiz, ssiz / sizeof (short int),
                   ssiz / sizeof (int));
        };


      if (key->type == 0xfa010102)
        {
          n_psa_fa010102++;

          /* this is data:psa */

          global = (GLOBAL *) frame;

          if (fp != NULL)
            {
              fprintf (fp, "\n");
              fprintf (fp, "special dump of frame (with offset float collumn-------+)\n");
              iframe = (unsigned int *) frame;
              fframe = (float *) frame;
              fframe1 = (float *) (frame - 2);
              for (j = 0; j < siz / sizeof (int); j++)
                {
//                  fprintf (fp, "i4 pos %6i; ", (ssiz  - (int) key->FrameLength  )/sizeof(int)+j);
                  fprintf (fp, "%2i[%3i], 0x%8.8x  ", j, j * sizeof (int), *(iframe + j));
                  fprintf (fp, "0x%4.4x ", (0x0000ffff & *(iframe + j)));
                  fprintf (fp, "0x%4.4x ", (0xffff0000 & *(iframe + j)) >> 16);
                  if (*(fframe + j) < 3000 && *(fframe + j) > -3000)
                    fprintf (fp, " %10.2f ", *(fframe + j));
                  else
                    fprintf (fp, "     -      ");

                  /* offset hit collumn */

                  if (*(fframe1 + j) < 3000 && *(fframe1 + j) > -3000)
                    fprintf (fp, " %10.2f ", *(fframe1 + j));
                  else
                    fprintf (fp, "     -      ");

                  type_identity (*(iframe + j));
                  fprintf (fp, "\n");
                };
              fprintf (fp, "\n");
            };

          if (fp != NULL)
            {
              fprintf (fp, "PSA data; ");
              fprintf (fp, "ID=%i ", global->ID);
              fprintf (fp, "status=%i\n", global->status);
              fprintf (fp, "e0=%6.1f ", global->e0);
              fprintf (fp, "e1=%6.1f ", global->e1);
              fprintf (fp, "t0=%6.2f ", global->t0);
              fprintf (fp, "t1=%6.2f\n", global->t1);
              fprintf (fp, "PSAStatus=%i ", global->PSAStatus);
              fprintf (fp, "NbHit=%i (follows)\n", global->NbHit);
            };
          for (i = 0; i < (global->NbHit); i++)
            {

              offset = sizeof (GLOBAL) + i * (sizeof (PSAHIT1) + sizeof (PSAHIT2));
              psahit1[i] = (PSAHIT1 *) (frame + offset);
              psahit2[i] = (PSAHIT2 *) (frame + offset + sizeof (PSAHIT1));

            };

          if (fp != NULL)
            {
              for (i = 0; i < (global->NbHit); i++)
                {
                  fprintf (fp, " *for PSAHit # %i: \n", i);
                  fprintf (fp, "  status=%i ", psahit1[i]->status);
                  fprintf (fp, "  seg=%i ", psahit1[i]->seg);
                  fprintf (fp, "  crys=%i\n", psahit1[i]->crys);
                  fprintf (fp, "  X= %6.2f ", psahit2[i]->X);
                  fprintf (fp, "  Y= %6.2f ", psahit2[i]->Y);
                  fprintf (fp, "  Z= %6.2f ", psahit2[i]->Z);
                  fprintf (fp, "  E= %6.1f ", psahit2[i]->E);
                  fprintf (fp, "  T= %6.2f\n", psahit2[i]->T);
                  fprintf (fp, "  tCCe=%f ", psahit2[i]->tCCe);
                  fprintf (fp, "  tCCh=%f ", psahit2[i]->tCCh);
                  fprintf (fp, "  tSGe=%f ", psahit2[i]->tSGe);
                  fprintf (fp, "  tSGh=%f\n", psahit2[i]->tSGh);
                };
              fprintf (fp, "\n");
            };


          /* write out as GT mode 2 data */

          gd->type = GEB_TYPE_DECOMP;
          gd->length = sizeof (CRYS_INTPTS3);
          gd->timestamp = key->TS;

          bzero ((void *) payload3, sizeof (CRYS_INTPTS3));

          /* Pass on as much as we have of the the header. */
          /* What we can't/don't pass will be zero */

          payload3->type = (int) 0xabcd5678;

          moduleno = psahit1[0]->crys / 3;
          crystalno = psahit1[0]->crys - moduleno * 3;
          payload3->crystal_id = crystalno;
          payload3->crystal_id |= (moduleno << 2);
          payload3->num = global->NbHit;
          payload3->tot_e = global->e0;
          payload3->timestamp = key->TS;
          payload3->trig_time = 0;
          payload3->t0 = 0;
          payload3->cfd = 0;
          payload3->chisq = 0;
          payload3->norm_chisq = 0;
          payload3->baseline = 0;
          payload3->pad = 0;
          for (i = 0; i < payload3->num; i++)
            {
              if (psahit1[0]->crys != psahit1[i]->crys)
                {
                  printf ("crys problem at event # %i\n", eventNo);
//                  printf (", QUIT HERE!!!\n");
//                  goto done;
                }
//              assert (psahit1[0]->crys == psahit1[i]->crys);
              payload3->intpts[i].x = psahit2[i]->X;
              payload3->intpts[i].y = psahit2[i]->Y;
              payload3->intpts[i].z = psahit2[i]->Z;
              payload3->intpts[i].e = psahit2[i]->E;
              payload3->intpts[i].seg = psahit1[i]->seg;
              payload3->intpts[i].seg_ener = psahit2[i]->E;
            };

          /* writeout new format event */

          if (eventNo >= minev)
            {
              siz = write (outData, (char *) gd, sizeof (GEBDATA));
              siz = write (outData, (char *) payload3, gd->length);
              if (fp != NULL)
                fprintf (fp, "wrote event with TS=%lli CCe=%6.0f to mode2 file\n", gd->timestamp, payload3->tot_e);
            }


        }
      else if (key->type == 0xfa000200)
        {

          n_config_fa000200++;

          /* this is a configuration frame, we print */
          /* it out for good measure */

          if (fp != NULL)
            {
              fprintf (fp, "configuration frame:\n");
              for (i = 12; i < (key->FrameLength - 26); i++)
                fprintf (fp, "%c", *(frame + i));
              fprintf (fp, "\n");
            };

        }
      else if (key->type == 0xfa010105)
        {
          n_tracked_fa010105++;

          if (npinfo < 10)
            {
              npinfo++;
              printf ("tracked frame, ignore\n");
            }

        }
      else if (key->type == 0xfa0201a0)
        {
          n_vamos_fa0201a0++;

          if (npinfo < 20)
            {
              npinfo++;
              printf ("vamos frame, ignore- pour le moment\n");
            }

        }
      else if (key->type == 0xca000100)
        {
          n_unknown_ca000100++;

          if (npinfo < 20)
            {
              npinfo++;
              printf ("unkown frame, ignore- pour le moment\n");
            }
        }
      else if (key->type == 0xfa011101)
        {
          n_unknown_fa011101++;

          if (npinfo < 20)
            {
              npinfo++;
              printf ("unkown frame, ignore- pour le moment\n");
            }

        }
      else
        {
          printf ("don't know this frame type,0x%x, quit\n", key->type);
          goto done;
        };

      if (fp != NULL)
        fclose (fp);
      fp = NULL;

    }

done:

  printf ("**** done ****\n");
  printf ("we found %i events \n", nevents);
  printf ("we recovered from %i bad headers, %6.2f%%\n", badHeaders, 100.0 * badHeaders / (nevents + badHeaders));
  i1 = 2048;
  wr_spe ("ag2mode2_dtsp.spe", &i1, dtsp);
  printf ("wrote ag2mode2_dtsp.spe\n");
  fflush (stdout);

  printf ("\n");
  printf (" n_composite_ca010102= %lli\n", n_composite_ca010102);
  printf (" n_psa_fa010102= %lli\n", n_psa_fa010102);
  printf (" n_config_fa000200= %lli\n", n_config_fa000200);
  printf (" n_tracked_fa010105= %lli\n", n_tracked_fa010105);
  printf (" n_vamos_fa0201a0= %lli\n", n_vamos_fa0201a0);
  printf (" n_unknown_ca000100= %lli\n", n_unknown_ca000100);
  printf (" n_unknown_fa011101= %lli\n", n_unknown_fa011101);
  printf ("\n");

  /* done */

  printf ("close (inData);\n");
  fflush (stdout);
  close (inData);
  printf ("done\n");
  exit (0);


}
