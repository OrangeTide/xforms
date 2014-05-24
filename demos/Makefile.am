include $(top_srcdir)/config/common.am

EXTRA_DIST = Readme iconvert.1 demo.menu crossbut.h \
	bm1.xbm bm2.xbm newmail.xbm nomail.xbm picture.xbm srs.xbm \
	crab45.xpm crab_tran.xpm crab.xpm porsche.xpm \
	test.xpm xconq.xpm xterm.xpm colorwheel.xpm

SUBDIRS = fd

SUFFIXES = .fd .c

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib $(X_CFLAGS)

if BUILD_GL
GL = gl glwin
else
GL =
endif

noinst_PROGRAMS = \
	arrowbutton \
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
	$(GL) \
	goodies \
	grav \
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
	newbutton \
	new_popup \
	nmenu \
	objinactive \
	objpos \
	objreturn \
	pmbrowse \
	popup \
	positioner \
	positioner_overlay \
	positionerXOR \
	preemptive \
	pup \
	pushbutton \
	pushme \
	rescale \
	scrollbar \
	secretinput \
	select \
	sld_alt \
	sld_radio \
	sldinactive \
	sldsize \
	sliderall \
	strange_button \
	strsize \
	symbols \
	thumbwheel \
	timer \
	timerprec \
	timeoutprec \
	touchbutton \
	xyplotactive \
	xyplotactivelog \
	xyplotall \
	xyplotover \
	yesno \
	yesno_cb

#	menubar

# Most of these demos link against libforms only. For them this default is
# sufficient:

LDFLAGS = ../lib/libforms.la $(X_LIBS) $(X_PRE_LIBS) $(LIBS) $(X_EXTRA_LIBS)

arrowbutton_SOURCES = arrowbutton.c
borderwidth_SOURCES = borderwidth.c
boxtype_SOURCES = boxtype.c
browserall_SOURCES = browserall.c
browserop_SOURCES = browserop.c

buttonall_SOURCES = buttonall.c
nodist_buttonall_SOURCES = fd/buttons_gui.c fd/buttons_gui.h
buttonall.$(OBJEXT): fd/buttons_gui.c

butttypes_SOURCES = butttypes.c
nodist_butttypes_SOURCES = fd/butttypes_gui.c fd/butttypes_gui.h
butttypes.$(OBJEXT): fd/butttypes_gui.c

canvas_SOURCES = canvas.c
canvas_LDADD  = ../lib/libforms.la \
	$(X_LIBS) $(X_PRE_LIBS) -lX11 $(LIBS) $(X_EXTRA_LIBS)

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
demo27_LDADD  = ../lib/libforms.la \
	$(X_LIBS) $(X_PRE_LIBS) -lX11 $(LIBS) $(X_EXTRA_LIBS)

demo33_SOURCES = demo33.c

demotest_SOURCES = demotest.c
nodist_demotest_SOURCES = fd/folder_gui.c fd/folder_gui.h
demotest.$(OBJEXT): fd/folder_gui.c

demotest2_SOURCES = demotest2.c
demotest3_SOURCES = demotest3.c

dirlist_SOURCES = dirlist.c
nodist_dirlist_SOURCES = fd/fbtest_gui.c fd/fbtest_gui.h
dirlist.$(OBJEXT): fd/fbtest_gui.c

fbrowse_SOURCES = fbrowse.c
fbrowse1_SOURCES = fbrowse1.c
fdial_SOURCES = fdial.c
flclock_SOURCES = flclock.c

folder_SOURCES = folder.c
nodist_folder_SOURCES = fd/folder_gui.c fd/folder_gui.h
folder.$(OBJEXT): fd/folder_gui.c

fonts_SOURCES = fonts.c

formbrowser_SOURCES = formbrowser.c
nodist_formbrowser_SOURCES = fd/formbrowser_gui.c fd/formbrowser_gui.h
formbrowser.$(OBJEXT): fd/formbrowser_gui.c

free1_SOURCES = free1.c

freedraw_SOURCES = freedraw.c
freedraw_LDADD  = ../lib/libforms.la \
	$(X_LIBS) $(X_PRE_LIBS) -lX11 $(LIBS) $(X_EXTRA_LIBS)

freedraw_leak_SOURCES = freedraw_leak.c
freedraw_leak_LDADD  = ../lib/libforms.la \
	$(X_LIBS) $(X_PRE_LIBS) -lX11 $(LIBS) $(X_EXTRA_LIBS)

