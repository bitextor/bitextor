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

if [ ! -f "${WORK}"/permanent/en-fr.dic ]; then
	mkdir -p "${WORK}"/permanent
	wget https://github.com/bitextor/bitextor-data/releases/download/bitextor-v1.0/en-fr.dic -P "${WORK}"/permanent
fi

if [ ! -f "${WORK}"/permanent/en-ru.dic ]; then
        mkdir -p "${WORK}"/permanent
        wget https://github.com/bitextor/bitextor-data/releases/download/bitextor-v1.0/en-ru.dic -P "${WORK}"/permanent
fi

if [ ! -f "${WORK}"/permanent/en-el.dic ]; then
        mkdir -p "${WORK}"/permanent
        wget https://github.com/bitextor/bitextor-data/releases/download/bitextor-v1.0/en-el.dic -P "${WORK}"/permanent
fi

if [ ! -f "${WORK}"/bicleaner-model/en-fr/en-fr.yaml ]; then
	mkdir -p "${WORK}"/bicleaner-model
	wget https://github.com/bitextor/bicleaner-data/releases/latest/download/en-fr.tar.gz -P "${WORK}"/bicleaner-model
	tar zxvf "${WORK}"/bicleaner-model/en-fr.tar.gz -C "${WORK}"/bicleaner-model
fi


if [ ! -f "${WORK}"/data/warc/greenpeace.org/wget.warc.gz ]; then
	mkdir -p "${WORK}"/data/warc/greenpeace.org
	wget -qO- https://github.com/bitextor/bitextor-data/releases/download/bitextor-warc-v1.1/greenpeace.canada.warc.gz -O "${WORK}"/data/warc/greenpeace.org/wget.warc.gz
fi

if [ ! -f "${WORK}"/data/warc/kremlin.ru/wget.warc.gz ]; then
        mkdir -p "${WORK}"/data/warc/kremlin.ru
        wget -qO- https://github.com/bitextor/bitextor-data/releases/download/bitextor-warc-v1.1/kremlin.warc.gz -O "${WORK}"/data/warc/kremlin.ru/wget.warc.gz
fi

if [ ! -f "${WORK}"/data/warc/primeminister.gr/wget.warc.gz ]; then
        mkdir -p "${WORK}"/data/warc/primeminister.gr
        wget -qO- https://github.com/bitextor/bitextor-data/releases/download/bitextor-warc-v1.1/primeminister.warc.gz -O "${WORK}"/data/warc/primeminister.gr/wget.warc.gz
fi


BITEXTOR="$(dirname "$0")"
mkdir -p "${WORK}"/reports


(
snakemake --snakefile "${BITEXTOR}/snakemake/Snakefile" --notemp --configfile "${BITEXTOR}/snakemake/example/tests/only-preprocess-greenpeace.yaml" --config bitextor="${BITEXTOR}" lang1='en' lang2='fr' permanentDir="${WORK}"/permanent/bitextor-output-preprocess-greenpeace/ dataDir="${WORK}"/data transientDir="${WORK}"/transient-greenpeace preprocessLangs="en,fr" boilerpipeCleaning=True -j 1 2> "${WORK}"/reports/preprocess-greenpeace.report
snakemake --snakefile "${BITEXTOR}/snakemake/Snakefile" --notemp --configfile "${BITEXTOR}/snakemake/example/tests/default.en-fr.yaml" --config bitextor="${BITEXTOR}" permanentDir="${WORK}"/permanent/bitextor-output-default-en-fr dataDir="${WORK}"/data transientDir="${WORK}"/transient-default-en-fr temp="${WORK}"/transient-default-en-fr dic="${WORK}"/permanent/en-fr.dic bicleaner="${WORK}"/bicleaner-model/en-fr/en-fr.yaml langId="cld3" bifixer=True -j 1 2> "${WORK}"/reports/default.en-fr.report &
snakemake --snakefile "${BITEXTOR}/snakemake/Snakefile" --notemp --configfile "${BITEXTOR}/snakemake/example/tests/external-mt.en-fr.yaml" --config bitextor="${BITEXTOR}" permanentDir="${WORK}"/permanent/bitextor-output-mt-en-fr dataDir="${WORK}"/data transientDir="${WORK}"/transient-mt-en-fr temp="${WORK}"/transient-mt-en-fr bleualign=False bicleaner="${WORK}"/bicleaner-model/en-fr/en-fr.yaml mosesDir="${WORK}"/permanent/software/mosesdecoder mgiza="${BITEXTOR}"/mgiza alignerCmd="bash ${BITEXTOR}/snakemake/example/dummy-translate.sh" -j 1 2> "${WORK}"/reports/external-mt.en-fr.report &
wait
) &

