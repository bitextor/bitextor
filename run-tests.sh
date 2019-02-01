#!/bin/bash

set -o pipefail

exit_program()

{
  >&2 echo "run-tests"
  >&2 echo ""
  >&2 echo "Runs several tests to check Bitextor is working"
  exit 1
}


ARGS=$(getopt -o h -- "$@")
eval set -- $ARGS
for i
do
  case "$i" in
    -h | --help)
      exit_program $(basename $0)
      ;;
    --)
      shift
      break
      ;;
  esac
done

if [ ! -f ~/permanent/en-fr.dic ]; then
	mkdir ~/permanent
	wget https://github.com/bitextor/bitextor-data/releases/download/bitextor-v1.0/en-fr.dic -P ~/permanent
fi

if [ ! -f ~/bicleaner-model/en-fr/training.en-fr.yaml ]; then
	mkdir ~/bicleaner-model
	wget https://github.com/bitextor/bitextor-data/releases/download/bicleaner-v1.0/en-fr.tar.gz -P ~/bicleaner-model
	tar zxvf ~/bicleaner-model/en-fr.tar.gz -C ~/bicleaner-model
fi



if [ ! -f ~/permanent/bitextor-output-default-en-fr/warc/greenpeace.org/creepy.warc.xz ]; then
	mkdir -p ~/permanent/bitextor-output-default-en-fr/warc/greenpeace.org
	wget -qO- https://github.com/bitextor/bitextor-data/releases/download/bitextor-warc/greenpeace.canada.warc.xz > ~/permanent/bitextor-output-default-en-fr/warc/greenpeace.org/creepy.warc.xz
fi

if [ ! -f ~/permanent/bitextor-output-mt-en-fr/warc/greenpeace.org/creepy.warc.xz ]; then
        mkdir -p ~/permanent/bitextor-output-mt-en-fr/warc/greenpeace.org
        wget -qO- https://github.com/bitextor/bitextor-data/releases/download/bitextor-warc/greenpeace.canada.warc.xz > ~/permanent/bitextor-output-mt-en-fr/warc/greenpeace.org/creepy.warc.xz
fi



if [ ! -f ~/permanent/bitextor-output-default-en-el/warc/primeminister.gr/creepy.warc.xz ]; then
        mkdir -p ~/permanent/bitextor-output-default-en-el/warc/primeminister.gr
        wget -qO- https://github.com/bitextor/bitextor-data/releases/download/bitextor-warc/primeminister.warc.xz > ~/permanent/bitextor-output-default-en-el/warc/primeminister.gr/creepy.warc.xz
fi

if [ ! -f ~/permanent/bitextor-output-mt-en-el/warc/primeminister.gr/creepy.warc.xz ]; then
        mkdir -p ~/permanent/bitextor-output-mt-en-el/warc/primeminister.gr
        wget -qO- https://github.com/bitextor/bitextor-data/releases/download/bitextor-warc/primeminister.warc.xz > ~/permanent/bitextor-output-mt-en-el/warc/primeminister.gr/creepy.warc.xz
fi



if [ ! -f ~/permanent/bitextor-output-default-en-ru/warc/kremlin.ru/creepy.warc.xz ]; then
        mkdir -p ~/permanent/bitextor-output-default-en-ru/warc/kremlin.ru
        wget -qO- https://github.com/bitextor/bitextor-data/releases/download/bitextor-warc/kremlin.warc.xz > ~/permanent/bitextor-output-default-en-ru/warc/kremlin.ru/creepy.warc.xz
fi

if [ ! -f ~/permanent/bitextor-output-mt-en-ru/warc/kremlin.ru/creepy.warc.xz ]; then
        mkdir -p ~/permanent/bitextor-output-mt-en-ru/warc/kremlin.ru
        wget -qO- https://github.com/bitextor/bitextor-data/releases/download/bitextor-warc/kremlin.warc.xz > ~/permanent/bitextor-output-mt-en-ru/warc/kremlin.ru/creepy.warc.xz
fi



snakemake --snakefile "$(dirname $0)/snakemake/Snakefile" --configfile "$(dirname $0)/snakemake/example/tests/default.en-fr.yaml" -j 4 --config bitextor="$(dirname $0)"
snakemake --snakefile "$(dirname $0)/snakemake/Snakefile" --configfile "$(dirname $0)/snakemake/example/tests/external-mt.en-fr.yaml" -j 4 --config bitextor="$(dirname $0)"

snakemake --snakefile "$(dirname $0)/snakemake/Snakefile" --configfile "$(dirname $0)/snakemake/example/tests/default.en-el.yaml" -j 4 --config bitextor="$(dirname $0)"
snakemake --snakefile "$(dirname $0)/snakemake/Snakefile" --configfile "$(dirname $0)/snakemake/example/tests/external-mt.en-el.yaml" -j 4 --config bitextor="$(dirname $0)"

snakemake --snakefile "$(dirname $0)/snakemake/Snakefile" --configfile "$(dirname $0)/snakemake/example/tests/default.en-ru.yaml" -j 4 --config bitextor="$(dirname $0)"
snakemake --snakefile "$(dirname $0)/snakemake/Snakefile" --configfile "$(dirname $0)/snakemake/example/tests/external-mt.en-ru.yaml" -j 4 --config bitextor="$(dirname $0)"

