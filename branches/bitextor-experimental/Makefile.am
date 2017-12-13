
bitextordir = $(prefix)/share/bitextor

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = bitextor-5.0.pc

SUBDIRS= hunalign-src hunalign-src/src/hunalign utils clustercat ulysses piped-boilerpipe piped-tika ulysses-data model

bitextor_SOURCES = bitextor.in bitextor-align-documents.in bitextor-align-segments.in bitextor-idx2ridx-freq.in \
            bitextor-cleantextalign.in bitextor-crawl2ett.in bitextor-rank.in bitextor-train-document-alignment.in \
            bitextor-ett2lett.in bitextor-idx2ridx.in bitextor-lett2idx.in bitextor-lett2idx-freq.in \
            bitextor-lett2lettr.in bitextor-ridx2filenames.in bitextor-crawl.in \
            bitextor-buildTMX.in bitextor-score-document-alignment.in bitextor-builddics.in install_nltk_data.py \
            features/bitextor-structuredistance.in features/bitextor-urlsetoverlap.in features/bitextor-urlsdistance.in \
            features/bitextor-imagesetoverlap.in features/bitextor-mutuallylinked.in features/bitextor-urlscomparison.in

bin_SCRIPTS=bitextor bitextor-align-documents bitextor-align-segments bitextor-cleantextalign bitextor-rank \
            bitextor-idx2ridx-freq bitextor-ett2lett bitextor-lett2idx-freq bitextor-crawl bitextor-train-document-alignment \
            bitextor-idx2ridx bitextor-lett2idx bitextor-lett2lettr bitextor-builddics bitextor-crawl2ett \
            bitextor-ridx2filenames bitextor-buildTMX bitextor-score-document-alignment bitextor-addtextstats \
            features/bitextor-structuredistance features/bitextor-urlsetoverlap features/bitextor-urlsdistance \
            features/bitextor-imagesetoverlap features/bitextor-mutuallylinked features/bitextor-urlscomparison 
            
bitextor_MANS=bitextor.1 bitextor-cleantextalign.1 \
         bitextor-distancefilter.1 bitextor-ett2lett.1 \
         bitextor-idx2ridx.1 bitextor-lett2idx.1 bitextor-lett2lettr.1 \
         bitextor-buildTMX.1


EXTRA_DIST= $(man_MANS) $(bitextor_SOURCES) utils \
            model

CLEANFILES=$(bin_SCRIPTS)

compile-smttools:
	cd mgiza/mgizapp/ && cmake . && make 

install-smttools:
	cp mgiza/mgizapp/bin/* $(prefix)/bin/.
	cp clustercat/bin/* $(prefix)/bin/.

bitextor-train-document-alignment: bitextor-train-document-alignment.in
	sed "s!__PREFIX__!$(prefix)!g" $< | \
	sed "s!__PYTHON__!$(PYTHON)!g" | \
	sed "s!__ENV__!$(ENV)!g" >$@

install: install-recursive install-smttools

bitextor: bitextor.in compile-smttools
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

bitextor-addtextstats: bitextor-addtextstats.in
	sed "s!__PREFIX__!$(prefix)!g" $< | \
	sed "s!__ENV__!$(ENV)!g" | \
	sed "s!__PYTHON__!$(PYTHON)!g" | \
	sed "s!__BASH__!$(BASH)!g;" >$@	

