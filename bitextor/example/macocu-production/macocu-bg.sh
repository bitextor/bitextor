#!/usr/bin/bash

WORK=/data1/lpla/macocu
bitextor --notemp -j 32 \
            --config profiling=True permanentDir="${WORK}/permanent/bitextor-mt-output-en-bg-paragraph-and-loomchild-and-bicleanerai" \
                dataDir="${WORK}/data/data-mt-en-bg-paragraph-and-loomchild-and-bicleanerai" transientDir="${WORK}/transient-mt-en-bg-paragraph-and-loomchild-and-bicleanerai" \
                preverticalsFile="'/data1/lpla/prevertical_bg.list'" shards=1 batches=512 lang1=en lang2=bg \
                documentAligner="externalMT" alignerCmd="bash /home/lpla/bitextor/bitextor/example/marian-translate-mkbg.sh" translationDirection="bg2en" sentenceAligner="bleualign" \
                bifixer=True bicleaner=True bicleanerModel="${WORK}/bicleaner-model-ai/en-bg/metadata.yaml" deferred=True tmx=True boilerplateCleaning=True deduped=True paragraphIdentification=True \
            &> "${WORK}/reports/10-mt-en-bg-paragraph-and-loomchild-and-bicleanerai.report"

