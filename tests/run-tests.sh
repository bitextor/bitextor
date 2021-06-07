#!/bin/bash

exit_program()
{
  >&2 echo "$1 [-w <workdir>] [-f <force_command>] [-j <threads>] [-t <tests>]"
  >&2 echo ""
  >&2 echo "Runs several tests to check Bitextor is working"
  >&2 echo ""
  >&2 echo "OPTIONS:"
  >&2 echo "  -w <workdir>            Working directory. By default: \$HOME"
  >&2 echo "  -f <force_command>      Options which will be provided to snakemake"
  >&2 echo "  -j <threads>            Threads to use when running the tests"
  >&2 echo "  -t <tests>              Tests which will be executed. The way they are"
  >&2 echo "                            specified is similar to 'chmod'. The expected format"
  >&2 echo "                            is numeric (e.g. 2 means to run the 2nd function of"
  >&2 echo "                            tests, 4 means to run the 3rd function of tests, 3"
  >&2 echo "                            means to run the 2nd and 3rd function of tests). Hex"
  >&2 echo "                            numbers can also be provided. By default: 2^32-1,"
  >&2 echo "                            which means running all the tests"

  exit 1
}

download_warc()
{
    warc=$1
    remote=$2
    if [ ! -f "${warc}" ]; then
        wget -q "${remote}" -O "${warc}"
    fi
}

download_bicleaner_model()
{
    base="https://github.com/bitextor/bicleaner-data/releases/latest/download"
    langs=$1
    output=$2
    if [ ! -f "${output}/${langs}.tar.gz" ]; then
        wget -q "${base}/${langs}.tar.gz" -P "${output}"
        tar xzf "${output}/${langs}.tar.gz" -C "${output}"
    fi
}

download_dictionary()
{
    base="https://github.com/bitextor/bitextor-data/releases/download/bitextor-v1.0"
    langs=$1
    output=$2
    if [ ! -f "${output}/${langs}.dic" ]; then
        wget -q "${base}/${langs}.dic" -P "${output}"
    fi
}

WORK="${HOME}"
WORK="${WORK/#\~/$HOME}" # Expand ~ to $HOME
FORCE=""
THREADS=1
FLAGS="$(echo 2^32-1 | bc)"

while getopts "hf:w:j:t:" i; do
    case "$i" in
        h) exit_program "$(basename "$0")" ; break ;;
        w) WORK=${OPTARG};;
        f) FORCE="--${OPTARG}";;
        j) THREADS="${OPTARG}";;
        t) FLAGS="${OPTARG}";;
        *) exit_program "$(basename "$0")" ; break ;;
    esac
done
shift $((OPTIND-1))

BITEXTOR="$(dirname "$0")"
BICLEANER="${WORK}/bicleaner-model"
FAILS="${WORK}/data/fails.log"
mkdir -p "${WORK}"
mkdir -p "${WORK}/reports"
mkdir -p "${BICLEANER}"
mkdir -p "${BICLEANER}/new"
mkdir -p "${BICLEANER}/new-new"
mkdir -p "${WORK}/data/warc"
mkdir -p "${WORK}/data/parallel-corpus"
mkdir -p "${WORK}/data/parallel-corpus/Europarl"
mkdir -p "${WORK}/data/parallel-corpus/DGT"
rm -f "$FAILS"
touch "$FAILS"

