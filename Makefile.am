
bitextordir = $(prefix)/share/bitextor

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = bitextor-git.pc

SUBDIRS= utils model 


bin_SCRIPTS=bitextor
            
EXTRA_DIST= $(bitextor_SOURCEFILES) utils \
            model

CLEANFILES=$(bin_SCRIPTS)

compile-smttools:
	cd mgiza/mgizapp/ && cmake . && make

compile-hunalign:
	cd hunalign/src/hunalign && make 

compile-truecaser:
	cd preprocess && cmake . && make -j4

compile-warc2text:
	cd warc2text && cmake -DPREPROCESS_PATH=preprocess/ . && make -j4

compile-bleualign:
	cd bleualign-cpp && cmake -DBUILD_TEST=on -DCMAKE_BUILD_TYPE=Release -DPREPROCESS_PATH=preprocess/ . && make -j4

compile-document-aligner:
	cd document-aligner && cmake -DBUILD_TESTING=OFF . && make
compile-clustercat:
	cd clustercat && make 

install-smttools:
	cp mgiza/mgizapp/bin/* $(prefix)/bin/.
	cp clustercat/bin/* $(prefix)/bin/.
	touch $(prefix)/bin/__init__.py
	cp -R preprocess/moses $(prefix)/share/moses
	cp preprocess/bin/truecase $(prefix)/bin/.
	cp document-aligner/bin/* $(prefix)/bin/.

install-standoff:
	cp standoff/deferred_document.py $(prefix)/bin/deferred_document
	cp standoff/deferred_sentences.py $(prefix)/bin/deferred_sentences
	cp standoff/reconstructor_deferred_sentences.py $(prefix)/bin/reconstructor_deferred_sentences
	cp standoff/wagnerfischer.py $(prefix)/bin/.
	chmod a+x $(prefix)/bin/deferred_document $(prefix)/bin/deferred_sentences $(prefix)/bin/reconstructor_deferred_sentences

install-hunalign:
	cp hunalign/src/hunalign/hunalign $(prefix)/bin/.

bitextor-train-document-alignment: bitextor-train-document-alignment.py

install: install-recursive install-smttools install-hunalign install-standoff

bitextor: compile-clustercat compile-bleualign compile-smttools compile-document-aligner compile-hunalign compile-truecaser compile-warc2text

nltk_data:
	python3 install_nltk_data.py
