
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

bitextor: compile-smttools compile-hunalign compile-truecaser compile-kenlm compile-bleualign

nltk_data:
	python3 install_nltk_data.py

features/bitextor-urlscomparison: features/bitextor-urlscomparison.py

features/bitextor-structuredistance: features/bitextor-structuredistance.py

features/bitextor-mutuallylinked: features/bitextor-mutuallylinked.py

bitextor-dir2warc: bitextor-dir2warc.py

bitextor-dedup: bitextor-dedup.py

bitextor-warc2tt: bitextor-warc2tt.py

bitextor-get-html-text: bitextor-get-html-text.py

bitextor-identifyMIME: bitextor-identifyMIME.py

bitextor-rank: bitextor-rank.py

features/bitextor-urlsetoverlap: features/bitextor-urlsetoverlap.py

features/bitextor-imagesetoverlap: features/bitextor-imagesetoverlap.py

features/bitextor-urlsdistance: features/bitextor-urlsdistance.py

bitextor-creepy: bitextor-creepy.py

bitextor-crawl2ett: bitextor-crawl2ett.sh

bitextor-buildTMX: bitextor-buildTMX.py

bitextor-buildTMX-dedup: bitextor-buildTMX-dedup.py

bitextor-buildTMX-dedupRAM: bitextor-buildTMX-dedupRAM.py

bitextor-ridx2filenames: bitextor-ridx2filenames.py

bitextor-align-segments: bitextor-align-segments.py

bitextor-align-documents: bitextor-align-documents.py

bitextor-cleantextalign: bitextor-cleantextalign.py

bitextor-ett2lett: bitextor-ett2lett.sh

bitextor-lett-language-detector: bitextor-lett-language-detector.py

bitextor-idx2ridx: bitextor-idx2ridx.py

bitextor-lett2idx: bitextor-lett2idx.py nltk_data

bitextor-lett2lettr: bitextor-lett2lettr.py

bitextor-builddics: bitextor-builddics.sh nltk_data

bitextor-elrc-filtering: bitextor-elrc-filtering.py

bitextor-httrack: bitextor-httrack.py

bitextor-webdir2warc: bitextor-webdir2warc.sh

