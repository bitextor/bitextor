#!/bin/bash
DIR="$( cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
source $DIR/common.sh

exit_program()
{
  >&2 echo "$1 [-w <workdir>] [-f <force_command>] [-j <threads>] [-t <tests>]"
  >&2 echo ""
  >&2 echo "Runs several tests to check Bitextor is working"
  >&2 echo ""
  >&2 echo "OPTIONS:"
  >&2 echo "  -w <workdir>            Working directory. By default: \$HOME"
  >&2 echo "  -f <force_command>      Options which will be provided to snakemake"
  >&2 echo "  -j <threads>            Threads to use when running the tests"
  >&2 echo "  -t <tests>              Tests which will be executed. The way they are"
  >&2 echo "                            specified is similar to 'chmod'. The expected format"
  >&2 echo "                            is numeric (e.g. 2 means to run the 2nd function of"
  >&2 echo "                            tests, 4 means to run the 3rd function of tests, 3"
  >&2 echo "                            means to run the 2nd and 3rd function of tests). Hex"
  >&2 echo "                            numbers can also be provided. By default: 2^32-1,"
  >&2 echo "                            which means running all the tests"
  >&2 echo "  -n                      Dry run. Do not execute anything, only show which tests"
  >&2 echo "                            will be executed."

  exit 1
}

WORK="${HOME}"
WORK="${WORK/#\~/$HOME}" # Expand ~ to $HOME
FORCE=""
THREADS=1
FLAGS="$(echo 2^32-1 | bc)"
DRYRUN=false

while getopts "hf:w:j:t:n" i; do
    case "$i" in
        h) exit_program "$(basename "$0")" ; break ;;
        w) WORK=${OPTARG};;
        f) FORCE="--${OPTARG}";;
        j) THREADS="${OPTARG}";;
        t) FLAGS="${OPTARG}";;
        n) DRYRUN=true;;
        *) exit_program "$(basename "$0")" ; break ;;
    esac
done
shift $((OPTIND-1))

BITEXTOR="bitextor-full ${FORCE} --notemp -j ${THREADS} -c ${THREADS} --reason"
BITEXTOR_EXTRA_ARGS="profiling=True verbose=True"
BICLEANER="${WORK}/data/bicleaner-models"
BICLEANER_AI="${WORK}/data/bicleaner-ai-models"
FAILS="${WORK}/data/fails.log"

if [[ "$DRYRUN" == "false" ]]; then

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
download_file "${WORK}/data/warc/greenpeace.warc.gz" https://github.com/bitextor/bitextor-data/releases/download/bitextor-warc-v1.1/greenpeace.canada.warc.gz &
download_file "${WORK}/data/warc/primeminister.warc.gz" https://github.com/bitextor/bitextor-data/releases/download/bitextor-warc-v1.1/primeminister.warc.gz &
download_file "${WORK}/data/warc/kremlin.warc.gz" https://github.com/bitextor/bitextor-data/releases/download/bitextor-warc-v1.1/kremlin.warc.gz &
# Bicleaner models
download_bicleaner_model "en-fr" "${BICLEANER}" &
download_bicleaner_ai_model "en-fr" "${BICLEANER_AI}" &
# Dictionaries
download_dictionary "en-fr" "${WORK}/permanent" &
download_dictionary "en-el" "${WORK}/permanent" &
# Output reference
download_file "${WORK}/output_reference/run-tests.tgz" https://github.com/bitextor/bitextor-testing-output/releases/latest/download/run-tests.tgz &

# Parallel corpus
europarl_corpus_file="${WORK}/data/parallel-corpus/Europarl/en-fr.txt.zip"
dn_europarl_corpus_file=$(dirname "$europarl_corpus_file")
dgt_corpus_file="${WORK}/data/parallel-corpus/DGT/en-fr.txt.zip"
dn_dgt_corpus_file=$(dirname "$dgt_corpus_file")

mkdir -p "$dn_europarl_corpus_file"
mkdir -p "$dn_dgt_corpus_file"

