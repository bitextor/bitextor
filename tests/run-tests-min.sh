#!/bin/bash

DIR="$( cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
source "$DIR/common.sh"

exit_program()
{
    >&2 echo "$1 [-w workdir] [-f force_command] [-j threads]"
    >&2 echo ""
    >&2 echo "Runs several tests to check Bitextor is working"
    >&2 echo ""
    >&2 echo "OPTIONS:"
    >&2 echo "  -w <workdir>            Working directory. By default: \$HOME"
    >&2 echo "  -f <force_command>      Options which will be provided to snakemake"
    >&2 echo "  -j <threads>            Threads to use when running the tests"
    exit 1
}

WORK="${HOME}"
WORK="${WORK/#\~/$HOME}" # Expand ~ to $HOME
FORCE=""
THREADS=1

while getopts "hf:w:j:" i; do
    case "$i" in
        h) exit_program "$(basename "$0")" ; break ;;
        w) WORK=${OPTARG};;
        f) FORCE="--${OPTARG}";;
        j) THREADS="${OPTARG}";;
        *) exit_program "$(basename "$0")" ; break ;;
    esac
done
shift $((OPTIND-1))

BITEXTOR="bitextor-full ${FORCE} --notemp -j ${THREADS} -c ${THREADS} --reason"
BITEXTOR_EXTRA_ARGS="profiling=True verbose=True"
BICLEANER="${WORK}/data/bicleaner-models"
BICLEANER_AI="${WORK}/data/bicleaner-ai-models"
FAILS="${WORK}/data/fails.log"
mkdir -p "${WORK}"
mkdir -p "${WORK}/permanent"
mkdir -p "${WORK}/transient"
mkdir -p "${WORK}/data/warc"
mkdir -p "${WORK}/data/parallel-corpus"
mkdir -p "${WORK}/data/prevertical"
mkdir -p "${WORK}/reports"
mkdir -p "${WORK}/output_reference"
mkdir -p "${BICLEANER}"
mkdir -p "${BICLEANER_AI}"
rm -f "$FAILS"
touch "$FAILS"

# Download necessary files
# WARCs
download_file "${WORK}/data/warc/greenpeace.warc.gz" https://github.com/bitextor/bitextor-data/releases/download/bitextor-warc-v1.1/greenpeace.canada-small.warc.gz &
# Bicleaner models
download_bicleaner_model "en-fr" "${BICLEANER}" &
download_bicleaner_ai_model "en-fr" "${BICLEANER_AI}" lite &
# Dictionaries
download_dictionary "en-fr" "${WORK}/permanent" &
# Output reference
download_file "${WORK}/output_reference/run-tests-min.tgz" https://github.com/bitextor/bitextor-testing-output/releases/download/v1/run-tests-min.tgz &

wait

# Process downloaded files if necessary
tar -xzf "${WORK}/output_reference/run-tests-min.tgz" -C "${WORK}/output_reference/" && \
rm -f "${WORK}/output_reference/run-tests-min.tgz"

