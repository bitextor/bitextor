

DIRNAME=$(mktemp -d /tmp/translate_only.XXXXXX)
#DIRNAME=/tmp/ddd
#mkdir -p $DIRNAME
#echo $DIRNAME

cat /dev/stdin > $DIRNAME/in
snakemake --snakefile Snakefile.nmt --directory $DIRNAME --configfile config.hieu.json \
          --config permanentDir=/home/hieu/permanent/en-fr/nmt-dir -k -j 4 translate_only

cat $DIRNAME/evaluation/output
rm -rf $DIRNAME
