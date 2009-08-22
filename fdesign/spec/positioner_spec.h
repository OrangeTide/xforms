/** Header file generated with fdesign on Sat Aug 22 23:45:56 2009.**/

#ifndef FD_posattrib_h_
#define FD_posattrib_h_

/** Callbacks, globals and object handlers **/
extern void pos_returnsetting_change(FL_OBJECT *, long);
extern void pos_xminmax_change(FL_OBJECT *, long);
extern void pos_initialxvalue_change(FL_OBJECT *, long);
extern void pos_xstepchange_cb(FL_OBJECT *, long);
extern void pos_yminmax_change(FL_OBJECT *, long);
extern void pos_initialyvalue_change(FL_OBJECT *, long);
extern void pos_ystepchange_cb(FL_OBJECT *, long);


/**** Forms and Objects ****/
typedef struct {
	FL_FORM *posattrib;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *background;
	FL_OBJECT *returnsetting;
	FL_OBJECT *xminval;
	FL_OBJECT *xmaxval;
	FL_OBJECT *initialxval;
	FL_OBJECT *xstep;
	FL_OBJECT *yminval;
	FL_OBJECT *ymaxval;
	FL_OBJECT *initialyval;
	FL_OBJECT *ystep;
} FD_posattrib;

extern FD_posattrib * create_form_posattrib(void);

#endif /* FD_posattrib_h_ */
