#!/usr/bin/env python3

import sys

documents = {}

file = open(sys.argv[1], "r")

contador = 1
for j in file:
    campos = j.strip().split()
    if len(campos) > 4:
        documents[contador] = campos[3]
    contador = contador + 1

for ridx_line in sys.stdin:
    ridx_elements = []
    ridx_fields = ridx_line.strip().split("\t")
    ridx_elements.append(documents[int(ridx_fields[0])])
    for position in range(1, len(ridx_fields)):
        ridx_tuple = ridx_fields[position].split(":")
        ridx_elements.append(documents[int(ridx_tuple[0])] + ":" + ridx_tuple[1])
    print("\t".join(ridx_elements))
