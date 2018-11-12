

LANG1=$1
LANG2=$2
#echo "LANG1=$LANG1"
#echo "LANG2=$LANG2"

MOSES=~/permanent/software/mosesdecoder
MARIAN=~/permanent/software/marian-dev
AMUN=~/permanent/software/amun

SUBWORD_NMT=~/permanent/software/subword-nmt

TC_MODEL=~/permanent/en-fr/nmt-dir/model/truecaser/truecase-model.en
BPE_MODEL=~/permanent/en-fr/nmt-dir/model/vocab.enfr
MARIAN_MODEL=~/permanent/data/fr-en/model/marian/model.npz.decoder.yml
#AMUN_MODEL=~/permanent/data/fr-en/model/marian/model.npz.amun.yml

sudo -n nvidia-smi -c 1

#tee in |
LC_ALL=C $MOSES/scripts/tokenizer/tokenizer.perl -l $LANG1 |
LC_ALL=C $MOSES/scripts/recaser/truecase.perl -model $TC_MODEL |
$SUBWORD_NMT/subword_nmt/apply_bpe.py -c $BPE_MODEL |
#tee in.2 |
$MARIAN/build/marian-decoder -c $MARIAN_MODEL --beam-size 1 --mini-batch 1 --maxi-batch 1 --max-length-crop --max-length 50 |
#$AMUN/build/amun -c $AMUN_MODEL --beam-size 1 --mini-batch 1 --maxi-batch 1 --max-length 50 --cpu-threads 4 |
#tee out |
sed -r 's/(@@ )|(@@ ?$)//g' |
$MOSES/scripts/recaser/detruecase.perl |
$MOSES/scripts/tokenizer/detokenizer.perl -l $LANG2 #|
#tee out.2



