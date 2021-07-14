#!/usr/bin/env python3

#  This file is part of Bitextor.
#
#  Bitextor is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  Bitextor is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with Bitextor.  If not, see <https://www.gnu.org/licenses/>.

import sys
from nltk.data import load

language = sys.argv[1]
tokenizer = None
try:
        tokenizer = load('tokenizers/punkt/{0}.pickle'.format(language))
except:
        tokenizer = load('tokenizers/punkt/{0}.pickle'.format("english"))

for line in sys.stdin:
    for sentence in tokenizer.tokenize(line):
        for endlinesentence in sentence.split('\n'):
            print(endlinesentence)
