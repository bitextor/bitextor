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
import pickle
import codecs
from nltk import wordpunct_tokenize
from nltk.tokenize import sent_tokenize
import site
import traceback
site.addsitedir('__PYTHONPATH__')
import ulysses
from tempfile import NamedTemporaryFile
import gzip
from iso639 import languages


def runAligner(filename1, filename2, dic):
  # option -ppthresh=10?
  if dic == None or dic == "":
    hunalign = ["__PREFIX__/bin/hunalign", "-realign", "/dev/null", filename1, filename2]
  else:
    hunalign = ["__PREFIX__/bin/hunalign", dic, filename1, filename2]
  p = subprocess.Popen(hunalign, stdout=subprocess.PIPE)
  for line in p.stdout:
    yield line
  return

def runAnalyse(morph, text):
  panalyse = subprocess.Popen(morph, stdout=subprocess.PIPE, stderr=subprocess.PIPE, stdin=subprocess.PIPE)
  morph_stdout,error = panalyse.communicate(input=text)
  if len(error.strip()) == 0:
    tokenized_text = re.sub(r"\^", "", re.sub(r"[/<][^$]*\$", "", morph_stdout))
    return tokenized_text
  else:
    return text

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
    text1=base64.b64decode(fields[2].encode("utf8")).decode("utf-8")
    mitok_l1.feed_model(ulysses.splitinwords(text1))

    text2=base64.b64decode(fields[3].encode("utf8")).decode("utf-8")
    mitok_l2.feed_model(ulysses.splitinwords(text2))

  mitok_l1.update_model()
  mitok_l2.update_model()

  return mitok_l1, mitok_l2, reader_list

def extract_encoded_text(encodedtext, lang, tmp_file, tmp_file_origtext, mitok, morphanal, useNltkSentTok):
  tmp_tok_segs=[]
  for origseg in base64.b64decode(encodedtext).decode("utf-8").split("\n"):
    trimorigseg=origseg.strip()
    if trimorigseg != "":
      if useNltkSentTok:
        try:
          for seg in sent_tokenize(trimorigseg,languages.get(alpha2=lang).name.lower()):
            tmp_file_origtext.write(seg+b"\n")
            tmp_tok_segs.append(u" ".join(wordpunct_tokenize(seg)))
        except LookupError:
          # No language-specific sentence splitter available
          for seg in sent_tokenize(trimorigseg):
            tmp_file_origtext.write(seg+b"\n")
            tmp_tok_segs.append(u" ".join(wordpunct_tokenize(seg)))
      else:
        for seg in splitSegs(mitok, trimorigseg):
          tmp_file_origtext.write(seg+b"\n")
          tmp_tok_segs.append(u" ".join(wordpunct_tokenize(seg)))

  tokenized_text=u"\n".join(tmp_tok_segs)
  if morphanal is not None:
    morphanalyser = ["__BASH__", morphanal]
    tokenized_text=runAnalyse(morphanalyser, tokenized_text)
  tmp_file.write(tokenized_text.lower()+b"\n")

