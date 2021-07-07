#!/usr/bin/env bash

cat /dev/stdin | sed 's/</\&lt;/g' | sed 's/>/\&gt;/g' | sed 's/^/<p>/g' | sed 's/$/<\/p>/g' | apertium -f html -u cat-spa |  sed -r 's/<\/?p>//g' | perl -MHTML::Entities -pe 'decode_entities($_);'
