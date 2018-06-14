export PYTHONPATH = $(pythondir)

EXTRA_DIST=setup.py setup.cfg ulysses

install:
	mkdir -p $(PYTHONPATH)
	python3 setup.py install --prefix=$(prefix)
