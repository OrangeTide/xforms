include $(top_srcdir)/config/common.am

include_HEADERS = forms.h
CLEANFILES = forms.h
BUILT_SOURCES = forms.h

noinst_HEADERS = \
	AAA.h \
	Basic.h \
	XBasic.h \
	bitmap.h \
	box.h \
	browser.h \
	button.h \
	canvas.h \
	chart.h \
	choice.h \
	clipbd.h \
	clock.h \
	counter.h \
	cursor.h \
	dial.h \
	filesys.h \
	flps.h \
	formbrowser.h \
	frame.h \
	free.h \
	goodies.h \
	input.h \
	menu.h \
	menubar.h \
	popup.h \
	positioner.h \
	scrollbar.h \
	slider.h \
	tabfolder.h \
	text.h \
	thumbwheel.h \
	timer.h \
	xyplot.h \
	zzz.h

forms.h: $(noinst_HEADERS)
	@rm -f $@
	@for i in $(noinst_HEADERS); do \
		sed "1,/crop here for forms/d" ${srcdir}/$$i >> $@; \
	done
