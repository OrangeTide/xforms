include $(top_srcdir)/config/common.am

CLEANFILES = fd2ps.1

EXTRA_DIST = Change Readme fd2ps.man

SUBDIRS = test

man_MANS = fd2ps.1

fd2ps.1:
	cp -p $(srcdir)/fd2ps.man fd2ps.1

# forms.h is a generated file
INCLUDES = -I../lib -I$(top_srcdir)/lib

bin_PROGRAMS = fd2ps

fd2ps_LDADD = ../lib/libforms.la @X_LIBS@ @XPM_LIB@ -lX11 @LIBS@

fd2ps_SOURCES = \
        align.c \
        fd2ps.c \
        fd2ps.h \
        flsupport.c \
        global.h \
        image2ps.c \
        load.c \
        papers.c \
        pscol.c \
        psdraw.c \
        psobj.c \
        pstext.c \
        readxpm.c \
        sys.c \
        version.c \
        xbmtops.c \
        xpmtops.c