if [ ! -f "${dn_europarl_corpus_file}/Europarl.clipped.en-fr.en.gz" ] || [ ! -f "${dn_europarl_corpus_file}/Europarl.clipped.en-fr.fr.gz" ]; then
    # ~2000000 lines
    wget -q https://object.pouta.csc.fi/OPUS-Europarl/v8/moses/en-fr.txt.zip -P "$dn_europarl_corpus_file" && \
    unzip -qq "${europarl_corpus_file}" -d "$dn_europarl_corpus_file" && \
    rm -f "${europarl_corpus_file}" &
fi
if [ ! -f "${dn_dgt_corpus_file}/DGT.clipped.en-fr.en.gz" ] || [ ! -f "${dn_dgt_corpus_file}/DGT.clipped.en-fr.fr.gz" ]; then
    # ~5000000 lines
    wget -q https://object.pouta.csc.fi/OPUS-DGT/v2019/moses/en-fr.txt.zip -P "$dn_dgt_corpus_file" && \
    unzip -qq "${dgt_corpus_file}" -d "$dn_dgt_corpus_file" && \
    rm -f "${dgt_corpus_file}"
fi

wait

# Preprocess
### Europarl parallel corpus clipped
if [ ! -f "${dn_europarl_corpus_file}/Europarl.clipped.en-fr.en.gz" ]; then
    (corpus="${dn_europarl_corpus_file}/Europarl.en-fr.en"; \
        cat "${corpus}" \
            | tail -n 100000 \
            | pigz -c > "${dn_europarl_corpus_file}/Europarl.clipped.en-fr.en.gz" && \
        rm -f "${corpus}") &
fi
if [ ! -f "${dn_europarl_corpus_file}/Europarl.clipped.en-fr.fr.gz" ]; then
    (corpus="${dn_europarl_corpus_file}/Europarl.en-fr.fr"; \
        cat "${corpus}" \
            | tail -n 100000 \
            | pigz -c > "${dn_europarl_corpus_file}/Europarl.clipped.en-fr.fr.gz" && \
        rm -f "${corpus}") &
fi
### DGT parallel corpus clipped
if [ ! -f "${dn_dgt_corpus_file}/DGT.clipped.en-fr.en.gz" ]; then
    (corpus="${dn_dgt_corpus_file}/DGT.en-fr.en"; \
        cat "${corpus}" \
            | tail -n 100000 \
            | pigz -c > "${dn_dgt_corpus_file}/DGT.clipped.en-fr.en.gz" && \
        rm -f "${corpus}") &
fi
if [ ! -f "${dn_dgt_corpus_file}/DGT.clipped.en-fr.fr.gz" ]; then
    (corpus="${dn_dgt_corpus_file}/DGT.en-fr.fr"; \
        cat "${corpus}" \
            | tail -n 100000 \
            | pigz -c > "${dn_dgt_corpus_file}/DGT.clipped.en-fr.fr.gz" && \
        rm -f "${corpus}") &
fi

wait

tar -xzf "${WORK}/output_reference/run-tests.tgz" -C "${WORK}/output_reference/" && \
rm -f "${WORK}/output_reference/run-tests.tgz"

# Specific test values
test40_dic_hash_before=$(md5sum "${WORK}/permanent/en-fr.dic" | awk '{print $1}')

fi

wait-if-envvar-is-true()
{
    value="$CI"

    if [[ "$1" != "" ]]; then
        if [[ "$(eval echo \$$1)" != "" ]]; then
            value=$(eval echo \$$1)
        fi
    fi

    if [[ "$value" == "true" ]]; then
        wait # wait before starting and finishing a test

        return 0
    else
        return 1
    fi
}

create-p2t-from-warc()
{
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
}

