#!/bin/bash

set -o pipefail

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
      exit_program $(basename $0)
      break
      ;;
    w)
      WORK=$OPTARG
      break
      ;;
    --)
      shift
      break
      ;;
     *)
      exit_program $(basename $0)
      break
  esac
done
shift $((OPTIND-1))

if [ ! -f ${WORK}/permanent/en-fr.dic ]; then
	mkdir ${WORK}/permanent
	wget https://github.com/bitextor/bitextor-data/releases/download/bitextor-v1.0/en-fr.dic -P ${WORK}/permanent
fi

if [ ! -f ${WORK}/permanent/en-ru.dic ]; then
        mkdir ${WORK}/permanent
        wget https://github.com/bitextor/bitextor-data/releases/download/bitextor-v1.0/en-ru.dic -P ${WORK}/permanent
fi

if [ ! -f ${WORK}/permanent/en-el.dic ]; then
        mkdir ${WORK}/permanent
        wget https://github.com/bitextor/bitextor-data/releases/download/bitextor-v1.0/en-el.dic -P ${WORK}/permanent
fi

if [ ! -f ${WORK}/bicleaner-model/en-fr/training.en-fr.yaml ]; then
	mkdir ${WORK}/bicleaner-model
	wget https://github.com/bitextor/bitextor-data/releases/download/bicleaner-v1.0/en-fr.tar.gz -P ${WORK}/bicleaner-model
	tar zxvf ${WORK}/bicleaner-model/en-fr.tar.gz -C ${WORK}/bicleaner-model
fi



if [ ! -f ${WORK}/permanent/bitextor-output-default-en-fr/warc/greenpeace.org/creepy.warc.xz ]; then
	mkdir -p ${WORK}/permanent/bitextor-output-default-en-fr/warc/greenpeace.org
	wget -qO- https://github.com/bitextor/bitextor-data/releases/download/bitextor-warc/greenpeace.canada.warc.xz > ${WORK}/permanent/bitextor-output-default-en-fr/warc/greenpeace.org/creepy.warc.xz
fi

if [ ! -f ${WORK}/permanent/bitextor-output-mt-en-fr/warc/greenpeace.org/creepy.warc.xz ]; then
        mkdir -p ${WORK}/permanent/bitextor-output-mt-en-fr/warc/greenpeace.org
        wget -qO- https://github.com/bitextor/bitextor-data/releases/download/bitextor-warc/greenpeace.canada.warc.xz > ${WORK}/permanent/bitextor-output-mt-en-fr/warc/greenpeace.org/creepy.warc.xz
fi



if [ ! -f ${WORK}/permanent/bitextor-output-default-en-el/warc/primeminister.gr/creepy.warc.xz ]; then
        mkdir -p ${WORK}/permanent/bitextor-output-default-en-el/warc/primeminister.gr
        wget -qO- https://github.com/bitextor/bitextor-data/releases/download/bitextor-warc/primeminister.warc.xz > ${WORK}/permanent/bitextor-output-default-en-el/warc/primeminister.gr/creepy.warc.xz
fi

if [ ! -f ${WORK}/permanent/bitextor-output-mt-en-el/warc/primeminister.gr/creepy.warc.xz ]; then
        mkdir -p ${WORK}/permanent/bitextor-output-mt-en-el/warc/primeminister.gr
        wget -qO- https://github.com/bitextor/bitextor-data/releases/download/bitextor-warc/primeminister.warc.xz > ${WORK}/permanent/bitextor-output-mt-en-el/warc/primeminister.gr/creepy.warc.xz
fi



if [ ! -f ${WORK}/permanent/bitextor-output-default-en-ru/warc/kremlin.ru/creepy.warc.xz ]; then
        mkdir -p ${WORK}/permanent/bitextor-output-default-en-ru/warc/kremlin.ru
        wget -qO- https://github.com/bitextor/bitextor-data/releases/download/bitextor-warc/kremlin.warc.xz > ${WORK}/permanent/bitextor-output-default-en-ru/warc/kremlin.ru/creepy.warc.xz
fi

if [ ! -f ${WORK}/permanent/bitextor-output-mt-en-ru/warc/kremlin.ru/creepy.warc.xz ]; then
        mkdir -p ${WORK}/permanent/bitextor-output-mt-en-ru/warc/kremlin.ru
        wget -qO- https://github.com/bitextor/bitextor-data/releases/download/bitextor-warc/kremlin.warc.xz > ${WORK}/permanent/bitextor-output-mt-en-ru/warc/kremlin.ru/creepy.warc.xz
fi


mkdir -p ${WORK}/reports

snakemake --snakefile "$(dirname $0)/snakemake/Snakefile" --configfile "$(dirname $0)/snakemake/example/tests/default.en-fr.yaml" -j 4 --config bitextor="$(dirname $0)" 2> ${WORK}/reports/default.en-fr.report &
snakemake --snakefile "$(dirname $0)/snakemake/Snakefile" --configfile "$(dirname $0)/snakemake/example/tests/external-mt.en-fr.yaml" -j 4 --config bitextor="$(dirname $0)" 2> ${WORK}/reports/external-mt.en-fr.report &

snakemake --snakefile "$(dirname $0)/snakemake/Snakefile" --configfile "$(dirname $0)/snakemake/example/tests/default.en-el.yaml" -j 4 --config bitextor="$(dirname $0)" 2> ${WORK}/reports/default.en-el.report &
snakemake --snakefile "$(dirname $0)/snakemake/Snakefile" --configfile "$(dirname $0)/snakemake/example/tests/external-mt.en-el.yaml" -j 4 --config bitextor="$(dirname $0)" 2> ${WORK}/reports/external-mt.en-el.report &

snakemake --snakefile "$(dirname $0)/snakemake/Snakefile" --configfile "$(dirname $0)/snakemake/example/tests/default.en-ru.yaml" -j 4 --config bitextor="$(dirname $0)" 2> ${WORK}/reports/default.en-ru.report &
snakemake --snakefile "$(dirname $0)/snakemake/Snakefile" --configfile "$(dirname $0)/snakemake/example/tests/external-mt.en-ru.yaml" -j 4 --config bitextor="$(dirname $0)" 2> ${WORK}/reports/external-mt.en-ru.report &

wait
