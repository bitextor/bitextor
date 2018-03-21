#!/usr/bin/env python

# This script post process the snt file -- either in single-line format or in multi-line format
# The output, however, will always be in single-line format

from sys import *
from optparse import OptionParser
import re;
usage = """
The script post process the snt file, the input could be single-line snt 
file or multi-line, (triple line) and can insert sentence weight to the
file (-w) or add partial alignment to the file (-a)
Usage %prog -s sntfile -w weight-file -a alignfile -o outputfile
"""
parser = OptionParser(usage=usage)


parser = OptionParser()

parser.add_option("-s", "--snt", dest="snt",default=None,
		help="The input snt file", metavar="FILE")

parser.add_option("-w", "--weight", dest="weight",default=None,
		help="The input weight file", metavar="FILE")


parser.add_option("-o", "--output", dest="output",default="-",
		help="The input partial alignment file, one sentence per line", metavar="FILE")

parser.add_option("-a", "--align", dest="align",default=None,
		help="The input partial alignment file, one sentence per line", metavar="FILE")


(options, args) = parser.parse_args()

if options.snt == None:
	parser.print_help();
	exit();
else:
	sfile = open(options.snt,"r");

if options.output=="-":
	ofile = stdout;
else:
	ofile = open(options.output,"w");

wfile = None;

if options.weight <> None:
	wfile = open(options.weight,"r");

afile = None;
if options.align <> None:
	afile = open(options.align,"r");

rr = re.compile("[\\|\\#\\*]");
wt = 0.0;
al = {};
e = "";
f = "";

def parse_ax(line):
	alq = {};
	als = line.strip().split(" ");
	for e in als:
		if len(e.strip())>0:
			alo = e.split("-");
			if len(alo)==2:
				alq[tuple(alo)] = 1;
	return alq;
	





while True:
	l = sfile.readline();
	if len(l) == 0:
		break;
	lp = rr.split(l.strip());
	if len(lp)>=3:
		wt = float(lp[0]);
		e = lp[1];
		f = lp[2];
		if len(lp) > 3:
			al = parse_ax(lp[3]);
		else:
			al = {};
	else:
		wt = float(l);
		e = sfile.readline().strip();
		f = sfile.readline().strip();
		al={}
	if wfile <> None:
		lw = wfile.readline().strip();
		if len(lw)>0:
			wt = float(lw);
		else:
			wt = 1;
	if afile <> None:
		la = afile.readline().strip();
		if len(la)>0:
			al1 = parse_ax(la);
			for entry in al1.keys():
				al[entry] = 1;

	ofile.write("%g | %s | %s" % (wt, e, f));
	if len(al)>0:
		ofile.write(" |");

		for entry in al.keys():
			ofile.write(" %s-%s" % entry);
	ofile.write("\n");

	
