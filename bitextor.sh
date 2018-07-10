#!__BASH__

set -o pipefail

OUTPUT=/dev/stdout
URLFILE=""
URL=""
if [[ -z $TMPDIR ]]; then
  TMPDIR="/tmp"
fi
#Path to default dictionary: it can be replaced by using option -v
JOBS=""
TIMEOUT=""
LANG1=""
LANG2=""
BIDIDOCALIGN=1
DOCALIGNMENT=0
FORMAT="PLAIN"
INDEX=""
MAXLINES=5
MINQUALITY=0
INPUTMODE=0
CRAWLLOG=/dev/null
CRAWL2ETTLOG=/dev/null
ETT2LETTLOG=/dev/stderr
TAR2LETTLOG=/dev/null
WEBDIR2ETTLOG=/dev/null
LETT2LETTRLOG=/dev/stderr
LETT2IDXLOG=/dev/stderr
IDX2RIDXLOG=/dev/stderr
IDX2RIDX12LOG=/dev/stderr
IDX2RIDX21LOG=/dev/stderr
DISTANCEFILTERLOG=/dev/stderr
DISTANCEFILTER12LOG=/dev/stderr
DISTANCEFILTER21LOG=/dev/stderr
ALIGNDOCUMENTSLOG=/dev/stderr
ALIGNSEGMENTSLOG=/dev/null
CLEANTEXTLOG=/dev/null
MORPHANAL_OPTIONS=""
MORPH1=""
MORPH2=""
CRAWLOUT=""
CRAWL2ETTOUT=""
ETT2LETTOUT=""
LETT2LETTROUT=""
WEBDIR2ETTOUT=""
LETT2IDXOUT=""
IDX2RIDXOUT=""
IDX2RIDX12OUT=""
IDX2RIDX21OUT=""
DISTANCEFILTEROUT="/dev/null"
DISTANCEFILTER12OUT=""
DISTANCEFILTER21OUT=""
ALIGNDOCUMENTSOUT=""
ALIGNSEGMENTSOUT=""
MODEL="__PREFIX__/share/bitextor/model/keras.model"
WEIGHTS="__PREFIX__/share/bitextor/model/keras.weights"
SIZELIMIT=""
TIMELIMIT=""
DOCSIMTHRESHOLD=""
CRAWLINGSTATUSDUMP=""
CRAWLINGSTATUSCONTINUE=""
CRAWLINGDATACONTINUE=""
TLD=""
PRINTSTATS=""
FILTERLINES=""
HUNALIGNSCORE=""
ELRCSCORES=""
BICLEANERSCORE=""
ZIPPORAHSCORE=""
IGNOREBOILER=""
USENLTK="--nltk"
USEHTTRACK=0
USEJHULETT=0
CONFIGFILEOPTIONS=""
CONFIGFILE=""
DIRNAME=""
ALIGNEDDOCINPUT=""
ALIGNEDSENTSINPUT=""
ONLYCRAWL=""
ONLYLETT=""
BICLEANER=""
ZIPPORAH=""
BICLEANERTHRESHOLD=""
ZIPPORAHTHRESHOLD=""
DONOTPIPELETT=""
BUILDDICTTMP=$(mktemp -d $TMPDIR/BUILDDICTTMP.XXXXXX)


parse_config_file()
{
  if [ "$1" != "" ]; then
    cat $1 | sed -r 's:\#.*$::' | sed -r 's:^ *::' | sed -r 's: *$::' | grep -v '^$' | sed 's:^:\-\-:' | sed 's:^\-\-\-:\-:' | sed -r 's: *= *: :' | tr '\n' ' '
  fi
}

exit_program()

