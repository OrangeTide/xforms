include $(top_srcdir)/config/common.am

# List of all png files

figures_png = borderwidth.png \
			  boxtypes.png \
			  buttontypes.png \
			  counters.png \
			  colchooser.png \
			  crossbutton.png \
			  dials.png \
			  fd_align.png \
	          genattr.png \
			  fdesign.png \
			  FL_BLACK.png \
			  FL_BLUE.png \
			  FL_BOTTOM_BCOL.png \
			  FL_CHARTEUSE.png \
			  FL_COL1.png \
			  FL_CYAN.png \
			  FL_DARKCYAN.png \
			  FL_DARKER_COL1.png \
			  FL_DARKGOLD.png \
			  FL_DARKORANGE.png \
			  FL_DARKPINK.png \
			  FL_DARKTOMATO.png \
			  FL_DARKVIOLET.png \
			  FL_DODGERBLUE.png \
			  FL_GREEN.png \
			  FL_INACTIVE.png \
			  FL_INDIANARED.png \
			  FL_LEFT_BCOL.png \
			  FL_LIGHTER_COL1.png \
			  FL_MAGENTA.png \
			  FL_MCOL.png \
			  FL_ORCHID.png \
			  FL_PALEGREEN.png \
			  FL_RED.png \
			  FL_RIGHT_BCOL.png \
			  FL_SLATEBLUE.png \
			  FL_SPRINGGREEN.png \
			  FL_TOMATO.png \
			  FL_TOP_BCOL.png \
			  FL_WHEAT.png \
			  FL_WHITE.png \
			  FL_YELLOW.png \
			  folders.png \
			  fontsize.png \
			  fselect.png \
			  gravity.png \
			  input.png \
			  labelframe.png \
			  pushme.png \
			  scrollbars.png \
			  slider.png \
			  slidertypes.png \
			  symbols.png \
			  thumbwheels.png \
			  yesno.png

# All png files are to be distributed

EXTRA_DIST = $(figures_png)

# Copy the png images to a subdirectory both in the directory with the
# HTML and the one with the info documentation

install-data-local:
	@$(NORMAL_INSTALL)
	$(install_sh) -d  $(DESTDIR)$(htmldir)/xforms_images
	$(install_sh) -m 644 $(figures_png) $(DESTDIR)$(htmldir)/xforms_images
	$(install_sh) -d $(DESTDIR)$(infodir)/xforms_images
	$(install_sh) -m 644 $(figures_png) $(DESTDIR)$(infodir)/xforms_images

# Creates eps files from all pngs for inclusion in info documentation
# (as used by emacs info mode)

create_eps_files:
	for i in $(figures_png); do \
		$(MAKE) $${i%png}eps; \
	done

# Creates pdf files from all pngs, needed with some older versions of texi2dvi
# to create the PDF documentation successfully

create_pdf_files:
	for i in $(figures_png); do \
		$(MAKE) $${i%png}pdf; \
	done

.png.eps:
	$(CONVERT) $< $@

.png.pdf:
	$(CONVERT) $< $@

# On uninstall remove the subdirectory with the images from the directory
# for the info documentation (the corresponding subdirectory in the HTML
# documentation directory is taken care of by the Makefile in the 'doc'
# directory one level up)

uninstall-local:
	@$(NORMAL_UNINSTALL)
	rm -rf $(infodir)/xforms_images

# Get of junk

clean-local:
	-rm -f *~ *.eps *.pdf
