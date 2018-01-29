#!/usr/bin/env python

from sys import *

def loadvcb(fname,out):
	dict={};
	df = open(fname,"r");
	for line in df:
		out.write(line);
		ws = line.strip().split();
	        id = int(ws[0]);
		wd = ws[1];
		dict[wd]=id;
	return dict;

if len(argv)<9:
	stderr.write("Error, the input should be \n");
	stderr.write("%s evcb fvcb etxt ftxt esnt(out) fsnt(out) evcbx(out) fvcbx(out)\n" % argv[0]);
	stderr.write("You should concatenate the evcbx and fvcbx to existing vcb files\n");
	exit();

ein = open(argv[3],"r");
fin = open(argv[4],"r");

eout = open(argv[5],"w");
fout = open(argv[6],"w");

evcbx = open(argv[7],"w");
fvcbx = open(argv[8],"w");
evcb = loadvcb(argv[1],evcbx);
fvcb = loadvcb(argv[2],fvcbx);

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
				#el.append(1);
				nid = len(evcb)+1;
				evcb[w.lower()] = nid;
				evcbx.write("%d %s 1\n" % (nid, w));
				el.append(nid);

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
				nid = len(fvcb)+1;
				fvcb[w.lower()] = nid;
				fvcbx.write("%d %s 1\n" % (nid, w));
				fl.append(nid);
				#fl.append(1);
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

