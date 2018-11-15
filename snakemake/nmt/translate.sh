

CONFIGFILE=$1
WORK_DIR=$2
PERMANENT_NMT_DIR=$3
#echo $DIRNAME

cat /dev/stdin > $WORK_DIR/in
snakemake --snakefile Snakefile.nmt --directory $WORK_DIR --configfile $CONFIGFILE \
          --config permanentDir=$PERMANENT_NMT_DIR -k -j 4 translate_only

cat $WORK_DIR/evaluation/output
