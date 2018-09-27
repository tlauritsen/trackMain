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

if [ $1 = "GT_standard" ]
then

# 'standard' dataset, 166Ho [http://gretina.lbl.gov/tools-etc/standard-data]
 rm GTDATA; ln -s /home/tl/d6/gretina/std_data/166Ho GTDATA

if [ 1 == 1 ]
then
   echo "./trackMain"
   rm GTDATA/mode1.gtd
   rm ?,*.list
   ./trackMain \
      track_GT.chat  \
      GTDATA/Global.dat  \
      GTDATA/mode1.gtd 2> GTDATA/trackMain.log
   ls -lh GTDATA/mode1.gtd GTDATA/merged.gtd_000
fi

if [ 1 == 1 ]
then
  echo "GEBSort_nogeb"
  echo -n "GTDATA dir:"; (cd GTDATA; pwd -P)
  rm GTDATA/wsi.root
  ../../cur/GEBSort_nogeb \
    -input disk GTDATA/mode1.gtd \
    -rootfile GTDATA/wsi.root RECREATE \
    -chat ../../cur/GEBSort.chat > GTDATA/GEBSort_wsi.log
  ls -lt GTDATA/*.root
fi

    mv *.list GTDATA/
    cp *.chat GTDATA/

# to display 
# rootn.exe
#   .L ../../cur/GSUtil_cc.so
#   .x ../../cur/bar.cc
   
# check

  grep agata *.chat | grep -v grep | grep -v ~ | grep -v \#
  grep singlehitmaxdepth *.chat | grep -v grep | grep -v ~
  grep multlims *.chat | grep -v grep | grep -v ~
  grep ndetlim *.chat | grep -v grep | grep -v ~
  grep maxevents *.chat | grep -v grep | grep -v ~
  grep nevents *.chat | grep -v grep | grep -v ~
  grep enabled *.chat | grep -v grep | grep -v ~

fi

if [ $1 = "agata" ]
then

   echo "convert AGATA data to GT mode2"
   echo "then track and analyze the data"

   rm GTDATA; ln -s ...somewhere... GTDATA

   echo "agata to GT mode2 (standard name of Global.dat)"

   rm GTDATA/Global.dat
   ag2mode2 GTDATA/agata.adf GTDATA/Global.dat 0 20000000 100 > GTDATA/ag2mode2.log
   mv *.agevent GTDATA/

   echo "track GT data mode 2 data (decomposed data), experimental data "
   echo -n "GTDATA dir:"; (cd GTDATA; pwd -P)

   echo "./trackMain"
   rm GTDATA/mode1.gtd
   ./trackMain \
      track_GT.chat  \
      GTDATA/Global.dat  \
      GTDATA/mode1.gtd > GTDATA/trackMain.log
   ls -lt GTDATA/merged.gtd* GTDATA/mode1.gtd

if [ 1 == 0 ]
then
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

fi