{
  #echo "USAGE: $1 [-d dirname] [-l file] [-i index] [-r revindex] [-m max_lines]"
  #echo "          [-q min_quality] [url] lang1 lang2"
  echo "USAGE: $1 [OPTIONS] -u URL -e FILE -v VOCABULARY LANG1 LANG2"
  echo "USAGE: $1 [OPTIONS] -U FILE        -v VOCABULARY LANG1 LANG2"
  echo "USAGE: $1 [OPTIONS] -e FILE        -v VOCABULARY LANG1 LANG2"
  echo ""
  echo "WHERE:"
  echo "  -F configfile     (--config-file) path of the configuration file with Bitextor options."
  echo "                    It can be filled up with one line of consecutive space separated parameters as the usual command call,"
  echo "                    split that call in several lines, or writting an option per line with the argument separated by '=' or space"
  echo "  -u URL            (--url) URL of a website to crawl (one per line); if option -e is also"
  echo "                    enabled, the website is downloaded in the ETT fomat and stored"
  echo "                    in the file at the specified path, if not, it is downloaded in"
  echo "                    a temporal directory in '/tmp' (or a different directory if -T option is used)."
  echo "  -U FILE           (--url-list) tab-separated file containing a list of URLs to crawl and their"
  echo "                    corresponding destination path (one per line)."
  echo "  -e FILE           (--ett) uses as an input the output of the module bitextor-webdir2ett"
  echo "  -E FILE           (--lett) uses as an input a lett file"
  echo "  LANG1             selected language with two letters code (ISO 639-1): en, es, fr, de ..."
  echo "  LANG2             selected language with two letters code (ISO 639-1): en, es, fr, de ..."
  echo ""
  echo "OPTIONS:"
  echo "  -L PATH           (--logs-dir) custom path where the directory containing the logs of the"
  echo "                    different modules of bitextor will be stored"
  echo "  -H                (--httrack) use HTTrack instead of embedded Creepy crawling engine"
  echo "  --jhu-lett        (only with --httrack) use JHU pipeline process for ETT and LETT processing from HTTrack files"
  echo "  --paracrawl-aligner-command  COMMAND      Gives a translation command (Marian, Moses...) that is used by JHU document aligner"
  echo "  --aligned-document-input FILE       Performs sentence alignment, cleaning and optional TMX conversion of provided aligned documents"
  echo "  --aligned-sentences-input FILE      Performs cleaning and optional TMX conversion of provided aligned sentences"
  echo "  --only-crawl      Only performs crawling"
  echo "  --only-lett       Only performs crawling, ETT and LETT(r) processing, printing this last file"
  echo "  --bicleaner CONFIG     Performs Bicleaner score and attach it to the output, needs a configuration file YAML"
  echo "  --zipporah MODEL_FOLDER             Performs Zipporah score and attach it to the output"
  echo "  --filter-bicleaner SCORE            Performs filtering using the --bicleaner score, thresholding it. For example, 0.7"
  echo "  --filter-zipporah SCORE             Performs filtering using the --zipporah score, thresholding it. For example, 0.0"
  echo ""
  echo "  -l LETTR          (--lettr) custom path where the file with extension .lettr (language"
  echo "                    encoded and typed data with 'raspa') will be created"
  echo "                    (/lettr.XXXXXX by default)."
  echo "  -I PATH           (--intermediate-files-dir) custom path where the output of the intermediate files produced"
  echo "                    by the modules of bitextor will be stored."
  echo "  -B                (--ignore-boilerpipe-cleaning) ignores and skips the boilerpipe clean step"
  echo "  -n                (--ulysses) uses Ulysses sentence tokenizer instead of NLTK (deprecated in future major version)"
  echo "  -b NUM            (--num-accepted-candidates) NUM is the number of possible alignment candidates taken into"
  echo "                    account for every document in one language. If NUM is higher than 1, candidate document"
  echo "                    alignments are obtained from LANG1 to LANG2 and from LANG2 to LANG1 and then symmetrised, "
  echo "                    while if NUM=0, ony candidates from LANG1 to LANG2 are obtained. In addition, if NUM>1,"
  echo "                    several candidates can be considered for a single document, while If NUM=1, only the best"
  echo "                    candidate will be taken into account. The default value for this option is 1, which is"
  echo "                    the one that has proved to provide best results." 
  echo "  -v VOCABULARY     (--vocabulary) option for using a custom multilingual vocabulary for preliminar"
  echo "                    document alignment. The vocabulary must be a tab-separated file,"
  echo "                    in which the first line contains the names of the languages"
  echo "                    corresponding to each column, and the rest of the lines must"
  echo "                    contain the same word translated to all these languages."
  echo "  -m MAX_LINES      (--maximum-wrong-alignments) maximum number or wrong segment alignments tolerated to accept a"
  echo "                    pair of documents as a valid document alignment. If this number"
  echo "                    is reached, the whole document pair is discarded (5 by default)."
  echo "  -q MIN_QUALITY    (--seg-alignment-score-threshold) threshold for Hunalign confidence score; those pairs of segments"
  echo "                    with a score"
  echo "                    lower than MIN_QUALITY will be considered wrong and they will be"
  echo "                    removed (0 by default)."
  echo "  -W                (--elrc-quality-metrics) Print stats similar to ILSP-FC fields, as the Hunalign information in output stream, in added columns or TMX properties"
  echo "  --filter-with-elrc       Needs -W/--elrc-quality-metrics. Filter sentences using ELRC metrics"
  echo "  -T TMP-DIR        (--tmp-dir) alternative tmp directory (/tmp by default)."
  echo "  -x                (--tmx-output) if this option is enabled, the output of the tool will be"
  echo "                    formated in the standard XML-based format TMX."
  echo "  -a                (--only-document-alignment) if this option is enabled, Bitextor will perform the alignment only"
  echo "                    at the level of documents. The output will be tab-sepparated, with"
  echo "                    three fields: two with the name of the documents aligned and one with"
  echo "                    the score provided by hunalign for the pair of documents."
  echo "  -M                (--sl-morphological-analyser) morphological analyser in the Apertium platform for source language"
  echo "                    that will allow to apply word matching directly on lemmas; this is an"
  echo "                    important tool for aglutinant languages in order to obtain a good"
  echo "                    coverage with the bilingual lexicon approach."
  echo "  -N                (--tl-morphological-analyser) morphological analyser in the Apertium platform for target language"
  echo "                    that will allow to apply word matching directly on lemmas; this is an"
  echo "                    important tool for aglutinant languages in order to obtain a good"
  echo "                    coverage with the bilingual lexicon approach."
  echo "  -O FILE           (--output) if this option is enabled, the otput of bitextor will be redirected"
  echo "                    to file FILE, if not it is redirected to the standard output."
  echo "  -d THRESHOLD      (--doc-alignment-score-threshold) threshold for the parallel-document confidence score. This threshold"
  echo "                    can take real values in [0,1], being 0 equivalent to not setting any"
  echo "                    threshold and 1 the highest threshold possible."
  echo "  -s SIZE           (--size-limit) size limit for the crawling process; if this option is set, the crawling"
  echo "                    process will stop after the amount of data specified has been crawled."
  echo "                    This option must be an amount of Kylobytes (K), Megabytes (M) or Gigabytes (G)"
  echo "                    for example: '50M' for 50 Megabytes"
  echo "  -t TIME           (--time-limit) time limit for the crawling process; if this option is set, the crawling"
  echo "                    process will stop after the amount of time specified. This option must"
  echo "                    be an amount of time and a time unit (h for hours, m for minutes and"
  echo "                    s for seconds), for example: '35m' for 35 minutes"
  echo "  -j JOBS           (--num-threads) number of jobs to be run in parallel (threads) used during crawling;"
  echo "                    note that a number of jobs too high may cause that the server will not"
  echo "                    be able to attend all the connection requests (default value 4)."
  echo "  -c TIME           (--timeout-crawl) connection timeout; maximum time (in seconds) to wait until a connection"
  echo "                    request is attended from the servere during web crawling (defalut value 15)."
  echo "  -p FILE           (--write-crawling-file) write crawling status to FILE if the crawling ends because a time/size limit has been reached."
  echo "  -C FILE           (--continue-crawling-file) load crawling status from FILE. Use this option to continue a previously interrupted crawling. "
  echo "  -R FILE           (--reuse-crawling-file) reuse the crawled documents from FILE. FILE must be a crawl2ett file from a previous"
  echo "                    execution of Bitextor. This option must be used together with -C and the previous execution "
  echo "                    and the previous execution must have included the -p option in order to store the crawling status."
  echo "  -D                (--crawl-tld) if this flag is set, all the websites in the TLD will be crawled."

  exit 1
}

