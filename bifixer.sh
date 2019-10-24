#!/bin/bash 

INPUT_FILE=$(mktemp) 
cat > "$INPUT_FILE"
python3 bifixer/bifixer/bifixer.py -q "${INPUT_FILE}" "${INPUT_FILE}".o "$1" "$2" "$3"

cat "${INPUT_FILE}".o

rm -Rf "$INPUT_FILE" "${INPUT_FILE}".o
