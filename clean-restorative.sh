#!/bin/bash

SL=$1
TL=$2

INPUT_FILE=$(mktemp)

cat > $INPUT_FILE
mkdir ${INPUT_FILE}.d

for((i=1;i<=4;i++))
do cut -f$i $INPUT_FILE > ${INPUT_FILE}.d/f$i
done
cut -f 5- $INPUT_FILE > ${INPUT_FILE}.d/f5

restorative-cleaning/restorative-cleaning.sh -s $SL -t $TL -f ${INPUT_FILE}.d/f3 \
                     -l ${INPUT_FILE}.d/f4 -o ${INPUT_FILE}.od &>/dev/null


paste ${INPUT_FILE}.d/f1 ${INPUT_FILE}.d/f2 \
      ${INPUT_FILE}.od/clean-corpus.${SL} \
      ${INPUT_FILE}.od/clean-corpus.${TL} \
      ${INPUT_FILE}.d/f5


rm -Rf $INPUT_FILE ${INPUT_FILE}.d ${INPUT_FILE}.od
