/* Header file generated by fdesign on Thu Dec 12 21:27:07 2013 */

#ifndef FD_buttonattrib_h_
#define FD_buttonattrib_h_

#include  "../lib/include/forms.h" 

/* Callbacks, globals and object handlers */

void initialval_change( FL_OBJECT *, long );
void iconbutton_filename_change( FL_OBJECT *, long );
void pixalign_change( FL_OBJECT *, long );
void showfocus_change( FL_OBJECT *, long );
void lookfor_pixmapfile_cb( FL_OBJECT *, long );
void usedata_change( FL_OBJECT *, long );
void fullpath_cb( FL_OBJECT *, long );
void focusiconbutton_filename_change( FL_OBJECT *, long );
void react_to_button( FL_OBJECT *, long );


/* Forms and Objects */

typedef struct {
    FL_FORM   * buttonattrib;
    void      * vdata;
    char      * cdata;
    long        ldata;
    FL_OBJECT * background;
    FL_OBJECT * initialval;
    FL_OBJECT * filename;
    FL_OBJECT * pixalign;
    FL_OBJECT * showfocus;
    FL_OBJECT * browse;
    FL_OBJECT * use_data;
    FL_OBJECT * fullpath;
    FL_OBJECT * focus_filename;
    FL_OBJECT * browse2;
    FL_OBJECT * mbuttons_label;
    FL_OBJECT * react_left;
    FL_OBJECT * react_middle;
    FL_OBJECT * react_right;
    FL_OBJECT * react_up;
    FL_OBJECT * react_down;
} FD_buttonattrib;

FD_buttonattrib * create_form_buttonattrib( void );

#endif /* FD_buttonattrib_h_ */
