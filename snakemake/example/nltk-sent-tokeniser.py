#!/usr/bin/env python3
import sys
from nltk import tokenize

lang = sys.argv[1]
for line in sys.stdin:
    try:
        print("\n".join(tokenize.sent_tokenize(line,lang)))
    except:
        print("\n".join(tokenize.sent_tokenize(line,"english")))
