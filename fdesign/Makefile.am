include $(top_srcdir)/config/common.am

CLEANFILES = fdesign.1

EXTRA_DIST = Change Readme fdesign.man

SUBDIRS = fd spec xpm

man_MANS = fdesign.1

fdesign.1:
	cp -p $(srcdir)/fdesign.man fdesign.1

# forms.h is a generated file
INCLUDES = -I../lib -I../lib/include -I./xpm -I$(top_srcdir)/lib $(X_CFLAGS)

bin_PROGRAMS = fdesign

fdesign_LDADD = ../lib/libforms.la $(X_LIBS) $(X_PRE_LIBS) $(XPM_LIB) -lX11 $(LIBS) $(X_EXTRA_LIBS)

fdesign_SOURCES = \
	fd_attribs.c \
	fd_control.c \
	fd_fake.c \
	fd_file.c \
	fd_file_fun.c \
	fd_forms.c \
	fd_groups.c \
	fd_help.c \
	fd_help.h \
	fd_initforms.c \
	fd_main.c \
	fd_main.h \
	fd_names.c \
	fd_objects.c \
	fd_pallette.c \
	fd_printC.c \
	fd_rubber.c \
	fd_select.c \
	fd_spec.c \
	fd_spec.h \
	fd_super.c \
	fd_util.c \
	sp_browser.c \
	sp_button.c \
	sp_choice.c \
	sp_counter.c \
	sp_spinner.c \
	sp_dial.c \
	sp_freeobj.c \
	sp_menu.c \
	sp_pixmap.c \
	sp_positioner.c \
	sp_scrollbar.c \
	sp_slider.c \
	sp_twheel.c \
	sp_xyplot.c \
	sp_util.c \
	xpm.h

#	viewforms.c \
#	viewforms.h \
#	viewforms_main.c
