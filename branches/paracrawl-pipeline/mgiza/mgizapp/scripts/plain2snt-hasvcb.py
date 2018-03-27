#!/usr/bin/env python
#
# This file is part of mgiza++.  Its use is licensed under the GNU General
# Public License version 2 or, at your option, any later version.

from __future__ import unicode_literals
import sys
import codecs
import io

if sys.version_info < (3,0,0):
    sys.stdin = codecs.getreader('UTF-8')(sys.stdin)
    sys.stdout = codecs.getwriter('UTF-8')(sys.stdout)
    sys.stderr = codecs.getwriter('UTF-8')(sys.stderr)

def loadvcb(fname,out):
	dict={};
	df = io.open(fname,"r", encoding="UTF-8");
	for line in df:
		out.write(line);
		ws = line.strip().split();
	        id = int(ws[0]);
		wd = ws[1];
		dict[wd]=id;
	return dict, id

if len(sys.argv)<9:
	sys.stderr.write("Error, the input should be \n");
	sys.stderr.write("%s evcb fvcb etxt ftxt esnt(out) fsnt(out) evcbx(out) fvcbx(out)\n" % sys.argv[0]);
	sys.stderr.write("You should concatenate the evcbx and fvcbx to existing vcb files\n");
	sys.exit();

ein = io.open(sys.argv[3],"r", encoding="UTF-8");
fin = io.open(sys.argv[4],"r", encoding="UTF-8");

eout = io.open(sys.argv[5],"w", encoding="UTF-8");
fout = io.open(sys.argv[6],"w", encoding="UTF-8");

evcbx = io.open(sys.argv[7],"w", encoding="UTF-8");
fvcbx = io.open(sys.argv[8],"w", encoding="UTF-8");
evcb, esize = loadvcb(sys.argv[1],evcbx)
fvcb, fsize = loadvcb(sys.argv[2],fvcbx)

i=0
while True:
	i+=1;
	eline=ein.readline();
	fline=fin.readline();
	if len(eline)==0 or len(fline)==0:
		break;
	ewords = eline.strip().split();
	fwords = fline.strip().split();
	el = [];
	fl = [];
	j=0;
	for w in ewords:
		j+=1
		if evcb.has_key(w):
			el.append(evcb[w]);
		else:
			if evcb.has_key(w.lower()):
				el.append(evcb[w.lower()]);
			else:
				##stdout.write("#E %d %d %s\n" % (i,j,w))
				esize += 1
				evcb[w.lower()] = esize
				evcbx.write("%d %s 1\n" % (esize, w))
				el.append(esize)

	j=0;
	for w in fwords:
		j+=1
		if fvcb.has_key(w):
			fl.append(fvcb[w]);
		else:
			if fvcb.has_key(w.lower()):
				fl.append(fvcb[w.lower()]);
			else:
				#stdout.write("#F %d %d %s\n" % (i,j,w))
				fsize += 1
				fvcb[w.lower()] = fsize
				fvcbx.write("%d %s 1\n" % (fsize, w))
				fl.append(fsize)
	eout.write("1\n");
	fout.write("1\n");
	for I in el:
		eout.write("%d " % I);
	eout.write("\n");
	for I in fl:
		eout.write("%d " % I);
		fout.write("%d " % I);
	eout.write("\n");
	fout.write("\n");
	for I in el:
		fout.write("%d " % I);
	fout.write("\n");

fout.close();
eout.close();
fvcbx.close();
evcbx.close();

