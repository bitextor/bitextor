#!/bin/bash

exit_program()
{
  >&2 echo "$1 [-w workdir] [-f force_command] [-j threads]"
  >&2 echo ""
  >&2 echo "Runs several tests to check Bitextor is working"
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
        tar xzvf "${output}/${langs}.tar.gz" -C "${output}"
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

WORK=${HOME}
FORCE=""
THREADS=1

while getopts "hf:w:j:" i; do
    case "$i" in
        h) exit_program "$(basename "$0")" ; break ;;
        w) WORK=${OPTARG};;
        f) FORCE="--${OPTARG}";;
        j) THREADS="${OPTARG}";;
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
    unzip "${WORK}/data/parallel-corpus/Europarl/en-fr.txt.zip" -d "${WORK}/data/parallel-corpus/Europarl" &
fi
if [ ! -f "${WORK}/data/parallel-corpus/DGT/en-fr.txt.zip" ]; then
    # ~5000000 lines
    wget -q https://object.pouta.csc.fi/OPUS-DGT/v2019/moses/en-fr.txt.zip -P "${WORK}/data/parallel-corpus/DGT" && \
    unzip "${WORK}/data/parallel-corpus/DGT/en-fr.txt.zip" -d "${WORK}/data/parallel-corpus/DGT" &
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
# MT
snakemake --snakefile "${BITEXTOR}/snakemake/Snakefile" ${FORCE} --notemp --config bitextor="${BITEXTOR}" profiling=True permanentDir="${WORK}/permanent/bitextor-mt-output-en-fr" dataDir="${WORK}/data/data-mt-en-fr" transientDir="${WORK}/transient-mt-en-fr" warcs="['${WORK}/data/warc/greenpeace.warc.gz']" preprocessor="giawarc" shards=1 batches=512 lang1=en lang2=fr documentAligner="externalMT" alignerCmd="bash ${BITEXTOR}/snakemake/example/dummy-translate.sh" sentenceAligner="bleualign" bicleaner="${BICLEANER}/en-fr/en-fr.yaml" deferred=True tmx=True -j ${THREADS} &> "${WORK}/reports/mt-en-fr.report" && echo "Ok" || (status="$?"; echo "Failed 1 (status: $status)"; echo "fail (1 with status $status)" >> "$FAILS") &
snakemake --snakefile "${BITEXTOR}/snakemake/Snakefile" ${FORCE} --notemp --config bitextor="${BITEXTOR}" profiling=True permanentDir="${WORK}/permanent/bitextor-mt-output-en-el" dataDir="${WORK}/data/data-mt-en-el" transientDir="${WORK}/transient-mt-en-el" warcs="['${WORK}/data/warc/primeminister.warc.gz']" preprocessor="giawarc" shards=1 batches=512 lang1=en lang2=el documentAligner="externalMT" alignerCmd="bash ${BITEXTOR}/snakemake/example/dummy-translate.sh" sentenceAligner="bleualign" deferred=True tmx=True -j ${THREADS} &> "${WORK}/reports/mt-en-el.report" && echo "Ok" || (status="$?"; echo "Failed 2 (status: $status)"; echo "fail (2 with status $status)" >> "$FAILS") &
snakemake --snakefile "${BITEXTOR}/snakemake/Snakefile" ${FORCE} --notemp --config bitextor="${BITEXTOR}" profiling=True permanentDir="${WORK}/permanent/bitextor-mt-output-en-ru" dataDir="${WORK}/data/data-mt-en-ru" transientDir="${WORK}/transient-mt-en-ru" warcs="['${WORK}/data/warc/kremlin.warc.gz']" preprocessor="giawarc" shards=1 batches=512 lang1=en lang2=ru documentAligner="externalMT" alignerCmd="bash ${BITEXTOR}/snakemake/example/dummy-translate.sh" sentenceAligner="bleualign" deferred=True tmx=True -j ${THREADS} &> "${WORK}/reports/mt-en-ru.report" && echo "Ok" || (status="$?"; echo "Failed 3 (status: $status)"; echo "fail (3 with status $status)" >> "$FAILS") &

