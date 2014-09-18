#!/usr/bin/env python
# -*- coding:utf-8 -*-
from operator import pos
import sys
import codecs
import regex
from collections import namedtuple
from collections import Counter
import pickle 
import getopt
import tempfile
import os

reload(sys)  # Reload does the trick!
sys.setdefaultencoding('UTF8')

ENDINGS = set([u".", u"!", u"?", u".\"", u".''", u".”", u".˝", u".»", u"!\"", u"!''", u"!”", u"!˝", u"!»", u"?\"", u"?''", u"?”", u"?˝", u"?»", u".)", u".]", u".}"])

def im(a, b): #integrate maps
  if a is None:
    return b
  elif b is None:
    return a
  else:
    return dict(sum((Counter(x) for x in [a,b]), Counter()))

class Ulysses:
  def __init__(self):
    self.f_map_t = None
    self.f_map_l = None
    self.f_map_b = None
    self.f_map_wpunct = None
    self.f_map_bigram = None
    self.f_map_prenumdot_bigram = None
    self.f_map_prenum = None
    self.f_map_prepn = None
  
  def pprinttable(self, rows, output):
    if len(rows) > 1:
      headers = rows[0]._fields
      lens = []
      for i in range(len(rows[0])):
        lens.append(len(max([x[i] for x in rows] + [headers[i]],key=lambda x:len(unicode(x)))))
      formats = []
      hformats = []
      for i in range(len(rows[0])):
        if isinstance(rows[0][i], int):
          formats.append(u"%%%dd" % lens[i])
        else:
          formats.append(u"%%-%ds" % lens[i])
        hformats.append(u"%%-%ds" % lens[i])
      pattern = u" | ".join(formats)
      hpattern = u" | ".join(hformats)
      separator = u"-+-".join([u'-' * n for n in lens])
      output.write(hpattern % tuple(headers))
      output.write(u"\n")
      output.write(separator)
      output.write(u"\n")
      for line in rows:
        output.write(pattern % tuple(line))
        output.write(u"\n")
    elif len(rows) == 1:
      row = rows[0]
      hwidth = len(max(row._fields,key=lambda x: len(x)))
      for i in range(len(row)):
        output.write(u"%*s = %s" % (hwidth,row._fields[i],row[i]))
        output.write(u"\n")

  def endswith(self, mystr, endings):
    for i in endings:
      if mystr[-len(i):] == i:
        return True
    return False

  def simplify(self, tokens_list):
    pre = []
    post = []
    dot = []
  
    for i in xrange(len(tokens_list)):
      if self.endswith(tokens_list[i], ENDINGS):
        if i >= 1:
          pre.append(tokens_list[i-1])
        else:
          pre.append(u"")
    
        if i<(len(tokens_list)-1):
          post.append(tokens_list[i+1])
        else:
          post.append(u"")
      
        dot.append(tokens_list[i])

    return pre,dot,post

  def freqs(self, tokens_list):
    result = {}
    result[u""] = 1
  
    for i in tokens_list:
      if not i in result:
        result[i] = 1
      else:
        result[i] += 1          
    return result

  def veredict(self, pretoken, token, posttoken):
    try: 
      #if token[-2:] in [u".\"", u".”", u".˝", u".»"] or token[-3:] in [u".''"]:
      #  return u"OK0"
      if token[-3:] == u"..." and posttoken.lstrip(u"([{")[0:1].islower():
        return u"NO0"
      elif token[0:1] in [u"(",u"{",u"[",u"¿",u"¡"] and not token[-2:-1] in  [u")", u"}", u"]", u"?", u"!"]:
        return u"NO0.1"
      elif pretoken == u"" or pretoken[-1:] in [u":", u";", u".", u"?", u"!"][0:1]:
        return u"NO0.2"
      elif regex.match(ur"[[:upper:]][[:lower:]]?[.]", token) and regex.match(ur"[[:upper:]][[:lower:]]+[.!?]", posttoken):
        return u"NO0.3"
      elif regex.match(ur"[[:lower:]]{3,}[.!?]", pretoken) and regex.match(ur"[[:upper:]][[:lower:]]?[.]", token):
        return u"NO0.4"
      elif token[-1:] == u"." and posttoken[0:1] in [u"-", u"¿", u"¡", u"«"]:
        return u"OK1"
      elif regex.match(ur"[^[:alpha:]]*[[:upper:]][[:lower:]]?[.]", token):
        return u"NO1.1"
      #elif regex.match(ur"[[:upper:]][[:lower:]]?[.]", token) and float(self.f_map_t[token]) / float(self.f_map_l[token.lower()]) > 0.8 and float(self.f_map_t[posttoken]) > float(self.f_map_b[posttoken.lower()]) and float(self.f_map_t[posttoken]) / float(self.f_map_l[posttoken.lower()]) > 0.6:
      #  return u"[{0:.4f}] NO1.2".format(float(self.f_map_t[token]) / float(self.f_map_l[token.lower()]))

      elif not regex.match(ur"[[:alpha:]]{1,2}[.]", token) and regex.match(ur"[[:upper:]][[:lower:]]?[.]", token) and float(self.f_map_t[token]) / float(self.f_map_wpunct[token.rstrip(u".,:!?\"'")]) < 0.6:
        return u"[{0:.4f}] OK3".format(float(self.f_map_t[token]) / float(self.f_map_wpunct[token.rstrip(u".,:!?\"'")]))
      elif regex.match(ur"^[[:lower:]]{3,}[.]$", token) and regex.match(ur"[[:upper:]][[:lower:]]{2,}$", posttoken) and float(self.f_map_t[posttoken]) / float(self.f_map_l[posttoken.lower()]) < 0.9:
        return u"OK4"  # NEW - EXPERIMENTAL FOR GERMAN
      elif regex.match(ur"^[]})\"\'»][.]$", token[-2:]):
        return u"OK5"
      elif regex.match(ur"^[¿¡][[:upper:]]", posttoken[0:2]):
        return u"OK6"    
      elif self.f_map_prenum[token] > 5 and float(self.f_map_prenum[token])/float(self.f_map_t[token]) > 0.50 and regex.match(ur"^([0-9]|[iIvVXxlL]{1,4})", posttoken):
        return u"[{0:.4f}] NO2".format(float(self.f_map_prenum[token])/float(self.f_map_t[token]))
      elif self.f_map_prepn[token] > 5 and float(self.f_map_prepn[token])/float(self.f_map_t[token]) > 0.50 and posttoken[0:1].isupper():
        return u"[{0:.4f}] NO3".format(float(self.f_map_prepn[token])/float(self.f_map_t[token]))
      elif posttoken.lstrip(u"([{-")[0:1].islower():
        return u"NO4"
      elif token.rstrip(u".,:!?\"'").islower() and len(token.rstrip(u".,:!?\"'")) > 6:
        return u"OK7"
      elif (float(self.f_map_prenumdot_bigram[posttoken])/(float(self.f_map_t[posttoken]))) > 0.6:
        return u"NO0.5" # NEW - EXPERIMENTAL FOR GERMAN
      elif regex.match(ur"[0-9]+[.]$", token) and regex.match(ur"[[:upper:]][[:lower:]]{2,}$", posttoken) and float(self.f_map_t[posttoken]) / float(self.f_map_l[posttoken.lower()]) > 0.9:
        return u"NO0.6"  # NEW - EXPERIMENTAL FOR GERMAN
      elif regex.match(ur"^[0-9][.]$", token[-2:]):
        return u"OK8"
      elif regex.match(ur"^[[:upper:]][.]$", token) and pretoken.istitle() and posttoken.istitle():
        return u"NO5"
      elif regex.match(ur"^[[(][^)]+[.]$", token):
        return u"NO6"
      elif float(self.f_map_t[posttoken]) / float(self.f_map_l[posttoken.lower()]) < 0.6:
        return u"[{0:.4f}] OK2".format(float(self.f_map_t[posttoken]) / float(self.f_map_l[posttoken.lower()]))   # NEW - EXPERIMENTAL FOR GERMAN - RE-EDITED FROM <0.2 TO ORIGINAL 0.6
      else:
        return u"?"
    except KeyError, e:
      return u"?"

  def analysis(self, tokens, output):
    self.train(tokens)
    pre, dot, post = self.simplify(tokens)      
    Row = namedtuple('Row', [u'Previous', u'EOS', u'Next', u'Veredict'])
    table = []
    for i in xrange(len(dot)):
      table.append(Row(pre[i],                        
                       u"{0} ({1} / {2} / {3} / {4})".format(dot[i], self.f_map_t[dot[i]], self.f_map_wpunct[dot[i].rstrip(u".,:!?\"'")], self.f_map_l[dot[i].lower()], float(self.f_map_bigram[u"{0} {1}".format(dot[i], post[i])]) if u"{0} {1}".format(dot[i], post[i]) in self.f_map_bigram else 0),
                       u"{0} ({1} / {2} / {3})".format(post[i], self.f_map_t[post[i]], self.f_map_b[post[i].lower()], self.f_map_l[post[i].lower()]),
                       self.veredict(pre[i], dot[i], post[i])))
    self.pprinttable(table, output)
    
    
  def train(self, tokens):
    pre, dot, post = self.simplify(tokens)      
    self.f_map_t = im(self.f_map_t, self.freqs(tokens)) 
    # raw frequency of each token lowercased
    self.f_map_l = im(self.f_map_l, self.freqs([i.lower() for i in tokens]))
    # raw frequency of tokens considered as possible sentence beginning
    self.f_map_b = im(self.f_map_b, self.freqs([i.lower() for i in post]))
    # raw frequency of tokens without punctuation at the end
    self.f_map_wpunct = im(self.f_map_wpunct, self.freqs([i.rstrip(u".,:!?\"'") for i in tokens]))
    # raw frequency of bigrams with punctuation at the end of token and upper at the start of the posttoken
    self.f_map_bigram = im(self.f_map_bigram, self.freqs([u"{0} {1}".format(tokens[i],tokens[i+1]) for i in xrange(len(tokens)-1) if regex.match(ur"[[:upper:]][[:lower:]]?[.]", tokens[i]) and regex.match(ur"[[:upper:]]", tokens[i+1])]))
    #frequences that a token is preced by a number with dot
    self.f_map_prenumdot_bigram = im(self.f_map_prenumdot_bigram, self.freqs([u"{0}".format(tokens[i+1]) for i in xrange(len(tokens)-1) if regex.match(ur"[0-9]+[.]", tokens[i]) and regex.match(ur"[[:upper:]]", tokens[i+1])]))
  
    # dot before number
    f_prenum = self.freqs([dot[i] for i in xrange(len(dot)) if regex.match(ur"^([0-9]|[iIvVXxlL]{1,4})",post[i])])
    self.f_map_prenum = im(self.f_map_prenum, dict(f_prenum.items() + {key:0 for key in tokens if not key in f_prenum}.items()))

    # dot before proper_noun
    f_propernoun = self.freqs([dot[i] for i in xrange(len(dot)) if post[i][0:1].isupper() and float(self.f_map_t[post[i]])/float(self.f_map_l[post[i].lower()]) >= 0.95 and float(self.f_map_t[dot[i]])/float(self.f_map_wpunct[dot[i].rstrip(u".,:!?\"'")]) >= 0.95]) 
    self.f_map_prepn = im(self.f_map_prepn, dict(f_propernoun.items() + {key:0 for key in tokens if not key in f_propernoun}.items()))
    
  def split(self, words):
    posini = 0
    for i in xrange(len(words)):
      if self.endswith(words[i], ENDINGS):
        pre = words[i-1] if i > 0 else u""
        pos = words[i+1] if i < (len(words) - 1) else u""
        retval = self.veredict(pre, words[i], pos)
        if retval[-3:-1] == u"OK":
          yield u" ".join(words[posini:i+1])
          posini = i +1
    else:
      if posini <= len(words) - 1:
        yield u" ".join(words[posini:])
    
    return
    

    
  ### CODE ADDED BY MIQUEL ESPLÀ-GOMIS ON AUGUST'14 ###

  #Method that updates the counts in the list "previous" by counting the tokens in "tokens_list"
  def update_freqs(self, tokens_list, previous):
    for i in tokens_list:
      if not i in previous:
        previous[i] = 1
      else:
        previous[i] += 1          
    return
  
  #Method that simplifies a sub-list of tokens from a longer list by using
  #context (the word just before the sub-list and the next word just after
  #the list)
  def context_simplify(self, new_tokens_list, prev_last_tok, next_first_tok):
    pre = []
    post = []
    dot = []
    
    tokens_list=[]
    tokens_list.append(prev_last_tok)
    tokens_list.extend(new_tokens_list)
    tokens_list.append(next_first_tok)
  
    for i in xrange(1,len(tokens_list)-1):
      if self.endswith(tokens_list[i], ENDINGS):
        if i >= 1:
          pre.append(tokens_list[i-1])
        else:
          pre.append(u"")
    
        if i<(len(tokens_list)-1):
          post.append(tokens_list[i+1])
        else:
          post.append(u"")
      
        dot.append(tokens_list[i])

    return pre,dot,post
  
  #Method that initialises a model by setting up the internal variables
  def init_model(self):
    self.f_map_t={}
    self.f_map_t[u""] = 1
    self.f_map_l={}
    self.f_map_l[u""] = 1
    self.f_map_wpunct={}
    self.f_map_wpunct[u""] = 1
    self.f_map_bigram={}
    self.f_map_bigram[u""] = 1
    self.f_map_prenumdot_bigram={}
    self.f_map_prenumdot_bigram[u""] = 1
    self.pre=[]
    self.dot=[]
    self.post=[]
    
  #Method that updates the model after it has been fed (see feed_model)
  def update_model(self):
    # raw frequency of tokens considered as possible sentence beginning
    self.f_map_b=self.freqs([i.lower() for i in self.post])
    # dot before number
    f_prenum=self.freqs([self.dot[i] for i in xrange(len(self.dot)) if regex.match(ur"^([0-9]|[iIvVXxlL]{1,4})",self.post[i])])
    self.f_map_prenum=dict(f_prenum.items() + {key:0 for key in self.f_map_t if not key in f_prenum}.items())
    # dot before proper_noun
    f_propernoun=self.freqs([self.dot[i] for i in xrange(len(self.dot)) if self.post[i][0:1].isupper() and float(self.f_map_t[self.post[i]])/float(self.f_map_l[self.post[i].lower()]) >= 0.95 and float(self.f_map_t[self.dot[i]])/float(self.f_map_wpunct[self.dot[i].rstrip(u".,:!?\"'")]) >= 0.95])
    self.f_map_prepn=dict(f_propernoun.items() + {key:0 for key in self.f_map_t if not key in f_propernoun}.items())
    
  #Method that feeds the model with a sequence of tokens; this method can
  #be called several times to feed the model with the corresponding statistics;
  #after this, the method "update_model" has to be called to obtain the final
  #model
  def feed_model(self, tokens):
    tmp_pre, tmp_dot, tmp_post = self.simplify(tokens)
    self.pre.extend(tmp_pre)
    self.dot.extend(tmp_dot)
    self.post.extend(tmp_post)

    self.update_freqs(tokens, self.f_map_t) 
    # raw frequency of each token lowercased
    self.update_freqs([i.lower() for i in tokens], self.f_map_l)
    # raw frequency of tokens without punctuation at the end
    self.update_freqs([i.rstrip(u".,:!?\"'") for i in tokens], self.f_map_wpunct)
    # raw frequency of bigrams with punctuation at the end of token and upper at the start of the posttoken
    self.update_freqs([u"{0} {1}".format(tokens[i],tokens[i+1]) for i in xrange(len(tokens)-1) if regex.match(ur"[[:upper:]][[:lower:]]?[.]", tokens[i]) and regex.match(ur"[[:upper:]]", tokens[i+1])], self.f_map_bigram)
    #frequences that a token is preced by a number with dot
    self.update_freqs([u"{0}".format(tokens[i+1]) for i in xrange(len(tokens)-1) if regex.match(ur"[0-9]+[.]", tokens[i]) and regex.match(ur"[[:upper:]]", tokens[i+1])], self.f_map_prenumdot_bigram)
    last_word=tokens[-1]
  
  #Method that obtains a generator producing, in each iteration a list
  #of tokens corresponding to a sentence from the input
  def tokens_generator(self, reader):
    for l in reader:
      yield splitinwords(l.strip())

  def line_by_line_training(self, tokens_generator):
    last_word=u""
    tokens=tokens_generator.next()
    
    for next_step_tokens in tokens_generator:
      tmp_pre, tmp_dot, tmp_post = self.context_simplify(tokens, last_word, next_step_tokens[0])
      pre.extend(tmp_pre)
      dot.extend(tmp_dot)
      post.extend(tmp_post)

      self.update_freqs(tokens, self.f_map_t) 
      # raw frequency of each token lowercased
      self.update_freqs([i.lower() for i in tokens], self.f_map_l)
      # raw frequency of tokens without punctuation at the end
      self.update_freqs([i.rstrip(u".,:!?\"'") for i in tokens], self.f_map_wpunct)
      # raw frequency of bigrams with punctuation at the end of token and upper at the start of the posttoken
      self.update_freqs([u"{0} {1}".format(tokens[i],tokens[i+1]) for i in xrange(len(tokens)-1) if regex.match(ur"[[:upper:]][[:lower:]]?[.]", tokens[i]) and regex.match(ur"[[:upper:]]", tokens[i+1])], self.f_map_bigram)
      #frequences that a token is preced by a number with dot
      self.update_freqs([u"{0}".format(tokens[i+1]) for i in xrange(len(tokens)-1) if regex.match(ur"[0-9]+[.]", tokens[i]) and regex.match(ur"[[:upper:]]", tokens[i+1])], self.f_map_prenumdot_bigram)
      last_word=tokens[-1]
      tokens=next_step_tokens
      
    tmp_pre, tmp_dot, tmp_post = self.context_simplify(tokens, last_word, u"")
    pre.extend(tmp_pre)
    dot.extend(tmp_dot)
    post.extend(tmp_post)

    self.update_freqs(tokens, self.f_map_t) 
    # raw frequency of each token lowercased
    self.update_freqs([i.lower() for i in tokens], self.f_map_l)
    # raw frequency of tokens without punctuation at the end
    self.update_freqs([i.rstrip(u".,:!?\"'") for i in tokens], self.f_map_wpunct)
    # raw frequency of bigrams with punctuation at the end of token and upper at the start of the posttoken
    self.update_freqs([u"{0} {1}".format(tokens[i],tokens[i+1]) for i in xrange(len(tokens)-1) if regex.match(ur"[[:upper:]][[:lower:]]?[.]", tokens[i]) and regex.match(ur"[[:upper:]]", tokens[i+1])], self.f_map_bigram)
    #frequences that a token is preced by a number with dot
    self.update_freqs([u"{0}".format(tokens[i+1]) for i in xrange(len(tokens)-1) if regex.match(ur"[0-9]+[.]", tokens[i]) and regex.match(ur"[[:upper:]]", tokens[i+1])], self.f_map_prenumdot_bigram)
    last_word=tokens[-1]
    
    # raw frequency of tokens considered as possible sentence beginning
    self.f_map_b=self.freqs([i.lower() for i in post])
    # dot before number
    f_prenum=self.freqs([dot[i] for i in xrange(len(dot)) if regex.match(ur"^([0-9]|[iIvVXxlL]{1,4})",post[i])])
    self.f_map_prenum=sorted(dict(f_prenum.items() + {key:0 for key in self.f_map_t if not key in f_prenum}.items()))
    # dot before proper_noun
    f_propernoun=self.freqs([dot[i] for i in xrange(len(dot)) if post[i][0:1].isupper() and float(self.f_map_t[post[i]])/float(self.f_map_l[post[i].lower()]) >= 0.95 and float(self.f_map_t[dot[i]])/float(self.f_map_wpunct[dot[i].rstrip(u".,:!?\"'")]) >= 0.95]) 
    self.f_map_prepn=sorted(dict(f_propernoun.items() + {key:0 for key in self.f_map_t if not key in f_propernoun}.items()))
    
  ### END OF THE CODE ADDED BY MIQUEL ESPLÀ-GOMIS ####
    
  
