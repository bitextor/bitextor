#!/bin/bash

set -e

SCORE_THRESHOLD_COMMAND=""
BATCH_SIZE_COMMAND=""

usage() { echo -e "Usage: $0 <[options]>\nOptions:\n\t-f <lett_file>\n\t-l <foreign_language>\n\t-t <translation_script>\n\t-w <working_directory>\n\t[-s <score_threshold>]\n\t[-b <batch_size>]\n\t[-d]\n\t[-v]\n" 1>&2; exit 1; }

while getopts ":f:l:t:w:s:b:dv" arg; do
    case "${arg}" in
        f)
            LETT_FILE=${OPTARG}
            ;;
        l)
            WLANG=${OPTARG}
            ;;
        t)
            TRANSLATE_SCRIPT="${OPTARG}"
            ;;
        w)
            WDIR="${OPTARG}"
            ;;
        s)
            SCORE_THRESHOLD="${OPTARG}"
            ;;
        b)
            BATCH="${OPTARG}"
            ;;
        d)
            BUILD_DOCS="true"
            ;;
        v)
            VERBOSE="| pv"
            ;;
        *)
            usage
            ;;
    esac
done
shift $((OPTIND-1))

if [ -z "${LETT_FILE}" ] || [ -z "${WLANG}" ] || [ -z "${TRANSLATE_SCRIPT}" ] || [ -z "${WDIR}" ]; then
    usage
fi

mydir=`dirname $0`
WDIR=`dirname ${LETT_FILE}`

check_required_files() {
  F_WDIR=${1}
  F_LETT_FILE=${2}
  F_FLANG=${3}

  # LETT file
  if [ ! -f ${F_LETT_FILE} ]; then
    >&2 echo "`basename ${F_LETT_FILE}`: NOT FOUND!"
    exit 1
  else
    >&2 echo "`basename ${F_LETT_FILE}`: FOUND"
  fi
  LETT_PRINT_COMMAND="zcat -f ${F_LETT_FILE} $VERBOSE"

  # Extracted
  langs_to_extract=""
  if [ ! -f ${F_WDIR}/en.extracted.gz ]; then
    langs_to_extract="en,"$langs_to_extract
  else
    >&2 echo "en.extracted.gz: FOUND"
  fi
  if [ ! -f ${F_WDIR}/${F_FLANG}.extracted.gz ]; then
    langs_to_extract="${F_FLANG},"$langs_to_extract
  else
    >&2 echo "${F_FLANG}.extracted.gz: FOUND"
  fi
  if [ ! -z ${langs_to_extract} ]; then
    >&2 echo "# Extracting ${langs_to_extract} from the LETT file"
    eval $LETT_PRINT_COMMAND | \
      python3 ${mydir}/utils/extract_lett.py \
      --langs ${langs_to_extract} \
      --splitter ${mydir}/utils/split-sentences.perl \
      --prune_type "words" \
      --prune 80 \
      --output_dir ${WDIR}
  fi

}


>&2 echo "# Searching for required files in ${WDIR}"
check_required_files ${WDIR} ${LETT_FILE} ${WLANG}

>&2 echo "# Translating ${WLANG} to English"
${mydir}/translate_extracted.sh ${WDIR}/${WLANG}.extracted.gz "${TRANSLATE_SCRIPT}"

if [ "${SCORE_THRESHOLD}" != "" ]; then
  SCORE_THRESHOLD_COMMAND="--threshold ${SCORE_THRESHOLD}"
fi

if [ "${BATCH}" != "" ]; then
  BATCH_SIZE_COMMAND="--batch_size ${BATCH}"
fi

>&2 echo "# Computing distances and matching"
python3 ${mydir}/compute_matches.py \
  --english ${WDIR}/en.extracted.gz \
  --translated ${WDIR}/${WLANG}.extracted.translated.gz \
  --output_matches ${WDIR}/en-${WLANG}.matches \
  ${SCORE_THRESHOLD_COMMAND} \
  ${BATCH_SIZE_COMMAND}

if [ "${BUILD_DOCS}" != "" ]; then
  >&2 echo "# Extracting matched documents (threshold=${SCORE_THRESHOLD})"
  zcat -f $LETT_FILE | python3 ${mydir}/build_docs.py --matches ${WDIR}/en-${WLANG}.matches ${SCORE_THRESHOLD_COMMAND}
fi

>&2 echo "# Done"
touch ${WDIR}/doc_align.done
