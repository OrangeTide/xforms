include $(top_srcdir)/config/common.am

nodist_include_HEADERS = forms.h
CLEANFILES = forms.h AAA.h stamp-forms

noinst_HEADERS = \
	Basic.h \
	XBasic.h \
	popup.h \
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
	nmenu.h \
	positioner.h \
	scrollbar.h \
	select.h \
	slider.h \
	spinner.h \
	tabfolder.h \
	text.h \
	thumbwheel.h \
	timer.h \
	xpopup.h \
	xyplot.h \
	zzz.h

forms.h: stamp-forms
	@:

stamp-forms: AAA.h $(noinst_HEADERS)
	rm -f tmpforms
	sed "1,/crop here for forms/d" AAA.h > tmpforms
	for i in $(noinst_HEADERS); do \
		sed "1,/crop here for forms/d" ${srcdir}/`basename $$i` >> tmpforms; \
	done
	if cmp -s tmpforms forms.h ; then \
		rm -f tmpforms ;\
	else \
		rm -f forms.h ;\
		mv tmpforms forms.h ;\
	fi
	echo timestamp > $@