def help():
  sys.stderr.write("""\
Usage: {0} [options] [input [output]]
  Optional parameters:
    input           
    output           
  Options:
    -d, --data=file trained data file
    -t, --train     train sentence splitter and write training to the output
    -b, --batch     
    -a, --analysis
    -c, --compare
    -h, --help      shows this help\n""".format(os.path.basename(sys.argv[0])))
    

def splitinwords(mystr):
  return regex.split(ur"[ \n\r\t\xa0]+", mystr)

def main():

  try:
    opts, args = getopt.getopt(sys.argv[1:], "d:btach", ["data=", "batch", "train", "analysis", "compare", "help"])
  except getopt.GetoptError, err:
    help()
    sys.exit(2)
    
  train_mode = False
  analysis_mode = False
  compare_mode = False
  batch_mode = False
  datafile=""
  for o, v in opts:
    if o in ("-d", "--data"):
      datafile = v
    elif o in ("-b", "--batch"):
     batch_mode = True
    elif o in ("-t", "--train"):
      train_mode = True
    elif o in ("-a", "--analysis"):
      analysis_mode = True
    elif o in ("-c", "--compare"):
      compare_mode = True
    elif o in ("-h", "--help"):
      help()
      sys.exit(0)

  if len(args) == 0:
    if not batch_mode:
      input = codecs.getreader('utf-8')(sys.stdin)
    else:
      input = sys.stdin
    if train_mode:
      output = sys.stdout
    else:
      output = codecs.getwriter('utf-8')(sys.stdout)  
  elif len(args) == 1:
    if not batch_mode:
      input = codecs.getreader('utf-8')(open(args[0], "rb"))
    else:
      input = open(args[0], "rb")

    if train_mode:
      output = sys.stdout
    else:
      output = codecs.getwriter('utf-8')(sys.stdout)  
  elif len(args) == 2:
    if not batch_mode:
      input = codecs.getreader('utf-8')(open(args[0], "rb"))
    else:
      input = open(args[0], "rb")
    if train_mode:
      output = open(args[1], "wb")
    else:
      output = codecs.getwriter('utf-8')(open(args[1], "wb"))  
  else:
    help()
    sys.exit(2)  

  if analysis_mode:
    tokens = []
 
    for i in input:
      for j in splitinwords(i.strip()):
        tokens.append(j)

    mitok = Ulysses()

    mitok.analysis(tokens, output)
    sys.exit(0)

  if train_mode:
    tokens = []
    for i in input:
      for j in splitinwords(i.strip()):
        tokens.append(j)

    mitok = Ulysses()
    mitok.train(tokens)
    #mitok.line_by_line_training(mitok.tokens_generator(input))
    pickle.dump(mitok, output)
    sys.exit(0)


  mitok = Ulysses()
  name = ""

  if datafile != "":
    mitok = pickle.load(open(datafile, "r"))
  else:
    mitemp = tempfile.NamedTemporaryFile(delete=False)
    name = mitemp.name
  
    mitemp2 = codecs.getwriter('utf-8')(mitemp)
  
    for i in input:
      mitemp2.write(i)
    
    mitemp2.close()
  
    input = codecs.getreader('utf-8')(open(name, "r"))
    tokens = []

    for i in input:
      for j in splitinwords(i.strip()):
        tokens.append(j)
    mitok.train(tokens)
  
    input = codecs.getreader('utf-8')(open(name, "r"))
    
  if not batch_mode:
    for i in input:
      if compare_mode:
        output.write(u"{0}\n".format(i.strip()))
        output.write(u"---\n")
      for j in mitok.split(splitinwords(i.strip())):
        output.write(u"{0}\n".format(j))
      if compare_mode:
        output.write(u"\n")
  else:
    for f in input:
      finput = codecs.getreader('utf-8')(open(f.strip(), "r"))
      foutput = codecs.getwriter('utf-8')(open("{0}.ptk".format(f.strip()), "w"))
      for i in finput:
        if compare_mode:
          foutput.write(u"{0}\n".format(i.strip()))
          foutput.write(u"---\n")
        for j in mitok.split(splitinwords(i.strip())):
          foutput.write(u"{0}\n".format(j))
        if compare_mode:
          foutput.write(u"\n")
      finput.close()
      
  if input != sys.stdin:
    input.close()
  
  if name != "":
    os.unlink(name)

if __name__ == "__main__":
  main()
