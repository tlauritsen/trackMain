
  ndet = 6;
  nn = 0;
  fprintf (stderr,"--> order= %i <------------------\n", ndet);
  for (i1 = 0; i1 < ndet; i1++)
    for (i2 = 0; i2 < ndet; i2++)
      if (i2 != i1)
	for (i3 = 0; i3 < ndet; i3++)
	  if (i3 != i1)
	    if (i3 != i2)
	      for (i4 = 0; i4 < ndet; i4++)
		if (i4 != i1)
		  if (i4 != i2)
		    if (i4 != i3)
		      for (i5 = 0; i5 < ndet; i5++)
			if (i5 != i1)
			  if (i5 != i2)
			    if (i5 != i3)
			      if (i5 != i4)
				for (i6 = 0; i6 < ndet; i6++)
				  if (i6 != i1)
				    if (i6 != i2)
				      if (i6 != i3)
					if (i6 != i4)
					  if (i6 != i5)
					  {
					    assert(nn < MAXPERM);
					    assert(ndet < MAXNOSEG);
					    Pars.permlkup[ndet][nn][0] = i1;
					    Pars.permlkup[ndet][nn][1] = i2;
					    Pars.permlkup[ndet][nn][2] = i3;
					    Pars.permlkup[ndet][nn][3] = i4;
					    Pars.permlkup[ndet][nn][4] = i5;
					    Pars.permlkup[ndet][nn][5] = i6;
					    nn++;
					  };

