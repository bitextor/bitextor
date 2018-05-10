
bitextordir = $(prefix)/share/bitextor

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = bitextor-git.pc

SUBDIRS= utils clustercat ulysses piped-boilerpipe piped-tika ulysses-data model

bitextor_SOURCEFILES=bitextor.sh bitextor-align-documents.py bitextor-align-segments.py \
            bitextor-cleantextalign.py bitextor-crawl2ett.sh bitextor-rank.py bitextor-train-document-alignment.py \
            bitextor-ett2lett.sh bitextor-idx2ridx.py bitextor-lett2idx.py \
            bitextor-lett2lettr.py bitextor-ridx2filenames.py bitextor-crawl.py bitextor-downloadweb.sh bitextor-webdir2ett.sh \
            bitextor-buildTMX.sh bitextor-score-document-alignment.py bitextor-builddics.sh install_nltk_data.py \
            features/bitextor-structuredistance.py features/bitextor-urlsetoverlap.py features/bitextor-urlsdistance.py \
            features/bitextor-imagesetoverlap.py features/bitextor-mutuallylinked.py features/bitextor-urlscomparison.py

bin_SCRIPTS=bitextor bitextor-align-documents bitextor-align-segments bitextor-cleantextalign bitextor-rank \
            bitextor-idx2ridx-freq bitextor-ett2lett bitextor-lett2idx-freq bitextor-crawl bitextor-train-document-alignment \
            bitextor-idx2ridx bitextor-lett2idx bitextor-lett2lettr bitextor-builddics bitextor-crawl2ett bitextor-downloadweb bitextor-webdir2ett \
            bitextor-ridx2filenames bitextor-buildTMX bitextor-score-document-alignment bitextor-elrc-filtering \
            features/bitextor-structuredistance features/bitextor-urlsetoverlap features/bitextor-urlsdistance \
            features/bitextor-imagesetoverlap features/bitextor-mutuallylinked features/bitextor-urlscomparison 
            
bitextor_MANS=bitextor.1 bitextor-cleantextalign.1 \
         bitextor-distancefilter.1 bitextor-ett2lett.1 \
         bitextor-idx2ridx.1 bitextor-lett2idx.1 bitextor-lett2lettr.1 \
         bitextor-buildTMX.1


EXTRA_DIST= $(man_MANS) $(bitextor_SOURCEFILES) utils \
            model

CLEANFILES=$(bin_SCRIPTS)

compile-smttools:
	cd mgiza/mgizapp/ && cmake . && make

compile-hunalign:
	cd hunalign/src/hunalign && make 