# Dictionary-based
snakemake --snakefile "${BITEXTOR}/snakemake/Snakefile" ${FORCE} --notemp --config bitextor="${BITEXTOR}" profiling=True permanentDir="${WORK}/permanent/bitextor-output-en-fr" dataDir="${WORK}/data/data-en-fr" transientDir="${WORK}/transient-en-fr" warcs="['${WORK}/data/warc/greenpeace.warc.gz']" preprocessor="warc2preprocess" shards=1 batches=512 lang1=en lang2=fr documentAligner="DIC" dic="${WORK}/permanent/en-fr.dic" sentenceAligner="hunalign" bicleaner="${BICLEANER}/en-fr/en-fr.yaml" bicleanerThreshold=0.1 deferred=False tmx=True -j ${THREADS} &> "${WORK}/reports/en-fr.report" && echo "Ok" || (status="$?"; echo "Failed 4 (status: $status)"; echo "fail (4 with status $status)" >> "$FAILS") &
### Generate dictionary
dic_md5sum_before=$(md5sum "${WORK}/permanent/en-fr.dic" | awk '{print $1}')
rm -f "${WORK}/permanent/new-en-fr.dic"
snakemake --snakefile "${BITEXTOR}/snakemake/Snakefile" ${FORCE} --notemp --config bitextor="${BITEXTOR}" profiling=True permanentDir="${WORK}/permanent/bitextor-gendic-output-en-fr" dataDir="${WORK}/data/data-gendic-en-fr" transientDir="${WORK}/transient-gendic-en-fr" warcs="['${WORK}/data/warc/greenpeace.warc.gz']" preprocessor="warc2preprocess" shards=1 batches=512 lang1=en lang2=fr documentAligner="DIC" dic="${WORK}/permanent/new-en-fr.dic" initCorpusTrainingPrefix="['${WORK}/data/parallel-corpus/Europarl/Europarl.clipped.en-fr']" sentenceAligner="hunalign" bicleaner="${BICLEANER}/en-fr/en-fr.yaml" bicleanerThreshold=0.1 deferred=False tmx=True -j ${THREADS} &> "${WORK}/reports/gendic-en-fr.report" && echo "Ok" || (status="$?"; echo "Failed 5 (status: $status)"; echo "fail (5 with status $status)" >> "$FAILS") &

### Generate bicleaner model but use existant dictionary (a new dictionary will be generated anyways)
rm -f "${BICLEANER}/new-en-fr.yaml"
snakemake --snakefile "${BITEXTOR}/snakemake/Snakefile" ${FORCE} --notemp --config bitextor="${BITEXTOR}" profiling=True permanentDir="${WORK}/permanent/bitextor-genbicleaner-output-en-fr" dataDir="${WORK}/data/data-genbicleaner-en-fr" transientDir="${WORK}/transient-genbicleaner-en-fr" warcs="['${WORK}/data/warc/greenpeace.warc.gz']" preprocessor="warc2preprocess" shards=1 batches=512 lang1=en lang2=fr documentAligner="DIC" dic="${WORK}/permanent/en-fr.dic" initCorpusTrainingPrefix="['${WORK}/data/parallel-corpus/Europarl/Europarl.clipped.en-fr']" sentenceAligner="hunalign" bicleaner="${BICLEANER}/new-en-fr.yaml" bicleanerCorpusTrainingPrefix="['${WORK}/data/parallel-corpus/DGT/DGT.clipped.en-fr']" bicleanerThreshold=0.1 deferred=False tmx=True -j ${THREADS} &> "${WORK}/reports/genbicleaner-en-fr.report" && echo "Ok" || (status="$?"; echo "Failed 6 (status: $status)"; echo "fail (6 with status $status)" >> "$FAILS") && \
dic_md5sum_after=$(md5sum "${WORK}/permanent/en-fr.dic" | awk '{print $1}') &

### Generate dictionary and bicleaner model
rm -f "${WORK}/permanent/new-new-en-fr.dic"
rm -f "${BICLEANER}/new-new-en-fr.yaml"
snakemake --snakefile "${BITEXTOR}/snakemake/Snakefile" ${FORCE} --notemp --config bitextor="${BITEXTOR}" profiling=True permanentDir="${WORK}/permanent/bitextor-gendicbicleaner-output-en-fr" dataDir="${WORK}/data/data-gendicbicleaner-en-fr" transientDir="${WORK}/transient-gendicbicleaner-en-fr" warcs="['${WORK}/data/warc/greenpeace.warc.gz']" preprocessor="warc2preprocess" shards=1 batches=512 lang1=en lang2=fr documentAligner="DIC" dic="${WORK}/permanent/new-new-en-fr.dic" initCorpusTrainingPrefix="['${WORK}/data/parallel-corpus/Europarl/Europarl.clipped.en-fr']" sentenceAligner="hunalign" bicleaner="${BICLEANER}/new-new-en-fr.yaml" bicleanerCorpusTrainingPrefix="['${WORK}/data/parallel-corpus/DGT/DGT.clipped.en-fr']" bicleanerThreshold=0.1 deferred=False tmx=True -j ${THREADS} &> "${WORK}/reports/gendicbicleaner-en-fr.report" && echo "Ok" || (status="$?"; echo "Failed 7 (status: $status)"; echo "fail (7 with status $status)" >> "$FAILS") &

