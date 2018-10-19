#!__BASH__

OUTPUT=/dev/stdout

exit_program()
{
  echo "USAGE: $1 [-t tmp-dir] SL TL SL_FILE TL_FILE DIC [ PREPROCESSED_CORPUS [PRODUCED_MODELS] ]"
  echo "WHERE"
  echo "   -t tmp-dir            alternative tmp directory (/tmp by default)"
  echo "   SL                    source language code (two characters)"
  echo "   TL                    target language code (two characters)"
  echo "   SL_FILE               file containing the source language segments (can be gzipped)"
  echo "   TL_FILE               file containing the target language segments (can be gzipped)"
  echo "   DIC                   output dictionary"
  echo "   PREPROCESSED_CORPUS   folder to store the resulting pre-processed files (tokenised, lowercased and leared). If no folder is specified, a temporal folder is created"
  echo "   PRODUCED_MODELS       folder to store the resulting models obtained as a by-product of the dictionaries building. If no folder is specified, a temporal folder is created"
  exit 1
}

if [[ -z $TMPDIR ]]; then
  TMPDIR="/tmp"
fi

ARGS=$(getopt "ht:" $*)

set -- $ARGS
for i
do
  case "$i" in
    -h|--help)
      exit_program $(basename $0)
      ;;
    -t|--tmp-dir)
      shift
      TMPDIR=$1
      shift
      ;;
    --)
      shift
      break
      ;;
  esac
done

case $# in
  5)
    SL="$1"
    TL="$2"
    SL_CORPUS="$3"
    TL_CORPUS="$4"
    DIC=$5
    PREPROCCORPUS=$(mktemp -d $TMPDIR/tempcorpuspreproc.XXXXX)
    MODELSDIR=$(mktemp -d $TMPDIR/tempgizamodel.XXXXX)
    ;;
  6)
    SL="$1"
    TL="$2"
    SL_CORPUS="$3"
    TL_CORPUS="$4"
    DIC=$5
    PREPROCCORPUS=$6
    if [ ! -d $6 ]; then
      echo "The path specified for storing the preprocessed files for the corpus is not valid."
      exit_program $(basename $0)
    fi
    MODELSDIR=$(mktemp -d $TMPDIR/tempgizamodel.XXXXX)
    ;;
  7)
    SL="$1"
    TL="$2"
    SL_CORPUS="$3"
    TL_CORPUS="$4"
    DIC=$5
    PREPROCCORPUS=$6
    if [ ! -d $6 ]; then
      echo "The path specified for storing the preprocessed files for the corpus is not valid."
      exit_program $(basename $0)
    fi
    MODELSDIR=$7
    if [ ! -d $7 ]; then
      echo "The path specified for storing the models produced by GIZA++ is not valid."
      exit_program $(basename $0)
    fi
    ;;
  *)
    exit_program $(basename $0)
    ;;
esac

SL_TOKENISED="$PREPROCCORPUS/corpus.tok.$SL"
TL_TOKENISED="$PREPROCCORPUS/corpus.tok.$TL"
SL_LOW_TOKENISED="$PREPROCCORPUS/corpus.tok.low.$SL"
TL_LOW_TOKENISED="$PREPROCCORPUS/corpus.tok.low.$TL"

#Tokenising the corpus
echo "TOKENISING THE CORPUS..."
if [ "$(file $SL_CORPUS|cut -d ' ' -f 2)" == "gzip" ]; then
    cattool="zcat"
else
    cattool="cat"
fi
$cattool $SL_CORPUS | __PYTHON__ -c 'import sys
#from nltk.tokenize.punkt import PunktWordTokenizer
from nltk import wordpunct_tokenize
for line in sys.stdin:
  print(" ".join(wordpunct_tokenize(line.decode("utf-8").strip())).encode("utf-8"))' | sed "s/&apos;/'/g" | sed 's/&quot;/"/g' | sed 's/&amp;/\&/g' > $SL_TOKENISED &

if [ "$(file $TL_CORPUS|cut -d ' ' -f 2)" == "gzip" ]; then
    cattool="zcat"
else
    cattool="cat"
fi
$cattool $TL_CORPUS | __PYTHON__ -c 'import sys
#from nltk.tokenize.punkt import PunktWordTokenizer
from nltk import wordpunct_tokenize
for line in sys.stdin:
  print(" ".join(wordpunct_tokenize(line.decode("utf-8").strip())).encode("utf-8"))' | sed "s/&apos;/'/g" | sed 's/&quot;/"/g' | sed 's/&amp;/\&/g' > $TL_TOKENISED 
wait

#Lowercasing the corpus
echo "LOWERCASING THE CORPUS..."
cat $SL_TOKENISED | perl "$(dirname "$0")"/../share/moses/tokenizer/lowercase.perl > $SL_LOW_TOKENISED 2> /dev/null &
cat $TL_TOKENISED | perl "$(dirname "$0")"/../share/moses/tokenizer/lowercase.perl > $TL_LOW_TOKENISED 2> /dev/null
wait

#Cleaning the corpus
echo "FILTERING OUT TOO LONG SENTENCES..."
perl "$(dirname "$0")"/../share/bitextor/utils/clean-corpus-n.perl $PREPROCCORPUS/corpus.tok.low $SL $TL $PREPROCCORPUS/corpus.clean 1 50 2> /dev/null