install-smttools:
	cp mgiza/mgizapp/bin/* $(prefix)/bin/.
	cp clustercat/bin/* $(prefix)/bin/.

install-hunalign:
	cp hunalign/src/hunalign/hunalign $(prefix)/bin/.

bitextor-train-document-alignment: bitextor-train-document-alignment.in
	sed "s!__PREFIX__!$(prefix)!g" $< | \
	sed "s!__PYTHON__!$(PYTHON)!g" | \
	sed "s!__ENV__!$(ENV)!g" >$@

install: install-recursive install-smttools install-hunalign

bitextor: bitextor.in compile-smttools compile-hunalign
	sed "s!__PREFIX__!$(prefix)!g" $< | \
	sed "s!__WC__!$(WC)!g" | \
	sed "s!__GREP__!$(GREP)!g" | \
	sed "s!__BASH__!$(BASH)!g;" >$@

nltk_data:
	python install_nltk_data.py

features/bitextor-urlscomparison: features/bitextor-urlscomparison.in
	sed "s!__PREFIX__!$(prefix)!g" $< | \
	sed "s!__PYTHON__!$(PYTHON)!g" | \
	sed "s!__ENV__!$(ENV)!g" >$@

features/bitextor-structuredistance: features/bitextor-structuredistance.in
	sed "s!__PREFIX__!$(prefix)!g" $< | \
	sed "s!__PYTHON__!$(PYTHON)!g" | \
	sed "s!__ENV__!$(ENV)!g" >$@

features/bitextor-mutuallylinked: features/bitextor-mutuallylinked.in
	sed "s!__PREFIX__!$(prefix)!g" $< | \
	sed "s!__PYTHON__!$(PYTHON)!g" | \
	sed "s!__ENV__!$(ENV)!g" >$@

bitextor-rank: bitextor-rank.in
	sed "s!__PREFIX__!$(prefix)!g" $< | \
	sed "s!__PYTHON__!$(PYTHON)!g" | \
	sed "s!__ENV__!$(ENV)!g" >$@

features/bitextor-urlsetoverlap: features/bitextor-urlsetoverlap.in
	sed "s!__PREFIX__!$(prefix)!g" $< | \
	sed "s!__PYTHON__!$(PYTHON)!g" | \
	sed "s!__ENV__!$(ENV)!g" >$@

features/bitextor-imagesetoverlap: features/bitextor-imagesetoverlap.in
	sed "s!__PREFIX__!$(prefix)!g" $< | \
	sed "s!__PYTHON__!$(PYTHON)!g" | \
	sed "s!__ENV__!$(ENV)!g" >$@

features/bitextor-urlsdistance: features/bitextor-urlsdistance.in
	sed "s!__PREFIX__!$(prefix)!g" $< | \
	sed "s!__PYTHON__!$(PYTHON)!g" | \
	sed "s!__ENV__!$(ENV)!g" >$@

bitextor-crawl: bitextor-crawl.in
	sed "s!__PREFIX__!$(prefix)!g" $< | \
	sed "s!__PYTHON__!$(PYTHON)!g" | \
	sed "s!__ENV__!$(ENV)!g" >$@

bitextor-crawl2ett: bitextor-crawl2ett.in
	sed "s!__PREFIX__!$(prefix)!g" $< | \
	sed "s!__BASH__!$(BASH)!g" | \
	sed "s!__JAVA__!$(JAVA)!g" | \
	sed "s!__GAWK__!$(GAWK)!g" | \
	sed "s!__GREP__!$(GREP)!g" | \
	sed "s!__WC__!$(WC)!g" | \
	sed "s!__PYTHON__!$(PYTHON)!g" >$@

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
	sed "s!__ENV__!$(ENV)!g" | \
        sed "s!__BASH__!$(BASH)!g;" >$@

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

bitextor-idx2ridx-freq: bitextor-idx2ridx-freq.in
	sed "s!__PREFIX__!$(prefix)!g" $< | \
	sed "s!__ENV__!$(ENV)!g" | \
	sed "s!__PYTHON__!$(PYTHON)!g" >$@

bitextor-lett2idx: bitextor-lett2idx.in nltk_data
	sed "s!__PREFIX__!$(prefix)!g" $< | \
	sed "s!__ENV__!$(ENV)!g" | \
	sed "s!__PYTHON__!$(PYTHON)!g" | \
	sed "s!__BASH__!$(BASH)!g;" >$@

bitextor-lett2idx-freq: bitextor-lett2idx-freq.in nltk_data
	sed "s!__PREFIX__!$(prefix)!g" $< | \
	sed "s!__ENV__!$(ENV)!g" | \
	sed "s!__PYTHON__!$(PYTHON)!g" >$@

bitextor-lett2lettr: bitextor-lett2lettr.in
	sed "s!__PREFIX__!$(prefix)!g" $< | \
	sed "s!__PYTHON__!$(PYTHON)!g" | \
	sed "s!__ENV__!$(ENV)!g" >$@

bitextor-builddics: bitextor-builddics.in nltk_data
	sed "s!__PREFIX__!$(prefix)!g" $< | \
	sed "s!__PYTHON__!$(PYTHON)!g" | \
	sed "s!__BASH__!$(BASH)!g;" >$@

bitextor-elrc-filtering: bitextor-elrc-filtering.in
	sed "s!__PREFIX__!$(prefix)!g" $< | \
	sed "s!__ENV__!$(ENV)!g" | \
	sed "s!__PYTHON__!$(PYTHON)!g" | \
	sed "s!__BASH__!$(BASH)!g;" >$@	

bitextor-downloadweb: bitextor-downloadweb.in
	sed "s!__PREFIX__!$(prefix)!g" $< | \
	sed "s!__ENV__!$(ENV)!g" | \
	sed "s!__PYTHON__!$(PYTHON)!g" | \
	sed "s!__BASH__!$(BASH)!g;" >$@ 

bitextor-webdir2ett: bitextor-webdir2ett.in
	sed "s!__PREFIX__!$(prefix)!g" $< | \
	sed "s!__ENV__!$(ENV)!g" | \
	sed "s!__PYTHON__!$(PYTHON)!g" | \
	sed "s!__BASH__!$(BASH)!g;" >$@
