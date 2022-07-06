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
BICLEANER="${WORK}/bicleaner-model"
BICLEANER_AI="${WORK}/bicleaner-ai-model"
FAILS="${WORK}/data/fails.log"
mkdir -p "${WORK}"
mkdir -p "${WORK}/permanent"
mkdir -p "${WORK}/transient"
mkdir -p "${WORK}/data/warc"
mkdir -p "${WORK}/data/parallel-corpus"
mkdir -p "${WORK}/data/parallel-corpus/Europarl"
mkdir -p "${WORK}/data/parallel-corpus/DGT"
mkdir -p "${WORK}/data/prevertical"
mkdir -p "${WORK}/reports"
mkdir -p "${BICLEANER}"
mkdir -p "${BICLEANER}/new"
mkdir -p "${BICLEANER}/new-new"
mkdir -p "${BICLEANER_AI}"
rm -f "$FAILS"
touch "$FAILS"

# Download necessary files
# WARCs
download_warc "${WORK}/data/warc/greenpeace.warc.gz" https://github.com/bitextor/bitextor-data/releases/download/bitextor-warc-v1.1/greenpeace.canada.warc.gz &
download_warc "${WORK}/data/warc/primeminister.warc.gz" https://github.com/bitextor/bitextor-data/releases/download/bitextor-warc-v1.1/primeminister.warc.gz &
download_warc "${WORK}/data/warc/kremlin.warc.gz" https://github.com/bitextor/bitextor-data/releases/download/bitextor-warc-v1.1/kremlin.warc.gz &
# Bicleaner models
download_bicleaner_model "en-fr" "${BICLEANER}" &
download_bicleaner_ai_model "en-fr" "${BICLEANER_AI}" &
# Dictionaries
download_dictionary "en-fr" "${WORK}/permanent" &
download_dictionary "en-el" "${WORK}/permanent" &
# Parallel corpus
europarl_corpus_file="${WORK}/data/parallel-corpus/Europarl/en-fr.txt.zip"
dgt_corpus_file="${WORK}/data/parallel-corpus/DGT/en-fr.txt.zip"
if [ ! -f "${europarl_corpus_file}" ]; then
    # ~2000000 lines
    wget -q https://object.pouta.csc.fi/OPUS-Europarl/v8/moses/en-fr.txt.zip -P "${WORK}/data/parallel-corpus/Europarl" && \
    unzip -qq "${europarl_corpus_file}" -d "${WORK}/data/parallel-corpus/Europarl" &
    rm -f "${europarl_corpus_file}"
fi
if [ ! -f "${dgt_corpus_file}" ]; then
    # ~5000000 lines
    wget -q https://object.pouta.csc.fi/OPUS-DGT/v2019/moses/en-fr.txt.zip -P "${WORK}/data/parallel-corpus/DGT" && \
    unzip -qq "${dgt_corpus_file}" -d "${WORK}/data/parallel-corpus/DGT" &
    rm -f "${dgt_corpus_file}"
fi
wait

# Preprocess
### Europarl parallel corpus clipped
if [ ! -f "${WORK}/data/parallel-corpus/Europarl/Europarl.clipped.en-fr.en.gz" ]; then
    (corpus="${WORK}/data/parallel-corpus/Europarl/Europarl.en-fr.en"; \
        cat "${corpus}" \
            | tail -n 100000 \
            | pigz -c > "${WORK}/data/parallel-corpus/Europarl/Europarl.clipped.en-fr.en.gz" && \
        rm -f "${corpus}") &
fi
if [ ! -f "${WORK}/data/parallel-corpus/Europarl/Europarl.clipped.en-fr.fr.gz" ]; then
    (corpus="${WORK}/data/parallel-corpus/Europarl/Europarl.en-fr.fr"; \
        cat "${corpus}" \
            | tail -n 100000 \
            | pigz -c > "${WORK}/data/parallel-corpus/Europarl/Europarl.clipped.en-fr.fr.gz" && \
        rm -f "${corpus}") &