# Download necessary files
# WARCs
download_warc "${WORK}/data/warc/greenpeace.warc.gz" https://github.com/bitextor/bitextor-data/releases/download/bitextor-warc-v1.1/greenpeace.canada.warc.gz &
download_warc "${WORK}/data/warc/primeminister.warc.gz" https://github.com/bitextor/bitextor-data/releases/download/bitextor-warc-v1.1/primeminister.warc.gz &
download_warc "${WORK}/data/warc/kremlin.warc.gz" https://github.com/bitextor/bitextor-data/releases/download/bitextor-warc-v1.1/kremlin.warc.gz &
# Bicleaner models
download_bicleaner_model "en-fr" "${BICLEANER}" &
# Dictionaries
download_dictionary "en-fr" "${WORK}/permanent" &
# Parallel corpus
if [ ! -f "${WORK}/data/parallel-corpus/Europarl/en-fr.txt.zip" ]; then
    # ~2000000 lines
    wget -q https://object.pouta.csc.fi/OPUS-Europarl/v8/moses/en-fr.txt.zip -P "${WORK}/data/parallel-corpus/Europarl" && \
    unzip -qq "${WORK}/data/parallel-corpus/Europarl/en-fr.txt.zip" -d "${WORK}/data/parallel-corpus/Europarl" &
fi
if [ ! -f "${WORK}/data/parallel-corpus/DGT/en-fr.txt.zip" ]; then
    # ~5000000 lines
    wget -q https://object.pouta.csc.fi/OPUS-DGT/v2019/moses/en-fr.txt.zip -P "${WORK}/data/parallel-corpus/DGT" && \
    unzip -qq "${WORK}/data/parallel-corpus/DGT/en-fr.txt.zip" -d "${WORK}/data/parallel-corpus/DGT" &
fi
wait

# Preprocess
### Europarl parallel corpus clipped
if [ ! -f "${WORK}/data/parallel-corpus/Europarl/Europarl.clipped.en-fr.en.xz" ]; then
    cat "${WORK}/data/parallel-corpus/Europarl/Europarl.en-fr.en" | tail -n 100000 > "${WORK}/data/parallel-corpus/Europarl/Europarl.clipped.en-fr.en" && \
        xz "${WORK}/data/parallel-corpus/Europarl/Europarl.clipped.en-fr.en" &
fi
if [ ! -f "${WORK}/data/parallel-corpus/Europarl/Europarl.clipped.en-fr.fr.xz" ]; then
    cat "${WORK}/data/parallel-corpus/Europarl/Europarl.en-fr.fr" | tail -n 100000 > "${WORK}/data/parallel-corpus/Europarl/Europarl.clipped.en-fr.fr" && \
        xz "${WORK}/data/parallel-corpus/Europarl/Europarl.clipped.en-fr.fr" &
fi
### DGT parallel corpus clipped
if [ ! -f "${WORK}/data/parallel-corpus/DGT/DGT.clipped.en-fr.en.xz" ]; then
    cat "${WORK}/data/parallel-corpus/DGT/DGT.en-fr.en" | tail -n 100000 > "${WORK}/data/parallel-corpus/DGT/DGT.clipped.en-fr.en" && \
        xz "${WORK}/data/parallel-corpus/DGT/DGT.clipped.en-fr.en" &
fi
if [ ! -f "${WORK}/data/parallel-corpus/DGT/DGT.clipped.en-fr.fr.xz" ]; then
    cat "${WORK}/data/parallel-corpus/DGT/DGT.en-fr.fr" | tail -n 100000 > "${WORK}/data/parallel-corpus/DGT/DGT.clipped.en-fr.fr" && \
        xz "${WORK}/data/parallel-corpus/DGT/DGT.clipped.en-fr.fr" &
fi

wait

# Tests
annotate_and_echo_info()
{
  test_id=$1
  status=$2
  nolines=$3
  error_file="$FAILS"

  if [[ "$status" == "0" ]] && [[ "$nolines" != "0" ]]; then
    echo "Ok ${test_id} (nolines: ${nolines})"
  else if [[ "$status" != "0" ]]; then
    echo "Failed ${test_id} (status: ${status})"
    echo "fail ${test_id} ${status}" >> "$error_file"
  else if [[ "$nolines" == "0" ]]; then
    echo "Failed ${test_id} (nolines: ${nolines})"
    echo "fail ${test_id} '0 no. lines'" >> "$error_file"
  fi
  fi
  fi
}