# MT (id >= 10)
tests-mt()
{
    ## MT (en-fr)
    (
        init_test "10"

        ${BITEXTOR} \
            --config permanentDir="${WORK}/permanent/${TEST_ID}" \
                dataDir="${WORK}/data/${TEST_ID}" transientDir="${TRANSIENT_DIR}" \
                warcs="['${WORK}/data/warc/greenpeace.warc.gz']" preprocessor="warc2text" shards=1 batches=512 lang1=en lang2=fr \
                documentAligner="externalMT" alignerCmd="bash ${DIR}/../bitextor/example/dummy-translate.sh" \
                sentenceAligner="bleualign" bicleaner=True bicleanerModel="${BICLEANER}/en-fr/en-fr.yaml" \
                bicleanerFlavour="classic" deferred=True tmx=True ${BITEXTOR_EXTRA_ARGS} \
            &> "${WORK}/reports/${TEST_ID}.report"

        finish_test
    ) &
    ## MT (en-el)
    (
        init_test "11"

        ${BITEXTOR} \
            --config permanentDir="${WORK}/permanent/${TEST_ID}" \
                dataDir="${WORK}/data/${TEST_ID}" transientDir="${TRANSIENT_DIR}" \
                warcs="['${WORK}/data/warc/primeminister.warc.gz']" preprocessor="warc2text" shards=1 batches=512 \
                lang1=en lang2=el documentAligner="externalMT" alignerCmd="bash ${DIR}/../bitextor/example/dummy-translate.sh" \
                sentenceAligner="bleualign" deferred=True tmx=True ${BITEXTOR_EXTRA_ARGS} \
            &> "${WORK}/reports/${TEST_ID}.report"

        finish_test
    ) &
    ## MT (en-ru)
    (
        init_test "12"

        ${BITEXTOR} \
            --config permanentDir="${WORK}/permanent/${TEST_ID}" \
                dataDir="${WORK}/data/${TEST_ID}" transientDir="${TRANSIENT_DIR}" \
                warcs="['${WORK}/data/warc/kremlin.warc.gz']" preprocessor="warc2text" shards=1 batches=512 lang1=en lang2=ru \
                documentAligner="externalMT" alignerCmd="bash ${DIR}/../bitextor/example/dummy-translate.sh" \
                sentenceAligner="bleualign" deferred=True tmx=True ${BITEXTOR_EXTRA_ARGS} \
            &> "${WORK}/reports/${TEST_ID}.report"

        finish_test
    ) &
    ## MT and P2T where prevertical is converted to WARC (en-fr)
    create-p2t-from-warc && \
    (
        init_test "13"

        ${BITEXTOR} \
            --config permanentDir="${WORK}/permanent/${TEST_ID}" \
                dataDir="${WORK}/data/${TEST_ID}" transientDir="${TRANSIENT_DIR}" \
                preverticals="['${WORK}/data/prevertical/greenpeace.en.prevertical.gz', '${WORK}/data/prevertical/greenpeace.fr.prevertical.gz']" \
                shards=1 batches=512 lang1=en lang2=fr documentAligner="externalMT" alignerCmd="bash ${DIR}/../bitextor/example/dummy-translate.sh" \
                sentenceAligner="bleualign" bicleaner=True bicleanerModel="${BICLEANER}/en-fr/en-fr.yaml" bicleanerFlavour="classic" \
                deferred=True tmx=True paragraphIdentification=True ${BITEXTOR_EXTRA_ARGS} \
            &> "${WORK}/reports/${TEST_ID}.report"

        finish_test
    ) &
}

# Dictionary-based (id >= 20)
tests-db()
{
    ## Use dictionary pipeline (en-fr)
    (
        init_test "20"

        ${BITEXTOR} \
            --config permanentDir="${WORK}/permanent/${TEST_ID}" \
                dataDir="${WORK}/data/${TEST_ID}" transientDir="${TRANSIENT_DIR}" \
                warcs="['${WORK}/data/warc/greenpeace.warc.gz']" preprocessor="warc2text" shards=1 batches=512 lang1=en lang2=fr \
                documentAligner="DIC" dic="${WORK}/permanent/en-fr.dic" sentenceAligner="hunalign" bicleaner=True \
                bicleanerModel="${BICLEANER}/en-fr/en-fr.yaml" bicleanerFlavour="classic" bicleanerThreshold=0.1 \
                deferred=False tmx=True ${BITEXTOR_EXTRA_ARGS} \
            &> "${WORK}/reports/${TEST_ID}.report"

        finish_test
    ) &
}

