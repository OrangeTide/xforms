/*
 *
 * This file is part of XForms.
 *
 * XForms is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1, or
 * (at your option) any later version.
 *
 * XForms is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with XForms.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef FD_SPEC_H_
#define FD_SPEC_H_

extern void set_finput_value( FL_OBJECT *,
                              double,
                              int );
extern double get_finput_value( FL_OBJECT * );

/* Slider and value slider */

extern void *get_slider_spec_fdform( void );
extern void slider_spec_restore( FL_OBJECT *,
                                 long );
extern int set_slider_attrib( FL_OBJECT * );
extern void save_slider_attrib( FILE *,
                                FL_OBJECT * );
extern void emit_slider_code( FILE *,
                              FL_OBJECT  *);

/* Scrollbar */

extern void *get_scrollbar_spec_fdform( void );
extern void scrollbar_spec_restore( FL_OBJECT *,
                                    long );
extern int set_scrollbar_attrib( FL_OBJECT * );
extern void save_scrollbar_attrib( FILE *,
                                   FL_OBJECT * );
extern void emit_scrollbar_code( FILE *,
                                 FL_OBJECT * );

/* Thumbwheel */

extern void *get_twheel_spec_fdform( void );
extern void twheel_spec_restore( FL_OBJECT *,
                                 long );
extern int set_twheel_attrib( FL_OBJECT * );
extern void save_twheel_attrib( FILE *,
                                FL_OBJECT * );
extern void emit_twheel_code( FILE *,
                              FL_OBJECT * );

/* Browser */

extern void *get_browser_spec_fdform( void );
extern void browser_spec_restore( FL_OBJECT *,
                                  long );
extern int set_browser_attrib( FL_OBJECT * );
extern void save_browser_attrib( FILE *,
                                 FL_OBJECT * );
extern void emit_browser_code( FILE *,
                               FL_OBJECT * );

/* Choice */

extern void *get_choice_spec_fdform( void );
extern void choice_spec_restore( FL_OBJECT *,
                                 long );
extern int set_choice_attrib( FL_OBJECT * );
extern void save_choice_attrib( FILE *,
                                FL_OBJECT * );
extern void emit_choice_code( FILE *,
                              FL_OBJECT * );

/* Menu */

extern void *get_menu_spec_fdform( void );
extern void menu_spec_restore( FL_OBJECT *,
                               long );
extern int set_menu_attrib( FL_OBJECT * );
extern void save_menu_attrib( FILE *,
                              FL_OBJECT * );
extern void emit_menu_code( FILE *,
                            FL_OBJECT * );
extern void emit_menu_header( FILE *,
                              FL_OBJECT * );
extern void emit_menu_global( FILE *,
                              FL_OBJECT * );
extern void emit_menu_item_callback_headers( FILE      * fn,
                                             FL_OBJECT * ob,
                                             int         code );

/* Counters */

extern void *get_counter_spec_fdform( void );
extern void counter_spec_restore( FL_OBJECT *,
                                  long );
extern int set_counter_attrib( FL_OBJECT * );
extern void save_counter_attrib( FILE *,
                                 FL_OBJECT * );
extern void emit_counter_code( FILE *,
                               FL_OBJECT * );

/* Spiners */

extern void *get_spinner_spec_fdform( void );
extern void spinner_spec_restore( FL_OBJECT *,
                                  long );
extern int set_spinner_attrib( FL_OBJECT * );
extern void save_spinner_attrib( FILE *,
                                 FL_OBJECT * );
extern void emit_spinner_code( FILE *,
                               FL_OBJECT * );

/* Dials */

extern void *get_dial_spec_fdform( void );
extern void dial_spec_restore( FL_OBJECT *,
                               long );
extern int set_dial_attrib( FL_OBJECT * );
extern void save_dial_attrib( FILE *,
                              FL_OBJECT * );
extern void emit_dial_code( FILE *,
                            FL_OBJECT * );

/* Positioner */

extern void *get_pos_spec_fdform( void );
extern void pos_spec_restore( FL_OBJECT *,
                              long );
extern int set_pos_attrib( FL_OBJECT * );
extern void save_pos_attrib( FILE *,
                             FL_OBJECT * );
extern void emit_pos_code( FILE *,
                           FL_OBJECT * );

/* Xyplot */

extern void *get_xyplot_spec_fdform( void );
extern void xyplot_spec_restore( FL_OBJECT *,
                                 long );
extern int set_xyplot_attrib( FL_OBJECT * );
extern void save_xyplot_attrib( FILE *,
                                FL_OBJECT * );
extern void emit_xyplot_code( FILE *,
                              FL_OBJECT * );

/* Free */

extern void *get_freeobj_spec_fdform( void );
extern void freeobj_spec_restore( FL_OBJECT *,
                                  long );
extern int set_freeobj_attrib( FL_OBJECT * );
extern void save_freeobj_attrib( FILE *,
                                 FL_OBJECT * );
extern void emit_freeobj_code( FILE *,
                               FL_OBJECT * );

/* All buttons */

extern void *get_button_spec_fdform( void );
extern void destroy_button_spec_fdform( void * );
extern void button_spec_restore( FL_OBJECT *,
                                 long );
extern int set_button_attrib( FL_OBJECT * );
extern void save_button_attrib( FILE *,
                                FL_OBJECT * );
extern void emit_button_code( FILE *,
                              FL_OBJECT * );
extern void emit_button_header( FILE *,
                                FL_OBJECT * );

/* Pixmap/bitmap */

extern void *get_pixmap_spec_fdform( void );
extern void destroy_pixmap_spec_fdform( void * );
extern void pixmap_spec_restore( FL_OBJECT *,
                                 long );
extern int set_pixmap_attrib( FL_OBJECT * );
extern void save_pixmap_attrib( FILE *,
                                FL_OBJECT * );
extern void emit_pixmap_code( FILE *,
                              FL_OBJECT * );
extern void emit_pixmap_header( FILE *,
                                FL_OBJECT * );

extern const char *get_scrollbar_pref_name( int );
extern int get_scrollbar_pref_value( const char * );
extern const char *get_scrollbar_pref_string( void );

extern const char *get_scale_name( int );
extern int get_scale_value( const char * );
extern const char *get_scale_string( void );

extern const char *get_grid_name( int );
extern int get_grid_value( const char * );
extern const char *get_grid_string( void );

extern const char *get_linestyle_name( int );
extern int get_linestyle_value( const char * );
extern const char *get_linestyle_string( void );

extern const char *get_pupmode_name( int );
extern int get_pupmode_value( const char * );
extern const char *get_pupmode_string( void );

extern int get_direction_value( const char * );

#endif


/*
 * Local variables:
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
