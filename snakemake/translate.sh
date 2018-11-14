

CONFIGFILE=$1
DIRNAME=$2
PERMANENTDIR=$3
#echo $DIRNAME

cat /dev/stdin > $DIRNAME/in
snakemake --snakefile Snakefile.nmt --directory $DIRNAME --configfile $CONFIGFILE \
          --config permanentDir=$nmtPermanentDir -k -j 4 translate_only

cat $DIRNAME/evaluation/output
