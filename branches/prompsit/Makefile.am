
bitextordir = $(prefix)/share/bitextor

bitextor_DATA = diccionario.dic

SUBDIRS= hunalign-src hunalign-src/src/hunalign simpleHTMLreader \
         splitsentences tika

bin_SCRIPTS=bitextor bitextor-aligner bitextor-cleantextalign \
            bitextor-distancefilter bitextor-downloadweb bitextor-ett2lett \
            bitextor-idx2ridx bitextor-lett2idx bitextor-lett2lettr \
            bitextor-webdir2ett bitextor-documentaligner bitextor-sentencealign-prealignedfiles \
            bitextor-combineridx
            
man_MANS=bitextor.1 bitextor-aligner.1 bitextor-cleantextalign.1 \
         bitextor-distancefilter.1 bitextor-downloadweb.1 bitextor-ett2lett.1 \
         bitextor-idx2ridx.1 bitextor-lett2idx.1 bitextor-lett2lettr.1 \
         bitextor-webdir2ett.1

EXTRA_DIST= $(man_MANS) $(bin_SCRIPTS) $(bitextor_DATA) simpleHTMLreader \
            splitsentences DocumentacionBitextor-4.odt tika

CLEANFILES=$(bin_SCRIPTS)

bitextor: bitextor.in
	sed "s!__PREFIX__!$(prefix)!g" $< | \
	sed "s!__BASH__!$(BASH)!g;" >$@

bitextor-combineridx: bitextor-combineridx.in
	sed "s!__PREFIX__!$(prefix)!g" $< | \
	sed "s!__PYTHON__!$(PYTHON)!g" | \
	sed "s!__ENV__!$(ENV)!g" >$@

bitextor-aligner: bitextor-aligner.in
	sed "s!__PREFIX__!$(prefix)!g" $< | \
	sed "s!__BASH__!$(BASH)!g" | \
	sed "s!__GAWK__!$(GAWK)!g" | \
	sed "s!__PYTHON__!$(PYTHON)!g" | \
	sed "s!__PERL__!$(PERL)!g" >$@

bitextor-documentaligner: bitextor-documentaligner.in
	sed "s!__PREFIX__!$(prefix)!g" $< | \
	sed "s!__BASH__!$(BASH)!g" | \
	sed "s!__GAWK__!$(GAWK)!g" | \
	sed "s!__CUT__!$(CUT)!g" | \
	sed "s!__COMM__!$(COMM)!g" | \
	sed "s!__SORT__!$(SORT)!g" | \
	sed "s!__GREP__!$(GREP)!g" | \
	sed "s!__PYTHON__!$(PYTHON)!g" | \
	sed "s!__PERL__!$(PERL)!g" >$@

bitextor-sentencealign-prealignedfiles: bitextor-sentencealign-prealignedfiles.in
	sed "s!__PREFIX__!$(prefix)!g" $< | \
	sed "s!__BASH__!$(BASH)!g;" | \
	sed "s!__GAWK__!$(GAWK)!g" | \
	sed "s!__PERL__!$(PERL)!g" | \
	sed "s!__PYTHON__!$(PYTHON)!g" >$@

bitextor-cleantextalign: bitextor-cleantextalign.in
	sed "s!__BASH__!$(BASH)!g" $< | \
	sed "s!__PYTHON__!$(PYTHON)!g" >$@

bitextor-distancefilter: bitextor-distancefilter.in
	sed "s!__PREFIX__!$(prefix)!g" $< | \
	sed "s!__ENV__!$(ENV)!g" | \
	sed "s!__PYTHON__!$(PYTHON)!g" >$@

bitextor-downloadweb: bitextor-downloadweb.in
	sed "s!__HTTRACK__!$(HTTRACK)!g" $< | \
	sed "s!__BASH__!$(BASH)!g;" >$@

bitextor-ett2lett: bitextor-ett2lett.in
	sed "s!__PREFIX__!$(prefix)!g" $< | \
	sed "s!__BASH__!$(BASH)!g" | \
	sed "s!__GAWK__!$(GAWK)!g" | \
	sed "s!__PYTHON__!$(PYTHON)!g" | \
	sed "s!__PERL__!$(PERL)!g" >$@

bitextor-idx2ridx: bitextor-idx2ridx.in
	sed "s!__PREFIX__!$(prefix)!g" $< | \
	sed "s!__ENV__!$(ENV)!g" | \
	sed "s!__PYTHON__!$(PYTHON)!g" >$@

bitextor-lett2idx: bitextor-lett2idx.in
	sed "s!__BASH__!$(BASH)!g" $< | \
	sed "s!__GAWK__!$(GAWK)!g" | \
	sed "s!__PYTHON__!$(PYTHON)!g" >$@

bitextor-lett2lettr: bitextor-lett2lettr.in
	sed "s!__BASH__!$(BASH)!g" $< | \
	sed "s!__GAWK__!$(GAWK)!g" | \
	sed "s!__PYTHON__!$(PYTHON)!g" >$@

bitextor-webdir2ett: bitextor-webdir2ett.in
	sed "s!__BASH__!$(BASH)!g" $< | \
	sed "s!__GAWK__!$(GAWK)!g" | \
	sed "s!__KILL__!$(KILL)!g" | \
	sed "s!__JAVA__!$(JAVA)!g" | \
	sed "s!__LSOF__!$(LSOF)!g" | \
	sed "s!__WC__!$(WC)!g" | \
	sed "s!__TIKA__!$(prefix)/share/java/tika-app-1.4.jar!g" | \
	sed "s!__PYTHON__!$(PYTHON)!g" >$@

