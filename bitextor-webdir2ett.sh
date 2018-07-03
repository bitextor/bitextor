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
find "$WEBDIR" -type f -exec file -N --mime-type --mime-encoding {} + | grep -E "(text/html;|text/xml;)" | \
gawk '{ print gensub(/([^:]+): ([^;]+); (.+)/, "\\2\t\\3\t\\1", "g", $0) }' | grep -v 'hts-cache' | python -c " 
import sys
import magic
import base64

m=magic.open(magic.MAGIC_NONE)
m.load()
for line in sys.stdin:
  fields=line.strip().split('\t')
  if len(fields)>=3:
    filepath=fields[2]
    with open(filepath, 'r') as content_file:
      content = content_file.read()
    mime=fields[0]
    encoding=fields[1]
    newline = []
    newline.append(mime)
    newline.append(encoding)
    newline.append(filepath.replace('$WEBDIR/',''))

    charset = encoding.split('=')[1].replace('unknown-8bit','iso-8859-1').replace('us-ascii','utf-8')
    newline.append(base64.b64encode(content.decode(charset).encode('utf8')))

    print '\t'.join(newline)
  else:
    sys.stderr.write('Wrong line: '+line.strip()+'\n')
" | \
java -jar __PREFIX__/share/java/piped-tika.jar 2> /dev/null | java -jar __PREFIX__/share/java/piped-boilerpipe.jar 2> /dev/null | \
__PYTHON__ -c 'import sys
import hashlib
import base64

reload(sys)
sys.setdefaultencoding("UTF-8")

seen_md5={}
for i in sys.stdin:
  fields = i.strip().split("\t")
  e = fields[3]
  try:
    #e = base64.b64encode(fields[3])
    c = hashlib.md5()
    c.update(e)
    #checking for duplicate content (duplicates are discarded)
    if c.hexdigest() in seen_md5:
      sys.stderr.write("Repeated file:\t"+fields[2]+"\tfirst occurrence\t"+seen_md5[c.hexdigest()]+"\n")
    else:
      seen_md5[c.hexdigest()]=fields[2]
      print "{0}\t{1}\t{2}\t{3}".format(fields[0].strip(),fields[1],fields[2],e)
  except UnicodeDecodeError:
    sys.stderr.write("File "+fields[2]+" produced a character encoding error")
' > $OUTPUT


