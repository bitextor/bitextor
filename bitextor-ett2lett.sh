#!__BASH__
OUTPUT=/dev/stdout
exit_program()
{
  echo "USAGE: $1 webdir"
  echo "WHERE"
  echo "   webdir   folder downloaded directories"
  exit 1
}
langs=""
FILE="/dev/stdin"
ARGS=$(getopt "hl:" $*)
set -- $ARGS
for i
do
  case "$i" in
    -h|--help)
      exit_program $(basename $0)
      ;;
    -l|--languages)
      shift
      langs="-l $1"
      shift
      ;;
    --)
      shift
      break
      ;;
  esac
done
case $# in
  0);;
  1)
    FILE="$1"
    ;;
  *)
    exit_program $(basename $0)
    ;;
esac

cat $FILE | python3 $(dirname $0)/bitextor-get-html-text -t | \
$(dirname $0)/bitextor-lett-language-detector $langs
