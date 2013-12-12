include $(top_srcdir)/config/common.am

CLEANFILES = fd2ps.1

EXTRA_DIST = Readme fd2ps.man

SUBDIRS = test

man_MANS = fd2ps.1

fd2ps.1:
	cp -p $(srcdir)/fd2ps.man fd2ps.1

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib $(X_CFLAGS)

bin_PROGRAMS = fd2ps

fd2ps_LDADD = ../lib/libforms.la $(X_LIBS) $(X_PRE_LIBS) $(LIBS) $(X_EXTRA_LIBS)

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
