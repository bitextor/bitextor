export PYTHONPATH = $(pythondir)

EXTRA_DIST=setup.py setup.cfg ulysses

install:
	mkdir -p $(PYTHONPATH)
	python setup.py install --prefix=$(prefix)
