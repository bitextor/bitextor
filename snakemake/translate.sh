

MOSES=/home/hieu/workspace/github/mosesdecoder
MARIAN=/home/hieu/workspace/github/marian-dev
SUBWORD_NMT=/home/hieu/workspace/github/subword-nmt

LANG=en
TC_MODEL=~/permanent/en-fr/nmt-dir/model/truecaser/truecase-model.en
BPE_MODEL=~/permanent/en-fr/nmt-dir/model/vocab.enfr
MARIAN_MODEL=~/permanent/data/fr-en/model/marian/model.npz.decoder.yml

tee in |
 LC_ALL=C $MOSES/scripts/tokenizer/tokenizer.perl -l $LANG |
 LC_ALL=C $MOSES/scripts/recaser/truecase.perl -model $TC_MODEL |
 $SUBWORD_NMT/subword_nmt/apply_bpe.py -c $BPE_MODEL | tee in.2 |
 $MARIAN/build/marian-decoder -c $MARIAN_MODEL --beam-size 1 --mini-batch 1 --maxi-batch 1 --max-length-crop --max-length 50 |
 tee out



