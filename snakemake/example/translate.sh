#!/usr/bin/env bash
cd date >> /tmp/signal
cat /dev/stdin | tr '[' '(' | tr ']' ')' | /home/lpla/permanent/mosesdecoder/bin/moses -f /home/lpla/permanent/fast-fr-en/moses.ini