# MT (id >= 10)
## MT (en-fr)
(
    TEST_ID="10"
    TRANSIENT_DIR="${WORK}/transient/${TEST_ID}"

    mkdir -p "${TRANSIENT_DIR}" && \
    pushd "${TRANSIENT_DIR}" > /dev/null && \
    ${BITEXTOR} \
        --config permanentDir="${WORK}/permanent/${TEST_ID}" \
            dataDir="${WORK}/data/${TEST_ID}" transientDir="${TRANSIENT_DIR}" \
            warcs="['${WORK}/data/warc/greenpeace.warc.gz']" preprocessor="warc2text" shards=1 batches=512 lang1=en lang2=fr \
            documentAligner="externalMT" alignerCmd="bash ${DIR}/../bitextor/example/dummy-translate.sh" \
            sentenceAligner="bleualign" bicleaner=True bicleanerModel="${BICLEANER}/en-fr/en-fr.yaml" \
            bicleanerFlavour="classic" deferred=True tmx=True ${BITEXTOR_EXTRA_ARGS} \
        &> "${WORK}/reports/${TEST_ID}.report" && \
    popd > /dev/null

    annotate_and_echo_info_wrapper
) &
## MT and P2T where prevertical is converted to WARC (en-fr)
(
    TEST_ID="11"
    TRANSIENT_DIR="${WORK}/transient/${TEST_ID}"

    if [[ ! -f "${WORK}/data/prevertical/greenpeace.en.prevertical.gz" ]] || \
       [[ ! -f "${WORK}/data/prevertical/greenpeace.fr.prevertical.gz" ]]; then
        mkdir -p "${WORK}/data/tmp-w2t"

        warc2text -o "${WORK}/data/tmp-w2t" -s -f "text,url" "${WORK}/data/warc/greenpeace.warc.gz" && \
        (
            python3 ${DIR}/utils/text2prevertical.py --text-files "${WORK}/data/tmp-w2t/en/text.gz" \
                --url-files "${WORK}/data/tmp-w2t/en/url.gz" --document-langs English --seed 1 \
            | pigz -c > "${WORK}/data/prevertical/greenpeace.en.prevertical.gz"
            python3 ${DIR}/utils/text2prevertical.py --text-files "${WORK}/data/tmp-w2t/fr/text.gz" \
                --url-files "${WORK}/data/tmp-w2t/fr/url.gz" --document-langs French --seed 2 \
            | pigz -c > "${WORK}/data/prevertical/greenpeace.fr.prevertical.gz" \
        )

        rm -rf "${WORK}/data/tmp-w2t"
    fi

    mkdir -p "${TRANSIENT_DIR}" && \
    pushd "${TRANSIENT_DIR}" > /dev/null && \
    ${BITEXTOR} \
        --config permanentDir="${WORK}/permanent/${TEST_ID}" \
            dataDir="${WORK}/data/${TEST_ID}" transientDir="${TRANSIENT_DIR}" \
            preverticals="['${WORK}/data/prevertical/greenpeace.en.prevertical.gz', '${WORK}/data/prevertical/greenpeace.fr.prevertical.gz']" \
            shards=1 batches=512 lang1=en lang2=fr documentAligner="externalMT" alignerCmd="bash ${DIR}/../bitextor/example/dummy-translate.sh" \
            sentenceAligner="bleualign" bicleaner=True bicleanerModel="${BICLEANER}/en-fr/en-fr.yaml" bicleanerFlavour="classic" \
            deferred=True tmx=True paragraphIdentification=True ${BITEXTOR_EXTRA_ARGS} \
        &> "${WORK}/reports/${TEST_ID}.report" && \
    popd > /dev/null

    annotate_and_echo_info_wrapper
) &

# Dictionary-based (id >= 20)
## Use dictionary pipeline (en-fr)
(
    TEST_ID="20"
    TRANSIENT_DIR="${WORK}/transient/${TEST_ID}"

    mkdir -p "${TRANSIENT_DIR}" && \
    pushd "${TRANSIENT_DIR}" > /dev/null && \
    ${BITEXTOR} \
        --config permanentDir="${WORK}/permanent/${TEST_ID}" \
            dataDir="${WORK}/data/${TEST_ID}" transientDir="${TRANSIENT_DIR}" \
            warcs="['${WORK}/data/warc/greenpeace.warc.gz']" preprocessor="warc2text" shards=1 batches=512 lang1=en lang2=fr \
            documentAligner="DIC" dic="${WORK}/permanent/en-fr.dic" sentenceAligner="hunalign" bicleaner=True bicleanerFlavour="classic" \
            bicleanerModel="${BICLEANER}/en-fr/en-fr.yaml" bicleanerThreshold=0.1 deferred=False tmx=True ${BITEXTOR_EXTRA_ARGS} \
        &> "${WORK}/reports/${TEST_ID}.report" && \
    popd > /dev/null

    annotate_and_echo_info_wrapper
) &

wait

# MT and dictionary-based (id >= 60)
## Combine MT and dictionary (en-fr)
(
    TEST_ID="60"
    TRANSIENT_DIR="${WORK}/transient/${TEST_ID}"

    mkdir -p "${TRANSIENT_DIR}" && \
    pushd "${TRANSIENT_DIR}" > /dev/null && \
    ${BITEXTOR} \
        --config permanentDir="${WORK}/permanent/${TEST_ID}" \
            dataDir="${WORK}/data/${TEST_ID}" transientDir="${TRANSIENT_DIR}" \
            warcs="['${WORK}/data/warc/greenpeace.warc.gz']" preprocessor="warc2text" shards=1 batches=512 lang1=en lang2=fr \
            documentAligner="externalMT" alignerCmd="bash ${DIR}/../bitextor/example/dummy-translate.sh" \
            dic="${WORK}/permanent/en-fr.dic" sentenceAligner="hunalign" bicleaner=True bicleanerFlavour="classic" \
            bicleanerModel="${BICLEANER}/en-fr/en-fr.yaml" bicleanerThreshold=0.1 deferred=False tmx=True ${BITEXTOR_EXTRA_ARGS} \
        &> "${WORK}/reports/${TEST_ID}.report" && \
    popd > /dev/null

    annotate_and_echo_info_wrapper
) &

