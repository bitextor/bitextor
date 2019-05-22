
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

compile-bleualign:
	cd bleualign-cpp && cmake -DBUILD_TEST=on . && make

compile-clustercat:
	cd clustercat && make 

install-smttools:
	cp mgiza/mgizapp/bin/* $(prefix)/bin/.
	cp clustercat/bin/* $(prefix)/bin/.
	touch $(prefix)/bin/__init__.py
	cp -R preprocess/moses $(prefix)/share/moses
	cp preprocess/bin/truecase $(prefix)/bin/.

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

bitextor: compile-clustercat compile-bleualign compile-smttools compile-hunalign compile-truecaser

nltk_data:
	python3 install_nltk_data.py
