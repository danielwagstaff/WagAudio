#!/bin/bash

pldir=$1
outputfile=$2

declare -a filenames

nFiles=$(ls "$pldir" | wc -l) #The number of files in the directory

for i in $(seq 1 $nFiles)
do
    filenames[$i]=$(ls "$pldir" | head -$i | tail -1)
    echo $pldir${filenames[$i]} >> ./$outputfile.pl
done
