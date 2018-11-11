#!/usr/bin/env python3

import sys
sys.path.append('/home/hieu/permanent/software/mosesdecoder/scripts/tokenizer')

from toolwrapper import ToolWrapper

from mosestokenizer import *

tokenize = MosesTokenizer('en')

toks = tokenize('Hello World!')
print("toks", toks)
