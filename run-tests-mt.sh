#!/bin/bash

exit_program()

{
  >&2 echo "$1 [-w workdir]"
  >&2 echo ""
  >&2 echo "Runs several tests to check Bitextor is working"
  exit 1
}

WORK=${HOME}

while getopts "hw:" i; do
    case "$i" in
        h)
            exit_program "$(basename "$0")"
            break
            ;;
        w)
            WORK=$OPTARG
            break
            ;;
        *)
            exit_program "$(basename "$0")"
            break
    esac
done
shift $((OPTIND-1))

if [ ! -f "${WORK}"/bicleaner-model/en-fr/en-fr.yaml ]; then
	mkdir "${WORK}"/bicleaner-model
	wget https://github.com/bitextor/bicleaner-data/releases/latest/download/en-fr.tar.gz -P "${WORK}"/bicleaner-model
	tar zxvf "${WORK}"/bicleaner-model/en-fr.tar.gz -C "${WORK}"/bicleaner-model
fi

if [ ! -f "${WORK}"/data/warc/greenpeace.warc.gz ]; then
	mkdir -p "${WORK}"/data/warc
	wget -qO- https://github.com/bitextor/bitextor-data/releases/download/bitextor-warc-v1.1/greenpeace.canada.warc.gz -O "${WORK}"/data/warc/greenpeace.warc.gz
fi

if [ ! -f "${WORK}"/data/warc/primeminister.warc.gz ]; then
        mkdir -p "${WORK}"/data/warc
        wget -qO- https://github.com/bitextor/bitextor-data/releases/download/bitextor-warc-v1.1/primeminister.warc.gz -O "${WORK}"/data/warc/primeminister.warc.gz
fi

if [ ! -f "${WORK}"/data/warc/kremlin.warc.gz ]; then
        mkdir -p "${WORK}"/data/warc
        wget -qO- https://github.com/bitextor/bitextor-data/releases/download/bitextor-warc-v1.1/kremlin.warc.gz -O "${WORK}"/data/warc/kremlin.warc.gz
fi

BITEXTOR="$(dirname "$0")"
mkdir -p "${WORK}"/reports

snakemake --snakefile "${BITEXTOR}/snakemake/Snakefile" --notemp --config bitextor="${BITEXTOR}" permanentDir="${WORK}/permanent/bitextor-output-en-fr" dataDir="${WORK}/data/data-en-fr" transientDir="${WORK}/transient-en-fr" warcs="['${WORK}/data/warc/greenpeace.warc.gz']" preprocessor="giawarc" shards=1 batches=100 lang1=en lang2=fr documentAligner="externalMT" alignerCmd="bash ${BITEXTOR}/snakemake/example/dummy-translate.sh" sentenceAligner="bleualign" bicleaner="${WORK}/bicleaner-model/en-fr/en-fr.yaml" -j 4 &> "${WORK}/reports/en-fr.report" & 
snakemake --snakefile "${BITEXTOR}/snakemake/Snakefile" --notemp --config bitextor="${BITEXTOR}" permanentDir="${WORK}/permanent/bitextor-output-en-el" dataDir="${WORK}/data/data-en-el" transientDir="${WORK}/transient-en-el" warcs="['${WORK}/data/warc/primeminister.warc.gz']" preprocessor="giawarc" shards=1 batches=100 lang1=en lang2=el documentAligner="externalMT" alignerCmd="bash ${BITEXTOR}/snakemake/example/dummy-translate.sh" sentenceAligner="bleualign" -j 4 &> "${WORK}/reports/en-el.report" & 
snakemake --snakefile "${BITEXTOR}/snakemake/Snakefile" --notemp --config bitextor="${BITEXTOR}" permanentDir="${WORK}/permanent/bitextor-output-en-el" dataDir="${WORK}/data/data-en-ru" transientDir="${WORK}/transient-en-ru" warcs="['${WORK}/data/warc/kremlin.warc.gz']" preprocessor="giawarc" shards=1 batches=100 lang1=en lang2=ru documentAligner="externalMT" alignerCmd="bash ${BITEXTOR}/snakemake/example/dummy-translate.sh" sentenceAligner="bleualign" -j 4 &> "${WORK}/reports/en-ru.report" & 
wait
