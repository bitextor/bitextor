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

import argparse
import sys

parser = argparse.ArgumentParser(
    description='Converts mgiza e2f and f2e files into Hunalign symmetrized dictionaries, filtering hapax legomenon')

parser.add_argument('--vcb1', help='vcb file for source language', required=True)
parser.add_argument('--vcb2', help='vcb file for target language', required=True)
parser.add_argument('--e2f', help='e2f file', required=True)
parser.add_argument('--f2e', help='f2e file', required=True)
parser.add_argument('--lang1', help='language code for source', required=True)
parser.add_argument('--lang2', help='language code for target', required=True)
parser.add_argument('--output', help='Output file path', required=False, default=sys.stdout)

args = parser.parse_args()

dic = None
if args.output is sys.stdout or args.output == "-":
    dic = sys.stdout
else:
    dic = open(args.output, 'wt')

svocabulary = set()
tvocabulary = set()
svcb = open(args.vcb1, "r")
tvcb = open(args.vcb2, "r")
for line in svcb:
    item = line.strip().split("	")
    if int(item[2]) > 10:  # Hapax legomenon filter
        svocabulary.add(item[1])

for line in tvcb:
    item = line.strip().split("	")
    if int(item[2]) > 10:
        tvocabulary.add(item[1])

a3dic = {}

e2f = open(args.e2f, "r")
f2e = open(args.f2e, "r")
for line in f2e:
    item = line.strip().split(" ")
    if item[1] in a3dic:
        a3dic[item[1]][item[0]] = item[2]
    else:
        a3dic[item[1]] = {}
        a3dic[item[1]][item[0]] = item[2]

dic.write(args.lang1+"\t"+args.lang2+"\n")
for line in e2f:
    item = line.strip().split(" ")
    if item[0] in a3dic:
        if item[1] in a3dic[item[0]]:
            value1 = float(a3dic[item[0]][item[1]])
            value2 = float(item[2])
            if value1 == 0 or value2 == 0:
                hmean = 0.0
            else:
                hmean = 2/((1/value1)+(1/value2))
            if hmean > 0.1:
                if item[1] in svocabulary and item[0] in tvocabulary:
                    if item[0].isalpha() or item[1].isalpha():
                        dic.write("{0}\t{1}\n".format(item[0], item[1]))
