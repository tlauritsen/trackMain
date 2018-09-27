#!/bin/bash

if [ "$#" -ne 1 ]
then
  echo "you must give one of the arguments:"
  echo "-----------------------"
  grep "if \[" go | awk '{print $5}' | grep "\""
  echo "-----------------------"
  exit
fi

rm core.*


#-------------------------------------------------------
# this file will have examples of most things you would
# want to do with the GEBSorter and tracking codes
#-------------------------------------------------------

#-------------------------------------------------------
# to run the simulator:
#/home/tl/d6/gretina/devel/cur/GEBTapSim/bin/linux-x86/GEBTapSim /media/r20150911_1334/user/120621a/gtdata/anl1/Global.dat

if [ $1 = "sim_root" ]
then

 echo "get data from the GEBTapSim into a root file"GEBSort.chat
 rm GTDATA; ln -s ./ GTDATA  
 rm GTDATA/*.root

#                    +-- Global Event Builder (GEB) host IP (or simulator)
#                    |         + -- Number of events asked for on each read
#                    |         |  +-- desired data type (0 is all)
#                    |         |  |  +-- timeout (sec)
#                    |         |  |  |    
#                    |         |  |  |   
 ./GEBSort \
    -input geb 146.139.198.46 100 0 100.0  \
    -rootfile GTDATA/test.root RECREATE \
    -chat GEBSort.chat 
#> GTDATA/GEBSort_sim_root.log
   cp *.chat GTDATA/
#  ls -lt GTDATA/

fi



if [ $1 = "sim_map" ]
then

 echo "get data from the GEBTapSim into a map file"
 rm GTDATA; ln -s ./ GTDATA  
 rm GTDATA/*.map

# use sdummyload(200000000) to find start map address
# inside rootn.exe after compiling GSUtil

#                 +-- Global Event Builder (GEB) host IP (or simulator)
#                 |           + -- Number of events asked for on each read
#                 |           |  +-- desired data type (0 is all)
#                 |           |  |  +-- timeout (sec)
#                 |           |  |  |
#                 |           |  |  |
./GEBSort -chat GEBSort.chat \
   -input geb 146.139.198.46 100 0 100.0 \
   -mapfile  GTDATA/c1.map 200000000 0x9ef6e000 
#                 |            |        |
#                 |            |        +--- start map address
#                 |            +---- size of map (bytes)
#                 +--- map file name

#> GTDATA/GEBSort.log

#  ls -lt GTDATA/

fi

if [ $1 = "file_root" ]
then

 echo "sort data file into a root file"
 rm GTDATA; ln -s  /media/r20150911_1334/user/120621a/gtdata/anl1 GTDATA 
 rm GTDATA/*.root
 ls -lt GTDATA/

 ./GEBSort \
    -input disk GTDATA/Global.dat \
    -rootfile GTDATA/test.root RECREATE \
    -chat GEBSort.chat > GTDATA/GEBSort_file_root.log

  ls -lt GTDATA/

fi

if [ $1 = "file_map" ]
then

 echo "sort data file into a map file [odd thing to do but you can]"
 rm GTDATA; ln -s  /media/r20150911_1334/user/120621a/gtdata/anl1 GTDATA 
 rm GTDATA/*.map
 ls -lt GTDATA/

./GEBSort  \
   -input disk GTDATA/Global.dat \
   -mapfile  GTDATA/c1.map 200000000 0x9ef6e000 \
   -chat GEBSort.chat

  ls -lt GTDATA/
fi

if [ $1 = "geb_root" ]
then

  echo "getting data from the real thing: '10.0.1.100' into a root file"
  echo "you would not normaly do this. You would use a map file"

 rm GTDATA; ln -s ./ GTDATA  

#               +-- Global Event Builder (GEB) host IP (or simulator)
#               |           + -- Number of events asked for on each read
#               |           |  +-- desired data type (0 is all)
#               |           |  |  +-- timeout (sec)
#               |           |  |  |   
 ./GEBSort \
   -input geb 10.0.1.100 100  0 100.0  \
   -rootfile GTDATA/test.root RECREATE \
   -chat GEBSort.chat 

fi

if [ $1 = "geb_map" ]
then

 echo "============================================================"
 echo "get data from the geb: '10.0.1.100' into a map file"
 echo "** this is what you would do on-line during an experiment **"
 echo "============================================================"

# use sdummyload(200000000) to find start map address
# inside rootn.exe after compiling GSUtil

 rm GTDATA; ln -s ./ GTDATA  
 rm GTDATA/*.map

#               +-- Global Event Builder (GEB) host IP (or simulator)
#               |           + -- Number of events asked for on each read
#               |           |  +-- desired data type (0 is all)
#               |           |  |  +-- timeout (sec)
#               |           |  |  |   
  ./GEBSort \
    -input geb 10.0.1.100  100  0 100.0 \
    -mapfile  GTDATA/c1.map 200000000 0x9ef6e000 \
    -chat GEBSort.chat 
#                     |        |         |
#                     |        |         +----- start map address
#                     |        +---- size of map file
#                     +---- mapfile

  ls -lt GTDATA/ | head -10

fi

if [ $1 = "gtdata_direct" ]
then

#   rm GTDATA; ln -s /media/r20150911_1334/user/120621a/gtdata/co60/Run0050 GTDATA
#   rm GTDATA; ln -s /media/r20150911_1334/user/120621a/gtdata/20131218_1535/Run0023 GTDATA
#   rm GTDATA; ln -s /media/r20150911_1334/user/120621a/gtdata/20131220_1203/Run0003 GTDATA
#   rm GTDATA; ln -s /media/r20150911_1334/user/120621a/gtdata/20131220_1203/Run0004 GTDATA
#   rm GTDATA; ln -s ~/d6/tmp2  GTDATA

   ls -lt GTDATA/
   echo "track GT data mode 2 data (decomposed data), experimentaldata "
   echo -n "GTDATA dir:"; (cd GTDATA; pwd -P)
   echo "no time ordering with GEBMerge in this case"

   echo "./trackMain"
   rm GTDATA/mode1.gtd
   ./trackMain \
      track_GT.chat  \
      GTDATA/Global.dat  \
      GTDATA/mode1.gtd > GTDATA/trackMain.log
   ls -lt GTDATA/

  echo "./GEBSort_nogeb"
  rm GTDATA/test.root
  ./GEBSort_nogeb \
    -input disk GTDATA/mode1.gtd \
    -rootfile GTDATA/test.root RECREATE \
    -chat GEBSort.chat > GTDATA/GEBSort.log

   ls -lt GTDATA/ | head -20

fi

if [ $1 = "agata" ]
then

   echo "convert AGATA data to GT mode2"
   echo "then track and analyze the data"

# 60Co
#  rm GTDATA; ln -s /media/r20150911_1334/user/120621a/agata/20140414_a  GTDATA
#  rm GTDATA; ln -s /media/r20150911_1334/user/120621a/agata/20140415    GTDATA
#  rm GTDATA; ln -s /media/r20150911_1334/user/120621a/agata/20140711_a GTDATA
#  rm GTDATA; ln -s /media/r20150911_1334/user/120621a/agata/20140711_b GTDATA
# 166 ho
#   rm GTDATA; ln -s /media/r20150911_1334/user/120621a/agata/20140414_b  GTDATA

# new data 8/19/2014
#   rm GTDATA; ln -s /media/r20150911_1334/user/120621a/agata/20140819_1427_a GTDATA

# new data 9/3/2014, seems to be smoothed X,Y,Z data this time DINO data?
#  rm GTDATA; ln -s /media/r20150911_1334/user/120621a/agata/20140903 GTDATA
# ^^^^ we now analyze this in /home/tl/d6/gretina/devel/cur/AG_GSI

# data with tracked info, Tue Oct 14 13:36:20 CDT 2014
#  rm GTDATA; ln -s /media/r20150911_1334/user/120621a/agata/20141014 GTDATA

# 158Er run at ganil

#  rm GTDATA; ln -s /media/20141124_AG2014/user/ag2a/r0045/NoTrack/Out/Global GTDATA

# vamos data 4/18.../2015, sample data for software devel

#  rm GTDATA; ln -s /media/20141124_AG2014/user/201504_data/1 GTDATA
#  rm GTDATA; ln -s /media/20141124_AG2014/user/201504_data/2 GTDATA
#  rm GTDATA; ln -s /media/20141124_AG2014/user/201504_data/3 GTDATA
#  rm GTDATA; ln -s /media/20141124_AG2014/user/201504_data/4 GTDATA

# laptop test dierectory

   rm GTDATA; ln -s /home/tl/tmp GTDATA

   ls -lgta GTDATA/ | grep -v agevent
   rm core.*

if [ 1 = 1 ]
then
   rm GTDATA/*.agevent *.agevent
   echo "agata to GT mode2 (standard name of Global.dat)"

   rm GTDATA/Global.dat
#   ag2mode2 GTDATA/agata.adf GTDATA/Global.dat 0 2000000000 100 > GTDATA/ag2mode2.log
   ag2mode2 GTDATA/agata.adf GTDATA/Global.dat 0 20000000 100 > GTDATA/ag2mode2.log

   mv *.agevent GTDATA/
   echo "./GEBMerge"
   rm GTDATA/merged.gtd* TS*.list
   ./GEBMerge GEBMerge.chat GTDATA/merged.gtd GTDATA/Global.dat > GTDATA/GEBMerge.log
   ls -l GTDATA/merged.gtd*

#  echo "./GEBSort_nogeb"
#  rm GTDATA/test.root
#  ./GEBSort_nogeb \
#    -input disk GTDATA/merged.gtd_000 \
#    -rootfile GTDATA/test.root RECREATE \
#    -chat GEBSort.chat > GTDATA/GEBSort.log
fi

   echo "track GT data mode 2 data (decomposed data), experimental data "
   echo -n "GTDATA dir:"; (cd GTDATA; pwd -P)

   echo "./trackMain"
   rm GTDATA/mode1.gtd
   ./trackMain \
      track_GT.chat  \
      GTDATA/merged.gtd_000  \
      GTDATA/mode1.gtd > GTDATA/trackMain.log
   ls -lt GTDATA/merged.gtd* GTDATA/mode1.gtd

  echo "./GEBSort_nogeb"
  rm GTDATA/test.root
  ./GEBSort_nogeb \
    -input disk GTDATA/mode1.gtd \
    -rootfile GTDATA/test.root RECREATE \
    -chat GEBSort.chat > GTDATA/GEBSort.log

   cp *.chat GTDATA/

   ls -lt GTDATA/  | grep -v agevent

   ls -lt GTDATA/merged.gtd* GTDATA/mode1.gtd GTDATA/test.root

   grep "AGATA_data" *.chat | grep -v you | grep -v G4
   echo "^^^^^ this better be OK"
   grep singlehitmaxdepth *.chat | grep -v G4
   echo "^^^^^ this better be 23.5"

fi







if [ $1 = "mode3" ]
then

# data from Chris (part to Olivie)
#  rm GTDATA; ln -s /media/r20150911_1334/user/mode3 GTDATA
# rm GTDATA; ln -s /media/cdir2/user/GTmode3_data GTDATA
  rm GTDATA; ln -s /media/20171129_1623/user/GTmode3_data GTDATA

# other data
#  rm GTDATA; ln -s /media/r20150911_1334/user/120621a/gtdata/20140109_1538/Run0096 GTDATA
#  rm GTDATA; ln -s /media/r20150911_1334/user/120621a/gtdata/166Ho_1/Run0068/ GTDATA
#  rm GTDATA; ln -s /media/r20150911_1334/user/120621a/gtdata/20131220_1203/Run0003 GTDATA
#  rm GTDATA; ln -s /media/r20150911_1334/user/120621a/gtdata/dirk_runs/Run0130 GTDATA

# data from Shoufei (MSU 90 deg ring setup, big enough to show segment energies, good energies)
#  rm GTDATA; ln -s /media/r20150911_1334/user/MSU_May2013_90deg GTDATA 

  ls -lt GTDATA/
  echo "mode3 data processing"
  echo -n "GTDATA dir:"; (cd GTDATA; pwd -P)

  rm *.xy

  echo "./GEBSort_nogeb"
  rm GTDATA/test.root
  ./GEBSort_nogeb \
    -input disk GTDATA/GlobalRaw.dat \
    -rootfile GTDATA/test3.root RECREATE \
    -chat GEBSort.chat 
#> GTDATA/GEBSort.log
    cp *.chat GTDATA/

#    ls -lt GTDATA/ | head -10
fi


if [ $1 = "AG_mode3_to_GT" ]
then

# --uncompressed trace data (change data file name)
# rm GTDATA; ln -s /media/20171129_1623/user/AG2GT/1 GTDATA
# rm GTDATA; ln -s /media/20171129_1623/user/AG2GT/2 GTDATA

# --compressed  trace data (change data file name)
#rm GTDATA; ln -s  /media/r20150911_1334/user/AG_mode3 GTDATA
 rm GTDATA; ln -s  /media/20160705_1451/user/AG_sp/02A GTDATA

rm *.xy

if [ 1 == 1 ]
then
  rm GTDATA/GTmode3.dat
  AG_mode3_to_GT 25 0 \
    GTDATA/event_mezzdata.cdat.0003 \
    GTDATA/GTmode3.dat > GTDATA/AG_mode3_to_GT.log
  ls -l GTDATA/event_mezzdata.bdat.0000 GTDATA/GTmode3.dat GTDATA/AG_mode3_to_GT.log

  Dmp GTDATA/GTmode3.dat  1 0 l 0 10000 > x.dmp
  Dmp GTDATA/GTmode3.dat  1 1 l 0 10000 > y.dmp
fi

  rm GTDATA/test.root
  ./GEBSort_nogeb \
    -input disk GTDATA/GTmode3.dat \
    -rootfile GTDATA/test.root RECREATE \
    -chat GEBSort.chat > GTDATA/GEBSort.log
  ls -l GTDATA/test.root

fi





if [ $1 = "mode3_seg_cal" ]
then

  rm GTDATA; ln -s /media/r20150911_1334/user/120621a/gtdata/20140109_1538/Run0096 GTDATA
  ls -lt GTDATA/
  echo "mode3_seg_cal "
  echo -n "GTDATA dir:"; (cd GTDATA; pwd -P)

  echo "./GEBSort_nogeb"
  rm GTDATA/test.root
  ./GEBSort_nogeb \
    -input disk GTDATA/GlobalRaw.dat \
    -rootfile GTDATA/test.root RECREATE \
    -chat GEBSort.chat 
#> GTDATA/GEBSort.log
    cp *.chat GTDATA/

    ls -lt GTDATA/ | head -10

fi












if [ $1 = "dirk_3d" ]
then

  rm GTDATA; ln -s /media/20150625_1144/user/2015_06_dirk/Run0144 GTDATA

  rm GTDATA/test.root
  rm *.spe
  ./GEBSort_nogeb \
    -input disk GTDATA/Q4P4_IXT_Penalty_ORG.dat \
    -rootfile GTDATA/test.root RECREATE \
    -chat GEBSort.chat > GTDATA/GEBSort.log
    cp *.chat GTDATA/
    cp *.spe GTDATA/

fi





if [ $1 = "GT2AGG4" ]
then

  rm GTDATA; ln -s /media/r20150911_1334/user/20140424_1131/gsfma315_GT/Run0298 GTDATA

   echo "./GEBFilter - writing GT data in AGATA G4 ascii format"

  ./GEBFilter GEBFilter.chat GTDATA/merged.gtd_000 /dev/null 
#> GTDATA/GEBFilter.log

fi









if [ $1 = "agfa" ]
then

  echo "test of merging and sorting data "
  echo "on 64 bit machine and using latest" 
  echo "version of GEBSort package"

#  rm GTDATA; ln -s ~/tmp1 GTDATA
#  rm GTDATA; ln -s /media/r20150911_1334/user/agfa/1/run22 GTDATA
#  rm GTDATA; ln -s /export/home/data2/tmp/run22 GTDATA
#  rm GTDATA; ln -s ~/tmp1 GTDATA
  rm GTDATA; ln -s /media/r20150911_1334/user/agfa/1/run23 GTDATA
  echo -n "GTDATA dir:"; (cd GTDATA; pwd -P)

# test

if [ 1 == 1 ]
then

# merge 

  echo "merging data @ `date`"  

  ./GEBMerge \
     GEBMerge.chat \
     GTDATA/merged.gtd \
     GTDATA/run_gsfma330_23.gtd01_000_0201

fi

if [ 1 == 0 ]
then

# merge 

  echo "merging data @ `date`"  

  ./GEBMerge GEBMerge.chat \
     GTDATA/merged.gtd \
    `ls GTDATA/run_gsfma330_*` 2> GTDATA/merge.log
  ls -l GTDATA/run* | awk '{sum+=$5;print $5,sum}' | tail -1
  ls -l  GTDATA/merged.gtd_*
fi

# dfma sort

  echo "GEBSort_nogeb data @ `date`"

  rm -v GTDATA/wsi.root
  rm -v GTDATA/GEBSort.log
  ./GEBSort_nogeb \
    -input disk GTDATA/merged.gtd_000 \
    -rootfile GTDATA/wsi.root RECREATE\
    -chat GEBSort.chat > GTDATA/GEBSort.log
   ls -lh GTDATA/*.root


fi







if [ $1 = "GT_pipe" ]
then

# 'standard' dataset, 60Co [http://gretina.lbl.gov/tools-etc/standard-data]
# rm GTDATA; ln -s ~/d6/gretina/std_data/60Co GTDATA

# 'standard' dataset, 166Ho [http://gretina.lbl.gov/tools-etc/standard-data]
 rm GTDATA; ln -s /home/tl/d6/gretina/std_data/166Ho GTDATA
# rm GTDATA; ln -s /home/tl/tmp2 GTDATA

rm *.timing

if [ 1 == 0 ]
then

   echo "merge, track and sort, using files all the way"
   echo -n "GTDATA dir:"; (cd GTDATA; pwd -P)
   rm GTDATA/*.root 
   rm GTDATA/*.gtd 

   echo "start `date`" > oldway.timing

   echo "./GEBMerge the old way"
   rm GTDATA/merged.gtd*
   time \
   ./GEBMerge GEBMerge.chat GTDATA/merged.gtd GTDATA/Global.dat 2> GTDATA/GEBMerge1.log
   ls -l GTDATA/Global.dat GTDATA/merged.gtd*

   echo "./trackMain the old way"
   rm ?,*.list
   time \
   ./trackMain \
      track_GT.chat  \
      GTDATA/merged.gtd_000 \
      GTDATA/mode1.gtd 2> GTDATA/trackMain.log
   ls -l GTDATA/mode1.gtd GTDATA/merged.gtd* GTDATA/piped.gtd

   echo "./GEBSort_nogeb the old way"
   echo -n "GTDATA dir:"; (cd GTDATA; pwd -P)
   time \
   ./GEBSort_nogeb \
     -input disk GTDATA/mode1.gtd \
     -rootfile GTDATA/wsi_orig.root RECREATE\
     -chat GEBSort.chat > GTDATA/GEBSort_wsi.log
   ls -lt GTDATA/*.root

   echo "done `date`" >> oldway.timing

fi


if [ 1 == 1 ]
then

  echo "merge, track and sort, piping all the way, no intermediate files"
  echo -n "GTDATA dir:"; (cd GTDATA; pwd -P)
  rm GTDATA/*.root GTDATA/mode1_piped.gtd GTDATA/merged.gtd*

   echo "start `date`" > newway.timing
   time \
   ./GEBMerge GEBMerge.chat STDOUT GTDATA/Global.dat 2> GTDATA/GEBMerge.log | \
   ./trackMain track_GT.chat  STDIN  STDOUT 2> GTDATA/trackMain.log | \
   ./GEBSort_nogeb -input disk STDIN -rootfile GTDATA/wsi.root RECREATE \
      -chat GEBSort.chat > GTDATA/GEBSort_wsi.log
   echo "done `date`" >> newway.timing

fi

if [ 1 == 0 ]
then

  echo "merge, track and sort, piping all the way, no intermediate files with waitfordata"
  echo -n "GTDATA dir:"; (cd GTDATA; pwd -P)
  rm GTDATA/*.root GTDATA/mode1_piped.gtd GTDATA/merged.gtd*

   echo "start `date`" > newway.timing
   ./GEBMerge GEBMerge.chat STDOUT GTDATA/slow.dat 2> GTDATA/GEBMerge.log | \
   ./trackMain track_GT.chat  STDIN  STDOUT 2> GTDATA/trackMain.log | \
   ./GEBSort_nogeb -input disk STDIN -rootfile GTDATA/wsi.root RECREATE \
      -chat GEBSort.chat > GTDATA/GEBSort_wsi.log
   echo "done `date`" >> newway.timing

fi

fi








if [ $1 = "GT_standard" ]
then

# typical production sort of mode2 GT data off-line

# from MSU 
#  rm GTDATA; ln -s /media/r20150911_1334/user/120621a/gtdata/co60/Run0050 GTDATA
#  rm GTDATA; ln -s /media/r20150911_1334/user/120621a/gtdata/co60/Run0050_b GTDATA

# big angcor dataset from MSU
# rm GTDATA; ln -s /media/r20150911_1334/user/MSU_2015_b/Run0128/  GTDATA
# rm GTDATA; ln -s /media/r20150911_1334/user/MSU_2015_b/Run0128_veto GTDATA

# Edana data (really from DVD)

# rm GTDATA; ln -s /media/r20150911_1334/user/120621a/gtdata/edata/1 GTDATA

# ***** all the data from 2013 at ANL have wrong module IDs *****

# 60Co data ANL
#   rm GTDATA; ln -s /media/r20150911_1334/user/120621a/gtdata/20131218_1535/Run0023 GTDATA
#   rm GTDATA; ln -s /media/r20150911_1334/user/120621a/gtdata/20131220_1203/Run0003 GTDATA
#   rm GTDATA; ln -s /media/r20150911_1334/user/120621a/gtdata/20131220_1203/Run0004 GTDATA

# 60Co data ANL
#   rm GTDATA; ln -s /media/r20150911_1334/user/120621a/gtdata/20131221_1416/Run0021 GTDATA
#   rm GTDATA; ln -s /media/r20150911_1334/user/120621a/gtdata/20131222_1539/Run0026 GTDATA
#   rm GTDATA; ln -s /media/r20150911_1334/user/120621a/gtdata/20131222_1539/Run0029 GTDATA

# 207Bi data ANL
#   rm GTDATA; ln -s /media/r20150911_1334/user/120621a/gtdata/20131222_1539/Run0033 GTDATA

# 166Ho data ANL
#  rm GTDATA; ln -s /media/r20150911_1334/user/120621a/gtdata/20131222_1539/Run0034 GTDATA
#  rm GTDATA; ln -s /media/r20150911_1334/user/120621a/gtdata/20131222_1539/Run0039 GTDATA
#  rm GTDATA; ln -s /media/r20150911_1334/user/120621a/gtdata/20131222_1539/Run0040 GTDATA

# better later data, 166Ho data ANL

#   rm GTDATA; ln -s /media/LIN0031205/user/20140314/Run0365 GTDATA

# 56Co data ANL
#  rm GTDATA; ln -s /media/r20150911_1334/user/120621a/gtdata/20131222_1539/Run0041 GTDATA
#  rm GTDATA; ln -s /media/r20150911_1334/user/120621a/gtdata/20131222_1539/Run0042 GTDATA
#  rm GTDATA; ln -s /media/r20150911_1334/user/120621a/gtdata/20131222_1539/Run0052 GTDATA
#  rm GTDATA; ln -s /media/r20150911_1334/user/120621a/gtdata/20131222_1539/Run0055 GTDATA
#  rm GTDATA; ln -s /media/r20150911_1334/user/120621a/gtdata/20131222_1539/Run0062 GTDATA

# 137Cs data ANL
#  rm GTDATA; ln -s /media/r20150911_1334/user/120621a/gtdata/20140109_1538/Run0068 GTDATA

# NOTE: remember to use CCcal files in GEBSort.chat and track_GT.chat
#       for these files ^^^^

# sample data with CHICO2 chamber in, Ca source

#  rm GTDATA; ln -s /media/r20150911_1334/user/120621a/gtdata/20140219_1604/Run0302/ GTDATA

# shootout data, 60Co
#  rm GTDATA; ln -s /media/r20150911_1334/user/20140424_1131/gsfma315_GT/Run0298 GTDATA
#  rm GTDATA; ln -s /home/tl/Run0298 GTDATA

# shootout data, 158Er
#   rm GTDATA; ln -s /home/tl/Run0146 GTDATA

# post shootout, ext det data for calibrations
#  rm GTDATA; ln -s /media/20140519_1502/user/gsfma315_GT/extcoindata/Run0072 GTDATA


# shootout data, 166Ho
#  rm GTDATA; ln -s /media/r20150911_1334/user/20140424_1131/gsfma315_GT/Run0239_veto GTDATA
#  rm GTDATA; ln -s /media/r20150911_1334/user/20140424_1131/gsfma315_GT/Run0239 GTDATA

# big MSU 60Co dataset
# rm GTDATA; ln -s /media/r20150911_1334/user/MSU_2015_b/Run0128 GTDATA

# 'standard' dataset, 60Co [http://gretina.lbl.gov/tools-etc/standard-data]
#  rm GTDATA; ln -s ~/d6/gretina/std_data/60Co GTDATA
#  rm GTDATA; ln -s /media/20171129_1623/user/std_data/60Co GTDATA

# 'standard' dataset, 166Ho [http://gretina.lbl.gov/tools-etc/standard-data]
 rm GTDATA; ln -s /home/tl/d6/gretina/std_data/166Ho GTDATA
# rm GTDATA; ln -s /home/tl/tmp2 GTDATA

# agata data (remember to change crystal to world mapping and single hit range)
# rm GTDATA; ln -s /media/r20150911_1334/user/120621a/agata/20140903 GTDATA

#  rm GTDATA; ln -s /media/r20150911_1334/user/120621a/gtdata/dirk_runs/Run0130 GTDATA

#rm GTDATA; ln -s /media/20171129_1623/user/158Er_test GTDATA


   ls -lt GTDATA/
   echo "track GT data mode 2 data (decomposed data) "
   echo -n "GTDATA dir:"; (cd GTDATA; pwd -P)

if [ 1 == 0 ]
then
  echo "generate the veto_cube file"
  echo "working directly on the Global.dat file"

  cat GEBSort.chat | sed 's/;vetospotsFind/vetospotsFind/g' > x.chat
  rm xxxtest.root
  ./GEBSort_nogeb \
    -input disk GTDATA/Global.dat \
    -rootfile xxxtest.root RECREATE \
    -chat x.chat > GTDATA/vetospotsFind.log
  rm xxxtest.root
  ls -lh vetospots.dat GTDATA/vetospotsFind.log
  grep "nbadcubes..." GTDATA/vetospotsFind.log
else
  echo "**** REMOVING vetospots.dat"
  rm vetospots.dat
fi

if [ 1 == 0 ]
then
   echo "./GEBFilter for T0 processing and Veto processing"
  ./GEBFilter GEBFilter.chat GTDATA/Global.dat GTDATA/Global_filtered.dat > GTDATA/GEBFilter.log
  tail GTDATA/GEBFilter.log
  ls -lh GTDATA/Global.dat GTDATA/Global_filtered.dat GTDATA/GEBFilter.log
fi


if [ 1 == 0 ]
then
   echo "./GEBMerge"
   rm GTDATA/merged.gtd*
   ./GEBMerge GEBMerge.chat GTDATA/merged.gtd GTDATA/Global.dat 2> GTDATA/GEBMerge.log
   ls -lh GTDATA/merged.gtd*
fi

if [ 1 == 0 ]
then
   echo "./trackMain"
   rm GTDATA/mode1.gtd
   rm ?,*.list
   ./trackMain \
      track_GT.chat  \
      GTDATA/merged.gtd_000  \
      GTDATA/mode1.gtd 2> GTDATA/trackMain.log
   ls -lh GTDATA/mode1.gtd GTDATA/merged.gtd
fi

if [ 1 == 1 ]
then
  cat GEBSort.chat | sed 's/ndetlimits 2 8/ndetlimits 1 8/g' > x.chat
  grep ndetlimits x.chat
  echo "./GEBSort_nogeb"
  echo -n "GTDATA dir:"; (cd GTDATA; pwd -P)
  rm GTDATA/wsi.root
  ./GEBSort_nogeb \
    -input disk GTDATA/mode1.gtd \
    -rootfile GTDATA/wsi.root RECREATE\
    -chat x.chat > GTDATA/GEBSort_wsi.log
  ls -lt GTDATA/*.root
fi

if [ 1 == 0 ]
then
  cat GEBSort.chat | sed 's/ndetlimits 1 8/ndetlimits 2 8/g' > x.chat
  grep ndetlimits x.chat
  echo "./GEBSort_nogeb"
  rm GTDATA/nsi.root
  ./GEBSort_nogeb \
    -input disk GTDATA/mode1.gtd \
    -rootfile GTDATA/nsi.root RECREATE\
    -chat x.chat > GTDATA/GEBSort_nsi.log

    ls -lth GTDATA/ | grep -v list| head -15
fi

    cp *.list GTDATA/
    cp *.chat GTDATA/
   
#    cp GTDATA/test.root .

# check
  rm x.chat
  grep agata *.chat | grep -v grep | grep -v ~ | grep -v \#
  grep singlehitmaxdepth *.chat | grep -v grep | grep -v ~
  grep multlims *.chat | grep -v grep | grep -v ~
  grep ndetlim *.chat | grep -v grep | grep -v ~
  grep maxevents *.chat | grep -v grep | grep -v ~
  grep nevents *.chat | grep -v grep | grep -v ~
  grep enabled *.chat | grep -v grep | grep -v ~

fi








if [ $1 = "GT_mode2" ]
then

  rm GTDATA; ln -s /media/d20150930_1152/user/101sn GTDATA
  (cd GTDATA; ls -l )

  rm GTDATA/mode2.root
  ./GEBSort_nogeb \
    -input disk GTDATA/GEBMerged_101sn_run281.gtd_000 \
    -rootfile GTDATA/test.root RECREATE\
    -chat GEBSort.chat > GTDATA/GEBSort_mode2.log
  ls -lt GTDATA/*.root

fi









if [ $1 = "check_std_data"  ]
then

# meant to check the reproduction of fig 6 in the NIM3 paper

# 'standard' dataset, 60Co [http://gretina.lbl.gov/tools-etc/standard-data]
#  rm GTDATA; ln -s ~/d6/gretina/std_data/60Co GTDATA
#  rm GTDATA; ln -s /media/20171129_1623/user/std_data/60Co GTDATA

# 'standard' dataset, 166Ho [http://gretina.lbl.gov/tools-etc/standard-data]
 rm GTDATA; ln -s /home/tl/d6/gretina/std_data/166Ho GTDATA
# rm GTDATA; ln -s /home/tl/tmp2 GTDATA

if [ 1 == 1 ]
then
   echo "./GEBMerge"
   rm GTDATA/merged.gtd*
   ./GEBMerge GEBMerge.chat GTDATA/merged.gtd GTDATA/Global.dat 2> GTDATA/GEBMerge.log
   ls -lh GTDATA/merged.gtd*
fi

if [ 1 == 1 ]
then
   echo "./trackMain"
   rm GTDATA/mode1.gtd
   rm ?,*.list
   ./trackMain \
      track_std_60co.chat  \
      GTDATA/merged.gtd_000  \
      GTDATA/mode1.gtd 2> GTDATA/trackMain.log
   ls -lh GTDATA/mode1.gtd GTDATA/merged.gtd*
fi

  echo "./GEBSort_nogeb"
  echo -n "GTDATA dir:"; (cd GTDATA; pwd -P)
  rm GTDATA/wsi.root
  ./GEBSort_nogeb \
    -input disk GTDATA/mode1.gtd \
    -rootfile GTDATA/wsi.root RECREATE\
    -chat GEBSort_std_60co.chat > GTDATA/GEBSort_wsi.log
  ls -lt GTDATA/*.root


fi











if [ $1 = "lr_60Co_bin"  ]
then

  echo "This entry will process data written out "
  echo "directly in mode2 binary format from UCGretina."
  echo "To generate"
  echo "  cd /home/tl/geant4/lr_20140324/examples/60Co"
  echo "  UCGretina co60_mode2.mac"
  echo "The data is processed but there does not "
  echo "seem to be any energy smearing"

  #rm GTDATA; ln -s /media/r20150911_1334/user/120621a/gtdata/G4_US/co60_coin/ GTDATA

  #rm GTDATA; ln -s /media/r20150911_1334/user/120621a/gtdata/G4_US/co60_wuchamber GTDATA

  rm GTDATA; ln -s /media/r20150911_1334/user/120621a/gtdata/G4_US/co60_mode2/ GTDATA

  # --- are coincidence sources

  export DF="co60.out"

  echo "tracking the mode2 data from G4toMode2"
   ./trackMain track_G4.chat  GTDATA//$DF  GTDATA/g4_mode1.out      > GTDATA/trackMain.log

  echo "GEBSort the mode1/mode2 data"

  rm GTDATA/test.root
  ./GEBSort_nogeb \
    -input disk GTDATA/g4_mode1.out \
    -chat GEBSort.chat \
    -rootfile GTDATA/test.root RECREATE > GTDATA/GEBSort.log
   cp *.chat GTDATA/

   ls -lt GTDATA/

fi 













if [ $1 = "lr_60Co_ascii"  ]
then

# to generate
# cd /export/home/data1/tl/geant4_LR10_64bit/lr_v3.3/examples/co60
# . ../../../geant4.9.6.p04/geant4.9.6-build/geant4make.sh
# UCGretina co60_gammas.mac
 
  #rm GTDATA; ln -s /media/r20150911_1334/user/120621a/gtdata/G4_US/co60_ascii/ GTDATA
  #rm GTDATA; ln -s /media/r20150911_1334/user/120621a/gtdata/G4_US/co60_d/ GTDATA
  #rm GTDATA; ln -s /media/r20150911_1334/user/120621a/gtdata/G4_US/one_energy GTDATA
  rm GTDATA; ln -s /home/tl/tmp  GTDATA

  # new version with/without GT chamber, true coincidence

  echo "cd /home/tl/geant4/lr_20140715/examples/60Co"
  echo "UCGretina co60_ascii.mac"
#  rm GTDATA; ln -s /media/r20150911_1334/user/120621a/gtdata/G4_US/co60_wuchamber GTDATA
#  rm GTDATA; ln -s /media/r20150911_1334/user/120621a/gtdata/G4_US/co60_nochamber GTDATA

  
  ls -lt GTDATA/g4.out
  

  echo "running G4toMode2; US version, file GTDATA/g4.out"

  rm GTDATA/g4_mode2.out
  ./G4toMode2_USa_3 \
     GTDATA/g4_source.dat \
     GTDATA/g4_mode2.out \
     G4toMode2.chat \
     200000 > GTDATA/G4toMode2.log

   ls -lh GTDATA/g4_mode2.out

  echo "tracking the mode2 data from G4toMode2"
  ./trackMain track_G4.chat \
     GTDATA/g4_mode2.out \
     GTDATA/g4_mode1.out 2> GTDATA/trackMain.log

  echo "GEBSort the mode1/mode2 data"

  rm GTDATA/test.root
  ./GEBSort_nogeb \
    -input disk GTDATA/g4_mode1.out \
    -chat GEBSort.chat \
    -rootfile GTDATA/test.root RECREATE > GTDATA/GEBSort.log

  echo "documenting"

  cp *.chat GTDATA/

  ls -lt GTDATA/

fi 














if [ $1 = "lr_shootout_ascii"  ]
then

# still using old UCGretina so format is '1'

# (cd /home/tl/geant4/lr_20140324/examples/92Mo; UCGretina 92mo_ascii.mac > 92mo_ascii.log)
# NOTE: use 'beta 0.083' in GEBSort.chat
#  rm GTDATA; ln -s /media/r20150911_1334/user/120621a/gtdata/G4_US/92mo_1 GTDATA
#  export DF="92mo.dat"

# (cd /home/tl/geant4/lr_20140324/examples/158Er; UCGretina 158er_ascii.mac )
# NOTE: use 'beta 0.019' in GEBSort.chat
  rm GTDATA; ln -s /media/r20150911_1334/user/120621a/gtdata/G4_US/158er_1 GTDATA
  export DF="158er.dat"

  rm GTDATA; ln -s /media/r20150911_1334/user/G4_data/ GTDATA
  export DF="158er.dat"

  ls -lt GTDATA/$DF

  echo "running G4toMode2; US version, dir/file `(cd GTDATA; pwd -P; ls $DF)`"

  rm GTDATA/g4_mode2.out
  ./G4toMode2_USa_2 \
     GTDATA/$DF \
     GTDATA/g4_mode2.out \
     G4toMode2.chat \
     2000000000 > GTDATA/G4toMode2.log
   ls -lh GTDATA/g4_mode2.out

  echo "tracking the mode2 data from G4toMode2"
   ./trackMain track_G4.chat  GTDATA/g4_mode2.out  GTDATA/g4_mode1.out      > GTDATA/trackMain.log

  echo "GEBSort the mode1/mode2 data"

  rm GTDATA/test.root
  ./GEBSort_nogeb \
    -input disk GTDATA/g4_mode1.out \
    -chat GEBSort.chat \
    -rootfile GTDATA/test.root RECREATE > GTDATA/GEBSort.log
  cp *.chat GTDATA/
  cp sumTrackE.spe GTDATA/
  ls -lt GTDATA/

fi 


if [ $1 = "G4_EU"   ]
then

  echo "need to document how this data was made"

#  rm GTDATA; ln -s /media/r20150911_1334/user/120621a/gtdata/G4Co60_amel_1 GTDATA
#  export DF=Gretina-NSCL-co60-rot70deg.dat
#  rm GTDATA; ln -s /home/tl/tmp4 GTDATA
#  export DF=gretina-cristal-coord-geant4.dat

# rm GTDATA; ln -s /media/r20150911_1334/user/120621a/agata/2015_g4_sim GTDATA
# export DF=test_g4.out
 rm GTDATA; ln -s /media/r20150911_1334/user/sim_oct_15 GTDATA
# export DF=AG_G4_24crys_nom.dat
 export DF=AG_G4_180crys_nom.dat
# export DF=GT_G4_28crys_LewWalls_noChamber.dat
# export DF=GT_G4_120crys_LewWalls_noChamber.dat

  ls -lt GTDATA/$DF
  echo "running G4toMode2; EU version"

  rm G4toMode2_*.txt
  rm core.*
  rm GTDATA/g4_mode2.out
  ./G4toMode2_EU \
     GTDATA/$DF \
     GTDATA/g4_mode2.out \
     G4toMode2.chat 100000 > GTDATA/G4toMode2.log
   ls -lh GTDATA/g4_mode2.out

  echo "tracking the mode2 data from G4toMode2"
   ./trackMain track_G4.chat  GTDATA/g4_mode2.out  GTDATA/g4_mode1.out      > GTDATA/trackMain.log

  echo "GEBSort the mode1/mode2 data"

  rm GTDATA/test.root
  ./GEBSort_nogeb \
    -input disk GTDATA/g4_mode1.out \
    -rootfile GTDATA/test.root RECREATE \
    -chat GEBSort.chat > GTDATA/GEBSort.log
  cp *.chat GTDATA/
  ls -lt GTDATA/

fi 

if [ $1 = "example4"  ]
then

  echo "G4 data examples, these data came from Amel"
  echo "needs ;noworldtocrystalrot, NO! automatic now!"
  echo "needs v/c=0"

# rm GTDATA; ln -s ../data1/Geant4-data GTDATA
# rm GTDATA; ln -s /media/r20150911_1334/user/120621a/gtdata/amel_1 GTDATA
# rm GTDATA; ln -s /media/r20150911_1334/user/120621a/gtdata/amel_2 GTDATA
# rm GTDATA; ln -s /media/r20150911_1334/user/120621a/gtdata/amel_1_b GTDATA
# rm GTDATA; ln -s /home/tl/tmp GTDATA
 rm GTDATA; ln -s /media/49EF-44C4 GTDATA

   # data files, these work

  #export DF=gretina-ANL1-500k-co60.dat
  #export DF=gretina-ANL2-500k-co60.dat
  #export DF=gretina-ANL2plus-500k-co60.dat
  #export DF=gretina-NSCL-500k-co60.dat
  #export DF=Gretina-NSCL-co60-rot20deg.dat
  #export DF=data-check.dat
  export DF=GammaEvents.0001

  # data files, these do NOT work because it is shell data, no det numbers

  #export DF=g4.dat 

  ls -lt GTDATA/$DF

  echo "running G4toMode2; EU version on $DF"

  rm GTDATA/g4_mode2.out
  ./G4toMode2_EU \
     GTDATA/$DF \
     GTDATA/g4_mode2.out \
     G4toMode2.chat \
     500000 > GTDATA/G4toMode2.log
   mv G4toMode2_*.txt GTDATA/
   ls -lh GTDATA/g4_mode2.out

  echo "tracking the mode2 data from G4toMode2"
   ./trackMain track_G4.chat  GTDATA/g4_mode2.out  GTDATA/g4_mode1.out > GTDATA/trackMain.log
  mv *.list  GTDATA/

  echo "GEBSort the mode1/mode2 data"

  rm GTDATA/test.root
  ./GEBSort_nogeb \
    -input disk GTDATA/g4_mode1.out \
    -rootfile GTDATA/test.root RECREATE \
    -chat GEBSort.chat > GTDATA/GEBSort.log
  cp *.chat GTDATA/
  ls -lt GTDATA/

fi 

if [ $1 = "ANL_1"  ]
then

  echo "G4 data examples, these data came from Amel"
  echo "needs ;noworldtocrystalrot, no automatic now!"
  echo "needs v/c=0"

  rm GTDATA; ln -s /media/r20150911_1334/user/120621a/gtdata/G4Co60_ANL_1 GTDATA
   
  # data files, these work

  #export DF=ANL1-co60.dat
  #export DF=ANL1-rot-75_95-200k.dat
  #export DF=ANL2-co60.dat
  export DF=ANL2-rot-75_95-200k.dat
  #export DF=ANL3-co60.dat
  #export DF=ANL3-rot-75_95-200k.dat

  ls -lt GTDATA/$DF

  echo "running G4toMode2; EU version on $DF"

  rm GTDATA/g4_mode2.out
  ./G4toMode2_EU \
     GTDATA/$DF \
     GTDATA/g4_mode2.out \
     G4toMode2.chat \
     500000 > GTDATA/G4toMode2.log
   ls -lh GTDATA/g4_mode2.out

  echo "tracking the mode2 data from G4toMode2"
   ./trackMain track_G4.chat  GTDATA/g4_mode2.out  GTDATA/g4_mode1.out > GTDATA/trackMain.log

  echo "GEBSort the mode1/mode2 data"

  rm GTDATA/test.root
  ./GEBSort_nogeb \
    -input disk GTDATA/g4_mode1.out \
    -rootfile GTDATA/test.root RECREATE \
    -chat GEBSort.chat > GTDATA/GEBSort.log
  cp *.chat GTDATA/
  ls -lt GTDATA/

fi 

if [ $1 = "helios" ]
then

#  rm GTDATA; ln -s /export/home/data1/tmp/digios_2017_taste GTDATA
  rm GTDATA; ln -s ~/tmp GTDATA
  ls GTDATA/

# merge the data

  ./GEBMerge GEBMerge.chat GTDATA/merged.gtd GTDATA/h054_run_290* > GTDATA/GEBMerge.log
  ls -l GTDATA/merged.gtd*

# gebsort the data

   echo "** remember to enable bin_dgs **"
   rm GTDATA/test.root
  ./GEBSort_nogeb \
    -input disk GTDATA/merged.gtd_000 \
    -rootfile GTDATA/test.root RECREATE \
    -chat GEBSort.chat > GTDATA/GEBSort.log

fi

if [ $1 = "dgs" ]
then

   echo "dgs data in GEB header/payload format; Sep 2013 firmware format"
   echo ""

#   rm GTDATA; ln -s /media/r20150911_1334/user/120621a/dgsdata/20130914_1232/ GTDATA
#   export DF=z7_001_0127
#   rm GTDATA; ln -s /home/tl/d6/gtreceiver/cur/ GTDATA
#   export DF=test_4.gtd_000_0109
#   export DF=test.gtd_000_0109

# fatima data Jan/2016

#  rm GTDATA; ln -s /export/home/data1/tmp/fatima_taste GTDATA
#  export DF=GEBMerged_run183.gtd_000

#gsfma353 data

#  rm GTDATA; ln -s /export/home/data1/tmp/gsfma353_taste GTDATA
#  export DF=GEBMerged_runY88.gtd_000

# Daryls 2018 deep inelastic run1

#  rm GTDATA; ln -s /media/20180228_1436/user/gsfma355/5 GTDATA

# Daryls 2018 deep inelastic run2

  rm GTDATA; ln -s /media/d20150930_1152/user/gsfma356/2 GTDATA

# merge the data, dont catch the .save file by accident

  rm GTDATA/merged*
  GEBMerge GEBMerge.chat GTDATA/merged `ls  GTDATA/run_*.gtd*` 2> GTDATA/merge.log

  # make the map file, Dont, use the one from Mike

#  ./mkMap > map.dat

   rm core.*
   rm GTDATA/test.root
  ./GEBSort_nogeb \
    -input disk GTDATA/merged_000 \
    -rootfile GTDATA/test.root RECREATE \
    -chat GEBSort.chat > GTDATA/GEBSort.log
#   cp *.chat GTDATA/
#    ls -lt GTDATA/

fi




















if [ $1 = "lr" ]
then

   echo ""
   echo ""

   rm GTDATA; ln -s /media/r20150911_1334/user/120621a/gtdata/G4_20130917_1256 GTDATA
   ls -lt GTDATA/

   rm GTDATA/test.root
  ./GEBSort_nogeb \
    -input disk GTDATA/g4_mode2.dat \
    -rootfile GTDATA/test.root RECREATE \
    -chat GEBSort.chat > GTDATA/GEBSort.log
    cp *.chat GTDATA/
    ls -lt GTDATA/

fi

if [ $1 = "merge" ]
then

   echo "merge DGS data files and sort with GEBSort"
   echo ""

   rm GTDATA; ln -s /media/r20150911_1334/user/120621a/dgsdata/20130914_1232/ GTDATA

   echo "GEBMerge"
   ./GEBMerge GEBMerge.chat GTDATA/merged.gtd GTDATA/z* 
#> GTDATA/GEBMerge.log

   echo "GEBsort"
   ./GEBSort_nogeb \
     -input disk GTDATA/merged.gtd_000 \
     -rootfile GTDATA/test.root RECREATE \
     -chat GEBSort.chat  > GTDATA/GEBSort.log
   cp *.chat GTDATA/
   ls -lt GTDATA/*

fi

if [ $1 = "ER1" ]
then

echo "sort the old ER01 data from 4/2011 from LBL"

rm GTDATA; ln -s /media/LIN0031205/user/gtdata/ER1/Run056 GTDATA
ls GTDATA/
ls -l GTDATA/Global.dat

echo "this does not work yet, format_1_3.c need development first"
echo "and the data is really odd, logbook Mon Oct 14 09:37:48"

  format_1_3 GTDATA/Global.dat Global_mod.dat

  ./GEBSort \
     -input disk Global_mod \
     -rootfile GTDATA/test.root RECREATE \
     -chat GEBSort.chat 
   cp *.chat GTDATA/
fi


if [ $1 = "G4_one_energy"  ]
then  ls -lt GTDATA

  echo" generate as:"

  echo "cd /home/tl/geant4/v1.3.0"
  echo ". ../geant4.9.6.p04/geant4.9.6-build/geant4make.sh"
  echo "cd examples/one_energy"
  echo "ge one_energy.lvl --- change the enery to what you want"
  echo "UCGretina one_energy.mac.ascii"
  echo "^^^^ specify in mac file where you want to write the data"


  rm GTDATA; ln -s /media/r20150911_1334/user/120621a/gtdata/G4_US/one_energy/ GTDATA
  ls -lt GTDATA/
  export DF=335.dat
  ls -lt GTDATA/$DF

  echo "running G4toMode2; US version, file GTDATA/$DF"

  rm GTDATA/g4_mode2.out
  ./G4toMode2_USa_2 \
     GTDATA/$DF \
     GTDATA/g4_mode2.out \
     G4toMode2.chat \
     2000000000 > GTDATA/G4toMode2.log
   ls -lh GTDATA/g4_mode2.out

  echo "tracking the mode2 data from G4toMode2"
   ./trackMain track_G4.chat  GTDATA/g4_mode2.out  GTDATA/g4_mode1.out      2> GTDATA/trackMain.log
   ls -l GTDATA/g4_mode1.out

  echo "GEBSort the mode1/mode2 data"

  rm GTDATA/test.root
  ./GEBSort_nogeb \
    -input disk GTDATA/g4_mode1.out \
    -chat GEBSort.chat \
    -rootfile GTDATA/335.root RECREATE > GTDATA/GEBSort.log
  cp *.chat GTDATA/
  ls -lt GTDATA/

fi 

if [ $1 = "lr_superdef"  ]
then 

  echo "generate as:"

  echo " cd /export/home/data1/tl/geant4_LR10_32bit-SL62/lr_20131118/examples/cascades"
  echo ". ../../../geant4.9.6.p01/geant4.9.6-build/geant4make.sh"
  echo "UCGretina superDef.mac"
  echo "mv superDef.dat /media/r20150911_1334/user/120621a/gtdata/G4_US/superdef1"


  rm GTDATA; ln -s /media/r20150911_1334/user/120621a/gtdata/G4_US/superdef1/ GTDATA
  ls -lt GTDATA/
  export DF=superDef.dat
  ls -lt GTDATA/$DF

  rm GTDATA/g4_mode2.out
  rm GTDATA/g4_mode1.out
  rm GTDATA/*.log
  rm GTDATA/*.root

  echo "running G4toMode2; US version, file GTDATA/$DF"

  ./G4toMode2_US \
     GTDATA/$DF \
     GTDATA/g4_mode2.out \
     G4toMode2.chat \
     100000 > GTDATA/G4toMode2.log
   ls -lh GTDATA/$DF GTDATA/g4_mode2.out GTDATA/G4toMode2.log

  echo "tracking the mode2 data from G4toMode2"
   ./trackMain track_G4.chat  GTDATA/g4_mode2.out  GTDATA/g4_mode1.out      > GTDATA/trackMain.log

  echo "GEBSort the mode1/mode2 data"

  ./GEBSort_nogeb \
    -input disk GTDATA/g4_mode1.out \
    -chat GEBSort.chat \
    -rootfile GTDATA/test.root RECREATE > GTDATA/GEBSort.log

  ls -lt GTDATA/

fi 

if [ $1 = "genQC"  ]
then 

 rm GTDATA; ln -s /media/r20150911_1334/user/120621a/gtdata/G4_US/genQC/ GTDATA
 ls -lt GTDATA/

 rm GTDATA/test.root
  ./GEBSort_nogeb \
    -input disk GTDATA/genQC_mode1.dat \
    -chat GEBSort.chat \
    -rootfile GTDATA/test.root RECREATE 
#> GTDATA/GEBSort.log
   cp *.chat GTDATA/
  ls -lt GTDATA/

fi

if [ $1 = "example2a"  ]
then

  echo "for completeness, this data was generate as:"


  rm GTDATA; ln -s /media/r20150911_1334/user/120621a/gtdata/G4_US/eu152_b GTDATA
  ls -lt GTDATA/
  export DF=eu152.dat
  ls -lt GTDATA/$DF

  echo "running G4toMode2; US version, file GTDATA/$DF"

  rm GTDATA/g4_mode2.out
  ./G4toMode2_US \
     GTDATA/$DF \
     GTDATA/g4_mode2.out \
     G4toMode2.chat \
     1000000 > GTDATA/G4toMode2.log
   ls -lh GTDATA/g4_mode2.out

  echo "tracking the mode2 data from G4toMode2"
   ./trackMain track_G4.chat  GTDATA/g4_mode2.out  GTDATA/g4_mode1.out > GTDATA/trackMain.log

  echo "GEBSort the mode1/mode2 data"

  rm GTDATA/test.root
  ./GEBSort_nogeb \
    -input disk GTDATA/g4_mode1.out \
    -chat GEBSort.chat \
    -rootfile GTDATA/test.root RECREATE > GTDATA/GEBSort.log
  cp *.chat GTDATA/
  ls -lt GTDATA/

fi 

if [ $1 = "158er"  ]
then 

  echo "generate as:"

  echo " cd /export/home/data1/tl/geant4_LR10_32bit-SL62/lr_20131118/examples/cascades"
  echo ". ../../../geant4.9.6.p01/geant4.9.6-build/geant4make.sh"
  echo "UCGretina 158Er.mac"

  rm GTDATA; ln -s /media/r20150911_1334/user/120621a/gtdata/G4_US/158Er_1 GTDATA
  ls -lt GTDATA/
  export DF=158Er.dat
  ls -lt GTDATA/$DF

  rm GTDATA/g4_mode2.out
  rm GTDATA/g4_mode1.out
  rm GTDATA/*.log
  rm GTDATA/*.root
  rm TS.list

  echo "running G4toMode2; US version, file GTDATA/$DF"

  ./G4toMode2_US \
     GTDATA/$DF \
     GTDATA/g4_mode2.out \
     G4toMode2.chat \
     100000 > GTDATA/G4toMode2.log
   ls -lh GTDATA/$DF GTDATA/g4_mode2.out GTDATA/G4toMode2.log

  echo "tracking the mode2 data from G4toMode2"
   ./trackMain track_G4.chat  GTDATA/g4_mode2.out  GTDATA/g4_mode1.out      > GTDATA/trackMain.log

  echo "GEBSort the mode1/mode2 data"

  ./GEBSort_nogeb \
    -input disk GTDATA/g4_mode1.out \
    -chat GEBSort.chat \
    -rootfile GTDATA/test.root RECREATE > GTDATA/GEBSort.log
  cp *.chat GTDATA/
  ls -lt GTDATA/

fi 

if [ $1 = "lr_166ho_bin"  ]
then

  echo "generate as: (with smearing)"

  echo "cd /home/tl/geant4/lr_20140715/examples/ho166"
  echo "setupg4"
  echo "go"

  rm GTDATA; ln -s /media/r20150911_1334/user/120621a/gtdata/G4_US/166ho_b GTDATA
  export DF=ho166_smear.dat
  ls -lt GTDATA/$DF

  ls -lt GTDATA/

  rm GTDATA/g4_mode2.out
  rm GTDATA/g4_mode1.out
  rm GTDATA/*.log
  rm GTDATA/*.root
  rm TS.list

  echo "tracking the mode2 data from G4toMode2"
   ./trackMain track_G4.chat  GTDATA/$DF  GTDATA/g4_mode1.out      > GTDATA/trackMain.log

  echo "GEBSort the mode1/mode2 data"

  ./GEBSort_nogeb \
    -input disk GTDATA/g4_mode1.out \
    -chat GEBSort.chat \
    -rootfile GTDATA/test.root RECREATE > GTDATA/GEBSort.log
  cp *.chat GTDATA/
  ls -lt GTDATA/

fi 

if [ $1 = "lr_166ho_ascii"  ]
then

  echo "generate as"

  echo "cd /home/tl/geant4/lr_20140715/examples/ho166"
  echo "setupg4"
  echo "go"

#  rm GTDATA; ln -s /media/r20150911_1334/user/120621a/gtdata/G4_US/166ho_c GTDATA
#  rm GTDATA; ln -s /media/r20150911_1334/user/166Ho/G4 GTDATA
  rm GTDATA; ln -s /media/r20150911_1334/user/unfold/166ho/ GTDATA
#  export DF=ho166.ascii
#  export DF=ho166.dat.ascii
  export DF=g4_j.dat

  ls -lt GTDATA/$DF

  ls -lt GTDATA/

  rm GTDATA/g4_mode2.out
  rm GTDATA/g4_mode1.out
  rm GTDATA/*.log
#  rm GTDATA/*.root
  rm TS.list

  echo "running G4toMode2; US version, file GTDATA/$DF"

  ./G4toMode2_USa_2 \
     GTDATA/$DF \
     GTDATA/g4_mode2.out \
     G4toMode2.chat \
     2000000000 > GTDATA/G4toMode2.log
   ls -lh GTDATA/$DF GTDATA/g4_mode2.out  GTDATA/G4toMode2.log

  echo "tracking the mode2 data from G4toMode2"
   ./trackMain track_GT.chat  GTDATA/g4_mode2.out  GTDATA/g4_mode1.out 2> GTDATA/trackMain.log

  echo "GEBSort the mode1/mode2 data"

  ./GEBSort_nogeb \
    -input disk GTDATA/g4_mode1.out \
    -chat GEBSort.chat \
    -rootfile GTDATA/test_j.root RECREATE > GTDATA/GEBSort.log
  cp *.chat GTDATA/
  ls -lt GTDATA/

fi 

if [ $1 = "HK_rsp" ]
then



  rm GTDATA; ln -s /media/r20150911_1334/user/20140424_1131/gsfma315_GT/Run0240 GTDATA

  rm MM*
  GEBbunch GTDATA/merged.gtd_000 MM 30 1831 1840 
  ls -l MM*

  GEBSort_nogeb \
    -input disk MM10.dat \
    -rootfile GTDATA/test.root RECREATE\
    -chat GEBSort.chat > GEBSort.log

fi

exit

