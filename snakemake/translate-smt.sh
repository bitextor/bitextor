#!/usr/bin/env bash

LANG1=$1
MOSES=$2
smtDir=$3

TRUECASE_MODEL=$smtDir/truecaser/truecase-model.1.$LANG1
MOSES_MODEL=$smtDir/tuning/moses.tuned.ini.1

cat /dev/stdin |
  $MOSES/scripts/tokenizer/tokenizer.perl -a -l $LANG1 |
  $MOSES/scripts/recaser/truecase.perl -model $TRUECASE_MODEL |
  $MOSES/bin/moses -f $MOSES_MODEL --threads all -v 0 -search-algorithm 1 -cube-pruning-pop-limit 1000 |
  $MOSES/scripts/recaser/detruecase.perl |
  $MOSES/scripts/tokenizer/detokenizer.perl

