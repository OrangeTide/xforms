include $(top_srcdir)/config/common.am

EXTRA_DIST = xforms.css xforms.init

# Set up which parts of the documentation are to be build (some may be
# impossible to build since the required tools are unavailable)

if BUILD_HTML_DOCS
html_docs = xforms.html
endif
if BUILD_PDF_DOCS
pdf_docs = xforms.pdf
endif

info_TEXINFOS = xforms.texi
xforms_TEXINFOS = part0_preface.texi \
				  part1.texi \
				  part1_defining_forms.texi \
				  part1_doing_interaction.texi \
				  part1_free_objects.texi \
				  part1_getting_started.texi \
				  part1_goodies.texi \
				  part1_introduction.texi \
				  part2.texi \
				  part2_command_line.texi \
				  part2_creating_forms.texi \
				  part2_generating_hardcopies.texi \
				  part2_getting_started.texi \
				  part2_introduction.texi \
				  part2_language_filters.texi \
				  part2_saving_loading.texi \
				  part3.texi \
				  part3_buttons.texi \
				  part3_choice_objects.texi \
				  part3_container_objects.texi \
				  part3_deprecated_objects.texi \
				  part3_input_objects.texi \
				  part3_introduction.texi \
				  part3_other_objects.texi \
				  part3_popups.texi \
				  part3_static_objects.texi \
				  part3_valuator_objects.texi \
				  part4.texi \
				  part4_an_example.texi \
				  part4_drawing_objects.texi \
				  part4_events.texi \
				  part4_fl_object.texi \
				  part4_global_structure.texi \
				  part4_introduction.texi \
				  part4_new_buttons.texi \
				  part4_preemptive_handler.texi \
				  part5_dirty_tricks.texi \
				  part5_overview.texi \
				  part5_resources.texi \
				  part5.texi \
				  part5_trouble_shooting.texi \
				  part5_useful_functions.texi \
				  part6.texi \
				  part6_images.texi

SUBDIRS = xforms_images

all-local: $(html_docs) $(pdf_docs)

install-data-local:
	@$(NORMAL_INSTALL)
	if [ -e xforms.pdf ]; then \
		$(install_sh) -d $(DESTDIR)$(pdfdir); \
		$(install_sh) -m 644 xforms.pdf $(DESTDIR)$(pdfdir); \
	fi
	if [ -e xforms.html ]; then \
		$(install_sh) -d $(DESTDIR)$(htmldir); \
		$(install_sh) -m 644 xforms.html/*.html $(DESTDIR)$(htmldir); \
		$(install_sh) -m 644 xforms.html/*.css $(DESTDIR)$(htmldir); \
	fi


uninstall-local:
	@$(NORMAL_UNINSTALL)
	rm -rf $(DESTDIR)$(htmldir)/xforms_images
	rm -rf $(DESTDIR)$(infodir)/xforms_images

# Rule for creating the HTML documentation from the texi files - afterwards
# all generated HTML files get copie to their own directory - the lower level
# Makefile in 'xforms_images' will also copy the required figures into it

.texi.html:
	$(TEXI2HTML) --init-file xforms.init xforms.texi
	/bin/mkdir -p xforms.html
	mv index.html xforms_*.html xforms.html
	cp -r xforms.css xforms.html
	cp -r xforms_images xforms.html

.texi.pdf:
	if [ $(TEXI2DVI_NEEDS_PDF) = "yes" ]; then \
		$(MAKE) -C xforms_images create_pdf_files; \
	fi
	texi2dvi --pdf --batch xforms.texi

.texi.dvi:
	if [ $(CONVERT) != "no" ]; then \
		$(MAKE) -C xforms_images create_eps_files;
	fi
	$(TEXI2DVI) --batch xforms.texi

clean-local:
	-rm -rf xforms.5 xforms.info* xforms.html *~
