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

BITEXTOR="bitextor-full ${FORCE} --notemp -j ${THREADS} -c ${THREADS}"
BITEXTOR_EXTRA_ARGS="profiling=True verbose=True"
FAILS="${WORK}/data/fails.log"
mkdir -p "${WORK}"
mkdir -p "${WORK}/permanent"
mkdir -p "${WORK}/data/warc"
mkdir -p "${WORK}/data/warc/clipped"
mkdir -p "${WORK}/data/parallel-corpus"
mkdir -p "${WORK}/data/parallel-corpus/Europarl"
mkdir -p "${WORK}/data/parallel-corpus/DGT"
mkdir -p "${WORK}/reports"
rm -f "$FAILS"
touch "$FAILS"

# Download necessary files
# WARCs
download_warc "${WORK}/data/warc/greenpeace.warc.gz" https://github.com/bitextor/bitextor-data/releases/download/bitextor-warc-v1.1/greenpeace.canada-small.warc.gz &
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
if [ ! -f "${WORK}/data/parallel-corpus/Europarl/Europarl.clipped.en-fr.en.gz" ]; then
    cat "${WORK}/data/parallel-corpus/Europarl/Europarl.en-fr.en" \
        | tail -n 10000 \
        | pigz -c > "${WORK}/data/parallel-corpus/Europarl/Europarl.clipped.en-fr.en.gz" &
fi
if [ ! -f "${WORK}/data/parallel-corpus/Europarl/Europarl.clipped.en-fr.fr.gz" ]; then
    cat "${WORK}/data/parallel-corpus/Europarl/Europarl.en-fr.fr" \
        | tail -n 10000 \
        | pigz -c > "${WORK}/data/parallel-corpus/Europarl/Europarl.clipped.en-fr.fr.gz" &
fi
### DGT parallel corpus clipped
if [ ! -f "${WORK}/data/parallel-corpus/DGT/DGT.clipped.en-fr.en.gz" ]; then
    cat "${WORK}/data/parallel-corpus/DGT/DGT.en-fr.en" \
        | tail -n 10000 \
        | pigz -c > "${WORK}/data/parallel-corpus/DGT/DGT.clipped.en-fr.en.gz" &
fi
if [ ! -f "${WORK}/data/parallel-corpus/DGT/DGT.clipped.en-fr.fr.gz" ]; then
    cat "${WORK}/data/parallel-corpus/DGT/DGT.en-fr.fr" \
        | tail -n 10000 \
        | pigz -c > "${WORK}/data/parallel-corpus/DGT/DGT.clipped.en-fr.fr.gz" &
fi

wait

# MT (id >= 10)
## MT (en-fr)
(
    TEST_ID="10"
    TRANSIENT_DIR="${WORK}/transient/${TEST_ID}"

    mkdir -p "${TRANSIENT_DIR}" && \
    pushd "${TRANSIENT_DIR}" > /dev/null && \
    ${BITEXTOR} \
        --config permanentDir="${WORK}/permanent/${TEST_ID}" \
            dataDir="${WORK}/data/${TEST_ID}" transientDir="${TRANSIENT_DIR}" \
            warcs="['${WORK}/data/warc/greenpeace.warc.gz']" preprocessor="warc2text" shards=1 batches=512 lang1=en lang2=fr \
            documentAligner="externalMT" alignerCmd="bash ${DIR}/../bitextor/example/dummy-translate.sh" sentenceAligner="bleualign" \
            deferred=True tmx=True ${BITEXTOR_EXTRA_ARGS} \
        &> "${WORK}/reports/${TEST_ID}.report" && \
    popd > /dev/null

    annotate_and_echo_info "${TEST_ID}" "$?" "$(get_nolines ${WORK}/permanent/${TEST_ID}/en-fr.sent.gz)"
) &

# Dictionary-based (id >= 20)
## Use dictionary pipeline (en-fr)
(
    TEST_ID="20"
    TRANSIENT_DIR="${WORK}/transient/${TEST_ID}"

    mkdir -p "${TRANSIENT_DIR}" && \
    pushd "${TRANSIENT_DIR}" > /dev/null && \
    ${BITEXTOR} \
        --config permanentDir="${WORK}/permanent/${TEST_ID}" dataDir="${WORK}/data/${TEST_ID}" \
            transientDir="${TRANSIENT_DIR}" warcs="['${WORK}/data/warc/greenpeace.warc.gz']" preprocessor="warc2text" \
            shards=1 batches=512 lang1=en lang2=fr documentAligner="DIC" dic="${WORK}/permanent/en-fr.dic" \
            sentenceAligner="hunalign" deferred=False tmx=True ${BITEXTOR_EXTRA_ARGS} \
        &> "${WORK}/reports/${TEST_ID}.report" && \
    popd > /dev/null

    annotate_and_echo_info "${TEST_ID}" "$?" "$(get_nolines ${WORK}/permanent/${TEST_ID}/en-fr.sent.gz)"
) &

