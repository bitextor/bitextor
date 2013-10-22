#!__BASH__

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
  2)
    URL="$1"
    DOWNLOAD_PATH="-O $2"
    ;;
  1)
    URL="$1"
    DOWNLOAD_PATH="-O web"
    ;;
  *)
    exit_program $(basename $0)
    ;;
esac

if [ $(command -v httrack|__WC__ -l) -eq 0 ]; then
  echo "Error: the tool 'httrack' could not be found and it is necessary to download the websites. Please, first install this tool and then try again to run this script."
else
  $(command -v httrack) --skeleton -Q -q -%i0 -I0 $URL $DOWNLOAD_PATH
fi

