#!/bin/bash

mydir=`dirname $0`


if [ -d "${mydir}/tmp" ]; then
  rm -R "${mydir}/tmp"
fi

mkdir -p ${mydir}/tmp

cp ${mydir}/data/v2.lett.gz ${mydir}/tmp/
${mydir}/../doc_align.sh -f ${mydir}/tmp/v2.lett.gz -l de -t ${mydir}/translate_script.sh -w ${mydir}/tmp/

echo "-------------------"

while read p; do
  RES=$(cat ${mydir}/tmp/en-de.matches | grep "$p" | wc -l)
  if [ ${RES} -ne 1 ]; then
    echo "Test Failed"
    echo $p
    exit 1
  fi
done <${mydir}/expected
echo "Test OK"

if [ -d "${mydir}/tmp" ]; then
  rm -R "${mydir}/tmp"
fi
