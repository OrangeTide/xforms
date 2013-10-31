include $(top_srcdir)/config/common.am

EXTRA_DIST = xforms.man xforms.css xforms.init

# Tool for converting texi files to HTML

TEXI2HTML = texi2html


docs = xforms.info xforms.pdf xforms.html

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


man_MANS = xforms.5

xforms.5:
	cp -p ./xforms.man xforms.5

SUBDIRS = images

all-local: $(docs)
doc_DATA = xforms.pdf xforms.man

install-data-local:
	@$(NORMAL_INSTALL)
	/bin/mkdir -p $(htmldir)
	/usr/bin/install -c -m 644 xforms.html/*.html xforms.html/xforms.css $(htmldir)


uninstall-local:
	@$(NORMAL_UNINSTALL)
	rm -rf $(htmldir)
	rm -rf $(infodir)/images

# Rule for creating the HTML documentation from the texi files - afterwards
# all generated files and the css file to its own directory - the lower level
# Makefile in 'images' will also copy the required figures into it

.texi.html:
	$(TEXI2HTML) --init-file xforms.init xforms.texi
	/bin/mkdir -p xforms.html
	mv index.html xforms_*.html xforms.html
	cp -p xforms.css xforms.html
	cp -r images xforms.html

clean-local:
	-rm -rf xforms.info* xforms.pdf xforms.html *.html xforms.5 *~
