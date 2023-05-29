#!/usr/bin/bash

WORK=/data1/lpla/macocu-release-1-using-release-2-code
bitextor --notemp -j 32 \
            --config profiling=True permanentDir="${WORK}/permanent/bitextor-mt-output-en-sl-r2code" granularity='["sentences", "documents"]' \
                dataDir="${WORK}/data/data-mt-en-sl-r2code" transientDir="${WORK}/transient-mt-en-sl-r2code" \
                preverticalsFile="'/data1/lpla/prevertical_sl_outgood.list'" shards=0 batches=999999 lang1=en lang2=sl \
                documentAligner="externalMT" alignerCmd="bash /home/lpla/bitextor/bitextor/example/marian-translate-sl.sh" translationDirection="sl2en" sentenceAligner="bleualign" \
                bifixer=True bicleaner=True bicleanerModel="${WORK}/bicleaner-model-ai/en-sl/metadata.yaml" deferred=True tmx=True boilerplateCleaning=True deduped=True paragraphIdentification=True additionalMetadata=True bicleanerExtraArgs="--disable_minimal_length" bicleanerThreshold=0.5 \
            &> "${WORK}/reports/10-mt-en-sl-r2code.report"

