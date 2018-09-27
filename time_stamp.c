
#include <time.h>
#include <stddef.h>
#include <stdio.h>

int
time_stamp (FILE * fp)
{
  /* declarations */

  struct tm *local;
  time_t t;

  /* get current time and print */

  t = time (NULL);
  local = localtime (&t);
  fprintf (fp, "%s", asctime (local));

  /* done */

  return (0);

}