fi
### DGT parallel corpus clipped
if [ ! -f "${WORK}/data/parallel-corpus/DGT/DGT.clipped.en-fr.en.gz" ]; then
    (corpus="${WORK}/data/parallel-corpus/DGT/DGT.en-fr.en"; \
        cat "${corpus}" \
            | tail -n 100000 \
            | pigz -c > "${WORK}/data/parallel-corpus/DGT/DGT.clipped.en-fr.en.gz" && \
        rm -f "${corpus}") &
fi
if [ ! -f "${WORK}/data/parallel-corpus/DGT/DGT.clipped.en-fr.fr.gz" ]; then
    (corpus="${WORK}/data/parallel-corpus/DGT/DGT.en-fr.fr"; \
        cat "${corpus}" \
            | tail -n 100000 \
            | pigz -c > "${WORK}/data/parallel-corpus/DGT/DGT.clipped.en-fr.fr.gz" && \
        rm -f "${corpus}") &
fi

wait

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

        annotate_and_echo_info "${TEST_ID}" "$?" "$(get_nolines ${WORK}/permanent/${TEST_ID}/en-fr.sent.gz)"
    ) &
    ## MT (en-el)
    (
        TEST_ID="11"
        TRANSIENT_DIR="${WORK}/transient/${TEST_ID}"

        mkdir -p "${TRANSIENT_DIR}" && \
        pushd "${TRANSIENT_DIR}" > /dev/null && \
        ${BITEXTOR} \
            --config permanentDir="${WORK}/permanent/${TEST_ID}" \
                dataDir="${WORK}/data/${TEST_ID}" transientDir="${TRANSIENT_DIR}" \
                warcs="['${WORK}/data/warc/primeminister.warc.gz']" preprocessor="warc2text" shards=1 batches=512 \
                lang1=en lang2=el documentAligner="externalMT" alignerCmd="bash ${DIR}/../bitextor/example/dummy-translate.sh" \
                sentenceAligner="bleualign" deferred=True tmx=True ${BITEXTOR_EXTRA_ARGS} \
            &> "${WORK}/reports/${TEST_ID}.report" && \
        popd > /dev/null

        annotate_and_echo_info "${TEST_ID}" "$?" "$(get_nolines ${WORK}/permanent/${TEST_ID}/en-el.sent.gz)"
    ) &
    ## MT (en-ru)
    (
        TEST_ID="12"
        TRANSIENT_DIR="${WORK}/transient/${TEST_ID}"

        mkdir -p "${TRANSIENT_DIR}" && \
        pushd "${TRANSIENT_DIR}" > /dev/null && \
        ${BITEXTOR} \
            --config permanentDir="${WORK}/permanent/${TEST_ID}" \
                dataDir="${WORK}/data/${TEST_ID}" transientDir="${TRANSIENT_DIR}" \
                warcs="['${WORK}/data/warc/kremlin.warc.gz']" preprocessor="warc2text" shards=1 batches=512 lang1=en lang2=ru \
                documentAligner="externalMT" alignerCmd="bash ${DIR}/../bitextor/example/dummy-translate.sh" \
                sentenceAligner="bleualign" deferred=True tmx=True ${BITEXTOR_EXTRA_ARGS} \
            &> "${WORK}/reports/${TEST_ID}.report" && \
        popd > /dev/null

        annotate_and_echo_info "${TEST_ID}" "$?" "$(get_nolines ${WORK}/permanent/${TEST_ID}/en-ru.sent.gz)"
    ) &
    ## MT and P2T where prevertical is converted to WARC (en-fr)
    create-p2t-from-warc && \
    (
        TEST_ID="13"
        TRANSIENT_DIR="${WORK}/transient/${TEST_ID}"

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

        annotate_and_echo_info "${TEST_ID}" "$?" "$(get_nolines ${WORK}/permanent/${TEST_ID}/en-fr.sent.gz)"
    ) &
}

# Dictionary-based (id >= 20)
tests-db()
{
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
                documentAligner="DIC" dic="${WORK}/permanent/en-fr.dic" sentenceAligner="hunalign" bicleaner=True \
                bicleanerModel="${BICLEANER}/en-fr/en-fr.yaml" bicleanerFlavour="classic" bicleanerThreshold=0.1 \
                deferred=False tmx=True ${BITEXTOR_EXTRA_ARGS} \
            &> "${WORK}/reports/${TEST_ID}.report" && \
        popd > /dev/null

        annotate_and_echo_info "${TEST_ID}" "$?" "$(get_nolines ${WORK}/permanent/${TEST_ID}/en-fr.sent.gz)"
    ) &
}

