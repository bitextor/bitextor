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
BITEXTOR_EXTRA_ARGS=""
FAILS="${WORK}/data/fails.log"
mkdir -p "${WORK}"
mkdir -p "${WORK}/reports"
mkdir -p "${WORK}/data/warc"
rm -f "$FAILS"
touch "$FAILS"

# Download necessary files
# WARCs
download_warc "${WORK}/data/warc/primeminister.warc.gz" https://github.com/bitextor/bitextor-data/releases/download/bitextor-warc-v1.1/primeminister.warc.gz
WARC="${WORK}/data/warc/primeminister.warc.gz"

# MT (id >= 10)
TRANSIENT_DIR="${WORK}/transient-mt-en-el"

mkdir -p "${TRANSIENT_DIR}" && \
pushd "${TRANSIENT_DIR}" > /dev/null && \
${BITEXTOR} \
  --config profiling=True permanentDir="${WORK}/permanent/bitextor-mt-output-en-el" \
    dataDir="${WORK}/data/data-mt-en-el" transientDir="${TRANSIENT_DIR}" \
    warcs="['${WARC}']" preprocessor="warc2text" shards=1 batches=512 lang1=en lang2=el \
    documentAligner="externalMT" alignerCmd="bash ${DIR}/../bitextor/example/dummy-translate.sh" \
    sentenceAligner="bleualign" deferred=True tmx=True bifixer=True deduped=True ${BITEXTOR_EXTRA_ARGS} \
  &> "${WORK}/reports/10-mt-en-el.report" && \
popd > /dev/null

BITEXTOR_STATUS=$?

if [ ${BITEXTOR_STATUS} -eq 0 ]; then
  BITEXTOR_OUTPUT_DEDUPED="${WORK}/permanent/bitextor-mt-output-en-el/en-el.deduped.txt.gz"
  RECONSTRUCTOR="${DIR}/../deferred-crawling/deferred-annotation-reconstructor.py --header"

  python3 ${RECONSTRUCTOR} ${BITEXTOR_OUTPUT_DEDUPED} en el ${WARC} \
    | bifixer -q --sdeferredcol 6 --tdeferredcol 7 --ignore_duplicates - - en el  \
    > "${WORK}/outputdeferred"

  diff ${WORK}/outputdeferred <(zcat ${BITEXTOR_OUTPUT_DEDUPED}) -q > /dev/null
  annotate_and_echo_info 10 "$?" "$(wc -l ${WORK}/outputdeferred)"
else
  annotate_and_echo_info 10 "${BITEXTOR_STATUS}" "0"
fi

# Results
failed=$(cat "$FAILS" | wc -l)

echo "------------------------------------"
echo "           Fails Summary            "
echo "------------------------------------"
echo "status | test-id | exit code / desc."
cat "$FAILS"

exit "$failed"
