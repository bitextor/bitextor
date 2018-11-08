#!/bin/bash

INPUT=/dev/stdin
OUTPUT=/dev/stdout

exit_program()
{
  echo "USAGE: $1 url [dirname]"
  echo "WHERE"
  echo "   url      web address to crawl"
  echo "   dirname  folder to store downloaded directories (web/ by default)"
  exit 1
}

ARGS=$(getopt "h" $*)

set -- $ARGS
for i
do
  case "$i" in
    -h)
      exit_program $(basename $0)
      ;;
    --)
      shift
      break
      ;;
  esac
done

case $# in
  3)
    URL="$1"
    DOWNLOAD_PATH="-O $2"
    CRAWL_TIME="-E$3"
    ;;
  2)
    URL="$1"
    DOWNLOAD_PATH="-O $2"
    ;;
  *)
    exit_program $(basename $0)
    ;;
esac

if [ $(command -v httrack| wc -l) -eq 0 ]; then
  echo "Error: the tool 'httrack' could not be found and it is necessary to download the websites. Please, first install this tool and then try again to run this script."
else
  $(command -v httrack) --skeleton -Q -q -%i0 -I0 -u2 $CRAWL_TIME $URL $DOWNLOAD_PATH
fi