run_bitextor_ett(){
  zcat -f $1 | __JAVA__ -jar __PREFIX__/share/java/piped-tika.jar -t 2> $ETT2LETTLOG | \
  __PREFIX__/bin/bitextor-ett2lett -l ${LANG1},$LANG2 2>> $ETT2LETTLOG | tee $ETT2LETTOUT > $LETT &
  if [ "$ONLYLETT" != "" ]; then
    cat $LETT
  else
    align_documents_and_segments $LETT
  fi 
}

run_bitextor_lett(){

  if [ "$ONLYLETT" != "" ]; then
    cat $1
  else
    align_documents_and_segments $1
  fi
}

trapsigint(){
  if [ $# -eq 0 ]; then
    echo "Bitextor finished the crawling process and is in the alignment phase; please, use SIGTERM to stop it."
  else
    kill $1
    echo "Bitextor just stopped crawling and the alignment process will start inmediatelly."
    wait
  fi
}

run_bitextor(){
  local URL=$1
  tmpcrawl=$(mktemp $BUILDDICTTMP/crawl.XXXXXX)
  rm $tmpcrawl
  mkfifo $tmpcrawl

  DUMPARGS=""
  if [ "$CRAWLINGSTATUSDUMP" != "" ]; then
      DUMPARGS="-d $CRAWLINGSTATUSDUMP"
  fi
  CONTINUEARGS=""
  if [ "$CRAWLINGSTATUSCONTINUE" != "" ]; then
      CONTINUEARGS="-l $CRAWLINGSTATUSCONTINUE"
  fi
  if [ "$CRAWLINGDATACONTINUE" != "" ]; then
      CONTINUEARGS="$CONTINUEARGS -e $CRAWLINGDATACONTINUE"
  fi

  if [ "$USEHTTRACK" == "0" ]; then #HTTRACK not used
    __PREFIX__/bin/bitextor-crawl $TLD_CRAWL $URL $SIZELIMIT $TIMELIMIT $JOBS $TIMEOUT $DUMPARGS $CONTINUEARGS 2> $CRAWLLOG | tee $CRAWLOUT > $tmpcrawl &
    crawl_pid=$(jobs -p)
    trap "trapsigint $crawl_pid" SIGINT
    trap "trapsigint $crawl_pid" SIGUSR1
    if [ "$ONLYCRAWL" == "" ] ; then
      __PREFIX__/bin/bitextor-crawl2ett $IGNOREBOILER < $tmpcrawl 2> $CRAWL2ETTLOG | tee $CRAWL2ETTOUT | \
      __JAVA__ -jar __PREFIX__/share/java/piped-tika.jar -t 2> $ETT2LETTLOG | \
      __PREFIX__/bin/bitextor-ett2lett -l ${LANG1},$LANG2 2>> $ETT2LETTLOG | tee $ETT2LETTOUT > $LETT &
    else
      cat $tmpcrawl
    fi
  else
    if [ "$DIRNAME" == "" ]; then
      DIRNAME=$(mktemp -d $TMPDIR/downloaded_websites.XXXXXX)
    fi
    __PREFIX__/bin/bitextor-downloadweb $URL $DIRNAME > $CRAWLLOG 2>&1
    if [ "$ONLYCRAWL" == "" ] ; then
      if [ "$USEJHULETT" == "0" ]; then
        __PREFIX__/bin/bitextor-webdir2ett $DIRNAME 2> $WEBDIR2ETTLOG | tee $WEBDIR2ETTOUT | \
        __JAVA__ -jar __PREFIX__/share/java/piped-tika.jar -t 2> $ETT2LETTLOG | \
        __PREFIX__/bin/bitextor-ett2lett -l ${LANG1},$LANG2 2>> $ETT2LETTLOG | tee $ETT2LETTOUT > $LETT & 
      else
        TARNAME=$(mktemp $TMPDIR/tar.XXXXXX.tar.gz)
        tar czf $TARNAME -C $DIRNAME/ .
        __PREFIX__/bin/tar2lett $TARNAME $LANG1 $LANG2 2> $TAR2LETTLOG > $LETT & 
      fi
    else
      echo "Crawling finished at $DIRNAME"
    fi
  fi
  
  if [ "$DONOTPIPELETT" != "" ]; then
    wait
  fi
  
  if [ "$ONLYLETT" == "" -a "$ONLYCRAWL" == "" ]; then
    align_documents_and_segments $LETT
    if [ "$DIRNAME" != "" ]; then
      rm -r $DIRNAME
    fi
  else
    cat $LETT
  fi

  rm $tmpcrawl
  if [ "$TARNAME" != "" ]; then
    rm -r $TARNAME
  fi
}

align_segments(){
  __PREFIX__/bin/bitextor-align-segments $MORPHANAL_OPTIONS -d "$1" -t $TMPDIR --lang1 $LANG1 --lang2 $LANG2 $USENLTK 2> $ALIGNSEGMENTSLOG | tee $ALIGNSEGMENTSOUT
}

clean_segments(){
  OUTPUTCLEANERS=$(mktemp $BUILDDICTTMP/outputcleaners.XXXXXX)
  cat - > $OUTPUTCLEANERS
  > $CLEANTEXTLOG
  if [ "$ZIPPORAH" != "" ]; then
    cat $OUTPUTCLEANERS | __PREFIX__/bin/zipporah-classifier $ZIPPORAH $LANG1 $LANG2 2>> $CLEANTEXTLOG | python3 -c "
import sys

for line in sys.stdin:
  parts = line.split('\t')
  parts[-1]=parts[-1].strip()
  if '$ZIPPORAHTHRESHOLD' != '':
    if float(parts[-1]) >= float('$ZIPPORAHTHRESHOLD'):
      print(line)
  else:
    print(line)

    " > ${OUTPUTCLEANERS}-tmp
    mv ${OUTPUTCLEANERS}-tmp $OUTPUTCLEANERS
  fi
  if [ "$BICLEANER" != "" ]; then
    python3 __PREFIX__/bin/bicleaner-classifier-full.py $OUTPUTCLEANERS ${OUTPUTCLEANERS}-tmp -m $BICLEANER -s $LANG1 -t $LANG2 2>> $CLEANTEXTLOG
    cat ${OUTPUTCLEANERS}-tmp | python3 -c "
import sys

for line in sys.stdin:
  parts = line.split('\t')
  parts[-1]=parts[-1].strip()
  if '$BICLEANERTHRESHOLD' != '':
    if float(parts[-1]) >= float('$BICLEANERTHRESHOLD'):
      print(line)
  else:
    print(line)
    " > $OUTPUTCLEANERS
  fi
  cat $OUTPUTCLEANERS | __PREFIX__/bin/bitextor-cleantextalign -q $MINQUALITY -m $MAXLINES $PRINTSTATS 2>> $CLEANTEXTLOG | \
  __PREFIX__/bin/bitextor-elrc-filtering $PRINTSTATS $FILTERLINES -c url1,url2,seg1,seg2$HUNALIGNSCORE$ZIPPORAHSCORE$BICLEANERSCORE
  rm -r $OUTPUTCLEANERS
}

convert_to_tmx(){
  if [ $FORMAT == "TMX" ]; then
    __PREFIX__/bin/bitextor-buildTMX --lang1 $LANG1 --lang2 $LANG2 -c url1,url2,seg1,seg2$HUNALIGNSCORE$ZIPPORAHSCORE$BICLEANERSCORE$ELRCSCORES,idnumber 
  elif [ "$PRINTSTATS" != "" ]; then
    cat -
  else
    cat - | cut -f 1-4
  fi
}

align_documents_and_segments(){
  trap '' SIGINT

  TLD_IDX2RIDX=""
  if [ "$TLD_CRAWL" != "" ]; then
    TLD_IDX2RIDX=" -l $1 "
  fi

  output_pipe=$(mktemp $BUILDDICTTMP/output_pipe.XXXXXX)
  rm $output_pipe
  mkfifo $output_pipe

  HUNALIGN_DIC=$(mktemp $BUILDDICTTMP/hunalign_dic.XXXXXX)
  tail -n +2 $VOCABULARY | sed -r 's/^([^\s]+)\t([^\s]+)$/\2 @ \1/g' > $HUNALIGN_DIC
  if [ "$TRANSLATIONCOMMAND" != "" ]; then
    cat $LETT > $LETT.txt
    DOCALIGNTEMP=$(mktemp -d $BUILDDICTTMP/docaligntemp.XXXXXX)
    if [ $DOCALIGNMENT -eq 0 ]; then
        __PREFIX__/bin/doc_align.sh -f $LETT.txt -l $LANG2 -t "$TRANSLATIONCOMMAND" -d -w $DOCALIGNTEMP 2> $ALIGNDOCUMENTSLOG | tee $ALIGNDOCUMENTSOUT | \
        align_segments $HUNALIGN_DIC | \
        clean_segments > $output_pipe &
    else
        __PREFIX__/bin/doc_align.sh -f $LETT.txt -l $LANG2 -t "$TRANSLATIONCOMMAND" -d -w $DOCALIGNTEMP 2> $ALIGNDOCUMENTSLOG | tee $ALIGNDOCUMENTSOUT | \
        __PREFIX__/bin/bitextor-score-document-alignment -t $TMPDIR --lang1 $LANG1 --lang2 $LANG2 -d $HUNALIGN_DIC $USENLTK > $output_pipe &
    fi
  elif [ $BIDIDOCALIGN -ge 1 ]; then #Use dictionaries to pair indexes between documents
    #Named pipe for paralelising obtaining the initial index for the ridx 1
    index_pipe1=$(mktemp $BUILDDICTTMP/index_pipe.XXXXXX)
    rm $index_pipe1
    mkfifo $index_pipe1

    #Named pipe for paralelising obtaining the initial index for the ridx 2
    index_pipe2=$(mktemp $BUILDDICTTMP/index_pipe.XXXXXX)
    rm $index_pipe2
    mkfifo $index_pipe2

    INDEX=$(mktemp $BUILDDICTTMP/idx.XXXXXX)
    zcat -f $LETT | __PREFIX__/bin/bitextor-lett2lettr 2> $LETT2LETTRLOG | tee $LETT2LETTROUT > $LETTR
    __PREFIX__/bin/bitextor-lett2idx $MORPHANAL_OPTIONS --lang1 $LANG1 --lang2 $LANG2 -m 15 $LETTR 2> $LETT2IDXLOG | tee $LETT2IDXOUT | \
    tee $INDEX |tee $index_pipe1 > $index_pipe2 &


    #INDEX=$(mktemp $BUILDDICTTMP/idx.XXXXXX)
    RINDEX1=$(mktemp $BUILDDICTTMP/ridx.XXXXXX)
    RINDEX2=$(mktemp $BUILDDICTTMP/ridx.XXXXXX)

    __PREFIX__/bin/bitextor-idx2ridx $TLD_IDX2RIDX -d $VOCABULARY --lang1 $LANG1 --lang2 $LANG2 < $index_pipe1 2> $IDX2RIDX12LOG | tee $IDX2RIDX12OUT | \
    __PREFIX__/bin/bitextor-imagesetoverlap -l $LETTR | \
    __PREFIX__/bin/bitextor-structuredistance -l $LETTR | \
    __PREFIX__/bin/bitextor-urlsdistance -l $LETTR | \
    __PREFIX__/bin/bitextor-mutuallylinked -l $LETTR | \
    __PREFIX__/bin/bitextor-urlscomparison -l $LETTR | \
    __PREFIX__/bin/bitextor-urlsetoverlap -l $LETTR | \
    __PREFIX__/bin/bitextor-rank $DOCSIMTHRESHOLD -m $MODEL -w $WEIGHTS 2> >(grep -v 'Using TensorFlow backend.' > $DISTANCEFILTER12LOG) | tee $DISTANCEFILTER12OUT > $RINDEX1 &

    #rindex1_pid=$!

    __PREFIX__/bin/bitextor-idx2ridx $TLD_IDX2RIDX -d $VOCABULARY --lang1 $LANG2 --lang2 $LANG1 < $index_pipe2 2> $IDX2RIDX21LOG | tee $IDX2RIDX21OUT | \
    __PREFIX__/bin/bitextor-imagesetoverlap -l $LETTR | \
    __PREFIX__/bin/bitextor-structuredistance -l $LETTR | \
    __PREFIX__/bin/bitextor-urlsdistance -l $LETTR | \
    __PREFIX__/bin/bitextor-mutuallylinked -l $LETTR | \
    __PREFIX__/bin/bitextor-urlscomparison -l $LETTR | \
    __PREFIX__/bin/bitextor-urlsetoverlap -l $LETTR | \
    __PREFIX__/bin/bitextor-rank $DOCSIMTHRESHOLD -m $MODEL -w $WEIGHTS 2> >(grep -v 'Using TensorFlow backend.' > $DISTANCEFILTER21LOG) | tee $DISTANCEFILTER21OUT > $RINDEX2 &

    #wait $rindex1_pid
    wait

    rm $index_pipe1 $index_pipe2

    L1WORDS=$(mktemp $BUILDDICTTMP/l1words.XXXXXX)
    L2WORDS=$(mktemp $BUILDDICTTMP/l2words.XXXXXX)
    WORDS=$(mktemp $BUILDDICTTMP/words.XXXXXX)

    grep "^$LANG1"$'\t' $INDEX | cut -f 2 | sort > $L1WORDS
    grep "^$LANG2"$'\t' $INDEX | cut -f 2 | sort > $L2WORDS
    rm $INDEX

    comm -12 $L1WORDS $L2WORDS > $WORDS
    rm $L1WORDS $L2WORDS

    paste $WORDS $WORDS | sed 's/\t/ @ /g' >> $HUNALIGN_DIC
    rm $WORDS


    if [ $DOCALIGNMENT -eq 0 ]; then
        __PREFIX__/bin/bitextor-align-documents -l $LETTR -n $BIDIDOCALIGN -r $DISTANCEFILTEROUT -i converge $RINDEX1 $RINDEX2 2> $ALIGNDOCUMENTSLOG | tee $ALIGNDOCUMENTSOUT | \
        align_segments $HUNALIGN_DIC | \
        clean_segments > $output_pipe &
    else
        __PREFIX__/bin/bitextor-align-documents  -i converge -l $LETTR -n $BIDIDOCALIGN -r $DISTANCEFILTEROUT $RINDEX1 $RINDEX2 2> $ALIGNDOCUMENTSLOG | tee $ALIGNDOCUMENTSOUT | \
        __PREFIX__/bin/bitextor-score-document-alignment -t $TMPDIR --lang1 $LANG1 --lang2 $LANG2 -d $HUNALIGN_DIC $USENLTK > $output_pipe &
    fi
  else #Use Apertium to index both documents
    cat $LETT | __PREFIX__/bin/bitextor-lett2lettr 2> $LETT2LETTRLOG | tee $LETT2LETTROUT > $LETTR
    if [ $DOCALIGNMENT -eq 0 ]; then
        __PREFIX__/bin/bitextor-lett2idx $MORPHANAL_OPTIONS --lang1 $LANG1 --lang2 $LANG2 -m 15 $LETTR 2> $LETT2IDXLOG | tee $LETT2IDXOUT | \
        __PREFIX__/bin/bitextor-idx2ridx $TLD_IDX2RIDX -d $VOCABULARY --lang1 $LANG1 --lang2 $LANG2 2> $IDX2RIDXLOG | tee $IDX2RIDXOUT | \
        __PREFIX__/bin/bitextor-imagesetoverlap -l $LETTR | \
        __PREFIX__/bin/bitextor-structuredistance -l $LETTR | \
        __PREFIX__/bin/bitextor-urlsdistance -l $LETTR | \
        __PREFIX__/bin/bitextor-mutuallylinked -l $LETTR | \
        __PREFIX__/bin/bitextor-urlscomparison -l $LETTR | \
        __PREFIX__/bin/bitextor-urlsetoverlap -l $LETTR | \
	__PREFIX__/bin/bitextor-rank $DOCSIMTHRESHOLD -m $MODEL -w $WEIGHTS 2> >(grep -v 'Using TensorFlow backend.' > $DISTANCEFILTER12LOG) | tee $DISTANCEFILTER12OUT | \
        __PREFIX__/bin/bitextor-align-documents  -i converge -l $LETTR 2> $ALIGNDOCUMENTSLOG | tee $ALIGNDOCUMENTSOUT | \
        align_segments $HUNALIGN_DIC | \
        clean_segments > $output_pipe &
    else
        __PREFIX__/bin/bitextor-lett2idx $MORPHANAL_OPTIONS --lang1 $LANG1 --lang2 $LANG2 -m 15 $LETTR 2> $LETT2IDXLOG | tee $LETT2IDXOUT | \
        __PREFIX__/bin/bitextor-idx2ridx $TLD_IDX2RIDX -d $VOCABULARY --lang1 $LANG1 --lang2 $LANG2 2> $IDX2RIDXLOG | tee $IDX2RIDXOUT | \
        __PREFIX__/bin/bitextor-imagesetoverlap -l $LETTR | \
        __PREFIX__/bin/bitextor-structuredistance -l $LETTR | \
        __PREFIX__/bin/bitextor-urlsdistance -l $LETTR | \
        __PREFIX__/bin/bitextor-mutuallylinked -l $LETTR | \
        __PREFIX__/bin/bitextor-urlscomparison -l $LETTR | \
        __PREFIX__/bin/bitextor-urlsetoverlap -l $LETTR | \
	__PREFIX__/bin/bitextor-rank $DOCSIMTHRESHOLD -m $MODEL -w $WEIGHTS 2> >(grep -v 'Using TensorFlow backend.' > $DISTANCEFILTER12LOG) | tee $DISTANCEFILTER12OUT | \
        __PREFIX__/bin/bitextor-align-documents  -i converge -l $LETTR 2> $ALIGNDOCUMENTSLOG | tee $ALIGNDOCUMENTSOUT | \
        __PREFIX__/bin/bitextor-score-document-alignment -t $TMPDIR --lang1 $LANG1 --lang2 $LANG2 -d $HUNALIGN_DIC $USENLTK > $output_pipe &
    fi
  fi
  convert_to_tmx < $output_pipe > $OUTPUT

  rm $HUNALIGN_DIC

  rm -Rf $TMPLETTR $TMPRINDEX $output_pipe $RINDEX1 $MORPH1 $MORPH2 $RINDEX2 $DOCALIGNTEMP
}


trap '' SIGINT

OLDARGS="$@"
ARGS=$(getopt -o xaWDBHnf:q:m:v:b:l:u:U:d:D:L:D:e:E:I:t:O:M:N:T:s:j:c:p:C:R:F: -l jhu-lett,tmx-output,only-document-alignment,elrc-quality-metrics,crawl-tld,ignore-boilerpipe-cleaning,httrack,ulysses,url:,url-list:,ett:,lett:,logs-dir:,lettr:,intermediate-files-dir:,num-accepted-candidates:,vocabulary:,tmp-dir:,num-threads:,sl-morphological-analyser:,tl-morphological-analyser:,output:,doc-alignment-score-threshold:,maximum-wrong-alignments:,seg-alignment-score-threshold:,continue-crawling-file:,reuse-crawling-file:,size-limit:,time-limit:,write-crawling-file:,timeout-crawl:,dirname:,config-file:,aligned-document-input:,aligned-sentences-input:,only-crawl,only-lett,bicleaner:,zipporah:,filter-bicleaner:,filter-zipporah:,paracrawl-aligner-command:,filter-with-elrc -- "$@")

eval set -- $ARGS
for i
do
  case "$i" in
    -F | --config-file)
      shift
      CONFIGFILE=$1
      CONFIGFILEOPTIONS=`parse_config_file $CONFIGFILE`
      ;;
  esac
  shift
