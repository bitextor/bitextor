#!/usr/bin/bash

WORK=/data1/lpla/macocu
bitextor --notemp -j 32 \
            --config profiling=True permanentDir="${WORK}/permanent/bitextor-mt-output-en-mt-paragraph-and-loomchild-and-bicleanerai" \
                dataDir="${WORK}/data/data-mt-en-mt-paragraph-and-loomchild-and-bicleanerai" transientDir="${WORK}/transient-mt-en-mt-paragraph-and-loomchild-and-bicleanerai" \
                preverticalsFile="'/data1/lpla/prevertical_mt.list'" shards=1 batches=512 lang1=en lang2=mt \
                documentAligner="externalMT" alignerCmd="bash /home/lpla/bitextor/bitextor/example/marian-translate-mt.sh" translationDirection="mt2en" sentenceAligner="bleualign" \
                bifixer=True bicleaner=True bicleanerModel="${WORK}/bicleaner-model-ai/en-mt/metadata.yaml" deferred=True tmx=True boilerplateCleaning=True deduped=True paragraphIdentification=True \
            &> "${WORK}/reports/10-mt-en-mt-paragraph-and-loomchild-and-bicleanerai.report"

