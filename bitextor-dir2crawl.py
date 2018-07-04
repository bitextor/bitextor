#!__ENV__ __PYTHON__


import sys
import magic
import base64
import argparse

reload(sys)
sys.setdefaultencoding("UTF-8")

oparser = argparse.ArgumentParser(description="Script that takes a list of information about a file (obtained with UNIX command find and produces an ETT output.")
oparser.add_argument('dir', metavar='DIR', nargs='?', help='Information about the file to be processed.', default=None)
options = oparser.parse_args()

if options.dir == None:
  reader = sys.stdin
else:
  reader = open(options.dir,"r")

m=magic.open(magic.MAGIC_NONE)
m.load()
for line in reader:
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
    newline.append(base64.b64encode(content.decode(encoding.split('=')[1].replace('unknown-8bit','iso-8859-1')).encode('utf8')))
    print '\t'.join(newline)
  else:
    sys.stderr.write('Wrong line: '+line.strip()+'\n')