done

ARGS=$(getopt -o xaWDBHnf:q:m:v:b:l:u:U:d:D:L:D:e:E:I:t:O:M:N:T:s:j:c:p:C:R:F: -l jhu-lett,tmx-output,only-document-alignment,elrc-quality-metrics,crawl-tld,ignore-boilerpipe-cleaning,httrack,ulysses,url:,url-list:,ett:,lett:,logs-dir:,lettr:,intermediate-files-dir:,num-accepted-candidates:,vocabulary:,tmp-dir:,num-threads:,sl-morphological-analyser:,tl-morphological-analyser:,output:,doc-alignment-score-threshold:,maximum-wrong-alignments:,seg-alignment-score-threshold:,continue-crawling-file:,reuse-crawling-file:,size-limit:,time-limit:,write-crawling-file:,timeout-crawl:,dirname:,config-file:,aligned-document-input:,aligned-sentences-input:,only-crawl,only-lett,bicleaner:,zipporah:,filter-bicleaner:,filter-zipporah:,paracrawl-aligner-command:,filter-with-elrc -- $CONFIGFILEOPTIONS $OLDARGS)
eval set -- $ARGS
for i
do
  case "$i" in
    -I | --intermediate-files-dir )
      shift
      INTERMEDIATEFILE=$1
      mkdir -p $INTERMEDIATEFILE
      CRAWLOUT=$INTERMEDIATEFILE/crawl
      CRAWL2ETTOUT=$INTERMEDIATEFILE/crawl2ett
      ETT2LETTOUT=$INTERMEDIATEFILE/ett2lett
      LETT2LETTROUT=$INTERMEDIATEFILE/lett2lettr
      WEBDIR2ETTOUT=$INTERMEDIATEFILE/webdir2ett
      LETT2IDXOUT=$INTERMEDIATEFILE/lett2idx
      IDX2RIDXOUT=$INTERMEDIATEFILE/idx2ridx
      IDX2RIDX12OUT=$INTERMEDIATEFILE/idx2ridx-lang1-lang2
      IDX2RIDX21OUT=$INTERMEDIATEFILE/idx2ridx-lang2-lang1
      DISTANCEFILTEROUT=$INTERMEDIATEFILE/distancefilter
      DISTANCEFILTER12OUT=$INTERMEDIATEFILE/distancefilter-lang1-lang2
      DISTANCEFILTER21OUT=$INTERMEDIATEFILE/distancefilter-lang2-lang1
      ALIGNDOCUMENTSOUT=$INTERMEDIATEFILE/aligndocuments
      ALIGNSEGMENTSOUT=$INTERMEDIATEFILE/alignsegments
      shift
      ;;
    -M | --sl-morphological-analyser)
      shift
      MORPH1=$(mktemp $BUILDDICTTMP/morph1.XXXXXX)
      sed 's/lt-proc /apertium-destxt | lt-proc /' < $1 | sed 's/\(^.*lt-proc.*$\)/\1 | apertium-retxt/' > $MORPH1
      #sed 's/lt-proc /lt-proc -n /' < $1 > $MORPH1
      MORPHANAL_OPTIONS="$MORPHANAL_OPTIONS --morphanalyser_sl $MORPH1"
      shift
      ;;
    -N | --tl-morphological-analyser)
      shift
      MORPH2=$(mktemp $BUILDDICTTMP/morph2.XXXXXX)
      sed 's/lt-proc /apertium-destxt | lt-proc /' < $1 | sed 's/\(^.*lt-proc.*$\)/\1 | apertium-retxt/' > $MORPH2
      MORPHANAL_OPTIONS="$MORPHANAL_OPTIONS --morphanalyser_tl $MORPH2"
      shift
      ;;
    -L | --logs-dir)
      shift
      LOGDIR=$1
      mkdir -p $LOGDIR
      CRAWLLOG=$LOGDIR/bitextorcrawl.log
      CRAWL2ETTLOG=$LOGDIR/bitextorcrawl2ett.log
      ETT2LETTLOG=$LOGDIR/bitextorett2lett.log
      TAR2LETTLOG=$LOGDIR/bitextortar2lett.log
      WEBDIR2ETTLOG=$LOGDIR/bitextorwebdir2ett.log
      LETT2LETTRLOG=$LOGDIR/bitextorlett2lettr.log
      LETT2IDXLOG=$LOGDIR/bitextorlett2idx.log
      IDX2RIDXLOG=$LOGDIR/bitextoridx2ridx.log
      IDX2RIDX12LOG=$LOGDIR/bitextoridx2ridx_lang1-lang2.log
      IDX2RIDX21LOG=$LOGDIR/bitextoridx2ridx_lang2-lang1.log
      DISTANCEFILTERLOG=$LOGDIR/bitextordistancefilter.log
      DISTANCEFILTER12LOG=$LOGDIR/bitextordistancefilter_lang1-lang2.log
      DISTANCEFILTER21LOG=$LOGDIR/bitextordistancefilter_lang2-lang1.log
      ALIGNDOCUMENTSLOG=$LOGDIR/bitextoraligndocuments.log
      ALIGNSEGMENTSLOG=$LOGDIR/bitextoralignsegments.log
      CLEANTEXTLOG=$LOGDIR/bitextorcleantextalign.log
      ADDTEXTSTATSLOG=$LOGDIR/bitextorelrcfiltering.log
      shift
      ;;
    -e | --ett)
      shift
      ETT=$1
      if [ $INPUTMODE -eq 0 ]; then
        INPUTMODE=3
      fi
      shift
      ;;
    -E | --lett)
      shift
      LETT=$1
      if [ $INPUTMODE -eq 0 ]; then
        INPUTMODE=4
      else
        DONOTPIPELETT='yes'
      fi
      shift
      ;;
    --aligned-document-input)
      shift
      ALIGNEDDOCINPUT=$1
      if [ $INPUTMODE -eq 0 ]; then
        INPUTMODE=5
      fi
      shift
      ;;
    --aligned-sentences-input)
      shift
      ALIGNEDSENTSINPUT=$1
      if [ $INPUTMODE -eq 0 ]; then
        INPUTMODE=6
      fi
      shift
      ;;
    --only-crawl)
      shift
      ONLYCRAWL="yes"
      ;;
    --only-lett)
      shift
      ONLYLETT="yes"
      ;;
    -u | --url)
      shift
      URL=$1
      INPUTMODE=1
      shift
      ;;
    -U | --url-list)
      shift
      URLFILE=$1
      INPUTMODE=2
      shift
      ;;
    -l | --lettr)
      shift
      LETTR=$1
      shift
      ;;
    -x | --tmx-output)
      FORMAT="TMX"
      shift
      ;;
    -B | --ignore-boilerpipe-cleaning)
      IGNOREBOILER="-b "
      shift
      ;;
    -a | --only-document-alignment)
      DOCALIGNMENT=1
      shift
      ;;
    -v | --vocabulary)
      shift
      VOCABULARY=$1
      shift
      ;;
    -b | --num-accepted-candidates)
      shift
      BIDIDOCALIGN=$1
      shift
      ;;
    -m | --maximum-wrong-alignments)
      shift
      MAXLINES=$1
      BYTEXT=1
      shift
      ;;
    -q | --seg-alignment-score-threshold)
      shift
      MINQUALITY=$1
      shift
      ;;
    -T | --tmp-dir)
      shift
      TMPDIR=$1
      mkdir -p $1
      shift
      ;;
    -s | --size-limit)
      shift
      SIZELIMIT="-s $1"
      shift
      ;;
    -t | --time-limit)
      shift
      TIMELIMIT="-t $1"
      shift
      ;;
    -O | --output)
      shift
      OUTPUT=$1
      shift
      ;;
    -d | --doc-alignment-score-threshold)
      shift
      DOCSIMTHRESHOLD="-t $1"
      shift
      ;;
    -j | --num-threads)
      shift
      JOBS="-j $1"
      shift
      ;;
    -c | --timeout-crawl)
      shift
      TIMEOUT="-o $1"
      shift
      ;;
    -p | --write-crawling-file)
      shift
      CRAWLINGSTATUSDUMP="$1"
      shift
      ;;
    -C | --continue-crawling-file)
      shift
      CRAWLINGSTATUSCONTINUE="$1"
      shift
      ;;
    -R | --reuse-crawling-file)
      shift
      CRAWLINGDATACONTINUE="$1"
      shift
      ;;
    -f | --dirname)
      shift
      DIRNAME="$1"
      shift
      ;;
    -D | --crawl-tld)
      TLD_CRAWL=" -D "
      shift
      ;;
    -W | --elrc-quality-metrics)
      PRINTSTATS="-s"
      HUNALIGNSCORE=",hunalign"
      ELRCSCORES=",lengthratio,numTokensSL,numTokensTL"
      shift
      ;;
    --filter-with-elrc)
      FILTERLINES="-f"
      shift
      ;;
    -n | --ulysses)
      SENLTK=""
      shift
      ;;
    -F | --config-file)
      shift
      shift
      ;;
    -H | --httrack)
      shift
      if [ $(which httrack| wc -l) -eq 0 ]; then
        echo "Error: the tool 'httrack' could not be found and it is necessary to download the websites. Please, first install this tool and then try again to run this script."
        exit
      else
        USEHTTRACK=1
      fi
      ;;
    --jhu-lett)
      shift
      USEJHULETT=1
      ;;
    --bicleaner)
      shift
      BICLEANER=$1
      BICLEANERSCORE=",bicleaner"
      shift
      ;;
    --zipporah)
      shift
      ZIPPORAH=$1
      ZIPPORAHSCORE=",zipporah"
      shift
      ;;
    --filter-bicleaner)
      shift
      BICLEANERTHRESHOLD=$1
      shift
      ;;
    --filter-zipporah)
      shift
      ZIPPORAHTHRESHOLD=$1
      shift
      ;;
    --paracrawl-aligner-command)
      shift
      TRANSLATIONCOMMAND="$1"
      shift
      ;;
    -h | --help)
      exit_program $(basename $0)
      ;;
    --)
      shift
      break
      ;;
  esac