# Generate dictionary (id >= 30)
tests-gendic()
{
    wait-if-envvar-is-true

    dic_md5sum_before=$(md5sum "${WORK}/permanent/en-fr.dic" | awk '{print $1}')
    rm -f "${WORK}/permanent/new-en-fr.dic"

    TEST_ID="30"
    TRANSIENT_DIR="${WORK}/transient/${TEST_ID}"

    mkdir -p "${TRANSIENT_DIR}"

    (
        pushd "${TRANSIENT_DIR}" > /dev/null && \
        ${BITEXTOR} \
            --config permanentDir="${WORK}/permanent/${TEST_ID}" \
                dataDir="${WORK}/data/${TEST_ID}" transientDir="${TRANSIENT_DIR}" \
                warcs="['${WORK}/data/warc/greenpeace.warc.gz']" preprocessor="warc2text" shards=1 batches=512 lang1=en lang2=fr \
                documentAligner="DIC" dic="${WORK}/permanent/new-en-fr.dic" generateDic=True sentenceAligner="hunalign" \
                initCorpusTrainingPrefix="['${WORK}/data/parallel-corpus/Europarl/Europarl.clipped.en-fr']" bicleaner=True \
                bicleanerModel="${BICLEANER}/en-fr/en-fr.yaml" bicleanerFlavour="classic" bicleanerThreshold=0.1 \
                deferred=False tmx=True ${BITEXTOR_EXTRA_ARGS} \
            &> "${WORK}/reports/${TEST_ID}.report" && \
        popd > /dev/null

        annotate_and_echo_info "${TEST_ID}" "$?" "$(get_nolines ${WORK}/permanent/${TEST_ID}/en-fr.sent.gz)"
    ) &

    wait-if-envvar-is-true && \
    echo "Removing '${TRANSIENT_DIR}' ($(du -sh ${TRANSIENT_DIR} | awk '{print $1}'))" && \
    rm -rf "${TRANSIENT_DIR}"
}

# Generate bicleaner model but use existant dictionary (a new dictionary will be generated anyways) (id >= 40)
tests-genbicleaner()
{
    wait-if-envvar-is-true

    rm -f "${BICLEANER}/new/new-en-fr.yaml"

    TEST_ID="40"
    TRANSIENT_DIR="${WORK}/transient/${TEST_ID}"

    mkdir -p "${TRANSIENT_DIR}"

    (
        pushd "${TRANSIENT_DIR}" > /dev/null && \
        ${BITEXTOR} \
            --config permanentDir="${WORK}/permanent/${TEST_ID}" \
                dataDir="${WORK}/data/${TEST_ID}" transientDir="${TRANSIENT_DIR}" \
                warcs="['${WORK}/data/warc/greenpeace.warc.gz']" preprocessor="warc2text" shards=1 batches=512 lang1=en lang2=fr \
                documentAligner="DIC" dic="${WORK}/permanent/en-fr.dic" generateDic=False sentenceAligner="hunalign" \
                initCorpusTrainingPrefix="['${WORK}/data/parallel-corpus/Europarl/Europarl.clipped.en-fr']" \
                bicleaner=True bicleanerModel="${BICLEANER}/new/new-en-fr.yaml" bicleanerGenerateModel=True \
                bicleanerParallelCorpusTrainingPrefix="['${WORK}/data/parallel-corpus/DGT/DGT.clipped.en-fr']" \
                bicleanerThreshold=0.1 deferred=False tmx=True bicleanerFlavour="classic" ${BITEXTOR_EXTRA_ARGS} \
            &> "${WORK}/reports/${TEST_ID}.report" && \
        popd > /dev/null

        annotate_and_echo_info "${TEST_ID}" "$?" "$(get_nolines ${WORK}/permanent/${TEST_ID}/en-fr.sent.gz)" && \
            dic_md5sum_after=$(md5sum "${WORK}/permanent/en-fr.dic" | awk '{print $1}')
    ) &


    wait-if-envvar-is-true && \
    echo "Removing '${TRANSIENT_DIR}' ($(du -sh ${TRANSIENT_DIR} | awk '{print $1}'))" && \
    rm -rf "${TRANSIENT_DIR}"
}

