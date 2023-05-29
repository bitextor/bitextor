#!/usr/bin/bash

WORK=/data1/lpla/macocu-release-1-using-release-2-code
bitextor --notemp -j 32 \
            --config profiling=True permanentDir="${WORK}/permanent/bitextor-mt-output-en-tr-r2code-v2" granularity='["sentences", "documents"]' \
                dataDir="${WORK}/data/data-mt-en-tr-r2code-v2" transientDir="${WORK}/transient-mt-en-tr-r2code-v2" \
                preverticalsFile="'/data1/lpla/prevertical_tr_outgood_cld2.list'" shards=8 batches=999999 lang1=en lang2=tr \
                documentAligner="externalMT" alignerCmd="bash /home/lpla/bitextor/bitextor/example/marian-translate-tr.sh" translationDirection="tr2en" sentenceAligner="bleualign" \
                bifixer=True bicleaner=True bicleanerModel="${WORK}/bicleaner-model-ai/en-tr/metadata.yaml" deferred=True tmx=True boilerplateCleaning=True deduped=True paragraphIdentification=True additionalMetadata=True bicleanerExtraArgs="--disable_minimal_length" bicleanerThreshold=0.5 \
            &> "${WORK}/reports/10-mt-en-tr-r2code-v2.report"

