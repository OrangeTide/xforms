include $(top_srcdir)/config/common.am

# forms.h is a generated file
INCLUDES = -DMAKING_FORMS -I../lib -I$(top_srcdir)/lib $(X_CFLAGS)

include_HEADERS = glcanvas.h

lib_LTLIBRARIES = libformsGL.la

libformsGL_la_LDFLAGS = -version-info 1:0:0

libformsGL_la_SOURCES = \
	glcanvas.c \
	glcanvas.h