# Generate dictionary (id >= 30)
tests-gendic()
{
    wait-if-envvar-is-true

    (
        init_test "30"

        DIC_PATH="${WORK}/permanent/${TEST_ID}-generated-en-fr.dic"

        [[ -f "${DIC_PATH}" ]] && \
            >&2 echo "WARNING: ${TEST_ID}: dic file already exists: $DIC_PATH"

        ${BITEXTOR} \
            --config permanentDir="${WORK}/permanent/${TEST_ID}" \
                dataDir="${WORK}/data/${TEST_ID}" transientDir="${TRANSIENT_DIR}" \
                warcs="['${WORK}/data/warc/greenpeace.warc.gz']" preprocessor="warc2text" shards=1 batches=512 lang1=en lang2=fr \
                documentAligner="DIC" dic="${DIC_PATH}" generateDic=True sentenceAligner="hunalign" \
                initCorpusTrainingPrefix="['${dn_europarl_corpus_file}/Europarl.clipped.en-fr']" bicleaner=True \
                bicleanerModel="${BICLEANER}/en-fr/en-fr.yaml" bicleanerFlavour="classic" bicleanerThreshold=0.1 \
                deferred=False tmx=True ${BITEXTOR_EXTRA_ARGS} \
            &> "${WORK}/reports/${TEST_ID}.report"

        finish_test
    ) &

    wait-if-envvar-is-true
}

# Generate bicleaner model but use existant dictionary (a new dictionary will be generated anyways) (id >= 40)
tests-genbicleaner()
{
    if [[ "$CI" == "true" ]]; then
        # Disable these tests since they are very time-consuming and exceed the time limits of the CI
        for TEST_ID in $(echo "40"); do
            annotate_and_echo_info_wrapper "skipped test: very time-consuming"
        done

        test40_dic_hash_after="$test40_dic_hash_before"

        return
    fi

    wait-if-envvar-is-true

    (
        init_test "40"

        BICLEANER_MODEL_PATH="${BICLEANER}/${TEST_ID}/generated-en-fr.yaml"

        [[ -f "${BICLEANER_MODEL_PATH}" ]] && \
            >&2 echo "WARNING: ${TEST_ID}: bicleaner model already exists: $BICLEANER_MODEL_PATH"

        mkdir -p "$(dirname ${BICLEANER_MODEL_PATH})"

        ${BITEXTOR} \
            --config permanentDir="${WORK}/permanent/${TEST_ID}" \
                dataDir="${WORK}/data/${TEST_ID}" transientDir="${TRANSIENT_DIR}" \
                warcs="['${WORK}/data/warc/greenpeace.warc.gz']" preprocessor="warc2text" shards=1 batches=512 lang1=en lang2=fr \
                documentAligner="DIC" dic="${WORK}/permanent/en-fr.dic" generateDic=False sentenceAligner="hunalign" \
                initCorpusTrainingPrefix="['${dn_europarl_corpus_file}/Europarl.clipped.en-fr']" \
                bicleaner=True bicleanerModel="${BICLEANER_MODEL_PATH}" bicleanerGenerateModel=True \
                bicleanerParallelCorpusTrainingPrefix="['${dn_dgt_corpus_file}/DGT.clipped.en-fr']" \
                bicleanerThreshold=0.1 deferred=False tmx=True bicleanerFlavour="classic" ${BITEXTOR_EXTRA_ARGS} \
            &> "${WORK}/reports/${TEST_ID}.report"

        finish_test
    ) &

    wait "$!" # Wait for this specific test to finish
    test40_dic_hash_after=$(md5sum "${WORK}/permanent/en-fr.dic" | awk '{print $1}')

    wait-if-envvar-is-true
}

