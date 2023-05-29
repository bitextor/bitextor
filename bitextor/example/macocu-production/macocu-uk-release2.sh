#!/usr/bin/bash

WORK=/data1/lpla/macocu-release-2
bitextor --notemp -j 32 \
            --config profiling=True permanentDir="${WORK}/permanent/bitextor-mt-output-en-uk-release-2-extra" granularity='["sentences", "documents"]' \
                dataDir="${WORK}/data/data-mt-en-uk-release-2-extra" transientDir="${WORK}/transient-mt-en-uk-release-2-extra" \
                preverticalsFile="'/data1/lpla/prevertical_uk_release2.list'" shards=8 batches=999999 lang1=en lang2=uk \
                documentAligner="externalMT" alignerCmd="bash /home/lpla/bitextor/bitextor/example/marian-translate-uk.sh" translationDirection="uk2en" sentenceAligner="bleualign" \
                bifixer=True bicleaner=True bicleanerModel="${WORK}/bicleaner-model-ai/en-uk/metadata.yaml" deferred=True tmx=True boilerplateCleaning=True deduped=True paragraphIdentification=True additionalMetadata=True bicleanerExtraArgs="--disable_minimal_length" bicleanerThreshold=0.5 \
            &> "${WORK}/reports/10-mt-en-uk-release-2-extra.report"