# MT (id >= 10)
tests-mt()
{
    (snakemake --snakefile "${BITEXTOR}/workflow/Snakefile" ${FORCE} --notemp --config bitextor="${BITEXTOR}" profiling=True permanentDir="${WORK}/permanent/bitextor-mt-output-en-fr" dataDir="${WORK}/data/data-mt-en-fr" transientDir="${WORK}/transient-mt-en-fr" warcs="['${WORK}/data/warc/greenpeace.warc.gz']" preprocessor="warc2text" shards=1 batches=512 lang1=en lang2=fr documentAligner="externalMT" alignerCmd="bash ${BITEXTOR}/workflow/example/dummy-translate.sh" sentenceAligner="bleualign" bicleaner="${BICLEANER}/en-fr/en-fr.yaml" deferred=True tmx=True -j ${THREADS} &> "${WORK}/reports/10-mt-en-fr.report"; (status="$?"; nolines=$(zcat ${WORK}/permanent/bitextor-mt-output-en-fr/en-fr.sent.gz | wc -l); annotate_and_echo_info 10 "$status" "$nolines")) &
    (snakemake --snakefile "${BITEXTOR}/workflow/Snakefile" ${FORCE} --notemp --config bitextor="${BITEXTOR}" profiling=True permanentDir="${WORK}/permanent/bitextor-mt-output-en-el" dataDir="${WORK}/data/data-mt-en-el" transientDir="${WORK}/transient-mt-en-el" warcs="['${WORK}/data/warc/primeminister.warc.gz']" preprocessor="warc2text" shards=1 batches=512 lang1=en lang2=el documentAligner="externalMT" alignerCmd="bash ${BITEXTOR}/workflow/example/dummy-translate.sh" sentenceAligner="bleualign" deferred=True tmx=True -j ${THREADS} &> "${WORK}/reports/11-mt-en-el.report"; (status="$?"; nolines=$(zcat ${WORK}/permanent/bitextor-mt-output-en-el/en-el.sent.gz | wc -l); annotate_and_echo_info 11 "$status" "$nolines")) &
    (snakemake --snakefile "${BITEXTOR}/workflow/Snakefile" ${FORCE} --notemp --config bitextor="${BITEXTOR}" profiling=True permanentDir="${WORK}/permanent/bitextor-mt-output-en-ru" dataDir="${WORK}/data/data-mt-en-ru" transientDir="${WORK}/transient-mt-en-ru" warcs="['${WORK}/data/warc/kremlin.warc.gz']" preprocessor="warc2text" shards=1 batches=512 lang1=en lang2=ru documentAligner="externalMT" alignerCmd="bash ${BITEXTOR}/workflow/example/dummy-translate.sh" sentenceAligner="bleualign" deferred=True tmx=True -j ${THREADS} &> "${WORK}/reports/12-mt-en-ru.report"; (status="$?"; nolines=$(zcat ${WORK}/permanent/bitextor-mt-output-en-ru/en-ru.sent.gz | wc -l); annotate_and_echo_info 12 "$status" "$nolines")) &
}

# Dictionary-based (id >= 20)
tests-db()
{
    (snakemake --snakefile "${BITEXTOR}/workflow/Snakefile" ${FORCE} --notemp --config bitextor="${BITEXTOR}" profiling=True permanentDir="${WORK}/permanent/bitextor-output-en-fr" dataDir="${WORK}/data/data-en-fr" transientDir="${WORK}/transient-en-fr" warcs="['${WORK}/data/warc/greenpeace.warc.gz']" preprocessor="warc2text" shards=1 batches=512 lang1=en lang2=fr documentAligner="DIC" dic="${WORK}/permanent/en-fr.dic" sentenceAligner="hunalign" bicleaner="${BICLEANER}/en-fr/en-fr.yaml" bicleanerThreshold=0.1 deferred=False tmx=True -j ${THREADS} &> "${WORK}/reports/20-en-fr.report"; (status="$?"; nolines=$(zcat ${WORK}/permanent/bitextor-output-en-fr/en-fr.sent.gz | wc -l); annotate_and_echo_info 20 "$status" "$nolines")) &
}

