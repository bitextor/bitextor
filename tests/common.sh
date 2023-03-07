#!/bin/bash

get_nolines()
{
    zcat $1 2> /dev/null | wc -l
}

test_finished_successfully()
{
    local report_file="$1"

    if [[ ! -f "$report_file" ]]; then
        echo "false"
        return
    fi

    local rule_all=$(cat "$report_file" | grep "^localrule all:$" | wc -l)
    local steps=$(cat "$report_file" | grep -E "^[0-9]+ of [0-9]+ steps \(100%\) done$" | wc -l)

    if [[ "$rule_all" != "0" ]] && [[ "$steps" != "0" ]]; then
        echo "true"
    else
        echo "false"
    fi
}

get_hash()
{
    if [ ! -f "$1" ]; then
        echo "file_not_found"
    else
        local CAT=$([[ "$1" == *.gz ]] && echo "zcat" || echo "cat")

        $CAT "$1" | md5sum | awk '{print $1}'
    fi
}

compare_hashes()
{
    local sent_file_hash="$1"
    local reference_hash="$2"

    if [ "$sent_file_hash" != "$reference_hash" ]; then
        echo "output hash != reference hash: ${sent_file_hash} != ${reference_hash}"
    else
        echo "output and reference hash values are the same: ${sent_file_hash}"
    fi
}

compare_nolines()
{
    local sent_file_nolines="$1"
    local reference_nolines="$2"

    if [ "$sent_file_nolines" != "$reference_nolines" ]; then
        echo "output nolines != reference nolines: ${sent_file_nolines} != ${reference_nolines}"
    else
        echo "output and reference nolines are the same: ${sent_file_nolines}"
    fi
}

download_file()
{
    local path=$1
    local remote=$2

    if [ ! -f "${path}" ]; then
        wget -q "${remote}" -O "${path}"
    fi
}

download_dictionary()
{
    local base="https://github.com/bitextor/bitextor-data/releases/download/bitextor-v1.0"
    local langs=$1
    local output=$2

    if [ ! -f "${output}/${langs}.dic" ]; then
        wget -q "${base}/${langs}.dic" -P "${output}"
    fi
}

download_bicleaner_model()
{
    local base="https://github.com/bitextor/bicleaner-data/releases/latest/download"
    local langs=$1
    local output=$2
    local output_file="${output}/${langs}.tar.gz"

    if [ ! -f "${output_file}" ]; then
        wget -q "${base}/${langs}.tar.gz" -P "${output}"
        tar xzf "${output_file}" -C "${output}"
        ln -s "${output}/${langs}/${langs}.yaml" "${output}/${langs}/${langs}.yaml.classic" # Models are shared and parallel instances might break
    fi
}

download_bicleaner_ai_model()
{
    local base="https://github.com/bitextor/bicleaner-ai-data/releases/latest/download"
    local langs=$1
    local output=$2
    local flavour=$([[ "$3" == "" ]] && echo "full" || echo "$3")
    local output_file="${output}/${flavour}-${langs}.tgz"

    if [ ! -f "${output_file}" ]; then
        wget -q "${base}/${flavour}-${langs}.tgz" -P "${output}"
        tar xzf "${output_file}" -C "${output}"
        ln -s "${output}/${langs}" "${output}/${langs}.ai" # Models are shared and parallel instances might break
    fi
}

# Run tests
annotate_and_echo_info()
{
  local test_id=$1
  local status=$2
  local nolines=$3
  local desc=$([[ "$4" != "" ]] && echo " / $4" || echo "")
  local force_ok=$5
  local error_file="$FAILS"

  if [[ "$force_ok" == "true" ]] || ([[ "$status" == "0" ]] && [[ "$nolines" != "0" ]]); then
    echo "Ok ${test_id} (nolines / desc: ${nolines}${desc})"
  elif [[ "$status" != "0" ]]; then
    echo "Failed ${test_id} (status / desc: ${status}${desc})"
    echo -e "fail\t${test_id}\t${status}${desc}" >> "$error_file"
  elif [[ "$nolines" == "0" ]]; then
    echo "Failed ${test_id} (nolines / desc: ${nolines}${desc})"
    echo -e "fail\t${test_id}\t0 no. lines${desc}" >> "$error_file"
  fi
}