#Obtaining the vocabulary and the encoded sentences files
echo "FORMATTING THE CORPUS FOR PROCESSING..."
"$(dirname "$0")"/plain2snt $PREPROCCORPUS/corpus.clean.$SL $PREPROCCORPUS/corpus.clean.$TL 2> /dev/null > /dev/null
mv $PREPROCCORPUS/corpus.clean.${SL}_corpus.clean.$TL.snt $MODELSDIR/$TL-$SL-int-train.snt
mv $PREPROCCORPUS/corpus.clean.${TL}_corpus.clean.$SL.snt $MODELSDIR/$SL-$TL-int-train.snt
mv $PREPROCCORPUS/corpus.clean.$SL.vcb $MODELSDIR/$SL.vcb
mv $PREPROCCORPUS/corpus.clean.$TL.vcb $MODELSDIR/$TL.vcb

#Building classes from the words in the corpus
echo "BUILDING WORD CLASSES FOR IMPROVING ALIGNMENT..."
"$(dirname "$0")"/mkcls -c50 -n2 -p$PREPROCCORPUS/corpus.clean.$SL -V$MODELSDIR/$SL.vcb.classes opt 2> /dev/null > /dev/null
"$(dirname "$0")"/mkcls -c50 -n2 -p$PREPROCCORPUS/corpus.clean.$TL -V$MODELSDIR/$TL.vcb.classes opt 2> /dev/null > /dev/null
wait

#Obtaining the coocurrence matrix of the fiels
echo "CHECKING COOCURRENCE OF WORDS IN THE CORPUS..."
"$(dirname "$0")"/snt2cooc $MODELSDIR/$TL-$SL.cooc $MODELSDIR/$SL.vcb $MODELSDIR/$TL.vcb $MODELSDIR/$TL-$SL-int-train.snt 2> /dev/null
"$(dirname "$0")"/snt2cooc $MODELSDIR/$SL-$TL.cooc $MODELSDIR/$TL.vcb $MODELSDIR/$SL.vcb $MODELSDIR/$SL-$TL-int-train.snt 2> /dev/null
wait

#Running GIZA++ in both directions
echo "BUILDING PROBABILISTIC DICTIONARIES..."
"$(dirname "$0")"/mgiza -ncpus 8 -CoocurrenceFile $MODELSDIR/$SL-$TL.cooc -c $MODELSDIR/$SL-$TL-int-train.snt -m1 5 -m2 0 -m3 3 -m4 3 -mh 5 -m5 0 -model1dumpfrequency 1 -o $MODELSDIR/$SL-$TL -s $MODELSDIR/$TL.vcb -t $MODELSDIR/$SL.vcb -emprobforempty 0.0 -probsmooth 1e-7 2> /dev/null > /dev/null

"$(dirname "$0")"/mgiza -ncpus 8 -CoocurrenceFile $MODELSDIR/$TL-$SL.cooc -c $MODELSDIR/$TL-$SL-int-train.snt -m1 5 -m2 0 -m3 3 -m4 3 -mh 5 -m5 0 -model1dumpfrequency 1 -o $MODELSDIR/$TL-$SL -s $MODELSDIR/$SL.vcb -t $MODELSDIR/$TL.vcb -emprobforempty 0.0 -probsmooth 1e-7 2> /dev/null > /dev/null
wait

echo "FILTERING DICTIONARY..."
#Filtering the vocabularies to keep only those words occurring, at least, 10 times
egrep ' [^ ][^ ]+$' $MODELSDIR/$SL.vcb > $MODELSDIR/$SL.filtered.vcb &
egrep ' [^ ][^ ]+$' $MODELSDIR/$TL.vcb > $MODELSDIR/$TL.filtered.vcb
wait

echo -e "${SL}\t${TL}" > $DIC

#Obtaining the harmonic probability of each pair of words in both directions and filtering out those with less than p=0.2; printing the dictionary
__PYTHON__ -c '
import sys


svocabulary={}
tvocabulary={}
svcb=open(sys.argv[1],"r")
tvcb=open(sys.argv[2],"r")
for line in svcb:
  item=line.decode("utf-8").strip().split(" ")
  svocabulary[item[0]]=item[1]

for line in tvcb:
  item=line.decode("utf-8").strip().split(" ")
  tvocabulary[item[0]]=item[1]

t3dic={}
t3s=open(sys.argv[3],"r")
t3t=open(sys.argv[4],"r")
for line in t3t:
  item=line.decode("utf-8").strip().split(" ")
  if item[1] in t3dic:
    t3dic[item[1]][item[0]]=item[2]
  else:
    t3dic[item[1]]={}
    t3dic[item[1]][item[0]]=item[2]

for line in t3s:
  item=line.decode("utf-8").strip().split(" ")
  if item[0] in t3dic:
    if item[1] in t3dic[item[0]]:
      value1=float(t3dic[item[0]][item[1]])
      value2=float(item[2])
      hmean=2/((1/value1)+(1/value2))
      if hmean > 0.1:
        if item[1] in svocabulary and item[0] in tvocabulary:
          word1=svocabulary[item[1]]
          word2=tvocabulary[item[0]]
          print("{0}\t{1}".format(word1, word2))' $MODELSDIR/$SL.filtered.vcb $MODELSDIR/$TL.filtered.vcb $MODELSDIR/$SL-$TL.t3.final $MODELSDIR/$TL-$SL.t3.final | egrep '^.\s.$' -v | egrep '^[[:alpha:]-]+\s[[:alpha:]-]+$' >> $DIC

echo "DONE!"

rm -rf $PREPROCCORPUS
rm -rf $MODELSDIR
