

MOSES=/home/hieu/workspace/github/mosesdecoder
MARIAN=/home/hieu/workspace/github/marian-dev
SUBWORD_NMT=/home/hieu/workspace/github/subword-nmt

LANG=en
TC_MODEL=~/permanent/en-fr/nmt-dir/model/truecaser/truecase-model.en
BPE_MODEL=~/permanent/en-fr/nmt-dir/model/vocab.enfr
MARIAN_MODEL=~/permanent/data/fr-en/marian/model.npz.decoder.yml

$MOSES/scripts/tokenizer/tokenizer.perl -l $LANG | $MOSES/scripts/recaser/truecase.perl -model $TC_MODEL | $SUBWORD_NMT/subword_nmt/apply_bpe.py -c $BPE_MODEL | $MARIAN/build/marian_decoder -c $MARIAN_MODEL

