#!__ENV__ __PYTHON__

#
# 1. The tool takes the output of bitextor-cleanalignments and formats it in TMX format
# 2. Option -c allows to define what is expected to find in each field of the input, which makes this script flexible about the expected fields.
# Default input format:
# url1    url2    seg1    seg2    hunalign    zipporah    bicleaner    lengthratio    numTokensSL    numTokensTL    idnumber
# where url1 and url2 are the URLs of the document, seg1 and seg2 are the aligned pair of segments, hunalign and zipporah are quality metrics
# (in this case, provided by these two tools), lengthratio is the ratio between the word-length of seg1 and seg2, numTokensSL and numTokensTL is
# the number of tokens in each segment and idnumber is the value to be assigned to each TU id parameter.
#

import sys
import argparse
import time
import locale
import re

reload(sys)
sys.setdefaultencoding("UTF-8")

oparser = argparse.ArgumentParser(description="This script reads the output of bitextor-cleantextalign and formats the aligned segments as a TMX translation memory.")
oparser.add_argument('clean_alignments', metavar='FILE', nargs='?', help='File containing the segment pairs produced by bitextor-cleantextalign (if undefined, the script will read from standard input)', default=None)
oparser.add_argument("--lang1", help="Two-characters-code for language 1 in the pair of languages", dest="lang1", required=True)
oparser.add_argument("--lang2", help="Two-characters-code for language 2 in the pair of languages", dest="lang2", required=True)
oparser.add_argument("-q", "--min-length", help="Minimum length ratio between two parts of TU", type=float, dest="minl", default=0.6)
oparser.add_argument("-m", "--max-length", help="Maximum length ratio between two parts of TU", type=float, dest="maxl", default=1.6)
oparser.add_argument("-t", "--min-tokens", help="Minimum number of tokens in a TU", type=int, dest="mint", default=3)
oparser.add_argument("-c", "--columns", help="Column names of the input tab separated file. Default: url1,url2,seg1,seg2,hunalign,zipporah,bicleaner,lengthratio,numTokensSL,numTokensTL,idnumber", default="url1,url2,seg1,seg2,hunalign,zipporah,bicleaner,lengthratio,numTokensSL,numTokensTL,idnumber")
options = oparser.parse_args()

if options.clean_alignments != None:
  reader = open(options.clean_alignments,"r")
else:
  reader = sys.stdin
print "<?xml version=\"1.0\"?>"
print "<tmx version=\"1.4\">"
print " <header"
print "   adminlang=\""+locale.setlocale(locale.LC_ALL, '').split(".")[0].split("_")[0]+"\""
print "   srclang=\""+options.lang1+"\""
print "   o-tmf=\"PlainText\""
print "   creationtool=\"bitextor\""
print "   creationtoolversion=\"4.0\""
print "   datatype=\"PlainText\""
print "   segtype=\"sentence\""
print "   creationdate=\""+time.strftime("%Y%m%dT%H%M%S")+"\""
print "   o-encoding=\"utf-8\">"
print " </header>"
print " <body>"

for line in reader:
  fields = line.split("\t")
  fields[-1] = fields[-1].strip()
  columns = options.columns.split(',')
  fieldsdict=dict()
  for field,column in zip(fields,columns):
    fieldsdict[column]=field
  print "   <tu tuid=\""+str(fieldsdict["idnumber"])+"\" datatype=\"Text\">"
  infoTag=[]
  if 'hunalign' in fieldsdict and  fieldsdict['hunalign'] != "":
    print "    <prop type=\"score\">"+fieldsdict['hunalign']+"</prop>"
  if 'zipporah' in fieldsdict and fieldsdict['zipporah'] != "":
    print "    <prop type=\"score-zipporah\">"+fieldsdict['zipporah']+"</prop>"
  if 'bicleaner' in fieldsdict and fieldsdict['bicleaner'] != "":
    print "    <prop type=\"score-bicleaner\">"+fieldsdict['bicleaner']+"</prop>"
  #Output info data ILSP-FC specification
  if re.sub("[^0-9]", "", fieldsdict["seg1"]) != re.sub("[^0-9]", "", fieldsdict["seg2"]):
    infoTag.append("different numbers in TUVs")
  print "    <prop type=\"type\">1:1</prop>"
  if re.sub(r'\W+', '', fieldsdict["seg1"]) == re.sub(r'\W+', '', fieldsdict["seg2"]):
    infoTag.append("equal TUVs")
  if len(infoTag) > 0:
    print "    <prop type=\"info\">"+"|".join(infoTag)+"</prop>"
    
  infoTagSL=[]
  infoTagTL=[]
  print "    <tuv xml:lang=\""+options.lang1+"\">"
  if "url1" in columns:
    print "     <prop type=\"source-document\">"+fieldsdict['url1'].replace("&","&amp;").replace("<","&lt;").replace(">","&gt;").replace("\"","&quot;").replace("'","&apos;")+"</prop>"
  print "     <seg>"+fieldsdict['seg1'].decode("utf-8").replace("&","&amp;").replace("<","&lt;").replace(">","&gt;").replace("\"","&quot;").replace("'","&apos;")+"</seg>"
  if "numTokensSL" in fieldsdict and fieldsdict["numTokensSL"] != "" and int(fieldsdict["numTokensSL"])<int(options.mint):
    infoTagSL.append("very short segments, shorter than "+str(options.mint))
  if len(infoTagSL) > 0:
    print "    <prop type=\"info\">"+"|".join(infoTagSL)+"</prop>"
  print "    </tuv>"
  print "    <tuv xml:lang=\""+options.lang2+"\">"
  if "url2" in columns:
    print "     <prop type=\"source-document\">"+fieldsdict["url2"].replace("&","&amp;").replace("<","&lt;").replace(">","&gt;").replace("\"","&quot;").replace("'","&apos;")+"</prop>"
  print "     <seg>"+fieldsdict["seg2"].decode("utf-8").replace("&","&amp;").replace("<","&lt;").replace(">","&gt;").replace("\"","&quot;").replace("'","&apos;")+"</seg>"
  if "numTokensTL" in fieldsdict and fieldsdict["numTokensTL"] != "" and int(fieldsdict["numTokensTL"])<int(options.mint):
    infoTagTL.append("very short segments, shorter than "+str(options.mint))
  if len(infoTagTL) > 0:
    print "    <prop type=\"info\">"+"|".join(infoTagTL)+"</prop>"
  print "    </tuv>"
  print "   </tu>"
print " </body>"
print "</tmx>"
reader.close()
