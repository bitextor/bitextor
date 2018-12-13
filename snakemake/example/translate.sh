cd date >> /tmp/signal
cat /dev/stdin | tr '[' '(' | tr ']' ')' | /home/lpla/permanent/mosesdecoder/bin/moses2 -f /home/lpla/permanent/fast-fr-en/moses.ini
