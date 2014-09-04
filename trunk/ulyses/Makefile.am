export PYTHONPATH = $(pythondir)

EXTRA_DIST=setup.py setup.cfg

ulyses_SOURCES=ulyses/__init__.py ulyses/ulyses.py 

all:
	mkdir -p $(PYTHONPATH)
	python setup.py install --prefix=$(prefix)
