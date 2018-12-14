#!/bin/bash

mydir=`dirname $0`


python ${mydir}/../utils/translate_table.py --translations ${mydir}/data/de.translations.gz
