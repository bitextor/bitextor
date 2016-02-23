EXTRA_DIST=de.pickle.gz en.pickle.gz es.pickle.gz fr.pickle.gz it.pickle.gz pt.pickle.gz
bitextordiru=$(prefix)/share/bitextor/ulysses-data

install:
	test -d $(bitextordiru) || mkdir $(bitextordiru)
	$(INSTALL_DATA) de.pickle.gz $(bitextordiru)
	$(INSTALL_DATA) en.pickle.gz $(bitextordiru)
	$(INSTALL_DATA) es.pickle.gz $(bitextordiru)
	$(INSTALL_DATA) fr.pickle.gz $(bitextordiru)
	$(INSTALL_DATA) it.pickle.gz $(bitextordiru)
	$(INSTALL_DATA) pt.pickle.gz $(bitextordiru)
	
