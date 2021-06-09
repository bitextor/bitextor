#!/bin/bash

get_nolines()
{
  zcat $1 2> /dev/null | wc -l
}

download_warc()
{
    warc=$1
    remote=$2
    if [ ! -f "${warc}" ]; then
        wget -q "${remote}" -O "${warc}"
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
    if [ ! -f "${output}/${langs}.tar.gz" ]; then
        wget -q "${base}/${langs}.tar.gz" -P "${output}"
        tar xzf "${output}/${langs}.tar.gz" -C "${output}"
    fi
}

# Run tests
annotate_and_echo_info()
{
  test_id=$1
  status=$2
  nolines=$3
  error_file="$FAILS"

  if [[ "$status" == "0" ]] && [[ "$nolines" != "0" ]]; then
    echo "Ok ${test_id} (nolines: ${nolines})"
  else if [[ "$status" != "0" ]]; then
    echo "Failed ${test_id} (status: ${status})"
    echo "fail ${test_id} ${status}" >> "$error_file"
  else if [[ "$nolines" == "0" ]]; then
    echo "Failed ${test_id} (nolines: ${nolines})"
    echo "fail ${test_id} '0 no. lines'" >> "$error_file"
  fi
  fi
  fi
}