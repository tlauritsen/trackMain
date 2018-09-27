
# simplified Makefile extracted from GEBSort package
# on Thu Sep 27 15:43:49 CDT 2018

PLTFMOPT=Makefile.$(shell uname)
include $(PLTFMOPT)

all:
		make trackMain
		make ag2mode2

#-------------------

trackMain:	trackMain.o ctktk.o setupTrack.o setupTrack_crmat.o track.o \
		writeTrack_repeat.o writeTrack_addtrack.o ctkStats.o \
		ctkTrackOpt.o ctkPrTrkPar.o  \
		ctkinit.o  findVector.o spe_fun.o\
		findAngle.o findCAngle.o ctkTrackOpt.o \
		printEvent.o ctksort.o splitCluster.o \
                combineCluster.o matchMaker.o  pairProd.o\
		reCluster.o str_decomp.o getEvent.o \
                rotations.o time_stamp.o
		$(cc) -o $@ $^ -lm

rotations.o:	rotations.c
		$(cc) -c $^

trackMain.c:	ctktk.c ctk.h gdecomp.h
trackMain.o:	trackMain.c 
		$(cc) -c $^
		
ctkTrackOpt.o:	ctkTrackOpt.c
		$(cc) -c $^

ctkPrTrkPar.o:	ctkPrTrkPar.c
		$(cc) -c $^

ctkinit.o:	ctkinit.c
		$(cc) -c $^

findVector.o:	findVector.c
		$(cc) -c $^

findAngle.o:	findAngle.c
		$(cc) -c $^

findCAngle.o:	findCAngle.c
		$(cc) -c $^

ctksort.o:	ctksort.c
		$(cc) -c $^

getEvent.c:	ctk.h
getEvent.o:	getEvent.c 
		$(cc) -c $^

ctktk.o:	ctktk0.c 
		$(cc) -c ctktk.c
ctktk.c:	ctktk0.c ctk.h

reCluster.c:	ctk.h
reCluster.o:	reCluster.c	
		$(cc) -c $^

splitCluster.c:	ctk.h
splitCluster.o: splitCluster.c 
		$(cc) -c $^

combineCluster.c:	ctk.h
combineCluster.o: combineCluster.c
		$(cc) -c $^

str_decomp.o:	str_decomp.c 
		$(cc) -c  $^

matchMaker.c:	ctk.h
matchMaker.o:	matchMaker.c  
		$(cc) -c $^

pairProd.c:	ctk.h
pairProd.o:	pairProd.c 
		$(cc) -c $^

track.c:	ctk.h
track.o:	track.c 
		$(cc) -c $^	

writeTrack_repeat.c:	ctk.h
writeTrack_repeat.o:	writeTrack_repeat.c 
			$(cc) -DWRITETRACK -c $^

writeTrack_addtrack.c:	ctk.h	
writeTrack_addtrack.o:	writeTrack_addtrack.c
			$(cc) -DWRITETRACK -c $^		

setupTrack.c:	ctk.h 	
setupTrack.o:	setupTrack.c
		$(cc) -c $^	

setupTrack_crmat.c:	ctk.h 
setupTrack_crmat.o:	setupTrack_crmat.c 
		$(cc) -c $^	

ctkStats.c:	ctk.h	
ctkStats.o:	ctkStats.c
		$(cc) -c $^
		
printEvent.c:	ctk.h
printEvent.o:	printEvent.c 
		$(cc) -c $^

time_stamp.o:	time_stamp.c
		$(cc)  -c  $^

spe_fun.o:	spe_fun.c
		$(cc) -c $^

#-------------------

ag2mode2:	ag2mode2.c spe_fun.o
		$(cc) -o $@ $^
 
ag2mode2.c:	ag2mode2.h