# Generate dictionary and bicleaner model (id >= 50)
tests-gendic-genbicleaner()
{
    wait-if-envvar-is-true

    rm -f "${WORK}/permanent/new-new-en-fr.dic"
    rm -f "${BICLEANER}/new-new/new-new-en-fr.yaml"
    
    TEST_ID="50"
    TRANSIENT_DIR="${WORK}/transient/${TEST_ID}"

    mkdir -p "${TRANSIENT_DIR}"

    (
        pushd "${TRANSIENT_DIR}" > /dev/null && \
        ${BITEXTOR} \
            --config permanentDir="${WORK}/permanent/${TEST_ID}" \
                dataDir="${WORK}/data/${TEST_ID}" transientDir="${TRANSIENT_DIR}" \
                warcs="['${WORK}/data/warc/greenpeace.warc.gz']" preprocessor="warc2text" shards=1 batches=512 lang1=en lang2=fr \
                documentAligner="DIC" dic="${WORK}/permanent/new-new-en-fr.dic" generateDic=True sentenceAligner="hunalign" \
                initCorpusTrainingPrefix="['${WORK}/data/parallel-corpus/Europarl/Europarl.clipped.en-fr']" \
                bicleaner=True bicleanerModel="${BICLEANER}/new-new/new-new-en-fr.yaml" bicleanerGenerateModel=True \
                bicleanerParallelCorpusTrainingPrefix="['${WORK}/data/parallel-corpus/DGT/DGT.clipped.en-fr']" \
                bicleanerThreshold=0.1 deferred=False tmx=True bicleanerFlavour="classic" ${BITEXTOR_EXTRA_ARGS} \
            &> "${WORK}/reports/${TEST_ID}.report" && \
        popd > /dev/null

        annotate_and_echo_info "${TEST_ID}" "$?" "$(get_nolines ${WORK}/permanent/${TEST_ID}/en-fr.sent.gz)"
    ) &

    wait-if-envvar-is-true && \
    echo "Removing '${TRANSIENT_DIR}' ($(du -sh ${TRANSIENT_DIR} | awk '{print $1}'))" && \
    rm -rf "${TRANSIENT_DIR}"
}

# MT and dictionary-based (id >= 60)
tests-mt-db()
{
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
                dic="${WORK}/permanent/en-fr.dic" sentenceAligner="hunalign" bicleaner=True \
                bicleanerModel="${BICLEANER}/en-fr/en-fr.yaml" bicleanerFlavour="classic" bicleanerThreshold=0.1 \
                deferred=False tmx=True ${BITEXTOR_EXTRA_ARGS} \
            &> "${WORK}/reports/${TEST_ID}.report" && \
        popd > /dev/null

        annotate_and_echo_info "${TEST_ID}" "$?" "$(get_nolines ${WORK}/permanent/${TEST_ID}/en-fr.sent.gz)"
    ) &
}