### Generate dictionary (id >= 30)
tests-gendic()
{
    dic_md5sum_before=$(md5sum "${WORK}/permanent/en-fr.dic" | awk '{print $1}')
    rm -f "${WORK}/permanent/new-en-fr.dic"

    if [[ "$CI" == "true" ]]; then
        snakemake --snakefile "${BITEXTOR}/workflow/Snakefile" ${FORCE} --notemp --config bitextor="${BITEXTOR}" profiling=True permanentDir="${WORK}/permanent/bitextor-gendic-output-en-fr" dataDir="${WORK}/data/data-gendic-en-fr" transientDir="${WORK}/transient-gendic-en-fr" warcs="['${WORK}/data/warc/greenpeace.warc.gz']" preprocessor="warc2text" shards=1 batches=512 lang1=en lang2=fr documentAligner="DIC" dic="${WORK}/permanent/new-en-fr.dic" initCorpusTrainingPrefix="['${WORK}/data/parallel-corpus/Europarl/Europarl.clipped.en-fr']" sentenceAligner="hunalign" bicleaner="${BICLEANER}/en-fr/en-fr.yaml" bicleanerThreshold=0.1 deferred=False tmx=True -j ${THREADS} &> "${WORK}/reports/30-gendic-en-fr.report"; (status="$?"; nolines=$(zcat ${WORK}/permanent/bitextor-gendic-output-en-fr/en-fr.sent.gz | wc -l); annotate_and_echo_info 30 "$status" "$nolines")
        echo "Removing '${WORK}/transient-gendic-en-fr' ($(du -sh ${WORK}/transient-gendic-en-fr | awk '{print $1}'))"
        rm -rf "${WORK}/transient-gendic-en-fr"
    else
        (snakemake --snakefile "${BITEXTOR}/workflow/Snakefile" ${FORCE} --notemp --config bitextor="${BITEXTOR}" profiling=True permanentDir="${WORK}/permanent/bitextor-gendic-output-en-fr" dataDir="${WORK}/data/data-gendic-en-fr" transientDir="${WORK}/transient-gendic-en-fr" warcs="['${WORK}/data/warc/greenpeace.warc.gz']" preprocessor="warc2text" shards=1 batches=512 lang1=en lang2=fr documentAligner="DIC" dic="${WORK}/permanent/new-en-fr.dic" initCorpusTrainingPrefix="['${WORK}/data/parallel-corpus/Europarl/Europarl.clipped.en-fr']" sentenceAligner="hunalign" bicleaner="${BICLEANER}/en-fr/en-fr.yaml" bicleanerThreshold=0.1 deferred=False tmx=True -j ${THREADS} &> "${WORK}/reports/30-gendic-en-fr.report"; (status="$?"; nolines=$(zcat ${WORK}/permanent/bitextor-gendic-output-en-fr/en-fr.sent.gz | wc -l); annotate_and_echo_info 30 "$status" "$nolines")) &
    fi
}

