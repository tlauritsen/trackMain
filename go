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

    cp *.list GTDATA/
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


