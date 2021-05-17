#!/bin/bash

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

download_warc()
{
    warc=$1
    remote=$2
    if [ ! -f "${warc}" ]; then
        wget -q "${remote}" -O "${warc}"
    fi
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

BITEXTOR="$(dirname "$0")"
FAILS="${WORK}/data/fails.log"
mkdir -p "${WORK}"
mkdir -p "${WORK}/reports"
mkdir -p "${WORK}/data/warc"
rm -f "$FAILS"
touch "$FAILS"

# Download necessary files
# WARCs
download_warc "${WORK}/data/warc/primeminister.warc.gz" https://github.com/bitextor/bitextor-data/releases/download/bitextor-warc-v1.1/primeminister.warc.gz


# Run tests
# MT (id >= 10)

snakemake --snakefile "${BITEXTOR}/workflow/Snakefile" ${FORCE} --notemp --config bitextor="${BITEXTOR}" profiling=True permanentDir="${WORK}/permanent/bitextor-mt-output-en-el" dataDir="${WORK}/data/data-mt-en-el" transientDir="${WORK}/transient-mt-en-el" warcs="['${WORK}/data/warc/primeminister.warc.gz']" preprocessor="warc2text" shards=1 batches=512 lang1=en lang2=el documentAligner="externalMT" alignerCmd="bash ${BITEXTOR}/workflow/example/dummy-translate.sh" sentenceAligner="bleualign" deferred=True tmx=True bifixer=True deduped=True -j ${THREADS} &> "${WORK}/reports/10-mt-en-el.report" && python3 "${BITEXTOR}/deferred-annotation-reconstructor.py" "${WORK}/permanent/bitextor-mt-output-en-el/en-el.deduped.txt.gz" en el "${WORK}/data/warc/primeminister.warc.gz" | python3 "${BITEXTOR}/bifixer/bifixer/bifixer.py" --ignore_duplicates - - en el > "${WORK}/outputdeferred" && [ "$(diff ${WORK}/outputdeferred <(zcat ${WORK}/permanent/bitextor-mt-output-en-el/en-el.deduped.txt.gz))" == "" ] && echo "Ok 10 (no. lines in deferred reconstruction: $(zcat -f ${WORK}/outputdeferred | wc -l))" || (status="$?"; echo "Failed 10 (status: $status)"; echo "fail 10 $status" >> "$FAILS")

# Results
failed=$(cat "$FAILS" | wc -l)

echo "------------------------------------"
echo "           Fails Summary            "
echo "------------------------------------"
echo "status | test-id | exit code / desc."
cat "$FAILS"

exit "$failed"
