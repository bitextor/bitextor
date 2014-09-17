export PYTHONPATH = $(pythondir)

EXTRA_DIST=setup.py setup.cfg ulysses

ulysses_SOURCES=ulysses/__init__.py ulysses/ulysses.py

install:
	mkdir -p $(PYTHONPATH)
	python setup.py install --prefix=$(prefix)
