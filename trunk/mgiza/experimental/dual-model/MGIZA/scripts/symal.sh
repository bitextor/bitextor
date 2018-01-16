#!/usr/bin/env bash

OUTPUT=$1
shift
GIZA2BAL=$1
shift
SYMAL=$1
shift
STOT=$1
shift
TTOS=$1
shift

perl $GIZA2BAL -d ${STOT} -i ${TTOS} | $SYMAL $* > $OUTPUT 

