include $(top_srcdir)/config/common.am

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

.texi.html:
	texi2html --init-file xforms.init xforms.texi
	/bin/mkdir -p xforms.html
	mv index.html xforms_*.html xforms.html
	cp -r images xforms.html

clean-local:
	-rm -r xforms.info* xforms.pdf xforms.html *~

install-html:
	/bin/mkdir -p $(htmldir)/xforms.html
	/bin/mkdir -p $(htmldir)/xforms.html/images
	/usr/bin/install -c -m 644 xforms.html/*.html $(htmldir)/xforms.html
	/usr/bin/install -c -m 644 images/*.png $(htmldir)/xforms.html/images
