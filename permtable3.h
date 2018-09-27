
  ndet = 3;
  nn = 0;
  fprintf (stderr,"--> order= %i <------------------\n", ndet);
  for (i1 = 0; i1 < ndet; i1++)
    for (i2 = 0; i2 < ndet; i2++)
      if (i2 != i1)
	for (i3 = 0; i3 < ndet; i3++)
	  if (i3 != i1)
	    if (i3 != i2)
	    {
	      fprintf (stderr,"%4i: %i %i %i\n", nn, i1, i2, i3);
	      assert(nn < MAXPERM);
	      assert(ndet < MAXNOSEG);
	      Pars.permlkup[ndet][nn][0] = i1;
	      Pars.permlkup[ndet][nn][1] = i2;
	      Pars.permlkup[ndet][nn][2] = i3;
	      nn++;
	    };

