#!/bin/bash
DIR="$( cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
source $DIR/common.sh

exit_program()
{
  >&2 echo "$1 [-w workdir] [-f force_command] [-j threads]"
  >&2 echo ""
  >&2 echo "Runs several tests to check Bitextor is working"
  >&2 echo ""
  >&2 echo "OPTIONS:"
  >&2 echo "  -w <workdir>            Working directory. By default: \$HOME"
  >&2 echo "  -f <force_command>      Options which will be provided to snakemake"
  >&2 echo "  -j <threads>            Threads to use when running the tests"
  exit 1
}

WORK="${HOME}"
WORK="${WORK/#\~/$HOME}" # Expand ~ to $HOME
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

BITEXTOR="bitextor-full"
BITEXTOR_EXTRA_ARGS="-j ${THREADS} -c ${THREADS}"
FAILS="${WORK}/data/fails.log"
mkdir -p "${WORK}"
mkdir -p "${WORK}/reports"
mkdir -p "${WORK}/data/warc"
mkdir -p "${WORK}/data/warc/clipped"
mkdir -p "${WORK}/data/parallel-corpus"
mkdir -p "${WORK}/data/parallel-corpus/Europarl"
mkdir -p "${WORK}/data/parallel-corpus/DGT"
rm -f "$FAILS"
touch "$FAILS"

# Download necessary files
# WARCs
download_warc "${WORK}/data/warc/greenpeace.warc.gz" https://github.com/bitextor/bitextor-data/releases/download/bitextor-warc-v1.1/greenpeace.canada.warc.gz &
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
    cat "${WORK}/data/parallel-corpus/Europarl/Europarl.en-fr.en" | tail -n 10000 > "${WORK}/data/parallel-corpus/Europarl/Europarl.clipped.en-fr.en" && \
        xz "${WORK}/data/parallel-corpus/Europarl/Europarl.clipped.en-fr.en" &
fi
if [ ! -f "${WORK}/data/parallel-corpus/Europarl/Europarl.clipped.en-fr.fr.xz" ]; then
    cat "${WORK}/data/parallel-corpus/Europarl/Europarl.en-fr.fr" | tail -n 10000 > "${WORK}/data/parallel-corpus/Europarl/Europarl.clipped.en-fr.fr" && \
        xz "${WORK}/data/parallel-corpus/Europarl/Europarl.clipped.en-fr.fr" &
fi
### DGT parallel corpus clipped
if [ ! -f "${WORK}/data/parallel-corpus/DGT/DGT.clipped.en-fr.en.xz" ]; then
    cat "${WORK}/data/parallel-corpus/DGT/DGT.en-fr.en" | tail -n 10000 > "${WORK}/data/parallel-corpus/DGT/DGT.clipped.en-fr.en" && \
        xz "${WORK}/data/parallel-corpus/DGT/DGT.clipped.en-fr.en" &
fi
if [ ! -f "${WORK}/data/parallel-corpus/DGT/DGT.clipped.en-fr.fr.xz" ]; then
    cat "${WORK}/data/parallel-corpus/DGT/DGT.en-fr.fr" | tail -n 10000 > "${WORK}/data/parallel-corpus/DGT/DGT.clipped.en-fr.fr" && \
        xz "${WORK}/data/parallel-corpus/DGT/DGT.clipped.en-fr.fr" &
fi
### WARC clipped
if [ ! -f "${WORK}/data/warc/clipped/greenpeaceaa.warc.gz" ]; then
    ${DIR}/split-warc.py -r 1000 "${WORK}/data/warc/greenpeace.warc.gz" "${WORK}/data/warc/clipped/greenpeace" &
fi

wait

# Remove unnecessary clipped WARCs
ls "${WORK}/data/warc/clipped/" | grep -v "^greenpeaceaa[.]" | xargs -I{} rm "${WORK}/data/warc/clipped/{}"
# Rename and link
mv "${WORK}/data/warc/greenpeace.warc.gz" "${WORK}/data/warc/greenpeace.original.warc.gz"
ln -s "${WORK}/data/warc/clipped/greenpeaceaa.warc.gz" "${WORK}/data/warc/greenpeace.warc.gz"

# MT (id >= 10)
(
    TRANSIENT_DIR="${WORK}/transient-mt-en-fr"

    mkdir -p "${TRANSIENT_DIR}" && \
    pushd "${TRANSIENT_DIR}" > /dev/null && \
    ${BITEXTOR} ${FORCE} --notemp -j ${THREADS} \
        --config profiling=True permanentDir="${WORK}/permanent/bitextor-mt-output-en-fr" \
            dataDir="${WORK}/data/data-mt-en-fr" transientDir="${TRANSIENT_DIR}" \
            warcs="['${WORK}/data/warc/greenpeace.warc.gz']" preprocessor="warc2text" shards=1 batches=512 lang1=en lang2=fr \
            documentAligner="externalMT" alignerCmd="bash ${DIR}/../bitextor/example/dummy-translate.sh" sentenceAligner="bleualign" \
            deferred=True tmx=True ${BITEXTOR_EXTRA_ARGS} \
        &> "${WORK}/reports/10-mt-en-fr.report" && \
    popd > /dev/null

    annotate_and_echo_info 10 "$?" "$(get_nolines ${WORK}/permanent/bitextor-mt-output-en-fr/en-fr.sent.gz)"
) &

