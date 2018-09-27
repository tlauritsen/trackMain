
#include <stdlib.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>

int
ctkTrackOpt (int i)
{

  /* print tracking option */

  switch (i)
    {
    case 0:
      fprintf (stderr,"full tracking\n");
      return (0);
      break;
    case 1:
      fprintf (stderr,"largest energy point (==first in E sort)\n");
      return (0);
      break;
    case 2:
      fprintf (stderr,"random point\n");
      return (0);
      break;
    case 3:
      fprintf (stderr,"'kickout', break @ first worse permutation\n");
      return (0);
      break;
    case 4:
      fprintf (stderr,"'goodenough', break @ first OK FOM encountered\n");
      return (0);
      break;
    case 5:
      fprintf (stderr,"'jump/skip/hop' I-Yang style\n");
      return (0);
      break;
    default:
      fprintf (stderr,"unknown\n");
      return (1);
      break;
    };


}
