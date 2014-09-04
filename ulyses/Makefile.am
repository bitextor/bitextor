export PYTHONPATH = $(pythondir)

EXTRA_DIST=setup.py setup.cfg ulyses

ulyses_SOURCES=ulyses/__init__.py ulyses/ulyses.py

install:
	mkdir -p $(PYTHONPATH)
	python setup.py install --prefix=$(prefix)