# Generate dictionary and bicleaner model (id >= 50)
tests-gendic-genbicleaner()
{
    if [[ "$CI" == "true" ]]; then
        # Disable these tests since they are very time-consuming and exceed the time limits of the CI
        for TEST_ID in $(echo "50"); do
            annotate_and_echo_info_wrapper "skipped test: very time-consuming"
        done
        return
    fi

    wait-if-envvar-is-true

    (
        init_test "50"

        DIC_PATH="${WORK}/permanent/${TEST_ID}-generated-en-fr.dic"
        BICLEANER_MODEL_PATH="${BICLEANER}/${TEST_ID}/generated-en-fr.yaml"

        [[ -f "${DIC_PATH}" ]] && \
            >&2 echo "WARNING: ${TEST_ID}: dic file already exists: $DIC_PATH"
        [[ -f "${BICLEANER_MODEL_PATH}" ]] && \
            >&2 echo "WARNING: ${TEST_ID}: bicleaner model already exists: $BICLEANER_MODEL_PATH"

        mkdir -p "$(dirname ${BICLEANER_MODEL_PATH})"

        ${BITEXTOR} \
            --config permanentDir="${WORK}/permanent/${TEST_ID}" \
                dataDir="${WORK}/data/${TEST_ID}" transientDir="${TRANSIENT_DIR}" \
                warcs="['${WORK}/data/warc/greenpeace.warc.gz']" preprocessor="warc2text" shards=1 batches=512 lang1=en lang2=fr \
                documentAligner="DIC" dic="${DIC_PATH}" generateDic=True sentenceAligner="hunalign" \
                initCorpusTrainingPrefix="['${dn_europarl_corpus_file}/Europarl.clipped.en-fr']" \
                bicleaner=True bicleanerModel="${BICLEANER_MODEL_PATH}" bicleanerGenerateModel=True \
                bicleanerParallelCorpusTrainingPrefix="['${dn_dgt_corpus_file}/DGT.clipped.en-fr']" \
                bicleanerThreshold=0.1 deferred=False tmx=True bicleanerFlavour="classic" ${BITEXTOR_EXTRA_ARGS} \
            &> "${WORK}/reports/${TEST_ID}.report"

        finish_test
    ) &

    wait-if-envvar-is-true
}

# MT and dictionary-based (id >= 60)
tests-mt-db()
{
    ## Combine MT and dictionary (en-fr)
    (
        init_test "60"

        ${BITEXTOR} \
            --config permanentDir="${WORK}/permanent/${TEST_ID}" \
                dataDir="${WORK}/data/${TEST_ID}" transientDir="${TRANSIENT_DIR}" \
                warcs="['${WORK}/data/warc/greenpeace.warc.gz']" preprocessor="warc2text" shards=1 batches=512 lang1=en lang2=fr \
                documentAligner="externalMT" alignerCmd="bash ${DIR}/../bitextor/example/dummy-translate.sh" \
                dic="${WORK}/permanent/en-fr.dic" sentenceAligner="hunalign" bicleaner=True \
                bicleanerModel="${BICLEANER}/en-fr/en-fr.yaml" bicleanerFlavour="classic" bicleanerThreshold=0.1 \
                deferred=False tmx=True ${BITEXTOR_EXTRA_ARGS} \
            &> "${WORK}/reports/${TEST_ID}.report"

        finish_test
    ) &
}

