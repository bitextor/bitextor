#!/bin/bash 

CURPATH=$(dirname $(readlink -f $0))
INPUT_FILE=$(mktemp) 
cat > "$INPUT_FILE"
python3 $CURPATH/bifixer/bifixer/bifixer.py "${INPUT_FILE}" "${INPUT_FILE}".o "$1" "$2" "$3"

cat "${INPUT_FILE}".o

rm -Rf "$INPUT_FILE" "${INPUT_FILE}".o
