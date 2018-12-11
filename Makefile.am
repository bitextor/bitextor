
bitextordir = $(prefix)/share/bitextor

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = bitextor-git.pc

SUBDIRS= utils piped-boilerpipe model 

bitextor_SOURCEFILES=bitextor-align-documents.py bitextor-align-segments.py bitextor-dedup.py \
            bitextor-cleantextalign.py bitextor-crawl2ett.sh bitextor-rank.py bitextor-train-document-alignment.py \
            bitextor-ett2lett.sh bitextor-lett-language-detector.py bitextor-idx2ridx.py bitextor-lett2idx.py bitextor-identifyMIME.py bitextor-dir2warc.py \
            bitextor-lett2lettr.py bitextor-ridx2filenames.py bitextor-creepy.py bitextor-httrack.py \
            bitextor-buildTMX.sh bitextor-builddics.sh install_nltk_data.py \
            features/bitextor-structuredistance.py features/bitextor-urlsetoverlap.py features/bitextor-urlsdistance.py \
            features/bitextor-imagesetoverlap.py features/bitextor-mutuallylinked.py features/bitextor-urlscomparison.py \
            bitextor-buildTMX-dedup.py bitextor-buildTMX-dedupRAM.py bitextor-warc2tt.py bitextor-webdir2warc.sh bitextor-get-html-text.py

bin_SCRIPTS=bitextor bitextor-align-documents bitextor-align-segments bitextor-cleantextalign bitextor-rank \
            bitextor-ett2lett bitextor-lett-language-detector bitextor-creepy bitextor-train-document-alignment bitextor-identifyMIME bitextor-dedup \
            bitextor-idx2ridx bitextor-lett2idx bitextor-lett2lettr bitextor-builddics bitextor-crawl2ett bitextor-httrack \
            bitextor-ridx2filenames bitextor-buildTMX bitextor-elrc-filtering bitextor-dir2warc \
            features/bitextor-structuredistance features/bitextor-urlsetoverlap features/bitextor-urlsdistance \
            features/bitextor-imagesetoverlap features/bitextor-mutuallylinked features/bitextor-urlscomparison \
            bitextor-buildTMX-dedup bitextor-buildTMX-dedupRAM bitextor-get-html-text bitextor-warc2tt bitextor-webdir2warc
            
EXTRA_DIST= $(bitextor_SOURCEFILES) utils \
            model

CLEANFILES=$(bin_SCRIPTS)

compile-smttools:
	cd mgiza/mgizapp/ && cmake . && make

compile-hunalign:
	cd hunalign/src/hunalign && make 

compile-truecaser:
	cd preprocess && cmake . && make -j4

compile-kenlm:
	cd kenlm && cmake . && make -j4

compile-bleualign:
	cd bleualign-cpp && cmake . && make

compile-clustercat:
	cd clustercat && make 

install-smttools:
	cp mgiza/mgizapp/bin/* $(prefix)/bin/.
	cp clustercat/bin/* $(prefix)/bin/.
	touch $(prefix)/bin/__init__.py
	cp -R preprocess/moses $(prefix)/share/moses
	cp preprocess/bin/truecase $(prefix)/bin/.
	cp kenlm/bin/query kenlm/bin/build_binary $(prefix)/bin/.

install-standoff:
	cp standoff/deferred-document.py $(prefix)/bin/deferred-document
	cp standoff/deferred-sentences.py $(prefix)/bin/deferred-sentences
	cp standoff/reconstructor-deferred-sentences.py $(prefix)/bin/reconstructor-deferred-sentences
	cp standoff/wagnerfischer.py $(prefix)/bin/.
	chmod a+x $(prefix)/bin/deferred-document $(prefix)/bin/deferred-sentences $(prefix)/bin/reconstructor-deferred-sentences

install-hunalign:
	cp hunalign/src/hunalign/hunalign $(prefix)/bin/.

bitextor-train-document-alignment: bitextor-train-document-alignment.py

install: install-recursive install-smttools install-hunalign install-standoff

bitextor: bitextor.sh compile-clustercat compile-bleualign compile-smttools compile-hunalign compile-truecaser compile-kenlm
	cp $< $@

nltk_data:
	python3 install_nltk_data.py

features/bitextor-urlscomparison: features/bitextor-urlscomparison.py
	cp $< $@

features/bitextor-structuredistance: features/bitextor-structuredistance.py
	cp $< $@

features/bitextor-mutuallylinked: features/bitextor-mutuallylinked.py
	cp $< $@

bitextor-dir2warc: bitextor-dir2warc.py
	cp $< $@

bitextor-dedup: bitextor-dedup.py
	cp $< $@

bitextor-get-html-text: bitextor-get-html-text.py
	cp $< $@

bitextor-warc2tt: bitextor-warc2tt.py
	cp $< $@

bitextor-identifyMIME: bitextor-identifyMIME.py
	cp $< $@

bitextor-rank: bitextor-rank.py
	cp $< $@

features/bitextor-urlsetoverlap: features/bitextor-urlsetoverlap.py
	cp $< $@

features/bitextor-imagesetoverlap: features/bitextor-imagesetoverlap.py
	cp $< $@

features/bitextor-urlsdistance: features/bitextor-urlsdistance.py
	cp $< $@

bitextor-creepy: bitextor-creepy.py
	cp $< $@

bitextor-crawl2ett: bitextor-crawl2ett.sh
	cp $< $@

bitextor-buildTMX: bitextor-buildTMX.py
	cp $< $@

bitextor-buildTMX-dedup: bitextor-buildTMX-dedup.py
	cp $< $@

bitextor-buildTMX-dedupRAM: bitextor-buildTMX-dedupRAM.py
	cp $< $@

bitextor-ridx2filenames: bitextor-ridx2filenames.py
	cp $< $@

bitextor-align-segments: bitextor-align-segments.py
	cp $< $@

bitextor-align-documents: bitextor-align-documents.py
	cp $< $@

bitextor-cleantextalign: bitextor-cleantextalign.py
	cp $< $@

bitextor-ett2lett: bitextor-ett2lett.sh
	cp $< $@

bitextor-lett-language-detector: bitextor-lett-language-detector.py
	cp $< $@

bitextor-idx2ridx: bitextor-idx2ridx.py
	cp $< $@

bitextor-lett2idx: bitextor-lett2idx.py nltk_data
	cp $< $@

bitextor-lett2lettr: bitextor-lett2lettr.py
	cp $< $@

bitextor-builddics: bitextor-builddics.sh nltk_data
	cp $< $@

bitextor-elrc-filtering: bitextor-elrc-filtering.py
	cp $< $@

bitextor-httrack: bitextor-httrack.py
	cp $< $@

bitextor-webdir2warc: bitextor-webdir2warc.sh
	cp $< $@
