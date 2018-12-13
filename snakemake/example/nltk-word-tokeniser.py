#!/usr/bin/env python3
import sys
from nltk import wordpunct_tokenize
for line in sys.stdin:
      print(" ".join(wordpunct_tokenize(line.strip())))
