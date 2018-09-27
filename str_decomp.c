
/* decompose a detector list like : 1-5,7,9-12 */
/* into a simple array */

/* tl/94/7/26 */


#include <stdio.h>
#include <stdlib.h>

/*----------------------------------------------- */

int
str_decomp (char *str, int dim, int *yy, int val)
{

  /* declarations */

  int i, j, pos, nn, ok, lo, hi;
  char lstr[100];

  fprintf (stderr,"str_decomp: decomposing string <%s> into array [0..%i]\n", str, dim - 1);

  /* zero array */

//  for (i = 0; i < dim; i++)
//    yy[i] = 0;

  pos = 0;
  ok = 1;
  while (ok == 1)
    {

      /* search for the next sub range */

      nn = 0;
      while ((str[pos + nn] != ',') && (ok == 1))
        {
          nn++;
          if (str[pos + nn] == 0)
            ok = 0;
        };

      /* create the local string */

      j = 0;
      for (i = pos; i < (pos + nn); i++)
        {
          lstr[j] = str[i];
          j++;
        };
      lstr[j] = 0;

      /* remove non numeric characters in this string */

      for (i = 0; i < nn; i++)
        if ((lstr[i] < 48) || (lstr[i] > 57))
          lstr[i] = ' ';

      /* extract limits */

      i = sscanf (lstr, "%i%i", &lo, &hi);
      if (i == 1)
        hi = lo;

      /* check limits */

      if (lo < 0)
        {
          fprintf (stderr,"%i is less than lower limits of zero\n", lo);
          exit (0);
        };
      if (hi < 0)
        {
          fprintf (stderr,"%i is less than lower limits of zero\n", hi);
          exit (0);
        };
      if (lo >= dim)
        {
          fprintf (stderr,"%i is more than higher limit of %i\n", lo, dim);
          exit (0);
        };
      if (hi >= dim)
        {
          fprintf (stderr,"%i is more than higher limit of %i\n", hi, dim);
          exit (0);
        };

      /* fill array */

      for (j = lo; j <= hi; j++)
        yy[j] = val;

      /* move pos pointer forward */

      pos += nn;
      pos++;

    };

  /* done */

  return (0);

}

/*------------------------------------------------------*/

#if(0)

int
main (int argc, char **argv)
{


  char str[50];
  int array[100];
  int i;

  str_decomp (argv[1], 100, array);

  for (i = 0; i < 20; i++)
    fprintf (stderr,"%2i: %i\n", i, array[i]);

  return (0);

}

#endif
