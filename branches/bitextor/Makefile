SHELL = /bin/sh

srcdir      = ./src/
doc_file    = ./bitextor.doxyfile


CC       = g++
CPPFLAGS = -Wall -g3

LIBS = -ltextcat -lxml++-2.6 -lxml2 -lglibmm-2.4 -lgobject-2.0 -lsigc-2.0 -lglib-2.0
INCLUDES = -I$(srcdir)

COMPILE = $(CC) $(INCLUDES) $(CPPFLAGS)
LINK    = $(CC) $(INCLUDES) $(CPPFLAGS) -o $@
RM = rm -f

SRC_OBJ = $(srcdir)Bitextor.o $(srcdir)WebFile.o $(srcdir)TagClean.o

.SUFFIXES: .cpp .o

.cpp.o:
	$(COMPILE) -c $<

# We make object files depend on every header.  Rather than attempt to
# track dependencies, everything gets recompiled when a header
# changes.  With a program of Wget's size this doesn't waste much
# time, and it's a lot safer than attempting to get all the
# dependencies right.

Bitextor: $(SRC_OBJ)  mk_srcdir
	$(LINK) $(SRC_OBJ) $(LIBS)

$(SRC_OBJ):
	$(MAKE) -C $(srcdir)

mk_srcdir:
	$(MAKE) -C $(srcdir)

all: Bitextor
.PHONY: all

clean:
	$(RM) $(srcdir)*.o $(srcdir)*~ $(srcdir)*.bak $(srcdir)core $(srcdir)core.[0-9]*
	$(RM) Bitextor

documentation:
	doxygen $(doc_file)

