#!__BASH__

OUTPUT=/dev/stdout

exit_program()
{
  echo "USAGE: $1 webdir"
  echo "WHERE"
  echo "   webdir   folder downloaded directories"
  exit 1
}

ARGS=$(getopt "bh" $*)
BOILERCOMMAND="__JAVA__ -jar __PREFIX__/share/java/piped-boilerpipe.jar"

set -- $ARGS
for i
do
  case "$i" in
    -h)
      exit_program $(basename $0)
      ;;
    -b)
      shift
      BOILERCOMMAND="cat -"
      ;;
    --)
      shift
      break
      ;;
  esac
done

case $# in
  0)
    WEBCRAWL="/dev/stdin"
    ;;
  1)
    WEBCRAWL="$1"
    ;;
  *)
    exit_program $(basename $0)
    ;;
esac


#
# 1. Duplicate files are deleted (reporthed through STDERR)
# 2. Guessing MIME type and encoding
# 3. Keeping only XML/HTML files
# 4. Normalizing XML/HTML with Tika and converting encoding into UTF-8
# 5. Adding file content encoded with base64
#
# Output format (ETT):
# encoding	mimetype	url	content(base_64)
#

# Not empty files are searched in WEBDIR and they are printer together with their mime type and their encoding
cat $WEBCRAWL | __PREFIX__/bin/bitextor-identifyMIME | __JAVA__ -jar __PREFIX__/share/java/piped-tika.jar -x 2> /dev/null | eval "$BOILERCOMMAND" 2> /dev/null | __PREFIX__/bin/bitextor-dedup > $OUTPUT

