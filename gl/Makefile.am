include $(top_srcdir)/config/common.am

# forms.h is a generated file
INCLUDES = -DMAKING_FORMS -I../lib -I$(top_srcdir)/lib $(X_CFLAGS)

include_HEADERS = glcanvas.h

lib_LTLIBRARIES = libformsGL.la

libformsGL_la_LDFLAGS = -no-undefined -version-info @SO_VERSION@

libformsGL_la_LIBADD = ../lib/libforms.la $(X_LIBS) -lGL -lX11

libformsGL_la_SOURCES = \
	glcanvas.c \
	glcanvas.h
