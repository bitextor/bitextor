#!/bin/bash

exit_program()

{
  >&2 echo "$1 [-w workdir]"
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

WORK=${HOME}
FORCE=""
THREADS=1

while getopts "hf:w:j:" i; do
    case "$i" in
        h) exit_program "$(basename "$0")" ; break ;;
        w) WORK=${OPTARG};;
        f) FORCE="--${OPTARG}";;
        j) THREADS="${OPTARG}";;
        # *) exit_program "$(basename "$0")" ; break ;;
    esac
done
shift $((OPTIND-1))

mkdir -p ${WORK}

if [ ! -f "${WORK}"/bicleaner-model/en-fr/en-fr.yaml ]; then
	mkdir -p "${WORK}"/bicleaner-model
	wget https://github.com/bitextor/bicleaner-data/releases/latest/download/en-fr.tar.gz -P "${WORK}"/bicleaner-model
	tar zxvf "${WORK}"/bicleaner-model/en-fr.tar.gz -C "${WORK}"/bicleaner-model
fi

mkdir -p "${WORK}/data/warc"
download_warc "${WORK}/data/warc/greenpeace.warc.gz" https://github.com/bitextor/bitextor-data/releases/download/bitextor-warc-v1.1/greenpeace.canada.warc.gz &
download_warc "${WORK}/data/warc/primeminister.warc.gz" https://github.com/bitextor/bitextor-data/releases/download/bitextor-warc-v1.1/primeminister.warc.gz &
download_warc "${WORK}/data/warc/kremlin.warc.gz" https://github.com/bitextor/bitextor-data/releases/download/bitextor-warc-v1.1/kremlin.warc.gz &
wait

BITEXTOR="$(dirname "$0")"
mkdir -p "${WORK}"/reports

snakemake --snakefile "${BITEXTOR}/snakemake/Snakefile" ${FORCE} --notemp --config bitextor="${BITEXTOR}" permanentDir="${WORK}/permanent/bitextor-output-en-fr" dataDir="${WORK}/data/data-en-fr" transientDir="${WORK}/transient-en-fr" warcs="['${WORK}/data/warc/greenpeace.warc.gz']" preprocessor="giawarc" shards=1 batches=512 lang1=en lang2=fr documentAligner="externalMT" alignerCmd="bash ${BITEXTOR}/snakemake/example/dummy-translate.sh" sentenceAligner="bleualign" bicleaner="${WORK}/bicleaner-model/en-fr/en-fr.yaml" -j ${THREADS} &> "${WORK}/reports/en-fr.report" &
snakemake --snakefile "${BITEXTOR}/snakemake/Snakefile" ${FORCE} --notemp --config bitextor="${BITEXTOR}" permanentDir="${WORK}/permanent/bitextor-output-en-el" dataDir="${WORK}/data/data-en-el" transientDir="${WORK}/transient-en-el" warcs="['${WORK}/data/warc/primeminister.warc.gz']" preprocessor="giawarc" shards=1 batches=512 lang1=en lang2=el documentAligner="externalMT" alignerCmd="bash ${BITEXTOR}/snakemake/example/dummy-translate.sh" sentenceAligner="bleualign" -j ${THREADS} &> "${WORK}/reports/en-el.report" &
snakemake --snakefile "${BITEXTOR}/snakemake/Snakefile" ${FORCE} --notemp --config bitextor="${BITEXTOR}" permanentDir="${WORK}/permanent/bitextor-output-en-ru" dataDir="${WORK}/data/data-en-ru" transientDir="${WORK}/transient-en-ru" warcs="['${WORK}/data/warc/kremlin.warc.gz']" preprocessor="giawarc" shards=1 batches=512 lang1=en lang2=ru documentAligner="externalMT" alignerCmd="bash ${BITEXTOR}/snakemake/example/dummy-translate.sh" sentenceAligner="bleualign" -j ${THREADS} &> "${WORK}/reports/en-ru.report" &
wait
