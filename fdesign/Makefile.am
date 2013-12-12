include $(top_srcdir)/config/common.am

CLEANFILES = fdesign.1

EXTRA_DIST = Readme fdesign.man

SUBDIRS = fd spec xpm

man_MANS = fdesign.1

fdesign.1:
	cp -p $(srcdir)/fdesign.man fdesign.1

INCLUDES = -I./xpm $(X_CFLAGS)

bin_PROGRAMS = fdesign

fdesign_LDADD = $(top_srcdir)/lib/libforms.la $(X_LIBS) $(X_PRE_LIBS) $(XPM_LIB) -lX11 $(LIBS) $(X_EXTRA_LIBS)

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
	fd_iconinfo.c \
	fd_iconinfo.h \
	fd_util.c \
	sp_browser.c \
	sp_browser.h \
	sp_button.c \
	sp_button.h \
	sp_choice.c \
	sp_choice.h \
	sp_counter.c \
	sp_counter.h \
	sp_spinner.c \
	sp_spinner.h \
	sp_dial.c \
	sp_dial.h \
	sp_freeobj.c \
	sp_freeobj.h \
	sp_menu.c \
	sp_menu.h \
	sp_pixmap.c \
	sp_pixmap.h \
	sp_positioner.c \
	sp_positioner.h \
	sp_scrollbar.c \
	sp_scrollbar.h \
	sp_slider.c \
	sp_slider.h \
	sp_twheel.c \
	sp_twheel.h \
	sp_xyplot.c \
	sp_xyplot.h \
	sp_util.c
