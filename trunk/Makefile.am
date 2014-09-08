
bitextordir = $(prefix)/share/bitextor

SUBDIRS= hunalign-src hunalign-src/src/hunalign utils giza-pp/mkcls-v2 giza-pp/GIZA++-v2 ulyses piped-boilerpipe piped-tika

bitextor_SOURCES = bitextor.in bitextor-align-documents.in bitextor-align-segments.in \
            bitextor-cleantextalign.in bitextor-distancefilter.in bitextor-downloadweb.in \
            bitextor-ett2lett.in bitextor-idx2ridx.in bitextor-lett2idx.in \
            bitextor-lett2lettr.in bitextor-webdir2ett.in bitextor-ridx2filenames.in \
            bitextor-buildTMX.in bitextor-score-document-alignment.in bitextor-builddics.in

bin_SCRIPTS=bitextor bitextor-align-documents bitextor-align-segments bitextor-cleantextalign \
            bitextor-distancefilter bitextor-downloadweb bitextor-ett2lett \
            bitextor-idx2ridx bitextor-lett2idx bitextor-lett2lettr bitextor-builddics\
            bitextor-webdir2ett bitextor-ridx2filenames bitextor-buildTMX bitextor-score-document-alignment
            
bitextor_MANS=bitextor.1 bitextor-cleantextalign.1 \
         bitextor-distancefilter.1 bitextor-downloadweb.1 bitextor-ett2lett.1 \
         bitextor-idx2ridx.1 bitextor-lett2idx.1 bitextor-lett2lettr.1 \
         bitextor-webdir2ett.1 bitextor-buildTMX.1

EXTRA_DIST= $(man_MANS) $(bitextor_SOURCES) utils \
            giza-pp 

CLEANFILES=$(bin_SCRIPTS)

bitextor: bitextor.in
	sed "s!__PREFIX__!$(prefix)!g" $< | \
	sed "s!__WC__!$(WC)!g" | \
	sed "s!__GREP__!$(GREP)!g" | \
	sed "s!__BASH__!$(BASH)!g;" >$@

bitextor-buildTMX: bitextor-buildTMX.in
	sed "s!__PREFIX__!$(prefix)!g" $< | \
	sed "s!__PYTHON__!$(PYTHON)!g" | \
	sed "s!__ENV__!$(ENV)!g" >$@

bitextor-ridx2filenames: bitextor-ridx2filenames.in
	sed "s!__PREFIX__!$(prefix)!g" $< | \
	sed "s!__PYTHON__!$(PYTHON)!g" | \
	sed "s!__ENV__!$(ENV)!g" >$@

bitextor-align-segments: bitextor-align-segments.in
	sed "s!__PREFIX__!$(prefix)!g" $< | \
	sed "s!__PYTHONPATH__!$(pythondir)!g" | \
	sed "s!__PYTHON__!$(PYTHON)!g" | \
	sed "s!__ENV__!$(ENV)!g" >$@

bitextor-score-document-alignment: bitextor-score-document-alignment.in
	sed "s!__PREFIX__!$(prefix)!g" $< | \
	sed "s!__PYTHONPATH__!$(pythondir)!g" | \
	sed "s!__PYTHON__!$(PYTHON)!g" | \
	sed "s!__ENV__!$(ENV)!g" >$@

bitextor-align-documents: bitextor-align-documents.in
	sed "s!__PREFIX__!$(prefix)!g" $< | \
	sed "s!__PYTHON__!$(PYTHON)!g" | \
	sed "s!__ENV__!$(ENV)!g" >$@

bitextor-cleantextalign: bitextor-cleantextalign.in
	sed "s!__ENV__!$(ENV)!g" $< | \
	sed "s!__PYTHON__!$(PYTHON)!g" >$@

bitextor-distancefilter: bitextor-distancefilter.in
	sed "s!__PREFIX__!$(prefix)!g" $< | \
	sed "s!__ENV__!$(ENV)!g" | \
	sed "s!__PYTHON__!$(PYTHON)!g" >$@

bitextor-downloadweb: bitextor-downloadweb.in
	sed "s!__BASH__!$(BASH)!g;" < $< | \
	sed "s!__WC__!$(WC)!g" >$@

bitextor-ett2lett: bitextor-ett2lett.in
	sed "s!__BASH__!$(BASH)!g" $< | \
	sed "s!__JAVA__!$(JAVA)!g" | \
	sed "s!__WC__!$(WC)!g" | \
	sed "s!__PREFIX__!$(prefix)!g" | \
	sed "s!__ENV__!$(ENV)!g" | \
	sed "s!__PYTHON__!$(PYTHON)!g" >$@

bitextor-idx2ridx: bitextor-idx2ridx.in
	sed "s!__PREFIX__!$(prefix)!g" $< | \
	sed "s!__ENV__!$(ENV)!g" | \
	sed "s!__PYTHON__!$(PYTHON)!g" >$@

bitextor-lett2idx: bitextor-lett2idx.in
	sed "s!__PREFIX__!$(prefix)!g" $< | \
	sed "s!__ENV__!$(ENV)!g" | \
	sed "s!__PYTHON__!$(PYTHON)!g" >$@

bitextor-lett2lettr: bitextor-lett2lettr.in
	sed "s!__PREFIX__!$(prefix)!g" $< | \
	sed "s!__PYTHON__!$(PYTHON)!g" | \
	sed "s!__ENV__!$(ENV)!g" >$@

bitextor-builddics: bitextor-builddics.in
	sed "s!__PREFIX__!$(prefix)!g" $< | \
	sed "s!__PYTHON__!$(PYTHON)!g" | \
	sed "s!__BASH__!$(BASH)!g;" >$@

bitextor-webdir2ett: bitextor-webdir2ett.in
	sed "s!__PREFIX__!$(prefix)!g" $< | \
	sed "s!__BASH__!$(BASH)!g" | \
	sed "s!__JAVA__!$(JAVA)!g" | \
	sed "s!__GAWK__!$(GAWK)!g" | \
	sed "s!__GREP__!$(GREP)!g" | \
	sed "s!__WC__!$(WC)!g" | \
	sed "s!__PYTHON__!$(PYTHON)!g" >$@

