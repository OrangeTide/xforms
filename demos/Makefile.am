include $(top_srcdir)/config/common.am

EXTRA_DIST = 01Readme iconvert.1 demo.menu test.ps \
	bm1.xbm bm2.xbm newmail.xbm nomail.xbm picture.xbm srs.xbm \
	crab45.xpm crab_tran.xpm crab.xpm porsche.xpm \
	test.xpm xconq.xpm xterm.xpm \
	demotest_fd.fd  pmbrowse.fd

SUBDIRS = fd

# forms.h is a generated file
INCLUDES = -I../lib -I$(top_srcdir)/lib -I$(top_srcdir)/image

noinst_PROGRAMS = arrowbutton \
	borderwidth \
	boxtype \
	browserall \
	browserop \
	buttonall \
	butttypes \
	canvas \
	chartall \
	chartstrip \
	choice \
	colbrowser \
	colsel \
	colsel1 \
	counter \
	cursor \
	demo \
	demo05 \
	demo06 \
	demo27 \
	demo33 \
	demotest \
	demotest2 \
	demotest3 \
	dirlist \
	fbrowse \
	fbrowse1 \
	fdial \
	flclock \
	folder \
	fonts \
	formbrowser \
	free1 \
	freedraw \
	freedraw_leak \
	goodies \
	group \
	ibrowser \
	iconify \
	iconvert \
	inputall \
	invslider \
	itest \
	lalign \
	ldial \
	ll \
	longlabel \
	menu \
	minput \
	minput2 \
	multilabel \
	ndial \
	objinactive \
	objpos \
	objreturn \
	pmbrowse \
	popup \
	positioner \
	positionerXOR \
	preemptive \
	pup \
	pushbutton \
	pushme \
	rescale \
	scrollbar \
	secretinput \
	sld_radio \
	sldinactive \
	sldsize \
	sliderall \
	strsize \
	symbols \
	thumbwheel \
	timer \
	timerprec \
	touchbutton \
	xyplotactive \
	xyplotall \
	xyplotover \
	yesno \
	yesno_cb \
	wwwl

#	menubar

# Most of these demos link against libforms only. For them this default is
# sufficient:
AM_LDFLAGS = ../lib/libforms.la $(X_LIBS) $(X_PRE_LIBS) $(XPM_LIB) -lX11 $(LIBS) $(X_EXTRA_LIBS)

arrowbutton_SOURCES = arrowbutton.c
borderwidth_SOURCES = borderwidth.c
boxtype_SOURCES = boxtype.c
browserall_SOURCES = browserall.c
browserop_SOURCES = browserop.c
buttonall_SOURCES = buttonall.c
butttypes_SOURCES = butttypes.c
canvas_SOURCES = canvas.c
chartall_SOURCES = chartall.c
chartstrip_SOURCES = chartstrip.c
choice_SOURCES = choice.c
colbrowser_SOURCES = colbrowser.c
colsel_SOURCES = colsel.c
colsel1_SOURCES = colsel1.c
counter_SOURCES = counter.c
cursor_SOURCES = cursor.c
demo_SOURCES = demo.c
demo05_SOURCES = demo05.c
demo06_SOURCES = demo06.c
demo27_SOURCES = demo27.c
demo33_SOURCES = demo33.c
demotest_SOURCES = demotest.c demotest_fd.c demotest_fd.h
demotest2_SOURCES = demotest2.c
demotest3_SOURCES = demotest3.c
dirlist_SOURCES = dirlist.c
fbrowse_SOURCES = fbrowse.c
fbrowse1_SOURCES = fbrowse1.c
fdial_SOURCES = fdial.c
flclock_SOURCES = flclock.c
folder_SOURCES = folder.c
fonts_SOURCES = fonts.c
formbrowser_SOURCES = formbrowser.c
free1_SOURCES = free1.c
freedraw_SOURCES = freedraw.c
freedraw_leak_SOURCES = freedraw_leak.c
goodies_SOURCES = goodies.c
group_SOURCES = group.c

ibrowser_SOURCES = ibrowser.c
ibrowser_LDADD  = ../image/libflimage.la ../lib/libforms.la \
	$(X_LIBS) $(X_PRE_LIBS) $(JPEG_LIB) $(XPM_LIB) -lX11 $(LIBS) \
	$(X_EXTRA_LIBS)

iconify_SOURCES = iconify.c

iconvert_SOURCES = iconvert.c
iconvert_LDADD  = ../image/libflimage.la ../lib/libforms.la \
	$(X_LIBS) $(X_PRE_LIBS) $(JPEG_LIB) $(XPM_LIB) -lX11 $(LIBS) \
	$(X_EXTRA_LIBS)

inputall_SOURCES = inputall.c
invslider_SOURCES = invslider.c

itest_SOURCES = itest.c
itest_LDADD  = ../image/libflimage.la ../lib/libforms.la \
	$(X_LIBS) $(X_PRE_LIBS) $(JPEG_LIB) $(XPM_LIB) -lX11 $(LIBS) \
	$(X_EXTRA_LIBS)

lalign_SOURCES = lalign.c
ldial_SOURCES = ldial.c

ll_SOURCES = ll.c
ll_LDADD  = ../image/libflimage.la ../lib/libforms.la \
	$(X_LIBS) $(X_PRE_LIBS) $(JPEG_LIB) $(XPM_LIB) -lX11 $(LIBS) \
	$(X_EXTRA_LIBS)

longlabel_SOURCES = longlabel.c
menu_SOURCES = menu.c

#menubar_SOURCES = menubar.c

minput_SOURCES = minput.c
minput2_SOURCES = minput2.c
multilabel_SOURCES = multilabel.c
ndial_SOURCES = ndial.c
objinactive_SOURCES = objinactive.c
objpos_SOURCES = objpos.c
objreturn_SOURCES = objreturn.c
pmbrowse_SOURCES = pmbrowse.c
popup_SOURCES = popup.c
positioner_SOURCES = positioner.c
positionerXOR_SOURCES = positionerXOR.c
preemptive_SOURCES = preemptive.c
pup_SOURCES = pup.c
pushbutton_SOURCES = pushbutton.c
pushme_SOURCES = pushme.c
rescale_SOURCES = rescale.c
scrollbar_SOURCES = scrollbar.c
secretinput_SOURCES = secretinput.c
sld_radio_SOURCES = sld_radio.c
sldinactive_SOURCES = sldinactive.c
sldsize_SOURCES = sldsize.c
sliderall_SOURCES = sliderall.c
strsize_SOURCES = strsize.c
symbols_SOURCES = symbols.c
thumbwheel_SOURCES = thumbwheel.c
timer_SOURCES = timer.c
timerprec_SOURCES = timerprec.c
touchbutton_SOURCES = touchbutton.c
xyplotactive_SOURCES = xyplotactive.c
xyplotall_SOURCES = xyplotall.c

xyplotover_SOURCES = xyplotover.c
xyplotover_LDADD  = ../image/libflimage.la ../lib/libforms.la \
	$(X_LIBS) $(X_PRE_LIBS) $(JPEG_LIB) $(XPM_LIB) -lX11 $(LIBS) \
	$(X_EXTRA_LIBS)

yesno_SOURCES = yesno.c
yesno_cb_SOURCES = yesno_cb.c
wwwl_SOURCES = wwwl.c
