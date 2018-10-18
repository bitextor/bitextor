#!__BASH__

if [[ -z $TMPDIR ]]; then
  TMPDIR="/tmp"
fi

corpus=`mktemp $TMPDIR/input.XXXXXX`
intermediatefile=`mktemp $TMPDIR/input.tok.truecased.XXXXXX`
ngram_order=5
model=$1
LANG1=$2
LANG2=$3
langfr=""

cat - > $corpus

cut -f 3 $corpus > $corpus.$LANG1
cut -f 4 $corpus > $corpus.$LANG2

for lang in $LANG1 $LANG2; do
  perl $(dirname $0)/../share/moses/tokenizer/tokenizer.perl -l $lang -threads 16 < $corpus.$lang 2>/dev/null | truecase --model $model/truecase-model.$lang 2>/dev/null | awk '{printf("<s> %s </s>\n", $0)}' >  $intermediatefile.$lang
  
  vocab="$model/vocab.$lang"
  map_unk=`tail -n 1 $vocab | sed "s/.$//g"`
 
  if [ ! -f $model/bin.lm.$lang ]; then
      build_binary $model/lm.$lang $model/bin.lm.$lang
  fi

#  ngram -map-unk $map_unk -lm $model/lm.$lang -order $ngram_order -ppl $intermediatefile.$lang -debug 1 2>&1 | egrep "(logprob.*ppl.*ppl1=)|( too many words per sentence)" | head -n -1 | awk '{print log($6)}' > $intermediatefile.ngram.$lang


cat $intermediatefile.$lang | awk -v v=$vocab -v u=$map_unk 'BEGIN{while((getline<v)>0) m[$1]=1;}{for(i=1;i<=NF;i++) {w=$i; if(m[w] !=1) w=u; printf("%s ", w)}; print""}' | query -v sentence  $model/bin.lm.$lang | grep ^Total | awk '{print -$2}' > $intermediatefile.ngram.$lang

  if [ "$lang" != "en" ]; then
    langfr="$lang"
  fi

done


f2e="$model/dict.$LANG1-$LANG2"
e2f="$model/dict.$LANG2-$LANG1"

$(dirname $0)/../share/bitextor/zipporah/generate-bow-xent $f2e $intermediatefile.ngram.$LANG1 $intermediatefile.ngram.$LANG2 0.0001 1 > $intermediatefile.tm.$LANG1-$LANG2 2>/dev/null &
$(dirname $0)/../share/bitextor/zipporah/generate-bow-xent $e2f $intermediatefile.ngram.$LANG2 $intermediatefile.ngram.$LANG1 0.0001 1 > $intermediatefile.tm.$LANG2-$LANG1 2>/dev/null &

wait
paste $intermediatefile.tm.$LANG1-$LANG2 $intermediatefile.tm.$LANG2-$LANG1 $intermediatefile.ngram.$LANG1 $intermediatefile.ngram.$LANG1 | awk '{print ($1)+($2),"\t",($3)+($4)}' | awk '{a=$1/10;b=$2/10;print a^8,b^8}' > $intermediatefile.feats.txt

python $(dirname $0)/../share/bitextor/zipporah/apply_logistic.py $model/model.$langfr $intermediatefile.feats.txt $intermediatefile.zipporah

paste $corpus $intermediatefile.zipporah
rm -rf $intermediatefile* $corpus*
