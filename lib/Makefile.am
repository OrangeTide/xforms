include $(top_srcdir)/config/common.am

CLEANFILES = forms.h xforms.5

EXTRA_DIST = xforms.man \
	dirent_vms.h \
	vms_readdir.c

SUBDIRS = bitmaps fd include private

# The Autobook recommends using "man5_MANS = xforms.man" which certainly
# simplifies things but appears not to work with automake 1.5.
man_MANS = xforms.5

xforms.5:
	cp -p $(srcdir)/xforms.man xforms.5

INCLUDES = -DMAKING_FORMS $(X_CFLAGS)

lib_LTLIBRARIES = libforms.la

libforms_la_LDFLAGS = -version-info @SO_VERSION@

libforms_la_SOURCES = \
	align.c \
	appwin.c \
	asyn_io.c \
	bitmap.c \
	box.c \
	browser.c \
	button.c \
	canvas.c \
	chart.c \
	checkbut.c \
	child.c \
	choice.c \
	clipboard.c \
	clock.c \
	cmd_br.c \
	colsel.c \
	combo.c \
	counter.c \
	cursor.c \
	dial.c \
	display.c \
	dmalloc.h \
	errmsg.c \
	events.c \
	extern.h \
	flcolor.c \
	fldraw.c \
	flinternal.h \
	flsnprintf.c \
	flresource.c \
	flvisual.c \
	fonts.c \
	formbrowser.c \
	forms.c \
	frame.c \
	free.c \
	fselect.c \
	global.h \
	goodie_alert.c \
	goodie_choice.c \
	goodie_input.c \
	goodie_msg.c \
	goodie_salert.c \
	goodies.c \
	goodie_sinput.c \
	goodie_yesno.c \
	input.c \
	interpol.c \
	keyboard.c \
	labelbut.c \
	lframe.c \
	lightbut.c \
	listdir.c \
	local.h \
	menu.c \
	objects.c \
	oneliner.c \
	pixmap.c \
	positioner.c \
	read2lsbf.c \
	read2msbf.c \
	read4lsb.c \
	read4msb.c \
	readint.c \
	round3d.c \
	roundbut.c \
	scrollbar.c \
	scrollbut.c \
	signal.c \
	simu.c \
	sldraw.c \
	slider.c \
	space.c \
	spline.c \
	strdup.c \
	symbols.c \
	sysdep.c \
	tabfolder.c \
	textbox.c \
	text.c \
	thumbwheel.c \
	timeout.c \
	timer.c \
	tooltip.c \
	ulib.h \
	util.c \
	valuator.c \
	version.c \
	vn_pair.c \
	vstrcat.c \
	win.c \
	xdraw.c \
	xpopup.c \
	xsupport.c \
	xtext.c \
	xyplot.c
