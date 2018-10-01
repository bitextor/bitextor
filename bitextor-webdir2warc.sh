#!__BASH__

OUTPUT=/dev/stdout

exit_program()
{
  echo "USAGE: $1 webdir"
  echo "WHERE"
  echo "   webdir   folder downloaded directories"
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
  1)
    WEBDIR="$1"
    ;;
  *)
    exit_program $(basename $0)
    ;;
esac

#
# 1. Delete repeated files -> (log)
# 2. Obtain mimetype and file encoding
# 3. Keep only html files
# 4. Convert non-UTF-8 to UTF-8 (if errors -> log)
# 5. Fix HTML errors using Tika and delete the HTML headers
# 6. Include the file content as base64 encoded string
#
# Final output format is .ett -> encoded and typed text:
# encoding	mimetype	url	content(base_64)
#
#

# Not empty files are searched in WEBDIR and they are printer together with their mime type and their encoding
find "$WEBDIR" -type f | grep -v 'hts-cache' | __PREFIX__/bin/bitextor-dir2warc > $OUTPUT
