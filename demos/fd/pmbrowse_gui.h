/** Header file generated with fdesign on Thu Nov 27 12:18:19 2003.**/

#ifndef FD_ttt_h_
#define FD_ttt_h_

/** Callbacks, globals and object handlers **/
extern void done(FL_OBJECT *, long);
extern void reload(FL_OBJECT *, long);


/**** Forms and Objects ****/
typedef struct {
	FL_FORM *ttt;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *bm;
	FL_OBJECT *pm;
	FL_OBJECT *done;
	FL_OBJECT *load;
} FD_ttt;

extern FD_ttt * create_form_ttt(void);

#endif /* FD_ttt_h_ */
