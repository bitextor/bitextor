#!/bin/bash

set -e
set -u
mydir=`dirname $0`

FR_EXTRACTED=`basename ${1%.gz}`
TRANSLATE_SCRIPT=${2}
WDIR=`dirname ${1}`


# Deduplicate input senteces
zcat ${WDIR}/${FR_EXTRACTED}.gz | \
  cut -d$'\t' -f 2 | \
  sort | uniq | \
  gzip -c > ${WDIR}/${FR_EXTRACTED}.deduped.gz

# Translate deduplicated to English
>&2 echo "Translating the foreign text to English..."
zcat ${WDIR}/${FR_EXTRACTED}.deduped.gz | \
  eval ${TRANSLATE_SCRIPT} | \
  gzip -c > ${WDIR}/${FR_EXTRACTED}.deduped.translated.gz

# Substitute original foreign text (before deduplication) with translated text
>&2 echo "Substituting..."
zcat ${WDIR}/${FR_EXTRACTED}.gz | \
  python3 ${mydir}/substitute_translated.py --deduplicated ${WDIR}/${FR_EXTRACTED}.deduped.gz --translated ${WDIR}/${FR_EXTRACTED}.deduped.translated.gz | \
  gzip -c > ${WDIR}/${FR_EXTRACTED}.translated.gz
>&2 echo "Done."

# Clean
rm ${WDIR}/${FR_EXTRACTED}.deduped.gz
rm ${WDIR}/${FR_EXTRACTED}.deduped.translated.gz

# Check the number of lines
if [ "$(zcat ${WDIR}/${FR_EXTRACTED}.gz | wc -l)" -eq "$(zcat ${WDIR}/${FR_EXTRACTED}.translated.gz | wc -l)" ];
then
  >&2 echo "Translation successfully finished."
else
  MSG="Translated file does not match with the original file in the number of lines!"
  >&2 echo ${MSG}
  exit 2
fi
