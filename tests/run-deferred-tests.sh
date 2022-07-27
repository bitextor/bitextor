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
mkdir -p "${WORK}/transient"
mkdir -p "${WORK}/data/warc"
mkdir -p "${WORK}/reports"
mkdir -p "${WORK}/output_reference"
rm -f "$FAILS"
touch "$FAILS"

# Download necessary files
# WARCs
download_file "${WORK}/data/warc/primeminister.warc.gz" https://github.com/bitextor/bitextor-data/releases/download/bitextor-warc-v1.1/primeminister.warc.gz &
# Output reference
download_file "${WORK}/output_reference/run-deferred-tests.tgz" https://github.com/bitextor/bitextor-testing-output/releases/download/v1/run-deferred-tests.tgz &

wait

WARC="${WORK}/data/warc/primeminister.warc.gz"

# Process downloaded files if necessary
tar -xzf "${WORK}/output_reference/run-deferred-tests.tgz" -C "${WORK}/output_reference/" && \
rm -f "${WORK}/output_reference/run-deferred-tests.tgz"

# MT (id >= 10)
## MT (en-el)
TEST_ID="10"
TRANSIENT_DIR="${WORK}/transient/${TEST_ID}"

mkdir -p "${TRANSIENT_DIR}" && \
pushd "${TRANSIENT_DIR}" > /dev/null && \
${BITEXTOR} \
  --config permanentDir="${WORK}/permanent/${TEST_ID}" \
    dataDir="${WORK}/data/${TEST_ID}" transientDir="${TRANSIENT_DIR}" \
    warcs="['${WARC}']" preprocessor="warc2text" shards=1 batches=512 lang1=en lang2=el \
    documentAligner="externalMT" alignerCmd="bash ${DIR}/../bitextor/example/dummy-translate.sh" \
    sentenceAligner="bleualign" deferred=True tmx=True bifixer=True deduped=True \
    bifixerIgnoreSegmentation=False ${BITEXTOR_EXTRA_ARGS} \
  &> "${WORK}/reports/${TEST_ID}.report" && \
popd > /dev/null

BITEXTOR_STATUS=$?

if [ ${BITEXTOR_STATUS} -eq 0 ]; then
  BITEXTOR_OUTPUT_DEDUPED="${WORK}/permanent/${TEST_ID}/en-el.deduped.txt.gz"
  RECONSTRUCTOR="${DIR}/../third_party/deferred-crawling/deferred-annotation-reconstructor.py --header"

  python3 ${RECONSTRUCTOR} ${BITEXTOR_OUTPUT_DEDUPED} en el ${WARC} --header \
    | bifixer -q --sdeferredcol src_deferred_hash --tdeferredcol trg_deferred_hash \
      --ignore_duplicates - - en el --header \
    > "${WORK}/${TEST_ID}.output_deferred"

  d=$(diff ${WORK}/${TEST_ID}.output_deferred <(zcat ${BITEXTOR_OUTPUT_DEDUPED}) | wc -l)
  exit_code=$([[ "$d" != "0" ]] && echo 1 || echo 0)
  annotate_and_echo_info_wrapper
else
  annotate_and_echo_info_wrapper
fi

# Get hashes from all files
for TEST_ID in $(echo "10"); do
    create_integrity_report "$WORK" "${WORK}/reports/hash_values_${TEST_ID}.report" "$TEST_ID"
done

# Results
failed=$(cat "$FAILS" | wc -l)

echo "-------------------------------------"
echo "            Fails Summary            "
echo "-------------------------------------"
echo -e "status\ttest-id\texit code / desc."
cat "$FAILS"

exit "$failed"
