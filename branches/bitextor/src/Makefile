SHELL = /bin/sh

CC       = g++
CPPFLAGS = -Wall -g3

srcdir      = .
INCLUDES = -I$(srcdir) -I /usr/include/libxml++-2.6 -I /usr/include/libxml++-2.6/libxml++ -I/usr/include/glibmm-2.4 -I/usr/lib/glibmm-2.4/include -I/usr/include/sigc++-2.0 -I/usr/lib/sigc++-2.0/include -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -I/usr/lib/libxml++-2.6/include

COMPILE = $(CC) $(INCLUDES) $(CPPFLAGS)
LINK    = $(CC) $(INCLUDES) $(CPPFLAGS) -o $@
RM = rm -f

OBJ = Bitextor.o WebFile.o TagClean.o

.SUFFIXES:
.SUFFIXES: .cpp .o .c

.cpp.o:
	$(COMPILE) -c $<

# We make object files depend on every header.  Rather than attempt to
# track dependencies, everything gets recompiled when a header
# changes.  With a program of Wget's size this doesn't waste much
# time, and it's a lot safer than attempting to get all the
# dependencies right.

all: $(OBJ)
.PHONY: all

$(OBJ): WebFile.h TagClean.h


clean:
	$(RM) *.o *~ *.bak core core.[0-9]*