# Dictionary-based (id >= 20)
(
    TRANSIENT_DIR="${WORK}/transient-en-fr"

    mkdir -p "${TRANSIENT_DIR}" && \
    pushd "${TRANSIENT_DIR}" > /dev/null && \
    ${BITEXTOR} ${FORCE} --notemp -j ${THREADS} \
        --config profiling=True permanentDir="${WORK}/permanent/bitextor-output-en-fr" dataDir="${WORK}/data/data-en-fr" \
            transientDir="${TRANSIENT_DIR}" warcs="['${WORK}/data/warc/greenpeace.warc.gz']" preprocessor="warc2text" \
            shards=1 batches=512 lang1=en lang2=fr documentAligner="DIC" dic="${WORK}/permanent/en-fr.dic" \
            sentenceAligner="hunalign" deferred=False tmx=True ${BITEXTOR_EXTRA_ARGS} \
        &> "${WORK}/reports/20-en-fr.report" && \
    popd > /dev/null

    annotate_and_echo_info 20 "$?" "$(get_nolines ${WORK}/permanent/bitextor-output-en-fr/en-fr.sent.gz)"
) &

wait

# MT and dictionary-based (id >= 60)
(
    TRANSIENT_DIR="${WORK}/transient-mtdb-en-fr"

    mkdir -p "${TRANSIENT_DIR}" && \
    pushd "${TRANSIENT_DIR}" > /dev/null && \
    ${BITEXTOR} ${FORCE} --notemp -j ${THREADS} \
        --config profiling=True permanentDir="${WORK}/permanent/bitextor-mtdb-output-en-fr" \
            dataDir="${WORK}/data/data-mtdb-en-fr" transientDir="${TRANSIENT_DIR}" \
            warcs="['${WORK}/data/warc/greenpeace.warc.gz']" preprocessor="warc2text" shards=1 batches=512 lang1=en lang2=fr \
            documentAligner="externalMT" alignerCmd="bash ${DIR}/../bitextor/example/dummy-translate.sh" \
            dic="${WORK}/permanent/en-fr.dic" sentenceAligner="hunalign" deferred=False tmx=True ${BITEXTOR_EXTRA_ARGS} \
        &> "${WORK}/reports/60-mtdb-en-fr.report" && \
    popd > /dev/null

    annotate_and_echo_info 60 "$?" "$(get_nolines ${WORK}/permanent/bitextor-mtdb-output-en-fr/en-fr.sent.gz)"
) &

# Other options (id >= 100)
(
    TRANSIENT_DIR="${WORK}/transient-mto1-en-fr"

    mkdir -p "${TRANSIENT_DIR}" && \
    pushd "${TRANSIENT_DIR}" > /dev/null && \
    ${BITEXTOR} ${FORCE} --notemp -j ${THREADS} \
        --config profiling=True permanentDir="${WORK}/permanent/bitextor-mto1-output-en-fr" \
            dataDir="${WORK}/data/data-mto1-en-fr" transientDir="${TRANSIENT_DIR}" \
            warcs="['${WORK}/data/warc/greenpeace.warc.gz']" preprocessor="warc2preprocess" shards=1 batches=512 lang1=en lang2=fr \
            documentAligner="externalMT" alignerCmd="bash ${DIR}/../bitextor/example/dummy-translate.sh" sentenceAligner="bleualign" \
            deferred=False ftfy=True tmx=True deduped=True ${BITEXTOR_EXTRA_ARGS} \
        &> "${WORK}/reports/100-mto1-en-fr.report" && \
    popd > /dev/null

    annotate_and_echo_info 100 "$?" "$(get_nolines ${WORK}/permanent/bitextor-mto1-output-en-fr/en-fr.sent.gz)"
) &
(
    TRANSIENT_DIR="${WORK}/transient-mto2-en-fr"

    mkdir -p "${TRANSIENT_DIR}" && \
    pushd "${TRANSIENT_DIR}" > /dev/null && \
    ${BITEXTOR} ${FORCE} --notemp -j ${THREADS} \
        --config profiling=True permanentDir="${WORK}/permanent/bitextor-mto2-output-en-fr" \
            dataDir="${WORK}/data/data-mto2-en-fr" transientDir="${TRANSIENT_DIR}" \
            warcs="['${WORK}/data/warc/greenpeace.warc.gz']" preprocessor="warc2text" shards=1 batches=512 lang1=en lang2=fr \
            documentAligner="externalMT" documentAlignerThreshold=0.1 alignerCmd="bash ${DIR}/../bitextor/example/dummy-translate.sh" \
            sentenceAligner="bleualign" sentenceAlignerThreshold=0.1 deferred=False tmx=True deduped=True ${BITEXTOR_EXTRA_ARGS} \
        &> "${WORK}/reports/101-mto2-en-fr.report" && \
    popd > /dev/null

    annotate_and_echo_info 101 "$?" "$(get_nolines ${WORK}/permanent/bitextor-mto2-output-en-fr/en-fr.sent.gz)"
) &

wait

# Results
failed=$(cat "$FAILS" | wc -l)

echo "------------------------------------"
echo "           Fails Summary            "
echo "------------------------------------"
echo "status | test-id | exit code / desc."
cat "$FAILS"

exit "$failed"
