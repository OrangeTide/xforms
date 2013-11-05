include $(top_srcdir)/config/common.am

if USE_BWC_BS_HACK
BWC = -DUSE_BWC_BS_HACK
else
BWC =
endif

CLEANFILES = forms.h xforms.5

EXTRA_DIST = dirent_vms.h \
	         vms_readdir.c

SUBDIRS = bitmaps fd include private

INCLUDES = -DMAKING_FORMS $(X_CFLAGS) $(BWC)

lib_LTLIBRARIES = libforms.la

libforms_la_LDFLAGS = -no-undefined -Wl,--no-undefined -version-info @SO_VERSION@

libforms_la_LIBADD =  $(X_LIBS) $(XPM_LIB) -lX11

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
	goodies.c \
	goodie_sinput.c \
	goodie_yesno.c \
	handling.c \
	input.c \
	interpol.c \
	keyboard.c \
	labelbut.c \
	lframe.c \
	lightbut.c \
	listdir.c \
	local.h \
	menu.c \
	nmenu.c \
	objects.c \
	oneliner.c \
	pixmap.c \
	popup.c \
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
	select.c \
	signal.c \
	sldraw.c \
	slider.c \
	space.c \
	spline.c \
	spinner.c \
	strdup.c \
	symbols.c \
	sysdep.c \
	tabfolder.c \
	tbox.c \
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
	win.c \
	xdraw.c \
	xpopup.c \
	xsupport.c \
	xtext.c \
	xyplot.c
