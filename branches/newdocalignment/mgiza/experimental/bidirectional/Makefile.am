## Process this file with automake to produce Makefile.in
## Created by Anjuta

SUBDIRS = src 

mgizadocdir = ${prefix}/doc/mgiza
mgizadoc_DATA = \
	README\
	COPYING\
	AUTHORS\
	ChangeLog\
	INSTALL\
	NEWS

mgizascriptsdir = ${prefix}/scripts/

mgizascripts_SCRIPTS = \
				   scripts/*

EXTRA_DIST = $(mgizadoc_DATA) \
	         ${mgizascripts_SCRIPTS}
# Copy all the spec files. Of cource, only one is actually used.
dist-hook:
	for specfile in *.spec; do \
		if test -f $$specfile; then \
			cp -p $$specfile $(distdir); \
		fi \
	done