# Neural (id >= 70)
tests-neural()
{
    if [[ "$CI" == "true" ]]; then
        # Disable these tests since they are very time-consuming and exceed the time limits of the CI
        for TEST_ID in $(echo "70 71 72 73"); do
            annotate_and_echo_info_wrapper "skipped test: very time-consuming"
        done
        return
    fi

    wait-if-envvar-is-true

    ## Neural pipeline (en-fr)
    (
        init_test "70"

        ${BITEXTOR} \
            --config permanentDir="${WORK}/permanent/${TEST_ID}" \
                dataDir="${WORK}/data/${TEST_ID}" transientDir="${TRANSIENT_DIR}" \
                warcs="['${WORK}/data/warc/greenpeace.warc.gz']" preprocessor="warc2text" shards=1 batches=512 lang1=en lang2=fr \
                documentAligner="NDA" sentenceAligner="vecalign" bicleaner=True bicleanerModel="${BICLEANER}/en-fr/en-fr.yaml" \
                bicleanerFlavour="classic" deferred=False tmx=True ${BITEXTOR_EXTRA_ARGS} \
            &> "${WORK}/reports/${TEST_ID}.report"

        finish_test
    ) &

    wait-if-envvar-is-true

    ## NDA and hunalign (en-el)
    (
        init_test "71"

        ${BITEXTOR} \
            --config permanentDir="${WORK}/permanent/${TEST_ID}" \
                dataDir="${WORK}/data/${TEST_ID}" transientDir="${TRANSIENT_DIR}" \
                warcs="['${WORK}/data/warc/primeminister.warc.gz']" preprocessor="warc2text" shards=1 batches=512 \
                lang1=en lang2=el documentAligner="NDA" dic="${WORK}/permanent/en-el.dic" sentenceAligner="hunalign" \
                deferred=True tmx=True ${BITEXTOR_EXTRA_ARGS} \
            &> "${WORK}/reports/${TEST_ID}.report"

        finish_test
    ) &

    ## Neural pipeline and deferred (en-ru)
    (
        init_test "72"

        ${BITEXTOR} \
            --config permanentDir="${WORK}/permanent/${TEST_ID}" \
                dataDir="${WORK}/data/${TEST_ID}" transientDir="${TRANSIENT_DIR}" \
                warcs="['${WORK}/data/warc/kremlin.warc.gz']" preprocessor="warc2text" shards=1 batches=512 lang1=en lang2=ru \
                documentAligner="NDA" sentenceAligner="vecalign" deferred=True tmx=True ${BITEXTOR_EXTRA_ARGS} \
            &> "${WORK}/reports/${TEST_ID}.report"

        finish_test
    ) &

    ## Neural pipeline and P2T and deferred and paragraph identification (en-fr)
    create-p2t-from-warc && \
    (
        init_test "73"

        ${BITEXTOR} \
            --config permanentDir="${WORK}/permanent/${TEST_ID}" \
                dataDir="${WORK}/data/${TEST_ID}" transientDir="${TRANSIENT_DIR}" \
                preverticals="['${WORK}/data/prevertical/greenpeace.en.prevertical.gz', '${WORK}/data/prevertical/greenpeace.fr.prevertical.gz']" \
                shards=1 batches=512 lang1=en lang2=fr documentAligner="NDA" sentenceAligner="vecalign" bicleaner=True \
                bicleanerModel="${BICLEANER}/en-fr/en-fr.yaml" bicleanerFlavour="classic" \
                deferred=True tmx=True paragraphIdentification=True ${BITEXTOR_EXTRA_ARGS} \
            &> "${WORK}/reports/${TEST_ID}.report"

        finish_test
    ) &
}

# Other options (id >= 100)
tests-others()
{
    ## MT and Biroamer (en-fr)
    (
        init_test "100"

        ${BITEXTOR} \
            --config permanentDir="${WORK}/permanent/${TEST_ID}" \
                dataDir="${WORK}/data/${TEST_ID}" transientDir="${TRANSIENT_DIR}" \
                warcs="['${WORK}/data/warc/kremlin.warc.gz']" preprocessor="warc2text" shards=1 batches=512 lang1=en lang2=ru \
                documentAligner="externalMT" alignerCmd="bash ${DIR}/../bitextor/example/dummy-translate.sh" sentenceAligner="bleualign" \
                deferred=False tmx=True deduped=True biroamer=True ${BITEXTOR_EXTRA_ARGS} \
            &> "${WORK}/reports/${TEST_ID}.report"

        finish_test
    ) &
    ## 2 tests in the same scope: remove parallelism because NLTK model installation can't run in parallel (bifixer=True)
    (
        ### MT and docalign / segalign threshold and Bifixer and Bicleaner (en-fr)
        init_test "101"

        ${BITEXTOR} \
            --config permanentDir="${WORK}/permanent/${TEST_ID}" \
                dataDir="${WORK}/data/${TEST_ID}" transientDir="${TRANSIENT_DIR}" \
                warcs="['${WORK}/data/warc/greenpeace.warc.gz']" preprocessor="warc2text" shards=1 batches=512 lang1=en lang2=fr \
                documentAligner="externalMT" documentAlignerThreshold=0.1 alignerCmd="bash ${DIR}/../bitextor/example/dummy-translate.sh" \
                sentenceAligner="bleualign" sentenceAlignerThreshold=0.1 bicleaner=True \
                bicleanerModel="${BICLEANER}/en-fr/en-fr.yaml" bicleanerFlavour="classic" bicleanerThreshold=0.0 \
                deferred=False bifixer=True tmx=True deduped=True biroamer=True ${BITEXTOR_EXTRA_ARGS} \
            &> "${WORK}/reports/${TEST_ID}.report"

        finish_test

        ### MT and docalign / segalign threshold and Bifixer and Bicleaner AI (en-fr)
        init_test "102"

        # TODO change WARC and use greenpeace.canada-small.warc.gz in order to let Bicleaner AI finish in CI
        ${BITEXTOR} \
            --config permanentDir="${WORK}/permanent/${TEST_ID}" \
                dataDir="${WORK}/data/${TEST_ID}" transientDir="${TRANSIENT_DIR}" \
                warcs="['${WORK}/data/warc/greenpeace.warc.gz']" preprocessor="warc2text" shards=1 batches=512 lang1=en lang2=fr \
                documentAligner="externalMT" documentAlignerThreshold=0.1 alignerCmd="bash ${DIR}/../bitextor/example/dummy-translate.sh" \
                sentenceAligner="bleualign" sentenceAlignerThreshold=0.1 bicleaner=True \
                bicleanerModel="${BICLEANER_AI}/en-fr/metadata.yaml" bicleanerFlavour="ai" bicleanerThreshold=0.0 \
                deferred=False bifixer=True tmx=True deduped=True biroamer=True ${BITEXTOR_EXTRA_ARGS} \
            &> "${WORK}/reports/${TEST_ID}.report"

        finish_test
    ) &
}

