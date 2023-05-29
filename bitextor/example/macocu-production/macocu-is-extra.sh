#!/usr/bin/bash

WORK=/data1/lpla/macocu-release-1-using-release-2-code
bitextor --notemp -j 32 \
            --config profiling=True permanentDir="${WORK}/permanent/bitextor-mt-output-en-is-extra" granularity='["sentences", "documents"]' \
                dataDir="${WORK}/data/data-mt-en-is-extra" transientDir="${WORK}/transient-mt-en-is-extra" \
                preverticalsFile="'/data1/lpla/prevertical_is_extra.list'" shards=8 batches=999999 lang1=en lang2=is \
                documentAligner="externalMT" alignerCmd="bash /home/lpla/bitextor/bitextor/example/marian-translate-is.sh" translationDirection="is2en" sentenceAligner="bleualign" \
                bifixer=True bicleaner=True bicleanerModel="${WORK}/bicleaner-model-ai/en-is/metadata.yaml" deferred=True tmx=True boilerplateCleaning=True deduped=True paragraphIdentification=True additionalMetadata=True bicleanerExtraArgs="--disable_minimal_length" bicleanerThreshold=0.5 \
            &> "${WORK}/reports/10-mt-en-is-extra.report"