done

case $# in
  2)
    LANG1="$1"
    LANG2="$2"
    if [[ -z $VOCABULARY ]]; then
      echo "THE VOCABULARY FILE WAS NOT SET: USE OPTION -v"
      exit_program $(basename $0)
    fi
    LANG1INVOC=$(head -n 1 $VOCABULARY | cut -f 1)
    LANG2INVOC=$(head -n 1 $VOCABULARY | cut -f 2)
    if [ "$LANG1" != "$LANG1INVOC" -a "$LANG1" != "$LANG2INVOC" ]; then
      echo -e "\nLANGUAGE \"$LANG1\" COULD NOT BE FOUND IN LEXICON \"$VOCABULARY\"; REMEMBER TO USE ISO 639-1 LANGUAGE CODES BOTH IN THE FIRST LINE OF THE LEXICON AND THE LANGUAGES CODES WHEN RUNNING BITEXTOR\n"
      exit_program $(basename $0)
    fi
    if [ "$LANG2" != "$LANG1INVOC" -a "$LANG2" != "$LANG2INVOC" ]; then
      echo -e "\nLANGUAGE \"$LANG2\" COULD NOT BE FOUND IN LEXICON \"$VOCABULARY\"; REMEMBER TO USE ISO 639-1 LANGUAGE CODES BOTH IN THE FIRST LINE OF THE LEXICON AND THE LANGUAGES CODES WHEN RUNNING BITEXTOR\n"
      exit_program $(basename $0)
    fi
    ;;
  *)
    exit_program $(basename $0)
    ;;
