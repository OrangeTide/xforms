/** Header file generated with fdesign on Sat Aug 22 23:47:12 2009.**/

#ifndef FD_xyplotattrib_h_
#define FD_xyplotattrib_h_

/** Callbacks, globals and object handlers **/
extern void grid_change_cb(FL_OBJECT *, long);
extern void xscale_change_cb(FL_OBJECT *, long);
extern void yscale_change_cb(FL_OBJECT *, long);
extern void ymajorminor_change_cb(FL_OBJECT *, long);
extern void xmajorminor_change_cb(FL_OBJECT *, long);
extern void xyplot_returnsetting_change(FL_OBJECT *, long);
extern void gridstyle_change_cb(FL_OBJECT *, long);
extern void markactive_change_cb(FL_OBJECT *, long);


/**** Forms and Objects ****/
typedef struct {
	FL_FORM *xyplotattrib;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *xgrid;
	FL_OBJECT *ygrid;
	FL_OBJECT *xscale;
	FL_OBJECT *yscale;
	FL_OBJECT *ymajor;
	FL_OBJECT *yminor;
	FL_OBJECT *xmajor;
	FL_OBJECT *xminor;
	FL_OBJECT *xbase;
	FL_OBJECT *ybase;
	FL_OBJECT *how_return;
	FL_OBJECT *gridstyle;
	FL_OBJECT *mark_active;
} FD_xyplotattrib;

extern FD_xyplotattrib * create_form_xyplotattrib(void);

#endif /* FD_xyplotattrib_h_ */
