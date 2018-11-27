#!/bin/bash

set -e
set -u
mydir=`dirname $0`

TRANSLATE_SCRIPT=${1}
PREFFIX=${2}

COMPRESSION="xz"
CSUFFIX="xz"

# Deduplicate input senteces
cat /dev/stdin | \
  cut -d$'\t' -f 2 | \
  sort | uniq | \
  $COMPRESSION -c > ${PREFFIX}.deduped.$CSUFFIX

# Translate deduplicated to English
>&2 echo "Translating the foreign text to English..."
$COMPRESSION -cd ${PREFFIX}.deduped.$CSUFFIX | \
  eval ${TRANSLATE_SCRIPT} | \
  $COMPRESSION -c > ${PREFFIX}.deduped.translated.$CSUFFIX

# Substitute original foreign text (before deduplication) with translated text
>&2 echo "Substituting..."
$COMPRESSION -cd ${PREFFIX}.$CSUFFIX | \
  python3 ${mydir}/substitute_translated.py --deduplicated ${PREFFIX}.deduped.$CSUFFIX --translated ${PREFFIX}.deduped.translated.$CSUFFIX | \
  $COMPRESSION -c > ${PREFFIX}.translated.$CSUFFIX
>&2 echo "Done."

# Clean
rm ${PREFFIX}.deduped.$CSUFFIX
rm ${PREFFIX}.deduped.translated.$CSUFFIX

# Check the number of lines
if [ "$($COMPRESSION -cd ${PREFFIX}.$CSUFFIX | wc -l)" -eq "$($COMPRESSION -cd ${PREFFIX}.translated.$CSUFFIX | wc -l)" ];
then
  >&2 echo "Translation successfully finished."
else
  MSG="Translated file does not match with the original file in the number of lines!"
  >&2 echo ${MSG}
  exit 2
fi