esac

TMPETT=$(mktemp $BUILDDICTTMP/ett.XXXXXX)
if [[ -z $ETT ]]; then
  ETT=$TMPETT
fi

TMPLETT=$(mktemp $BUILDDICTTMP/lett.XXXXXX)
rm $TMPLETT
mkfifo $TMPLETT
if [[ -z $LETT ]]; then
  LETT=$TMPLETT
fi

TMPLETTR=$(mktemp $BUILDDICTTMP/lettr.XXXXXX)
if [[ -z $LETTR ]]; then
  LETTR=$TMPLETTR
fi

case $INPUTMODE in
  1)
    run_bitextor $URL
    ;;
  2)
    zcat -f $URLFILE | \
    while read line;
    do
      URL=$(echo "$line" | cut -f 1)
      ETT=$(echo "$line" | cut -f 2)
      echo $line
      echo "$(echo $line | grep $'\t' | wc -l)"
      if [ $(echo $line | grep '\s' | wc -l) -eq 0 ]; then
        echo "Error in the format of the file containing the list of urls: in every line of the file, you have to include a URL and the path to the ETT file where the information downloaded will be stored, separated with a tab."
        exit -1
      else
        run_bitextor $URL
      fi
    done
    ;;
  3)
    run_bitextor_ett $ETT
    ;;
  4)
    run_bitextor_lett $LETT
    ;;
  5)
    TEMPHUNALIGN_DIC=$(mktemp $BUILDDICTTMP/hunalign_dic.XXXXXX)
    tail -n +2 $VOCABULARY | sed -r 's/^([^\s]+)\t([^\s]+)$/\2 @ \1/g' > $TEMPHUNALIGN_DIC
    cat $ALIGNEDDOCINPUT | align_segments $TEMPHUNALIGN_DIC | clean_segments | convert_to_tmx
    rm -rf $TEMPHUNALIGN_DIC
    ;;
  6)
    cat $ALIGNEDSENTSINPUT | clean_segments | convert_to_tmx
    ;;
  *)
    exit_program $(basename $0)
    ;;
esac

rm -rf $BUILDDICTTMP
