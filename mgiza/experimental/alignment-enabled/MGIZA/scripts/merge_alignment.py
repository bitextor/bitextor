#!/usr/bin/env python
# Author : Qin Gao
# Date   : Dec 31, 2007
# Purpose: Combine multiple alignment files into a single one, the files are
#          prodcuced by MGIZA, which has sentence IDs, and every file is 
#          ordered inside

import sys
import re

if len(sys.argv)<2:
	sys.stderr.write("Provide me the file names (at least 2)\n");
	sys.exit();

sent_id = 0;

files = [];
ids = [];

sents = [];
done = [];

for i in range(1,len(sys.argv)):
	files.append(open(sys.argv[i],"r"));
	ids.append(0);
	sents.append("");
	done.append(False);

r = re.compile("\\((\\d+)\\)");	
i = 0;
while i< len(files):
	st1 = files[i].readline();
	st2 = files[i].readline();
	st3 = files[i].readline();
	if len(st1)==0 or len(st2)==0 or len(st3)==0:
		done[i] = True;
	else:
		mt = r.search(st1);
		id = int(mt.group(1));
		ids[i] = id;
		sents[i] = (st1, st2, st3);
	i += 1
		
cont = True;
while (cont):
	sent_id += 1;
	writeOne = False;
# Now try to read more sentences
	i = 0;
	cont = False;
	while i < len(files):
		if done[i]:
			i+=1
			continue;
		cont = True;
		if ids[i] == sent_id:
			sys.stdout.write("%s%s%s"%(sents[i][0],sents[i][1],sents[i][2]));
			writeOne = True;
			st1 = files[i].readline();
			st2 = files[i].readline();
			st3 = files[i].readline();
			if len(st1)==0 or len(st2)==0 or len(st3)==0:
				done[i] = True;
			else:
				mt = r.search(st1);
				id = int(mt.group(1));
				ids[i] = id;
				sents[i] = (st1, st2, st3);
				cont = True;
			break;
		elif ids[i] < sent_id:
			sys.stderr.write("ERROR! DUPLICATED ENTRY %d\n" % ids[i]);
			sys.exit();
		else:
			cont = True;
		i+=1;
	if (not writeOne) and cont:
		sys.stderr.write("ERROR! MISSING ENTRy %d\n" % sent_id);
		#sys.exit();
sys.stderr.write("Combined %d files, totally %d sents \n" %(len(files),sent_id-1));