# Neural (id >= 70)
tests-neural()
{
    wait-if-envvar-is-true

    ## Neural pipeline (en-fr)
    (
        TEST_ID="70"
        TRANSIENT_DIR="${WORK}/transient/${TEST_ID}"
        NDA_BATCH_SIZE=$([[ "$CI" == "true" ]] && "embeddingsBatchSize=28" || echo "") # tests fail in CI due to OOM error

        mkdir -p "${TRANSIENT_DIR}" && \
        pushd "${TRANSIENT_DIR}" > /dev/null && \
        ${BITEXTOR} \
            --config permanentDir="${WORK}/permanent/${TEST_ID}" \
                dataDir="${WORK}/data/${TEST_ID}" transientDir="${TRANSIENT_DIR}" \
                warcs="['${WORK}/data/warc/greenpeace.warc.gz']" preprocessor="warc2text" shards=1 batches=512 lang1=en lang2=fr \
                documentAligner="NDA" sentenceAligner="vecalign" bicleaner=True bicleanerModel="${BICLEANER}/en-fr/en-fr.yaml" \
                bicleanerFlavour="classic" deferred=False tmx=True ${NDA_BATCH_SIZE} ${BITEXTOR_EXTRA_ARGS} \
            &> "${WORK}/reports/${TEST_ID}.report" && \
        popd > /dev/null

        annotate_and_echo_info "${TEST_ID}" "$?" "$(get_nolines ${WORK}/permanent/${TEST_ID}/en-fr.sent.gz)"
    ) &

    wait-if-envvar-is-true

    if [[ "$CI" == "true" ]]; then
        # Neural tests are very time-consuming
        return
    fi

    ## NDA and hunalign (en-el)
    (
        TEST_ID="71"
        TRANSIENT_DIR="${WORK}/transient/${TEST_ID}"

        mkdir -p "${TRANSIENT_DIR}" && \
        pushd "${TRANSIENT_DIR}" > /dev/null && \
        ${BITEXTOR} \
            --config permanentDir="${WORK}/permanent/${TEST_ID}" \
                dataDir="${WORK}/data/${TEST_ID}" transientDir="${TRANSIENT_DIR}" \
                warcs="['${WORK}/data/warc/primeminister.warc.gz']" preprocessor="warc2text" shards=1 batches=512 \
                lang1=en lang2=el documentAligner="NDA" dic="${WORK}/permanent/en-el.dic" sentenceAligner="hunalign" \
                deferred=True tmx=True ${BITEXTOR_EXTRA_ARGS} \
            &> "${WORK}/reports/${TEST_ID}.report" && \
        popd > /dev/null

        annotate_and_echo_info "${TEST_ID}" "$?" "$(get_nolines ${WORK}/permanent/${TEST_ID}/en-el.sent.gz)"
    ) &

    ## Neural pipeline and deferred (en-ru)
    (
        TEST_ID="72"
        TRANSIENT_DIR="${WORK}/transient/${TEST_ID}"

        mkdir -p "${TRANSIENT_DIR}" && \
        pushd "${TRANSIENT_DIR}" > /dev/null && \
        ${BITEXTOR} \
            --config permanentDir="${WORK}/permanent/${TEST_ID}" \
                dataDir="${WORK}/data/${TEST_ID}" transientDir="${TRANSIENT_DIR}" \
                warcs="['${WORK}/data/warc/kremlin.warc.gz']" preprocessor="warc2text" shards=1 batches=512 lang1=en lang2=ru \
                documentAligner="NDA" sentenceAligner="vecalign" deferred=True tmx=True ${BITEXTOR_EXTRA_ARGS} \
            &> "${WORK}/reports/${TEST_ID}.report" && \
        popd > /dev/null

        annotate_and_echo_info "${TEST_ID}" "$?" "$(get_nolines ${WORK}/permanent/${TEST_ID}/en-ru.sent.gz)"
    ) &

    ## Neural pipeline and P2T and deferred and paragraph identification (en-fr)
    create-p2t-from-warc && \
    (
        TEST_ID="73"
        TRANSIENT_DIR="${WORK}/transient/${TEST_ID}"

        mkdir -p "${TRANSIENT_DIR}" && \
        pushd "${TRANSIENT_DIR}" > /dev/null && \
        ${BITEXTOR} \
            --config permanentDir="${WORK}/permanent/${TEST_ID}" \
                dataDir="${WORK}/data/${TEST_ID}" transientDir="${TRANSIENT_DIR}" \
                preverticals="['${WORK}/data/prevertical/greenpeace.en.prevertical.gz', '${WORK}/data/prevertical/greenpeace.fr.prevertical.gz']" \
                shards=1 batches=512 lang1=en lang2=fr documentAligner="NDA" sentenceAligner="vecalign" bicleaner=True \
                bicleanerModel="${BICLEANER}/en-fr/en-fr.yaml" bicleanerFlavour="classic" \
                deferred=True tmx=True paragraphIdentification=True ${BITEXTOR_EXTRA_ARGS} \
            &> "${WORK}/reports/${TEST_ID}.report" && \
        popd > /dev/null

        annotate_and_echo_info "${TEST_ID}" "$?" "$(get_nolines ${WORK}/permanent/${TEST_ID}/en-fr.sent.gz)"
    ) &
}