# MT and dictionary-based
snakemake --snakefile "${BITEXTOR}/snakemake/Snakefile" ${FORCE} --notemp --config bitextor="${BITEXTOR}" profiling=True permanentDir="${WORK}/permanent/bitextor-mtdb-output-en-fr" dataDir="${WORK}/data/data-mtdb-en-fr" transientDir="${WORK}/transient-mtdb-en-fr" warcs="['${WORK}/data/warc/greenpeace.warc.gz']" preprocessor="warc2preprocess" shards=1 batches=512 lang1=en lang2=fr documentAligner="externalMT" alignerCmd="bash ${BITEXTOR}/snakemake/example/dummy-translate.sh" dic="${WORK}/permanent/en-fr.dic" sentenceAligner="hunalign" bicleaner="${BICLEANER}/en-fr/en-fr.yaml" bicleanerThreshold=0.1 deferred=False tmx=True -j ${THREADS} &> "${WORK}/reports/mtdb-en-fr.report" && echo "Ok" || (status="$?"; echo "Failed 8 (status: $status)"; echo "fail (8 with status $status)" >> "$FAILS") &

# Other options
snakemake --snakefile "${BITEXTOR}/snakemake/Snakefile" ${FORCE} --notemp --config bitextor="${BITEXTOR}" profiling=True permanentDir="${WORK}/permanent/bitextor-mto1-output-en-ru" dataDir="${WORK}/data/data-mto1-en-ru" transientDir="${WORK}/transient-mto1-en-ru" warcs="['${WORK}/data/warc/kremlin.warc.gz']" preprocessor="giawarc" shards=1 batches=512 lang1=en lang2=ru documentAligner="externalMT" alignerCmd="bash ${BITEXTOR}/snakemake/example/dummy-translate.sh" sentenceAligner="bleualign" deferred=True tmx=True deduped=True biroamer=True -j ${THREADS} &> "${WORK}/reports/mto1-en-ru.report" && echo "Ok" || (status="$?"; echo "Failed 9 (status: $status)"; echo "fail (9 with status $status)" >> "$FAILS") &
snakemake --snakefile "${BITEXTOR}/snakemake/Snakefile" ${FORCE} --notemp --config bitextor="${BITEXTOR}" profiling=True permanentDir="${WORK}/permanent/bitextor-mto2-output-en-ru" dataDir="${WORK}/data/data-mto2-en-ru" transientDir="${WORK}/transient-mto2-en-ru" warcs="['${WORK}/data/warc/kremlin.warc.gz']" preprocessor="warc2preprocess" shards=1 batches=512 lang1=en lang2=ru documentAligner="externalMT" documentAlignerThreshold=0.1 alignerCmd="bash ${BITEXTOR}/snakemake/example/dummy-translate.sh" sentenceAligner="bleualign" sentenceAlignerThreshold=0.1 bicleaner="${BICLEANER}/en-fr/en-fr.yaml" bicleanerThreshold=0.0 deferred=False bifixer=True aggressiveDedup=True tmx=True deduped=True biroamer=True -j ${THREADS} &> "${WORK}/reports/mto2-en-ru.report" && echo "Ok" || (status="$?"; echo "Failed 10 (status: $status)"; echo "fail (10 with status $status)" >> "$FAILS") &

wait

# Post checking
if [[ "$dic_md5sum_after" != "" ]] && [[ "$dic_md5sum_before" != "$dic_md5sum_after" ]]; then
    echo "Dictionary has been replaced ($dic_md5sum_before -> $dic_md5sum_after), what is not the expected"
    echo "fail (6.1 with dictionary replaced)" >> "$FAILS"
fi

# Results
failed=$(cat "$FAILS" | wc -l)

echo "Fails summary:"
echo "--------------"
cat "$FAILS"

exit "$failed"
