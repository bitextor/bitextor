#!/bin/bash
#
# Usage:
# ./bleualign.sh ${LETT_FILE} ${FR_LANG} ${DOC_THRESHOLD} ${BLEU_THRESHOLD} ${DIR_NAME=bleualign} {MOSES_DIR}
#

if [ -z ${7} ]; then MOSES_DIR="modules/bleualign-cpp/third_party/preprocess/moses/"; else MOSES_DIR=${7}; fi

set -e
set -u

COMPRESSION="gzip"
CSUFFIX="gz"

LETT_FILE=${1}
WLANG=${2}
WDIR=${3}
DOC_THRESHOLD=${4}
BLEU_THRESHOLD=${5}
OUTPUT=${6}

mydir=`dirname $0`
BLEU_DIR="bleualign"
WDIR=`dirname ${LETT_FILE}`
SEN_DIR=${WDIR}/${BLEU_DIR}
PROCESS_PARALLELISE=1

echoerr() { echo "$@" 1>&2; }
export -f echoerr


check_required_files() {
  F_WDIR=${1}
  F_LETT_FILE=${2}
  F_FLANG=${3}

  # LETT file
  if [ ! -f ${F_LETT_FILE} ]; then
    echo "`basename ${F_LETT_FILE}`: NOT FOUND!"
    exit 1
  else
    echo "`basename ${F_LETT_FILE}`: FOUND"
  fi

  # Matches file
  if [ ! -f ${F_WDIR}/en-${F_FLANG}.matches ]; then
    echo "en-${F_FLANG}.matches: NOT FOUND!"
    exit 1
  else
    echo "en-${F_FLANG}.matches: FOUND"
  fi

  # Extracted
  langs_to_extract=""
  if [ ! -f ${F_WDIR}/en.extracted.${CSUFFIX} ] && [ ! -f ${F_WDIR}/en.extracted.${CSUFFIX} ]; then
    langs_to_extract="en,"$langs_to_extract
  else
    echo "en.extracted.*: FOUND"
  fi

  if [ ! -f ${F_WDIR}/${F_FLANG}.extracted.${CSUFFIX} ] && [ ! -f ${F_WDIR}/${F_FLANG}.extracted.${CSUFFIX} ]; then
    langs_to_extract="${F_FLANG},"$langs_to_extract
  else
    echo "${F_FLANG}.extracted.*: FOUND"
  fi

  if [ ! -z ${langs_to_extract} ]; then
    echo "# Extracting ${langs_to_extract} from the LETT file $F_LETT_FILE"
    pv ${F_LETT_FILE} | ${COMPRESSION} -cd | \
      python3 ${mydir}/bin/utils/extract_lett.py \
      --langs ${langs_to_extract} \
      --splitter ${MOSES_DIR}/ems/support/split-sentences.perl \
      --prune_type "words" \
      --prune 80 \
      --output_dir ${WDIR}
  fi

  # Translated foreign text
  if [ ! -f ${F_WDIR}/${F_FLANG}.extracted.translated.${CSUFFIX} ] && [ ! -f ${F_WDIR}/${F_FLANG}.extracted.translated.${CSUFFIX} ]; then
    echo "${F_FLANG}.extracted.translated.*: NOT FOUND!"
    exit 1
  else
    echo "${F_FLANG}.extracted.translated.*: FOUND"
  fi

}
export -f check_required_files


if [ -d "${SEN_DIR}" ]; then
  echoerr "Folder ${SEN_DIR} already exists!"
else

  echo "# Searching for required files in ${WDIR}"
  check_required_files ${WDIR} ${LETT_FILE} ${WLANG}

  mkdir ${SEN_DIR}

  echo "# Running Bleualign"
  time ${mydir}/bleualign-cpp/bleualign_cpp \
    --text1 ${WDIR}/en.extracted.${CSUFFIX} \
    --text2 ${WDIR}/${WLANG}.extracted.${CSUFFIX} \
    --text2translated ${WDIR}/${WLANG}.extracted.translated.${CSUFFIX} \
    --matches ${WDIR}/en-${WLANG}.matches \
    --doc-threshold ${DOC_THRESHOLD} \
    --bleu-threshold ${BLEU_THRESHOLD} \
    --output-dir ${SEN_DIR}

  echo "# Collecting data"
  xzcat ${SEN_DIR}/aligned.*.xz > ${OUTPUT}

fi