### Generate bicleaner model but use existant dictionary (a new dictionary will be generated anyways) (id >= 40)
tests-genbicleaner()
{
    rm -f "${BICLEANER}/new/new-en-fr.yaml"

    if [[ "$CI" == "true" ]]; then
        # Run sequential instead of parallel in order to avoid get run out of memory (GitHub Actions)
        snakemake --snakefile "${BITEXTOR}/workflow/Snakefile" ${FORCE} --notemp --config bitextor="${BITEXTOR}" profiling=True permanentDir="${WORK}/permanent/bitextor-genbicleaner-output-en-fr" dataDir="${WORK}/data/data-genbicleaner-en-fr" transientDir="${WORK}/transient-genbicleaner-en-fr" warcs="['${WORK}/data/warc/greenpeace.warc.gz']" preprocessor="warc2text" shards=1 batches=512 lang1=en lang2=fr documentAligner="DIC" dic="${WORK}/permanent/en-fr.dic" initCorpusTrainingPrefix="['${WORK}/data/parallel-corpus/Europarl/Europarl.clipped.en-fr']" sentenceAligner="hunalign" bicleaner="${BICLEANER}/new/new-en-fr.yaml" bicleanerCorpusTrainingPrefix="['${WORK}/data/parallel-corpus/DGT/DGT.clipped.en-fr']" bicleanerThreshold=0.1 deferred=False tmx=True -j ${THREADS} &> "${WORK}/reports/40-genbicleaner-en-fr.report"; (status="$?"; nolines=$(zcat ${WORK}/permanent/bitextor-genbicleaner-output-en-fr/en-fr.sent.gz | wc -l); annotate_and_echo_info 40 "$status" "$nolines") && \
        dic_md5sum_after=$(md5sum "${WORK}/permanent/en-fr.dic" | awk '{print $1}')
        echo "Removing '${WORK}/transient-genbicleaner-en-fr' ($(du -sh ${WORK}/transient-genbicleaner-en-fr | awk '{print $1}'))"
        rm -rf "${WORK}/transient-genbicleaner-en-fr/"
    else
        (snakemake --snakefile "${BITEXTOR}/workflow/Snakefile" ${FORCE} --notemp --config bitextor="${BITEXTOR}" profiling=True permanentDir="${WORK}/permanent/bitextor-genbicleaner-output-en-fr" dataDir="${WORK}/data/data-genbicleaner-en-fr" transientDir="${WORK}/transient-genbicleaner-en-fr" warcs="['${WORK}/data/warc/greenpeace.warc.gz']" preprocessor="warc2text" shards=1 batches=512 lang1=en lang2=fr documentAligner="DIC" dic="${WORK}/permanent/en-fr.dic" initCorpusTrainingPrefix="['${WORK}/data/parallel-corpus/Europarl/Europarl.clipped.en-fr']" sentenceAligner="hunalign" bicleaner="${BICLEANER}/new/new-en-fr.yaml" bicleanerCorpusTrainingPrefix="['${WORK}/data/parallel-corpus/DGT/DGT.clipped.en-fr']" bicleanerThreshold=0.1 deferred=False tmx=True -j ${THREADS} &> "${WORK}/reports/40-genbicleaner-en-fr.report"; (status="$?"; nolines=$(zcat ${WORK}/permanent/bitextor-genbicleaner-output-en-fr/en-fr.sent.gz | wc -l); annotate_and_echo_info 40 "$status" "$nolines"; \
        dic_md5sum_after=$(md5sum "${WORK}/permanent/en-fr.dic" | awk '{print $1}')) &
    fi
}

### Generate dictionary and bicleaner model (id >= 50)
tests-gendic-genbicleaner()
{
    rm -f "${WORK}/permanent/new-new-en-fr.dic"
    rm -f "${BICLEANER}/new-new/new-new-en-fr.yaml"

    if [[ "$CI" == "true" ]]; then
        # Run sequential instead of parallel in order to avoid get run out of memory (GitHub Actions)
        snakemake --snakefile "${BITEXTOR}/workflow/Snakefile" ${FORCE} --notemp --config bitextor="${BITEXTOR}" profiling=True permanentDir="${WORK}/permanent/bitextor-gendicbicleaner-output-en-fr" dataDir="${WORK}/data/data-gendicbicleaner-en-fr" transientDir="${WORK}/transient-gendicbicleaner-en-fr" warcs="['${WORK}/data/warc/greenpeace.warc.gz']" preprocessor="warc2text" shards=1 batches=512 lang1=en lang2=fr documentAligner="DIC" dic="${WORK}/permanent/new-new-en-fr.dic" initCorpusTrainingPrefix="['${WORK}/data/parallel-corpus/Europarl/Europarl.clipped.en-fr']" sentenceAligner="hunalign" bicleaner="${BICLEANER}/new-new/new-new-en-fr.yaml" bicleanerCorpusTrainingPrefix="['${WORK}/data/parallel-corpus/DGT/DGT.clipped.en-fr']" bicleanerThreshold=0.1 deferred=False tmx=True -j ${THREADS} &> "${WORK}/reports/50-gendicbicleaner-en-fr.report"; (status="$?"; nolines=$(zcat ${WORK}/permanent/bitextor-gendicbicleaner-output-en-fr/en-fr.sent.gz | wc -l); annotate_and_echo_info 50 "$status" "$nolines")
        echo "Removing '${WORK}/transient-gendicbicleaner-en-fr' ($(du -sh ${WORK}/transient-gendicbicleaner-en-fr | awk '{print $1}'))"
        rm -rf "${WORK}/transient-gendicbicleaner-en-fr"
    else
        (snakemake --snakefile "${BITEXTOR}/workflow/Snakefile" ${FORCE} --notemp --config bitextor="${BITEXTOR}" profiling=True permanentDir="${WORK}/permanent/bitextor-gendicbicleaner-output-en-fr" dataDir="${WORK}/data/data-gendicbicleaner-en-fr" transientDir="${WORK}/transient-gendicbicleaner-en-fr" warcs="['${WORK}/data/warc/greenpeace.warc.gz']" preprocessor="warc2text" shards=1 batches=512 lang1=en lang2=fr documentAligner="DIC" dic="${WORK}/permanent/new-new-en-fr.dic" initCorpusTrainingPrefix="['${WORK}/data/parallel-corpus/Europarl/Europarl.clipped.en-fr']" sentenceAligner="hunalign" bicleaner="${BICLEANER}/new-new/new-new-en-fr.yaml" bicleanerCorpusTrainingPrefix="['${WORK}/data/parallel-corpus/DGT/DGT.clipped.en-fr']" bicleanerThreshold=0.1 deferred=False tmx=True -j ${THREADS} &> "${WORK}/reports/50-gendicbicleaner-en-fr.report"; (status="$?"; nolines=$(zcat ${WORK}/permanent/bitextor-gendicbicleaner-output-en-fr/en-fr.sent.gz | wc -l); annotate_and_echo_info 50 "$status" "$nolines")) &
    fi
}

# MT and dictionary-based (id >= 60)
tests-mt-db()
{
    (snakemake --snakefile "${BITEXTOR}/workflow/Snakefile" ${FORCE} --notemp --config bitextor="${BITEXTOR}" profiling=True permanentDir="${WORK}/permanent/bitextor-mtdb-output-en-fr" dataDir="${WORK}/data/data-mtdb-en-fr" transientDir="${WORK}/transient-mtdb-en-fr" warcs="['${WORK}/data/warc/greenpeace.warc.gz']" preprocessor="warc2text" shards=1 batches=512 lang1=en lang2=fr documentAligner="externalMT" alignerCmd="bash ${BITEXTOR}/workflow/example/dummy-translate.sh" dic="${WORK}/permanent/en-fr.dic" sentenceAligner="hunalign" bicleaner="${BICLEANER}/en-fr/en-fr.yaml" bicleanerThreshold=0.1 deferred=False tmx=True -j ${THREADS} &> "${WORK}/reports/60-mtdb-en-fr.report"; (status="$?"; nolines=$(zcat ${WORK}/permanent/bitextor-mtdb-output-en-fr/en-fr.sent.gz | wc -l); annotate_and_echo_info 60 "$status" "$nolines")) &
}

# Other options (id >= 100)
tests-others()
{
    (snakemake --snakefile "${BITEXTOR}/workflow/Snakefile" ${FORCE} --notemp --config bitextor="${BITEXTOR}" profiling=True permanentDir="${WORK}/permanent/bitextor-mto1-output-en-ru" dataDir="${WORK}/data/data-mto1-en-ru" transientDir="${WORK}/transient-mto1-en-ru" warcs="['${WORK}/data/warc/kremlin.warc.gz']" preprocessor="warc2text" shards=1 batches=512 lang1=en lang2=ru documentAligner="externalMT" alignerCmd="bash ${BITEXTOR}/workflow/example/dummy-translate.sh" sentenceAligner="bleualign" deferred=False tmx=True deduped=True biroamer=True -j ${THREADS} &> "${WORK}/reports/100-mto1-en-ru.report"; (status="$?"; nolines=$(zcat ${WORK}/permanent/bitextor-mto1-output-en-ru/en-ru.sent.gz | wc -l); annotate_and_echo_info 100 "$status" "$nolines")) &
    (snakemake --snakefile "${BITEXTOR}/workflow/Snakefile" ${FORCE} --notemp --config bitextor="${BITEXTOR}" profiling=True permanentDir="${WORK}/permanent/bitextor-mto2-output-en-fr" dataDir="${WORK}/data/data-mto2-en-fr" transientDir="${WORK}/transient-mto2-en-fr" warcs="['${WORK}/data/warc/greenpeace.warc.gz']" preprocessor="warc2text" shards=1 batches=512 lang1=en lang2=fr documentAligner="externalMT" documentAlignerThreshold=0.1 alignerCmd="bash ${BITEXTOR}/workflow/example/dummy-translate.sh" sentenceAligner="bleualign" sentenceAlignerThreshold=0.1 bicleaner="${BICLEANER}/en-fr/en-fr.yaml" bicleanerThreshold=0.0 deferred=False bifixer=True aggressiveDedup=True tmx=True deduped=True biroamer=True -j ${THREADS} &> "${WORK}/reports/101-mto2-en-fr.report"; (status="$?"; nolines=$(zcat ${WORK}/permanent/bitextor-mto2-output-en-fr/en-fr.sent.gz | wc -l); annotate_and_echo_info 101 "$status" "$nolines")) &
}

run-tests()
{
    flags="$1"

    if [[ "$(echo $flags | grep ^0x)" == "" ]]; then
        flags=$(printf '%x\n' "$1")
        flags="0x$flags"
    fi

    tests=(tests-mt tests-db tests-gendic tests-genbicleaner \
           tests-gendic-genbicleaner tests-mt-db tests-others)
    notests=$(echo "${#arr[@]}-1" | bc)

    for i in `seq 0 "$(echo ${#tests[@]}-1 | bc)"`; do
        # (flag & 2^notest) >> notest # will behaviour like chmod's mode
        # if we want to run the 1st and 2nd test, our flag must be 3, and would be like
        #  (3 & 2^0) >> 0 = (0b11 & 0b01) >> 0 = 0b01 >> 0 = 0b01 = 1 == 1
        #  (3 & 2^1) >> 1 = (0b11 & 0b10) >> 1 = 0b10 >> 1 = 0b01 = 1 == 1
        if [[ "$(( ($flags & (2**$i)) >> $i ))" == "1" ]]; then
            ${tests[$i]}
        fi
    done
}

run-tests "$FLAGS"

wait

# Post checking
if [[ "$dic_md5sum_before" != "" ]] && [[ "$dic_md5sum_after" != "" ]] && [[ "$dic_md5sum_before" != "$dic_md5sum_after" ]]; then
    echo "Failed 40.1 (dictionary has been replaced ($dic_md5sum_before -> $dic_md5sum_after), what is not the expected)"
    echo "fail 40.1 \"dictionary replaced\"" >> "$FAILS"
else
    echo "Ok 40.1"
fi

# Results
failed=$(cat "$FAILS" | wc -l)

echo "------------------------------------"
echo "           Fails Summary            "
echo "------------------------------------"
echo "status | test-id | exit code / desc."
cat "$FAILS"

exit "$failed"
