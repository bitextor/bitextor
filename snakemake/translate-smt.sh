#!/usr/bin/env bash

MOSES=~/workspace/github/mosesdecoder
LANG1=fr
TRUECASE_MODEL=/home/hieu/transient/fr-en/smt-dir/truecaser/truecase-model.1.fr
MOSES_MODEL=/home/hieu/transient/fr-en/smt-dir/tuning/moses.tuned.ini.1

cat /dev/stdin |
  $MOSES/scripts/tokenizer/tokenizer.perl -a -l $LANG1 |
  $MOSES/scripts/recaser/truecase.perl -model $TRUECASE_MODEL |
  $MOSES/bin/moses -f $MOSES_MODEL --threads all |
  $MOSES/scripts/recaser/detruecase.perl |
  $MOSES/scripts/tokenizer/detokenizer.perl