def align(file1, file2, file1orig, file2orig, file1name, file2name, dic):
  filereader1=open(file1orig, "r")
  filereader2=open(file2orig, "r")

  hunalign_output=runAligner(file1, file2, dic)
  try :
    prev_hun=next(hunalign_output).strip()
    prev_fields=prev_hun.split(b"\t")
    if int(prev_fields[0]) > 0:
      for i in range(int(prev_fields[0])):
        line1=filereader1.readline().strip()

    elif int(prev_fields[1]) > 1:
      for i in range(int(prev_fields[1])):
        line2=filereader2.readline().strip()

  except StopIteration :
    prev_hun=""
  for line in hunalign_output:
    hun_line=line.strip()
    last_position1 = filereader1.tell()
    last_position2 = filereader2.tell()
    line1=filereader1.readline().strip()
    line2=filereader2.readline().strip()
    prev_fields=prev_hun.split(b"\t")
    fields=hun_line.split(b"\t")

    if float(prev_fields[2])==-0.3:
      if int(fields[0])==int(prev_fields[0]):
        line1=""
        filereader1.seek(last_position1)
      elif int(fields[1])==int(prev_fields[1]):
        line2=""
        filereader2.seek(last_position2)

    if int(fields[0])-int(prev_fields[0]) > 1:
      for i in range((int(fields[0])-int(prev_fields[0]))-1):
        line1+=" "+filereader1.readline().strip()

    if int(fields[1])-int(prev_fields[1]) > 1:
      for i in range((int(fields[1])-int(prev_fields[1]))-1):
        line2+=" "+filereader2.readline().strip()

    print("{0}\t{1}\t{2}\t{3}\t{4}".format(filename1, filename2, line1, line2, prev_fields[2].decode("utf8")))

    prev_hun=hun_line

  filereader1.close()
  filereader2.close()
  
oparser = argparse.ArgumentParser(description="Tool that reads the output of bitextor-align-documents and aligns the segments of the aligned documents")
oparser.add_argument('aligned_docs', metavar='FILE', nargs='?', help='File containing the set of aliged documents provided by the script bitextor-align-documents (if undefined, the script reads from the standard input)', default=None)
oparser.add_argument("--lang1", help="Two-characters-code for language 1 in the pair of languages", dest="lang1", required=True)
oparser.add_argument("--lang2", help="Two-characters-code for language 2 in the pair of languages", dest="lang2", required=True)
oparser.add_argument("--nltk" , help="Use NLTK sentence splitter instead of Ulysses", dest="useNltkSentTok", action="store_true")
oparser.add_argument("-d", help="Bilingual dictionary used for aligning and scoring", dest="dic", required=False, default=None)
oparser.add_argument("-t", "--tmp-dir", help="Temporary directory to be used for internal temporary files (/tmp by default)", dest="tmpdir", required=False, default="/tmp")
oparser.add_argument("--morphanalyser_sl", help="Path to the Apertium's morphological analyser for SL to TL", dest="morphanal1", default=None)
oparser.add_argument("--morphanalyser_tl", help="Path to the Apertium's morphological analyser for TL to SL", dest="morphanal2", default=None)

options = oparser.parse_args()

useNltkSentTok=options.useNltkSentTok

if options.aligned_docs == None:
  reader = sys.stdin
else:
  reader = open(options.aligned_docs,"r")

mitok_l1,mitok_l2=None,None
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
  tmp_file1_origtext=NamedTemporaryFile(delete=False, dir=options.tmpdir)
  tmp_file2_origtext=NamedTemporaryFile(delete=False, dir=options.tmpdir)

  fields=line.split("\t")
  filename1=fields[0]
  filename2=fields[1]
  encodedtext1=fields[2]
  encodedtext2=fields[3]

  extract_encoded_text(encodedtext1, options.lang1, tmp_file1, tmp_file1_origtext, mitok_l1, options.morphanal1, useNltkSentTok)
  extract_encoded_text(encodedtext2, options.lang2, tmp_file2, tmp_file2_origtext, mitok_l2, options.morphanal2, useNltkSentTok)

  tmp_file1_name=tmp_file1.name
  tmp_file2_name=tmp_file2.name
  tmp_file1_orig_name=tmp_file1_origtext.name
  tmp_file2_orig_name=tmp_file2_origtext.name

  tmp_file1.close()
  tmp_file1_origtext.close()
  tmp_file2.close()
  tmp_file2_origtext.close()

  align(tmp_file1_name, tmp_file2_name, tmp_file1_orig_name, tmp_file2_orig_name, filename1, filename2, options.dic)

  os.remove(tmp_file1.name)
  os.remove(tmp_file1_origtext.name)
  os.remove(tmp_file2.name)
  os.remove(tmp_file2_origtext.name)
