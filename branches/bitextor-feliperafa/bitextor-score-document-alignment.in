#!__ENV__ __PYTHON__


# 1. Reading from STDIN a set of aligned documents. The input format is:
#   filename1	filename2	clean_text1_in_base64	clean_text2_in_base64
# 2. Text is cleaned and, for every aligned pair, both texts are dumped, in the same order in two temporary files. Every text block is sepparated to the previous one by a block:
#    <p>
#    <file lang="lang_id">file_name</file>
#    <p>
# 3. Running hunalign on the two temporary files
# 4. Removing unaligned segments and <p> mark
# 5. Identifying the filenames for every block of segments, and printing everything to the output
#
# Output format:
#   filename1    filename2    segment1    segment2    quality
#

import sys
import os
import argparse
import base64
import subprocess
import re
#from nltk.tokenize.punkt import PunktWordTokenizer
from nltk import wordpunct_tokenize
import site
site.addsitedir('__PYTHONPATH__')
import ulysses
from tempfile import NamedTemporaryFile
from iso639 import languages
from nltk.tokenize import sent_tokenize


reload(sys)
sys.setdefaultencoding("UTF-8")

def splitSegs(mitok, text):
  return mitok.split(ulysses.splitinwords(text))
      

def trainSegmenters(reader, l1, l2):
  reader_list=[]

  try:
    mitok_l1=pickle.load(gzip.open("__PREFIX__/share/bitextor/ulysses-data/{0}.pickle.gz".format(l1), "r"))
  except:
    mitok_l1=ulysses.Ulysses()
    mitok_l1.init_model()
  
  try:
    mitok_l2=pickle.load(gzip.open("__PREFIX__/share/bitextor/ulysses-data/{0}.pickle.gz".format(l2), "r"))
  except:
    mitok_l2=ulysses.Ulysses()
    mitok_l2.init_model()

  for line in reader:
    reader_list.append(line.strip())
    fields=reader_list[-1].split("\t")
    text1=base64.b64decode(fields[2]).decode("utf-8")
    mitok_l1.feed_model(ulysses.splitinwords(text1))

    text2=base64.b64decode(fields[3]).decode("utf-8")
    mitok_l2.feed_model(ulysses.splitinwords(text2))

  mitok_l1.update_model()
  mitok_l2.update_model()

  return mitok_l1, mitok_l2, reader_list

oparser = argparse.ArgumentParser(description="Tool that reads the output of bitextor-align-documents and aligns the segments of the aligned documents")
oparser.add_argument('aligned_docs', metavar='FILE', nargs='?', help='File containing the set of aliged documents provided by the script bitextor-align-documents (if undefined, the script reads from the standard input)', default=None)
oparser.add_argument("--lang1", help="Two-characters-code for language 1 in the pair of languages", dest="lang1", required=True)
oparser.add_argument("--lang2", help="Two-characters-code for language 2 in the pair of languages", dest="lang2", required=True)
oparser.add_argument("--nltk" , help="Use NLTK sentence splitter instead of Ulysses", dest="useNltkSentTok", action="store_true")
oparser.add_argument("-d", help="Bilingual dictionary used for aligning and scoring", dest="dic", required=False, default=None)
oparser.add_argument("-t", "--tmp-dir", help="Temporal directory to be used for internal temporary files (/tmp by default)", dest="tmpdir", required=False, default="/tmp")

options = oparser.parse_args()

useNltkSentTok=options.useNltkSentTok

tmp_file1=NamedTemporaryFile(delete=False, dir=options.tmpdir)
tmp_file2=NamedTemporaryFile(delete=False, dir=options.tmpdir)

if options.aligned_docs == None:
  reader = sys.stdin
else:
  reader = open(options.aligned_docs,"r")

if not useNltkSentTok:
  if options.aligned_docs == None:
    reader = sys.stdin
  else:
    reader = open(options.aligned_docs,"r")
  mitok_l1, mitok_l2, reader_list=trainSegmenters(reader, options.lang1, options.lang2)
else:
  if options.aligned_docs == None:
    reader_list = sys.stdin
  else:
    reader_list = open(options.aligned_docs,"r")


for line in reader_list:
  tmp_file1=NamedTemporaryFile(delete=False, dir=options.tmpdir)
  tmp_file2=NamedTemporaryFile(delete=False, dir=options.tmpdir)

  fields=line.split("\t")
  filename1=fields[0]
  filename2=fields[1]
  encodedtext1=fields[2]
  encodedtext2=fields[3]
  for origseg in base64.b64decode(encodedtext1).decode("utf-8").split("\n"):
    trimorigseg=origseg.strip()
    if trimorigseg != "":
      if useNltkSentTok:
        try:
          for seg in sent_tokenize(trimorigseg,languages.get(alpha2=options.lang1).name.lower()):
            tokenized_text=wordpunct_tokenize(seg)
            #tokenized_text=PunktWordTokenizer().tokenize(seg)
            tmp_file1.write(" ".join(tokenized_text)+"\n")
        except LookupError:
          for sentence in sent_tokenize(trimorigseg):
            tokenized_text=wordpunct_tokenize(seg)
            #tokenized_text=PunktWordTokenizer().tokenize(seg)
            tmp_file1.write(" ".join(tokenized_text)+"\n")
      else:
        for seg in splitSegs(mitok_l1, trimorigseg):
          tokenized_text=wordpunct_tokenize(seg)
          #tokenized_text=PunktWordTokenizer().tokenize(seg)
          tmp_file1.write(" ".join(tokenized_text)+"\n")

  for origseg in base64.b64decode(encodedtext2).decode("utf-8").split("\n"):
    trimorigseg=origseg.strip()
    if trimorigseg != "":
      if useNltkSentTok:
        try:
          for seg in sent_tokenize(trimorigseg,languages.get(alpha2=options.lang2).name.lower()):
            tokenized_text=wordpunct_tokenize(seg)
            #tokenized_text=PunktWordTokenizer().tokenize(seg)
            tmp_file2.write(" ".join(tokenized_text)+"\n")
        except LookupError:
          for sentence in sent_tokenize(trimorigseg):
            tokenized_text=wordpunct_tokenize(seg)
            #tokenized_text=PunktWordTokenizer().tokenize(seg)
            tmp_file2.write(" ".join(tokenized_text)+"\n")
      else:
        for seg in splitSegs(mitok_l2, trimorigseg):
          tokenized_text=wordpunct_tokenize(seg)
          #tokenized_text=PunktWordTokenizer().tokenize(seg)
          tmp_file2.write(" ".join(tokenized_text)+"\n")

  tmp_file1_name=tmp_file1.name
  tmp_file2_name=tmp_file2.name

  tmp_file1.close()
  tmp_file2.close()

  if options.dic == None:
    hunalign = ["__PREFIX__/bin/hunalign", "-realign", "/dev/null", tmp_file1_name, tmp_file2_name]
  else:
    hunalign = ["__PREFIX__/bin/hunalign", options.dic, tmp_file1_name, tmp_file2_name]

  p = subprocess.Popen(hunalign, stdout= open(os.devnull, "w"), stderr = subprocess.PIPE)
  parsed_alignment, errout = p.communicate()
  quality=errout.split("\n")
  if len(quality) < 16:
    sys.stderr.write("Error: files "+fields[0]+" and "+fields[1]+" cannot be aligned; hunalign says:\n"+"\n".join(quality))
  else:
    print "{0}\t{1}\t{2}".format(fields[0], fields[1], quality[15].split(' ')[1])

  os.remove(tmp_file1.name)
  os.remove(tmp_file2.name)