# Other options (id >= 100)
tests-others()
{
    ## MT and Biroamer (en-fr)
    (
        TEST_ID="100"
        TRANSIENT_DIR="${WORK}/transient/${TEST_ID}"

        mkdir -p "${TRANSIENT_DIR}" && \
        pushd "${TRANSIENT_DIR}" > /dev/null && \
        ${BITEXTOR} \
            --config permanentDir="${WORK}/permanent/${TEST_ID}" \
                dataDir="${WORK}/data/${TEST_ID}" transientDir="${TRANSIENT_DIR}" \
                warcs="['${WORK}/data/warc/kremlin.warc.gz']" preprocessor="warc2text" shards=1 batches=512 lang1=en lang2=ru \
                documentAligner="externalMT" alignerCmd="bash ${DIR}/../bitextor/example/dummy-translate.sh" sentenceAligner="bleualign" \
                deferred=False tmx=True deduped=True biroamer=True ${BITEXTOR_EXTRA_ARGS} \
            &> "${WORK}/reports/${TEST_ID}.report" && \
        popd > /dev/null

        annotate_and_echo_info "${TEST_ID}" "$?" "$(get_nolines ${WORK}/permanent/${TEST_ID}/en-ru.sent.gz)"
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
                sentenceAligner="bleualign" sentenceAlignerThreshold=0.1 bicleaner=True \
                bicleanerModel="${BICLEANER}/en-fr/en-fr.yaml" bicleanerFlavour="classic" bicleanerThreshold=0.0 \
                deferred=False bifixer=True tmx=True deduped=True biroamer=True ${BITEXTOR_EXTRA_ARGS} \
            &> "${WORK}/reports/${TEST_ID}.report" && \
        popd > /dev/null

        annotate_and_echo_info "${TEST_ID}" "$?" "$(get_nolines ${WORK}/permanent/${TEST_ID}/en-fr.sent.gz)"

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
                sentenceAligner="bleualign" sentenceAlignerThreshold=0.1 bicleaner=True bicleanerModelFlavour="full" \
                bicleanerModel="${BICLEANER_AI}/en-fr/metadata.yaml" bicleanerFlavour="ai" bicleanerThreshold=0.0 \
                deferred=False bifixer=True tmx=True deduped=True biroamer=True ${BITEXTOR_EXTRA_ARGS} \
            &> "${WORK}/reports/${TEST_ID}.report" && \
        popd > /dev/null

        annotate_and_echo_info "${TEST_ID}" "$?" "$(get_nolines ${WORK}/permanent/${TEST_ID}/en-fr.sent.gz)"
    ) &
}

run-tests()
{
    flags="$1"

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
            if [ $DRYRUN = true ] ; then
                echo "${tests[$i]}"
            else
                ${tests[$i]}
            fi
        fi
    done
}

run-tests "$FLAGS"

wait

# Post checking
if [ $DRYRUN = false ] && [[ "$(( ($flags & (2**3)) >> 3 ))" == "1" ]] && [[ "$dic_md5sum_before" != "" ]] && [[ "$dic_md5sum_after" != "" ]] && [[ "$dic_md5sum_before" != "$dic_md5sum_after" ]]; then
    echo "Failed 40.1 (dictionary has been replaced ($dic_md5sum_before -> $dic_md5sum_after), what is not the expected)"
    echo "fail 40.1 \"dictionary replaced\"" >> "$FAILS"
elif [ $DRYRUN = false ] && [[ "$(( ($flags & (2**3)) >> 3 ))" == "1" ]]; then
    echo "Ok 40.1"
fi

# Results
failed=$(cat "$FAILS" | wc -l)

echo "-------------------------------------"
echo "            Fails Summary            "
echo "-------------------------------------"
echo -e "status\ttest-id\texit code / desc."
cat "$FAILS"

exit "$failed"