gl_SOURCES = gl.c
gl_LDADD  = ../gl/libformsGL.la ../lib/libforms.la \
	$(X_LIBS) $(X_PRE_LIBS) -lGL -lX11 $(LIBS) $(X_EXTRA_LIBS)

glwin_SOURCES = glwin.c
glwin_LDADD  = ../gl/libformsGL.la ../lib/libforms.la \
	$(X_LIBS) $(X_PRE_LIBS) -lGL -lX11 $(LIBS) $(X_EXTRA_LIBS)

goodies_SOURCES = goodies.c

grav_SOURCES = grav.c

group_SOURCES = group.c

ibrowser_SOURCES = ibrowser.c
ibrowser.$(OBJEXT): fd/ibrowser_gui.c
ibrowser_LDADD  = ../image/libflimage.la ../lib/libforms.la \
	$(X_LIBS) $(X_PRE_LIBS) $(JPEG_LIB) $(XPM_LIB) -lX11 $(LIBS) \
	$(X_EXTRA_LIBS)

iconify_SOURCES = iconify.c

iconvert_SOURCES = iconvert.c
iconvert_LDADD  = ../image/libflimage.la ../lib/libforms.la \
	$(X_LIBS) $(X_PRE_LIBS) $(JPEG_LIB) $(XPM_LIB) -lX11 $(LIBS) \
	$(X_EXTRA_LIBS)

inputall_SOURCES = inputall.c
nodist_inputall_SOURCES = fd/inputall_gui.c fd/inputall_gui.h
inputall.$(OBJEXT): fd/inputall_gui.c

invslider_SOURCES = invslider.c

itest_SOURCES = itest.c
itest.$(OBJEXT): fd/is_gui.c
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
newbutton_SOURCES = newbutton.c crossbut.c
new_popup_SOURCES = new_popup.c
nmenu_SOURCES = nmenu.c
objinactive_SOURCES = objinactive.c
objpos_SOURCES = objpos.c
objreturn_SOURCES = objreturn.c

pmbrowse_SOURCES = pmbrowse.c
nodist_pmbrowse_SOURCES = fd/pmbrowse_gui.c fd/pmbrowse_gui.h
pmbrowse.$(OBJEXT): fd/pmbrowse_gui.c

popup_SOURCES = popup.c
positioner_SOURCES = positioner.c
positioner_overlay_SOURCES = positioner_overlay.c
positionerXOR_SOURCES = positionerXOR.c
preemptive_SOURCES = preemptive.c
pup_SOURCES = pup.c
pushbutton_SOURCES = pushbutton.c
pushme_SOURCES = pushme.c
rescale_SOURCES = rescale.c

scrollbar_SOURCES = scrollbar.c
nodist_scrollbar_SOURCES = fd/scrollbar_gui.c fd/scrollbar_gui.h
scrollbar.$(OBJEXT): fd/scrollbar_gui.c

secretinput_SOURCES = secretinput.c
select_SOURCES = select.c
sld_alt_SOURCES = sld_alt.c
sld_radio_SOURCES = sld_radio.c
sldinactive_SOURCES = sldinactive.c
sldsize_SOURCES = sldsize.c
sliderall_SOURCES = sliderall.c
strange_button_SOURCES = strange_button.c
strsize_SOURCES = strsize.c
symbols_SOURCES = symbols.c

thumbwheel_SOURCES = thumbwheel.c
nodist_thumbwheel_SOURCES = fd/twheel_gui.c fd/twheel_gui.h
thumbwheel.$(OBJEXT): fd/twheel_gui.c

timer_SOURCES = timer.c
timerprec_SOURCES = timerprec.c
timeoutprec_SOURCES = timeoutprec.c
touchbutton_SOURCES = touchbutton.c
xyplotactive_SOURCES = xyplotactive.c
xyplotactivelog_SOURCES = xyplotactivelog.c
xyplotall_SOURCES = xyplotall.c

xyplotover_SOURCES = xyplotover.c
xyplotover_LDADD  = ../image/libflimage.la ../lib/libforms.la \
	$(X_LIBS) $(X_PRE_LIBS) $(JPEG_LIB) $(XPM_LIB) -lX11 $(LIBS) \
	$(X_EXTRA_LIBS)

yesno_SOURCES = yesno.c
yesno_cb_SOURCES = yesno_cb.c

.fd.c: ../fdesign/fdesign
	../fdesign/fdesign -convert -dir $(top_srcdir)/demos/fd -I \"include/forms.h\" $<