annotate_and_echo_info_wrapper()
{
    local file="sent"
    local skip_reason=""
    while [ $# -gt 0 ]; do
        case "$1" in
            --output-file=*)
                file="${1#*=}"
                ;;
            --skipped-test=*)
                skip_reason="${1#*=}"
                ;;
        *)
        esac
        shift
    done

    local skip_count_nolines=$([[ "$2" != "" ]] && echo "$2" || echo "true")

    local status="$?"

    local output_file="$(ls ${WORK}/permanent/${TEST_ID}/*.$file.gz 2> /dev/null)"
    local output_file_nolines="$(get_nolines ${output_file})"
    local reference_file="$(ls ${WORK}/output_reference/${TEST_ID}/*.$file.gz 2> /dev/null)"
    local reference_file_nolines="$(get_nolines ${reference_file})"

    if [[ "$skip_reason" != "" ]]; then
        if [[ "$skip_count_nolines" == "true" ]]; then
            local output_file_nolines="0"
        fi
        annotate_and_echo_info "${TEST_ID}" "${status}" "${output_file_nolines}" "${skip_reason}" "${skip_count_nolines}"

        return
    fi

    local output_file_hash="$(get_hash ${output_file})"
    local reference_hash="$(get_hash ${reference_file})"
    local compared_hashes="$(compare_hashes ${output_file_hash} ${reference_hash})"
    local compared_nolines="$(compare_nolines ${output_file_nolines} ${reference_file_nolines})"
    local desc="test status: ${status} | ${compared_nolines} | ${compared_hashes}"
    local status="$([[ "${output_file_hash}" != "${reference_hash}" ]] && echo 1 || echo ${status})"

    annotate_and_echo_info "${TEST_ID}" "${status}" "${output_file_nolines}" "${desc}"
}

create_integrity_report()
{
    local WORK="$1"
    local INTEGRITY_REPORT="$2"
    local TEST_ID="$3"
    local DIR="$( cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
    local TEST_OK=$(test_finished_successfully "${WORK}/reports/${TEST_ID}.report")

    if [[ "$TEST_OK" == "false" ]]; then
        return
    fi

    if [[ -f "$INTEGRITY_REPORT" ]]; then
        local random=$(echo "$(echo $RANDOM)+$(date)" | md5sum | head -c 20)
        local new_name=$(echo "${INTEGRITY_REPORT}.${random}.report")

        >&2 echo "Integrity file already exists: moving '${INTEGRITY_REPORT}' to '${new_name}'"

        mv "$INTEGRITY_REPORT" "$new_name"
    fi

    for f in $(echo "${WORK}/permanent/${TEST_ID} ${WORK}/transient/${TEST_ID} ${WORK}/data/${TEST_ID}"); do
        if [[ ! -d "$f" ]]; then
            >&2 echo "Directory '$f' does not exist"
            continue
        fi

        find "$f" -type f \
            | grep -E -v "/[.]snakemake(/|$)" \
            | xargs -I{} bash -c 'source "'${DIR}'/common.sh"; h=$(get_hash "{}"); echo "{}: ${h}"' >> "${INTEGRITY_REPORT}"
    done
}

init_test()
{
    # Export these variables to the global scope
    TEST_ID="$1"
    TRANSIENT_DIR="${WORK}/transient/${TEST_ID}"

    mkdir -p "${TRANSIENT_DIR}"
    pushd "${TRANSIENT_DIR}" > /dev/null
}

finish_test()
{
    local output_file=$1
    annotate_and_echo_info_wrapper $output_file

    popd > /dev/null
}

check_nltk_models()
{
    # NLTK models can't be downloaded in parallel since the operation is not thread-safe
    local DIR="$( cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"

    # Bifixer
    python3 "${DIR}/utils/check_nltk_model.py" tokenizers/punkt punkt True
    # Biroamer
    python3 "${DIR}/utils/check_nltk_model.py" misc/perluniprops perluniprops True
}