run-tests()
{
    flags="$1" # Export variable to global scope

    if [[ "$(echo $flags | grep ^0x)" == "" ]]; then
        flags=$(printf '%x\n' "$1")
        flags="0x$flags"
    fi

    tests=(tests-mt tests-db tests-gendic tests-genbicleaner \
           tests-gendic-genbicleaner tests-mt-db tests-neural \
           tests-others)

    for i in $(seq 0 "$(echo ${#tests[@]}-1 | bc)"); do
        # (flag & 2^notest) >> notest # will behaviour like chmod's mode
        # if we want to run the 1st and 2nd test, our flag must be 3, and would be like
        #  (3 & 2^0) >> 0 = (0b11 & 0b01) >> 0 = 0b01 >> 0 = 0b01 = 1 == 1
        #  (3 & 2^1) >> 1 = (0b11 & 0b10) >> 1 = 0b10 >> 1 = 0b01 = 1 == 1
        if [[ "$(( ($flags & (2**$i)) >> $i ))" == "1" ]]; then
            if [[ "$DRYRUN" == "false" ]] ; then
                ${tests[$i]}
            else
                echo "${tests[$i]}"
            fi
        fi
    done
}

run-tests "$FLAGS"

wait

# Post checking
if [[ "$DRYRUN" == "false" ]]; then
    # Tests with id >= 40 has been executed?
    if [[ "$(( ($flags & (2**3)) >> 3 ))" == "1" ]]; then
        # Check if the dictionary has been replaced
        if [[ -z "$test40_dic_hash_before" ]] || [[ -z "$test40_dic_hash_after" ]] || \
           [[ "$test40_dic_hash_before" != "$test40_dic_hash_after" ]]; then
            echo "Failed 40.1 (dictionary has been replaced: '$test40_dic_hash_before' -> '$test40_dic_hash_after')"
            echo "fail 40.1 \"dictionary replaced\"" >> "$FAILS"
        else
            echo "Ok 40.1"
        fi
    fi
fi

# Get hashes from all files
for TEST_ID in $(echo "10 11 12 13 20 30 40 50 60 70 71 72 73 100 101 102"); do
    create_integrity_report "$WORK" "${WORK}/reports/hash_values_${TEST_ID}.report" "$TEST_ID"
done

# Results
if [[ "$DRYRUN" == "false" ]]; then
    failed=$(cat "$FAILS" | wc -l)
else
    failed="0"
fi

echo "-------------------------------------"
echo "            Fails Summary            "
echo "-------------------------------------"
echo -e "status\ttest-id\texit code / desc."
[[ "$DRYRUN" == "false" ]] && cat "$FAILS"

exit "$failed"