snakemake --snakefile "${BITEXTOR}/snakemake/Snakefile" --notemp --configfile "${BITEXTOR}/snakemake/example/tests/default.en-el.yaml" -j 1 --config bitextor="${BITEXTOR}" hosts=[""] WARCFiles="['${WORK}/data/warc/primeminister.gr/wget.warc.gz']" permanentDir="${WORK}"/permanent/bitextor-output-default-en-el dataDir="${WORK}"/permanent/bitextor-output-default-en-el transientDir="${WORK}"/transient-default-en-el temp="${WORK}"/transient-default-en-el dic="${WORK}"/permanent/en-el.dic preprocessLangs="en,el" parser="modest" 2> "${WORK}"/reports/default.en-el.report &
snakemake --snakefile "${BITEXTOR}/snakemake/Snakefile" --notemp --configfile "${BITEXTOR}/snakemake/example/tests/external-mt.en-el.yaml" -j 1 --config bitextor="${BITEXTOR}" hosts=[""] WARCFiles="['${WORK}/data/warc/primeminister.gr/wget.warc.gz']" permanentDir="${WORK}"/permanent/bitextor-output-mt-en-el dataDir="${WORK}"/permanent/bitextor-output-mt-en-el transientDir="${WORK}"/transient-mt-en-el temp="${WORK}"/transient-mt-en-el giawarc=True  mosesDir="${WORK}"/permanent/software/mosesdecoder mgiza="${BITEXTOR}"/mgiza docAlignWorkers=2 alignerCmd="bash ${BITEXTOR}/snakemake/example/dummy-translate.sh" 2> "${WORK}"/reports/external-mt.en-el.report &

(
snakemake --snakefile "${BITEXTOR}/snakemake/Snakefile" --notemp --configfile "${BITEXTOR}/snakemake/example/tests/only-preprocess-kremlin.yaml" --config bitextor="${BITEXTOR}" lang1='en' lang2='ru' permanentDir="${WORK}"/permanent/bitextor-output-preprocess-kremlin dataDir="${WORK}"/data transientDir="${WORK}"/transient-kremlin parser="simple" -j 1 2> "${WORK}"/reports/preprocess-kremlin.report
snakemake --snakefile "${BITEXTOR}/snakemake/Snakefile" --notemp --configfile "${BITEXTOR}/snakemake/example/tests/default.en-ru.yaml" -j 1 --config bitextor="${BITEXTOR}" permanentDir="${WORK}"/permanent/bitextor-output-default-en-ru dataDir="${WORK}"/data transientDir="${WORK}"/transient-default-en-ru temp="${WORK}"/transient-default-en-ru dic="${WORK}"/permanent/en-ru.dic deduped=True 2> "${WORK}"/reports/default.en-ru.report &
snakemake --snakefile "${BITEXTOR}/snakemake/Snakefile" --notemp --configfile "${BITEXTOR}/snakemake/example/tests/external-mt.en-ru.yaml" -j 1 --config bitextor="${BITEXTOR}" permanentDir="${WORK}"/permanent/bitextor-output-mt-en-ru dataDir="${WORK}"/data transientDir="${WORK}"/transient-mt-en-ru temp="${WORK}"/transient-mt-en-ru mosesDir="${WORK}"/permanent/software/mosesdecoder mgiza="${BITEXTOR}"/mgiza alignerCmd="bash ${BITEXTOR}/snakemake/example/dummy-translate.sh" deduped=True 2> "${WORK}"/reports/external-mt.en-ru.report &
wait
) &
wait
