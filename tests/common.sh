#!/bin/bash

get_nolines()
{
  zcat $1 2> /dev/null | wc -l
}

get_hash()
{
  if [ ! -f "$1" ]; then
    echo "file_not_found"
  fi

  zcat "$1" | md5sum | awk '{print $1}'
}

compare_hashes()
{
  sent_file_hash="$1"
  reference_hash="$2"
  if [ "$sent_file_hash" != "$reference_hash" ]; then
    echo "output hash != reference hash: ${sent_file_hash} != ${reference_hash}"
  else
    echo "output and reference hash values are the same: ${sent_file_hash}"
  fi
}

download_file()
{
    path=$1
    remote=$2
    if [ ! -f "${path}" ]; then
        wget -q "${remote}" -O "${path}"
    fi
}

download_dictionary()
{
    base="https://github.com/bitextor/bitextor-data/releases/download/bitextor-v1.0"
    langs=$1
    output=$2
    if [ ! -f "${output}/${langs}.dic" ]; then
        wget -q "${base}/${langs}.dic" -P "${output}"
    fi
}

download_bicleaner_model()
{
    base="https://github.com/bitextor/bicleaner-data/releases/latest/download"
    langs=$1
    output=$2
    output_file="${output}/${langs}.tar.gz"
    if [ ! -f "${output_file}" ]; then
        wget -q "${base}/${langs}.tar.gz" -P "${output}"
        tar xzf "${output_file}" -C "${output}"
        rm -f "${output_file}"
    fi
}

download_bicleaner_ai_model()
{
    base="https://github.com/bitextor/bicleaner-ai-data/releases/latest/download"
    langs=$1
    output=$2
    flavour=$([[ "$3" == "" ]] && echo "full" || echo "$3")
    output_file="${output}/${flavour}-${langs}.tgz"
    if [ ! -f "${output_file}" ]; then
        wget -q "${base}/${flavour}-${langs}.tgz" -P "${output}"
        tar xzf "${output_file}" -C "${output}"
        rm -f "${output_file}"
    fi
}

# Run tests
annotate_and_echo_info()
{
  test_id=$1
  status=$2
  nolines=$3
  desc=$([[ "$4" != "" ]] && echo " / $4" || echo "")
  error_file="$FAILS"

  if [[ "$status" == "0" ]] && [[ "$nolines" != "0" ]]; then
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
  status="$?"
  sent_file="$(ls ${WORK}/permanent/${TEST_ID}/*.sent.gz)"
  reference_file="$(ls ${WORK}/output_reference/${TEST_ID}/*.sent.gz)"
  sent_file_hash="$(get_hash ${sent_file})"
  reference_hash="$(get_hash ${reference_file})"
  compared_hashes="$(compare_hashes ${sent_file_hash} ${reference_hash})"
  desc="test status: ${status} | ${compared_hashes}"
  status="$([[ "${sent_file_hash}" != "${reference_hash}" ]] && echo 1 || echo ${status})"

  annotate_and_echo_info "${TEST_ID}" "${status}" "$(get_nolines ${sent_file})" "${desc}"
}