# Other options (id >= 100)
## MT and Biroamer (en-fr)
(
    TEST_ID="100"
    TRANSIENT_DIR="${WORK}/transient/${TEST_ID}"

    mkdir -p "${TRANSIENT_DIR}" && \
    pushd "${TRANSIENT_DIR}" > /dev/null && \
    ${BITEXTOR} \
        --config permanentDir="${WORK}/permanent/${TEST_ID}" \
            dataDir="${WORK}/data/${TEST_ID}" transientDir="${TRANSIENT_DIR}" \
            warcs="['${WORK}/data/warc/greenpeace.warc.gz']" preprocessor="warc2text" shards=1 batches=512 lang1=en lang2=fr \
            documentAligner="externalMT" alignerCmd="bash ${DIR}/../bitextor/example/dummy-translate.sh" sentenceAligner="bleualign" \
            deferred=False tmx=True deduped=True biroamer=True ${BITEXTOR_EXTRA_ARGS} \
        &> "${WORK}/reports/${TEST_ID}.report" && \
    popd > /dev/null

    annotate_and_echo_info_wrapper
) &
## 2 tests in the same scope: remove parallelism because NLTK model installation can't run in parallel (bifixer=True)
(
    ### MT and docalign / segalign threshold and Bifixer and Bicleaner (en-fr)
    TEST_ID="101"
    TRANSIENT_DIR="${WORK}/transient/${TEST_ID}"

    mkdir -p "${TRANSIENT_DIR}" && \
    pushd "${TRANSIENT_DIR}" > /dev/null && \
    ${BITEXTOR} \
        --config permanentDir="${WORK}/permanent/${TEST_ID}" \
            dataDir="${WORK}/data/${TEST_ID}" transientDir="${TRANSIENT_DIR}" \
            warcs="['${WORK}/data/warc/greenpeace.warc.gz']" preprocessor="warc2text" shards=1 batches=512 lang1=en lang2=fr \
            documentAligner="externalMT" documentAlignerThreshold=0.1 alignerCmd="bash ${DIR}/../bitextor/example/dummy-translate.sh" \
            sentenceAligner="bleualign" sentenceAlignerThreshold=0.1 \
            bicleaner=True bicleanerModel="${BICLEANER}/en-fr/en-fr.yaml" bicleanerFlavour="classic" bicleanerThreshold=0.0 \
            deferred=False bifixer=True tmx=True deduped=True biroamer=False ${BITEXTOR_EXTRA_ARGS} \
        &> "${WORK}/reports/${TEST_ID}.report" && \
    popd > /dev/null

    annotate_and_echo_info_wrapper

    ### MT and docalign / segalign threshold and Bifixer and Bicleaner AI (en-fr)
    TEST_ID="102"
    TRANSIENT_DIR="${WORK}/transient/${TEST_ID}"

    mkdir -p "${TRANSIENT_DIR}" && \
    pushd "${TRANSIENT_DIR}" > /dev/null && \
    ${BITEXTOR} \
        --config permanentDir="${WORK}/permanent/${TEST_ID}" \
            dataDir="${WORK}/data/${TEST_ID}" transientDir="${TRANSIENT_DIR}" \
            warcs="['${WORK}/data/warc/greenpeace.warc.gz']" preprocessor="warc2text" shards=1 batches=512 lang1=en lang2=fr \
            documentAligner="externalMT" documentAlignerThreshold=0.1 alignerCmd="bash ${DIR}/../bitextor/example/dummy-translate.sh" \
            sentenceAligner="bleualign" sentenceAlignerThreshold=0.1 \
            bicleaner=True bicleanerModel="${BICLEANER_AI}/en-fr/metadata.yaml" bicleanerThreshold=0.0 \
            deferred=False bifixer=True tmx=True deduped=True biroamer=False ${BITEXTOR_EXTRA_ARGS} \
        &> "${WORK}/reports/${TEST_ID}.report" && \
    popd > /dev/null

    annotate_and_echo_info_wrapper
) &

wait

# Get hashes from all files
create_integrity_report "$WORK" "${WORK}/reports/hash_values.report"

# Results
failed=$(cat "$FAILS" | wc -l)

echo "-------------------------------------"
echo "            Fails Summary            "
echo "-------------------------------------"
echo -e "status\ttest-id\texit code / desc."
cat "$FAILS"

exit "$failed"
