#!/usr/bin/bash

WORK=/data1/lpla/macocu-release-2
bitextor --notemp -j 32 \
            --config profiling=True permanentDir="${WORK}/permanent/bitextor-mt-output-en-hbs-cyrillic-release-2-v2" granularity='["sentences", "documents"]' \
                dataDir="${WORK}/data/data-mt-en-hbs-cyrillic-release-2-v2" transientDir="${WORK}/transient-mt-en-hbs-cyrillic-release-2-v2" \
                preverticalsFile="'/data1/lpla/prevertical_hbs.list'" shards=8 batches=999999 lang1=en lang2=hbs_cyrillic \
                documentAligner="externalMT" alignerCmd="bash /home/lpla/bitextor/bitextor/example/marian-translate-hbs-cyrillic.sh" translationDirection="hbs_cyrillic2en" sentenceAligner="bleualign" \
                bifixer=True bicleaner=True bicleanerModel="${WORK}/bicleaner-model-ai/en-hbs/metadata.yaml" deferred=True tmx=True boilerplateCleaning=True deduped=True paragraphIdentification=True additionalMetadata=True bicleanerExtraArgs="--disable_minimal_length" bicleanerThreshold=0.5 \
            &> "${WORK}/reports/10-mt-en-hbs-cyrillic-release-2-v2.report"