wait

# MT and dictionary-based (id >= 60)
## Combine MT and dictionary (en-fr)
(
    TEST_ID="60"
    TRANSIENT_DIR="${WORK}/transient/${TEST_ID}"

    mkdir -p "${TRANSIENT_DIR}" && \
    pushd "${TRANSIENT_DIR}" > /dev/null && \
    ${BITEXTOR} \
        --config permanentDir="${WORK}/permanent/${TEST_ID}" \
            dataDir="${WORK}/data/${TEST_ID}" transientDir="${TRANSIENT_DIR}" \
            warcs="['${WORK}/data/warc/greenpeace.warc.gz']" preprocessor="warc2text" shards=1 batches=512 lang1=en lang2=fr \
            documentAligner="externalMT" alignerCmd="bash ${DIR}/../bitextor/example/dummy-translate.sh" \
            dic="${WORK}/permanent/en-fr.dic" sentenceAligner="hunalign" deferred=False tmx=True ${BITEXTOR_EXTRA_ARGS} \
        &> "${WORK}/reports/${TEST_ID}.report" && \
    popd > /dev/null

    annotate_and_echo_info "${TEST_ID}" "$?" "$(get_nolines ${WORK}/permanent/${TEST_ID}/en-fr.sent.gz)"
) &

# Other options (id >= 100)
## MT and W2P with FTFY (en-fr)
(
    TEST_ID="100"
    TRANSIENT_DIR="${WORK}/transient/${TEST_ID}"

    mkdir -p "${TRANSIENT_DIR}" && \
    pushd "${TRANSIENT_DIR}" > /dev/null && \
    ${BITEXTOR} \
        --config permanentDir="${WORK}/permanent/${TEST_ID}" \
            dataDir="${WORK}/data/${TEST_ID}" transientDir="${TRANSIENT_DIR}" \
            warcs="['${WORK}/data/warc/greenpeace.warc.gz']" preprocessor="warc2preprocess" shards=1 batches=512 lang1=en lang2=fr \
            documentAligner="externalMT" alignerCmd="bash ${DIR}/../bitextor/example/dummy-translate.sh" sentenceAligner="bleualign" \
            deferred=False ftfy=True tmx=True deduped=True ${BITEXTOR_EXTRA_ARGS} \
        &> "${WORK}/reports/${TEST_ID}.report" && \
    popd > /dev/null

    annotate_and_echo_info "${TEST_ID}" "$?" "$(get_nolines ${WORK}/permanent/${TEST_ID}/en-fr.sent.gz)"
) &
## MT and docalign / segalign threshold and deduped (en-fr)
(
    TEST_ID="101"
    TRANSIENT_DIR="${WORK}/transient/${TEST_ID}"

    mkdir -p "${TRANSIENT_DIR}" && \
    pushd "${TRANSIENT_DIR}" > /dev/null && \
    ${BITEXTOR} \
        --config permanentDir="${WORK}/permanent/${TEST_ID}" \
            dataDir="${WORK}/data/${TEST_ID}" transientDir="${TRANSIENT_DIR}" \
            warcs="['${WORK}/data/warc/greenpeace.warc.gz']" preprocessor="warc2text" shards=1 batches=512 lang1=en lang2=fr \
            documentAligner="externalMT" documentAlignerThreshold=0.1 alignerCmd="bash ${DIR}/../bitextor/example/dummy-translate.sh" \
            sentenceAligner="bleualign" sentenceAlignerThreshold=0.1 deferred=False tmx=True deduped=True ${BITEXTOR_EXTRA_ARGS} \
        &> "${WORK}/reports/${TEST_ID}.report" && \
    popd > /dev/null

    annotate_and_echo_info "${TEST_ID}" "$?" "$(get_nolines ${WORK}/permanent/${TEST_ID}/en-fr.sent.gz)"
) &

wait

# Results
failed=$(cat "$FAILS" | wc -l)

echo "-------------------------------------"
echo "            Fails Summary            "
echo "-------------------------------------"
echo -e "status\ttest-id\texit code / desc."
cat "$FAILS"

exit "$failed"
