/*
 *	This file is part of the XForms library package.
 *
 *  XForms is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU Lesser General Public License as
 *  published by the Free Software Foundation; either version 2.1, or
 *  (at your option) any later version.
 *
 *  XForms is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with XForms.  If not, see <http://www.gnu.org/licenses/>.
 */

/********************** crop here for forms.h **********************/

/**
 * \file Basic.h
 *
 *	Basic definitions and limits.
 *	Window system independent prototypes
 *
 *	Modify with care
 *
 */

#ifndef FL_BASIC_H
#define FL_BASIC_H

#include <math.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <X11/Xresource.h>

#if defined __GNUC__
#define FL_UNUSED_ARG __attribute__ ((unused))
#else
#define FL_UNUSED_ARG
#endif

/* some general constants */

enum {
	FL_ON		   = 1,
	FL_OK		   = 1,
	FL_VALID	   = 1,
	FL_PREEMPT	   = 1,
	FL_AUTO		   = 2,
	FL_WHEN_NEEDED = FL_AUTO,
	FL_OFF		   = 0,
	FL_CANCEL	   = 0,
	FL_INVALID	   = 0,

	/* FL_NONE		  = 0,	defined elsewhere */

	/* WM_DELETE_WINDOW callback return */

	FL_IGNORE	   = -1,
	FL_CLOSE	   = -2,

	/* misc. return types */

	FL_ARGUMENT	   = -3,
	FL_ALLOC	   = -4,
	FL_BAD_OBJECT  = -5
};

/* Max	directory length  */

#ifndef FL_PATH_MAX
#ifndef PATH_MAX
#define FL_PATH_MAX		  1024
#else
#define FL_PATH_MAX		  PATH_MAX
#endif
#endif /* !def FL_PATH_MAX */

/* The screen coordinate unit, FL_Coord, must be of signed type.
 * If FL_Coord is float, FL_CoordIsFloat must be defined to be 1 so that
 * round-off error can be checked. **TODO Float not tested */

typedef int FL_Coord;

#define FL_COORD		 FL_Coord
#define FL_CoordIsFloat  0	/* make it 1 if FL_Coord is of type float */

typedef unsigned long FL_COLOR;

/* Coordinates can be in pixels, milli-meters or points (1/72inch) */

typedef enum {
	FL_COORD_PIXEL,			/* default, Pixel			*/
	FL_COORD_MM,			/* milli-meter				*/
	FL_COORD_POINT,			/* point					*/
	FL_COORD_centiMM,		/* one hundredth of a mm	*/
	FL_COORD_centiPOINT		/* one hundredth of a point */
} FL_COORD_UNIT;

/* All object classes. */

typedef enum {
	FL_INVALID_CLASS,	   /*  0 */
	FL_BUTTON,			   /*  1 */
	FL_LIGHTBUTTON,		   /*  2 */
	FL_ROUNDBUTTON,		   /*  3 */
	FL_ROUND3DBUTTON,	   /*  4 */
	FL_CHECKBUTTON,		   /*  5 */
	FL_BITMAPBUTTON,	   /*  6 */
	FL_PIXMAPBUTTON,	   /*  7 */
	FL_BITMAP,			   /*  8 */
	FL_PIXMAP,			   /*  9 */
	FL_BOX,				   /* 10 */
	FL_TEXT,			   /* 11 */
	FL_MENU,			   /* 12 */
	FL_CHART,			   /* 13 */
	FL_CHOICE,			   /* 14 */
	FL_COUNTER,			   /* 15 */
	FL_SLIDER,			   /* 16 */
	FL_VALSLIDER,		   /* 17 */
	FL_INPUT,			   /* 18 */
	FL_BROWSER,			   /* 19 */
	FL_DIAL,			   /* 20 */
	FL_TIMER,			   /* 21 */
	FL_CLOCK,			   /* 22 */
	FL_POSITIONER,		   /* 23 */
	FL_FREE,			   /* 24 */
	FL_XYPLOT,			   /* 25 */
	FL_FRAME,			   /* 26 */
	FL_LABELFRAME,		   /* 27 */
	FL_CANVAS,			   /* 28 */
	FL_GLCANVAS,		   /* 29 */
	FL_TABFOLDER,		   /* 30 */
	FL_SCROLLBAR,		   /* 31 */
	FL_SCROLLBUTTON,	   /* 32 */
	FL_MENUBAR,			   /* 33 */
	FL_TEXTBOX,			   /* 34, for internal use only	*/
	FL_LABELBUTTON,		   /* 35 */
	FL_COMBOBOX,		   /* 36 */
	FL_IMAGECANVAS,		   /* 37 */
	FL_THUMBWHEEL,		   /* 38 */
	FL_COLORWHEEL,		   /* 39 */
	FL_FORMBROWSER,		   /* 40 */
	FL_SELECT,             /* 41 */
	FL_NMENU,              /* 42 */
	FL_CLASS_END		   /* sentinel */
} FL_CLASS;

#define FL_BEGIN_GROUP		  10000
#define FL_END_GROUP		  20000

#define FL_USER_CLASS_START	  1001	/* min. user class	value */
#define FL_USER_CLASS_END	  9999	/* max. user class	value */

/* Maximum border width (in pixel) */

#define FL_MAX_BW          10

/* how to display a form onto screen */

typedef enum {
	FL_PLACE_FREE		=	0,		/* size remain resizable	  */
	FL_PLACE_MOUSE		=	1,		/* mouse centered on form	  */
	FL_PLACE_CENTER		=	2,		/* center of the screen		  */
	FL_PLACE_POSITION	=	4,		/* specific size			  */
	FL_PLACE_SIZE		=	8,		/* specific size			  */
	FL_PLACE_GEOMETRY	=  16,		/* specific position		  */
	FL_PLACE_ASPECT		=  32,		/* keep aspect ratio		  */
	FL_PLACE_FULLSCREEN =  64,		/* scale to fit screen		  */
	FL_PLACE_HOTSPOT	= 128,		/* so mouse fall on (x,y)	  */
	FL_PLACE_ICONIC		= 256,

	/* modifiers */

	FL_FREE_SIZE		= ( 1 << 14 ),
	FL_FIX_SIZE			= ( 1 << 15 )
} FL_PLACE;

#define FL_PLACE_FREE_CENTER ( FL_PLACE_CENTER | FL_FREE_SIZE )
#define FL_PLACE_CENTERFREE	 ( FL_PLACE_CENTER | FL_FREE_SIZE )

/* Window manager decoration request and forms attributes */

enum {
	FL_FULLBORDER = 1,		/* normal								   */
	FL_TRANSIENT,			/* set TRANSIENT_FOR property			   */
	FL_NOBORDER,			/* use override_redirect to supress decor. */
	FL_MODAL      = 1 << 8  /* not implemented yet					   */
};

/* All box types */

typedef enum {
	FL_NO_BOX,                /*  0 */
	FL_UP_BOX,                /*  1 */
	FL_DOWN_BOX,			  /*  2 */
	FL_BORDER_BOX,			  /*  3 */
	FL_SHADOW_BOX,			  /*  4 */
	FL_FRAME_BOX,			  /*  5 */
	FL_ROUNDED_BOX,			  /*  6 */
	FL_EMBOSSED_BOX,		  /*  7 */
	FL_FLAT_BOX,			  /*  8 */
	FL_RFLAT_BOX,			  /*  9 */
	FL_RSHADOW_BOX,			  /* 10 */
	FL_OVAL_BOX,			  /* 11 */
	FL_ROUNDED3D_UPBOX,		  /* 12 */
	FL_ROUNDED3D_DOWNBOX,	  /* 13 */
	FL_OVAL3D_UPBOX,		  /* 14 */
	FL_OVAL3D_DOWNBOX,		  /* 15 */
	FL_OVAL3D_FRAMEBOX,		  /* 16 */
	FL_OVAL3D_EMBOSSEDBOX,    /* 17 */

	/* for internal use only */

	FL_TOPTAB_UPBOX,
	FL_SELECTED_TOPTAB_UPBOX,
	FL_BOTTOMTAB_UPBOX,
	FL_SELECTED_BOTTOMTAB_UPBOX,
	FL_OSHADOW_BOX,					/* not used */

	FL_MAX_BOX_STYLES				/* sentinel */
} FL_BOX_TYPE;

#define FL_IS_UPBOX( t )	(	 ( t ) == FL_UP_BOX			  \
							  || ( t ) == FL_OVAL3D_UPBOX	  \
							  || ( t ) == FL_ROUNDED3D_UPBOX)

#define FL_IS_DOWNBOX( t )	(	 ( t ) == FL_DOWN_BOX		   \
							  || ( t ) == FL_OVAL3D_DOWNBOX	   \
							  || ( t ) == FL_ROUNDED3D_DOWNBOX)

#define FL_TO_DOWNBOX( t )	( ( t ) == FL_UP_BOX ? FL_DOWN_BOX:			\
							  ( ( t ) == FL_ROUNDED3D_UPBOX ?			\
								FL_ROUNDED3D_DOWNBOX:					\
								( ( t ) == FL_OVAL3D_UPBOX ?			\
								  FL_OVAL3D_DOWNBOX : ( t ) ) ) )

/* How to place text relative to a box */

typedef enum {
	FL_ALIGN_CENTER,
	FL_ALIGN_TOP		  = 1,
	FL_ALIGN_BOTTOM		  = 2,
	FL_ALIGN_LEFT		  = 4,
	FL_ALIGN_RIGHT		  = 8,
	FL_ALIGN_LEFT_TOP	  = ( FL_ALIGN_TOP    | FL_ALIGN_LEFT  ),
	FL_ALIGN_RIGHT_TOP	  = ( FL_ALIGN_TOP    | FL_ALIGN_RIGHT ),
	FL_ALIGN_LEFT_BOTTOM  = ( FL_ALIGN_BOTTOM | FL_ALIGN_LEFT  ),
	FL_ALIGN_RIGHT_BOTTOM = ( FL_ALIGN_BOTTOM | FL_ALIGN_RIGHT ),
	FL_ALIGN_INSIDE		  = ( 1 << 13 ),
	FL_ALIGN_VERT		  = ( 1 << 14 ),	/* not functional yet  */

	/* the rest is for backward compatibility only, don't use! */

	FL_ALIGN_TOP_LEFT	  = FL_ALIGN_LEFT_TOP,
	FL_ALIGN_TOP_RIGHT	  = FL_ALIGN_RIGHT_TOP,
	FL_ALIGN_BOTTOM_LEFT  = FL_ALIGN_LEFT_BOTTOM,
	FL_ALIGN_BOTTOM_RIGHT = FL_ALIGN_RIGHT_BOTTOM
} FL_ALIGN;

/* Mouse buttons. Don't have to be consecutive */

enum {
	FL_MBUTTON1 = 1,
	FL_MBUTTON2,
	FL_MBUTTON3,
	FL_MBUTTON4,
	FL_MBUTTON5
};

#define FL_LEFT_MOUSE		 FL_MBUTTON1
#define FL_MIDDLE_MOUSE		 FL_MBUTTON2
#define FL_RIGHT_MOUSE		 FL_MBUTTON3
#define FL_SCROLLUP_MOUSE	 FL_MBUTTON4
#define FL_SCROLLDOWN_MOUSE	 FL_MBUTTON5

#define FL_LEFTMOUSE		 FL_LEFT_MOUSE
#define FL_MIDDLEMOUSE		 FL_MIDDLE_MOUSE
#define FL_RIGHTMOUSE		 FL_RIGHT_MOUSE
#define FL_SCROLLUPMOUSE	 FL_SCROLLUP_MOUSE
#define FL_SCROLLDOWNMOUSE	 FL_SCROLLDOWN_MOUSE

/* control when to return input, slider and dial object. */

enum {
	FL_RETURN_END_CHANGED = 0,
	FL_RETURN_CHANGED	  = 1,
	FL_RETURN_END		  = 2,
	FL_RETURN_ALWAYS	  = 3,
	FL_RETURN_DBLCLICK	  = 4
};

/*	Some special color indices for FL private colormap. It does not matter
 *	what the value of each enum is, but it must start from 0 and be
 *	consecutive. */

typedef enum {
	FL_BLACK,
	FL_RED,
	FL_GREEN,
	FL_YELLOW,
	FL_BLUE,
	FL_MAGENTA,
	FL_CYAN,
	FL_WHITE,

	FL_TOMATO,
	FL_INDIANRED,
	FL_SLATEBLUE,

	FL_COL1,
	FL_RIGHT_BCOL,
	FL_BOTTOM_BCOL,
	FL_TOP_BCOL,
	FL_LEFT_BCOL,
	FL_MCOL,

	FL_INACTIVE,
	FL_PALEGREEN,
	FL_DARKGOLD,

	FL_ORCHID,
	FL_DARKCYAN,
	FL_DARKTOMATO,
	FL_WHEAT,
	FL_DARKORANGE,
	FL_DEEPPINK,
	FL_CHARTREUSE, 
	FL_DARKVIOLET,
	FL_SPRINGGREEN,
	FL_DODGERBLUE,

	/* these two colors can be used to shade COL1 */

	FL_LIGHTER_COL1,		/* a little lighter than COL1 */
	FL_DARKER_COL1,			/* a little darker	than COL1 */

	FL_FREE_COL1 = 256,
	FL_FREE_COL2,
	FL_FREE_COL3,
	FL_FREE_COL4,
	FL_FREE_COL5,
	FL_FREE_COL6,
	FL_FREE_COL7,
	FL_FREE_COL8,
	FL_FREE_COL9,
	FL_FREE_COL10,
	FL_FREE_COL11,
	FL_FREE_COL12,
	FL_FREE_COL13,
	FL_FREE_COL14,
	FL_FREE_COL15,
	FL_FREE_COL16,
	FL_NOCOLOR = INT_MAX
} FL_PD_COL;

#define FL_BUILT_IN_COLS  ( FL_DARKER_COL1 + 1 )
#define FL_INACTIVE_COL	  FL_INACTIVE
#define FL_DOGERBLUE	  FL_DODGERBLUE

/* Some aliases for the color. This is actually backwards ... */

#define FL_GRAY16			FL_RIGHT_BCOL
#define FL_GRAY35			FL_BOTTOM_BCOL
#define FL_GRAY80			FL_TOP_BCOL
#define FL_GRAY90			FL_LEFT_BCOL
#define FL_GRAY63			FL_COL1
#define FL_GRAY75			FL_MCOL
#define	 FL_LCOL			FL_BLACK
#define	 FL_NoColor			FL_NOCOLOR

/* Events that a form reacts to.  */

typedef enum {
	FL_NOEVENT,				   /*  0 */
	FL_DRAW,				   /*  1 */
	FL_PUSH,				   /*  2 */
	FL_RELEASE,				   /*  3 */
	FL_ENTER,				   /*  4 */
	FL_LEAVE,				   /*  5 */
	FL_MOTION,				   /*  6 */
	FL_FOCUS,				   /*  7 */
	FL_UNFOCUS,				   /*  8 */
	FL_KEYBOARD,			   /*  9 */
	FL_KEYPRESS = FL_KEYBOARD, /*  9 */
	FL_UPDATE,				   /* 10 for objects that need to update something
								     from time to time */
	FL_MOUSE = FL_UPDATE,	   /* 10 */
	FL_STEP,				   /* 11 */
	FL_SHORTCUT,			   /* 12 */
	FL_FREEMEM,				   /* 13 */
	FL_OTHER,				   /* 14 property, selection etc */
	FL_DRAWLABEL,			   /* 15 */
	FL_DBLCLICK,			   /* 16 double click			   */
	FL_TRPLCLICK,			   /* 17 triple click			   */
	FL_ATTRIB,				   /* 18 attribute change		   */
	FL_KEYRELEASE,			   /* 19 */
	FL_PS,					   /* 20 dump a form into EPS	   */
	FL_MOVEORIGIN,			   /* 21 dragging the form across the screen
									 changes its absolute x,y coords. Objects
									 that themselves contain forms should
									 ensure that they are up to date. */
	FL_RESIZED			   	   /* 22 the object has been resized by scale_form
									 Tell it that this has happened so that
									 it can resize any FL_FORMs that it
									 contains. */
} FL_EVENTS;

#define FL_MOVE	  FL_MOTION	   /* for compatibility */

/* Resize policies */

typedef enum {
	FL_RESIZE_NONE,
	FL_RESIZE_X,
	FL_RESIZE_Y,
	FL_RESIZE_ALL = ( FL_RESIZE_X | FL_RESIZE_Y )
} FL_RESIZE_T;

/* Keyboard focus control */

typedef enum {
	FL_KEY_NORMAL  = 1,		/* normal keys(0-255) - tab +left/right */
	FL_KEY_TAB	   = 2,		/* normal keys + 4 direction cursor		*/
	FL_KEY_SPECIAL = 4,		/* only needs special keys(>255)		*/
	FL_KEY_ALL	   = 7		/* all keys								*/
} FL_KEY;

#define FL_ALT_MASK		 ( 1L << 25 )	/* alt + Key --> FL_ALT_MASK + key */
#define FL_CONTROL_MASK	 ( 1L << 26 )
#define FL_SHIFT_MASK	 ( 1L << 27 )
#define FL_ALT_VAL		 FL_ALT_MASK    /* Don' use! */

#define MAX_SHORTCUTS    8

/* Pop-up menu item attributes. NOTE if more than 8, need to change
 * choice and menu class where mode is kept by a single byte */

enum {
	FL_PUP_NONE,
	FL_PUP_GREY	 = 1,
	FL_PUP_BOX	 = 2,
	FL_PUP_CHECK = 4,
	FL_PUP_RADIO = 8
};

#define FL_PUP_GRAY		 FL_PUP_GREY
#define FL_PUP_TOGGLE	 FL_PUP_BOX		   /* not used anymore */
#define FL_PUP_INACTIVE	 FL_PUP_GREY

/* Popup and menu entries */

typedef int ( * FL_PUP_CB )( int );		   /* callback prototype  */

typedef struct {
	const char * text;			/* label of a popup/menu item	 */
	FL_PUP_CB	 callback;		/* the callback function		 */
	const char * shortcut;		/* hotkeys						 */
	int			 mode;			/* FL_PUP_GRAY, FL_PUP_CHECK etc */
} FL_PUP_ENTRY;

#define FL_MENU_ENTRY  FL_PUP_ENTRY

/*******************************************************************
 * FONTS
 ******************************************************************/

#define FL_MAXFONTS		48	/* max number of fonts */

typedef enum {
	FL_INVALID_STYLE = -1,
	FL_NORMAL_STYLE,
	FL_BOLD_STYLE,
	FL_ITALIC_STYLE,
	FL_BOLDITALIC_STYLE,

	FL_FIXED_STYLE,
	FL_FIXEDBOLD_STYLE,
	FL_FIXEDITALIC_STYLE,
	FL_FIXEDBOLDITALIC_STYLE,

	FL_TIMES_STYLE,
	FL_TIMESBOLD_STYLE,
	FL_TIMESITALIC_STYLE,
	FL_TIMESBOLDITALIC_STYLE,

	FL_MISC_STYLE,
	FL_MISCBOLD_STYLE,
	FL_MISCITALIC_STYLE,
	FL_SYMBOL_STYLE,

	/* modfier masks. Need to fit a short  */

	FL_SHADOW_STYLE	  = ( 1 <<	9 ),
	FL_ENGRAVED_STYLE = ( 1 << 10 ),
	FL_EMBOSSED_STYLE = ( 1 << 11 )
} FL_TEXT_STYLE;

#define FL_FONT_STYLE FL_TEXT_STYLE

#define special_style( a )	(	 ( a ) >= FL_SHADOW_STYLE					  \
							  && ( a ) <= ( FL_EMBOSSED_STYLE + FL_MAXFONTS ) )

/* Standard sizes in XForms */

#define FL_TINY_SIZE		8
#define FL_SMALL_SIZE	   10
#define FL_NORMAL_SIZE	   12
#define FL_MEDIUM_SIZE	   14
#define FL_LARGE_SIZE	   18
#define FL_HUGE_SIZE	   24

#define FL_DEFAULT_SIZE	  FL_SMALL_SIZE

/* Defines for compatibility */

#define FL_TINY_FONT	  FL_TINY_SIZE
#define FL_SMALL_FONT	  FL_SMALL_SIZE
#define FL_NORMAL_FONT	  FL_NORMAL_SIZE
#define FL_MEDIUM_FONT	  FL_MEDIUM_SIZE
#define FL_LARGE_FONT	  FL_LARGE_SIZE
#define FL_HUGE_FONT	  FL_HUGE_SIZE

#define FL_NORMAL_FONT1	  FL_SMALL_FONT
#define FL_NORMAL_FONT2	  FL_NORMAL_FONT
#define FL_DEFAULT_FONT	  FL_SMALL_FONT

#define FL_BOUND_WIDTH	( FL_Coord ) 1	   /* Border width of boxes */

/* Definition of basic struct that holds an object */

#define	 FL_CLICK_TIMEOUT  400	/* double click interval */

struct forms_;
struct fl_pixmap;

typedef struct flobjs_ {
	struct forms_  * form;			 /* the form this object belongs to */
	void		   * u_vdata;		 /* anything the user likes */
	char		   * u_cdata;		 /* anything the user likes */
	long			 u_ldata;		 /* anything the user likes */

	int				 objclass;		 /* class of object, button, slider etc */
	int				 type;			 /* type within the class */
	int				 boxtype;		 /* what kind of box type */
	FL_Coord		 x,				 /* current obj. location and size */
					 y,
					 w,
					 h;
	double			 fl1,			 /* distances of upper left hand (1) and */
					 fr1,			 /* lower right hand corner (2) to left, */
					 ft1,			 /* right, top and bottom of enclosing	 */
					 fb1,			 /* form */
					 fl2,
					 fr2,
					 ft2,
					 fb2;
	FL_Coord		 bw;
	FL_COLOR		 col1,			 /* colors of obj */
					 col2;
	char		   * label;			 /* object label */
	FL_COLOR		 lcol;			 /* label color */
	int				 align;
	int				 lsize,			 /* label size and style */
					 lstyle;
	long		   * shortcut;
	int				 ( * handle )( struct flobjs_ *,
								   int,
								   FL_Coord,
								   FL_Coord,
								   int,
								   void * );
	void			 ( * object_callback )( struct flobjs_ *,
											 long );
	long			 argument;
	void		   * spec;		 /* instantiation					   */

	int				 ( * prehandle )( struct flobjs_ *,
									  int,
									  FL_Coord,
									  FL_Coord,
									  int,
									  void * );
	int				 ( * posthandle )( struct flobjs_ *,
									   int,
									   FL_Coord,
									   FL_Coord,
									   int,
									   void *);

	/* re-configure preference */

	unsigned int	 resize;		 /* what to do if WM resizes the FORM	  */
	unsigned int	 nwgravity;		 /* how to re-position top-left corner	  */
	unsigned int	 segravity;		 /* how to re-position lower-right corner */

	struct flobjs_ * prev;			 /* prev. obj in form */
	struct flobjs_ * next;			 /* next. obj in form */

	struct flobjs_ * parent;
	struct flobjs_ * child;
	struct flobjs_ * nc;
	int				 is_child;

	void *			 flpixmap;		 /* pixmap double buffering stateinfo */
	int				 use_pixmap;	 /* true to use pixmap double buffering*/

	/* some interaction flags */

	int				 double_buffer;	 /* only used by mesa/gl canvas */
	int				 pushed;
	int				 focus;
	int				 belowmouse;
	int				 active;		 /* if accept event */
	int				 input;
	int				 wantkey;
	int				 radio;
	int				 automatic;
	int				 redraw;
	int				 visible;
	int              is_under;       /* if (partially) hidden by other object */
	int				 clip;
	unsigned long	 click_timeout;
	void		   * c_vdata;		 /* for class use */
	char		   * c_cdata;		 /* for class use */
	long			 c_ldata;		 /* for class use */
	unsigned int	 spec_size;		 /* for internal use */
	FL_COLOR		 aux_col1,		 /* aux colors */
					 aux_col2;
	FL_COLOR		 dbl_background; /* double buffer background */
	int				 how_return;
	char		   * tooltip;
	int				 tipID;
	int				 group_id;
	int              want_motion;
	int              want_update;
} FL_OBJECT;

/* callback function for an entire form */

typedef void ( * FL_FORMCALLBACKPTR )( struct flobjs_ *,
									   void * );
/* object callback function		 */

typedef void ( * FL_CALLBACKPTR )( FL_OBJECT *,
								   long );

/* preemptive callback function	 */

typedef int ( * FL_RAW_CALLBACK )( struct forms_ *,
								   void * );

/* at close (WM menu delete/close etc.) */

typedef int ( * FL_FORM_ATCLOSE )( struct forms_ *,
								   void * );
/* deactivate/activate callback */

typedef void ( * FL_FORM_ATDEACTIVATE )( struct forms_ *,
										 void * );
typedef void ( * FL_FORM_ATACTIVATE )( struct forms_ *,
									   void * );

typedef int ( * FL_HANDLEPTR )( FL_OBJECT *,
								int,
								FL_Coord,
								FL_Coord,
								int,
								void * );

/* error callback */

typedef void ( * FL_ERROR_FUNC )( const char *,
								  const char *,
								  ... );

FL_EXPORT FL_OBJECT *FL_EVENT;

/*** FORM ****/

/* form visibility state: form->visible */

enum {
  FL_BEING_HIDDEN = -1,
  FL_HIDDEN		  = 0,
  FL_INVISIBLE	  = FL_HIDDEN,
  FL_VISIBLE	  = 1
};

typedef struct forms_ {
	void			     * fdui;		  /* for fdesign */
	void			     * u_vdata;		  /* for application */
	char			     * u_cdata;		  /* for application */
	long				   u_ldata;		  /* for application */

	char *				   label;		  /* window title */
	Window                 window;		  /* X resource ID for window */
	FL_Coord			   x,			  /* current geometry info */
						   y,
						   w,
						   h;
	int                    handle_dec_x,
	                       handle_dec_y;
	FL_Coord			   hotx,		  /* hot-spot of the form */
						   hoty;
	double				   w_hr,		  /* high resolution width and height */
						   h_hr;		  /* (needed for precise scaling) */

	struct flobjs_	     * first;
	struct flobjs_	     * last;
	struct flobjs_	     * focusobj;

	FL_FORMCALLBACKPTR	   form_callback;
	FL_FORM_ATACTIVATE	   activate_callback;
	FL_FORM_ATDEACTIVATE   deactivate_callback;
	void			     * form_cb_data;
	void			     * activate_data;
	void			     * deactivate_data;

	FL_RAW_CALLBACK		   key_callback;
	FL_RAW_CALLBACK		   push_callback;
	FL_RAW_CALLBACK		   crossing_callback;
	FL_RAW_CALLBACK		   motion_callback;
	FL_RAW_CALLBACK		   all_callback;

	unsigned long		   compress_mask;
	unsigned long		   evmask;

	/* WM_DELETE_WINDOW message handler */

	FL_FORM_ATCLOSE		   close_callback;
	void			     * close_data;

	void			     * flpixmap;		 /* back buffer */

	Pixmap		           icon_pixmap;
	Pixmap		           icon_mask;

	/* interaction and other flags */

	int					   vmode;			 /* current X visual class */
	int					   deactivated;	 	 /* true if sensitive */
	int					   use_pixmap;	 	 /* true if dbl buffering */
	int					   frozen;		 	 /* true if sync change */
	int					   visible;		 	 /* true if mapped */
	int					   wm_border;		 /* window manager info */
	unsigned int		   prop;			 /* other attributes */
	int					   has_auto_objects;
	int					   top;
	int					   sort_of_modal;	 /* internal use */
	struct forms_	     * parent;
	struct forms_	     * child;
	struct flobjs_	     * parent_obj;
	int					   attached;		 /* not independent anymore */
	void				   ( * pre_attach )( struct forms_ * );
	void			     * attach_data;
	int					   no_tooltip;
} FL_FORM;


/* All FD_xxx structure emitted by fdesign contains at least the
 * following */

typedef struct {
	FL_FORM * form;
	void	* vdata;
	char	* cdata;
	long	  ldata;
} FD_Any;

/* Async IO stuff */

enum {
	FL_READ	  = 1,
	FL_WRITE  = 2,
	FL_EXCEPT = 4
};

/* IO other than XEvent Q */

typedef void ( * FL_IO_CALLBACK )( int,
								   void * );

FL_EXPORT void fl_add_io_callback( int			    fd,
								   unsigned int	    mask,
								   FL_IO_CALLBACK   callback,
								   void		      * data );

FL_EXPORT void fl_remove_io_callback( int			 fd,
									  unsigned int   mask,
									  FL_IO_CALLBACK cb );

/* signals */

typedef void ( * FL_SIGNAL_HANDLER )( int,
									  void * );

FL_EXPORT void fl_add_signal_callback( int				   s,
									   FL_SIGNAL_HANDLER   cb,
									   void			     * data );

FL_EXPORT void fl_remove_signal_callback( int s );

FL_EXPORT void fl_signal_caught( int s );

FL_EXPORT void fl_app_signal_direct( int y );

/* timeouts */

typedef void (* FL_TIMEOUT_CALLBACK )( int, void * );

FL_EXPORT int fl_add_timeout( long				    msec,
							  FL_TIMEOUT_CALLBACK	callback,
							  void				  * data );

FL_EXPORT void fl_remove_timeout( int id );

/* Basic public routine prototypes */

FL_EXPORT int fl_library_version( int * ver,
								  int * rev );

/** Generic routines that deal with FORMS **/

FL_EXPORT FL_FORM * fl_bgn_form( int	  type,
								 FL_Coord w,
								 FL_Coord h );

FL_EXPORT void fl_end_form( void );

FL_EXPORT FL_OBJECT * fl_do_forms( void );

FL_EXPORT FL_OBJECT * fl_check_forms( void );

FL_EXPORT FL_OBJECT * fl_do_only_forms( void );

FL_EXPORT FL_OBJECT * fl_check_only_forms( void );

FL_EXPORT void fl_freeze_form( FL_FORM * form );

FL_EXPORT void fl_set_focus_object( FL_FORM	  * form,
									FL_OBJECT * obj );

FL_EXPORT FL_OBJECT *fl_get_focus_object( FL_FORM * form );

FL_EXPORT void fl_reset_focus_object( FL_OBJECT * ob );

#define fl_set_object_focus	  fl_set_focus_object

FL_EXPORT FL_FORM_ATCLOSE fl_set_form_atclose( FL_FORM         * form,
											   FL_FORM_ATCLOSE   fmclose,
											   void            * data );

FL_EXPORT FL_FORM_ATCLOSE fl_set_atclose( FL_FORM_ATCLOSE   fmclose,
										  void            * data );

FL_EXPORT FL_FORM_ATACTIVATE fl_set_form_atactivate(FL_FORM            * form,
													FL_FORM_ATACTIVATE   cb,
													void               * data );

FL_EXPORT FL_FORM_ATDEACTIVATE
	fl_set_form_atdeactivate( FL_FORM			   * form,
							  FL_FORM_ATDEACTIVATE   cb,
							  void				   * data );

FL_EXPORT void fl_unfreeze_form( FL_FORM * form );

FL_EXPORT void fl_deactivate_form( FL_FORM * form );

FL_EXPORT void fl_activate_form( FL_FORM * form );

FL_EXPORT void fl_deactivate_all_forms( void );

FL_EXPORT void fl_activate_all_forms( void );

FL_EXPORT void fl_freeze_all_forms( void );

FL_EXPORT void fl_unfreeze_all_forms( void );

FL_EXPORT void fl_scale_form( FL_FORM * form,
							  double	xsc,
							  double	ysc );

FL_EXPORT void fl_set_form_position( FL_FORM  * form,
									 FL_Coord   x,
									 FL_Coord   y );

FL_EXPORT void fl_set_form_title( FL_FORM    * form,
								  const char * name );

FL_EXPORT void fl_set_app_mainform( FL_FORM * form );

FL_EXPORT FL_FORM * fl_get_app_mainform( void );

FL_EXPORT void fl_set_app_nomainform( int flag );

FL_EXPORT void fl_set_form_callback( FL_FORM			* form,
									 FL_FORMCALLBACKPTR	  callback,
									 void			    * d );

#define	 fl_set_form_call_back	  fl_set_form_callback

FL_EXPORT void fl_set_form_size( FL_FORM  * form,
								 FL_Coord   w,
								 FL_Coord   h );

FL_EXPORT void fl_set_form_hotspot( FL_FORM	 * form,
									FL_Coord   x,
									FL_Coord   y );

FL_EXPORT void fl_set_form_hotobject( FL_FORM	* form,
									  FL_OBJECT * ob );

FL_EXPORT void fl_set_form_minsize( FL_FORM	 * form,
									FL_Coord   w,
									FL_Coord   h );

FL_EXPORT void fl_set_form_maxsize( FL_FORM	 * form,
									FL_Coord   w,
									FL_Coord   h );

FL_EXPORT void fl_set_form_event_cmask( FL_FORM		  * form,
										unsigned long	cmask );

FL_EXPORT unsigned long fl_get_form_event_cmask( FL_FORM * form );

FL_EXPORT void fl_set_form_geometry( FL_FORM  * form,
									 FL_Coord   x,
									 FL_Coord   y,
									 FL_Coord   w,
									 FL_Coord   h );

#define fl_set_initial_placement fl_set_form_geometry

FL_EXPORT Window fl_show_form( FL_FORM	  * form,
							   int			place,
							   int		    border,
							   const char * name );

FL_EXPORT void fl_hide_form( FL_FORM * form );

FL_EXPORT void fl_free_form( FL_FORM * form );

FL_EXPORT void fl_redraw_form( FL_FORM * form );

FL_EXPORT void fl_set_form_dblbuffer( FL_FORM * form,
									  int		y );

FL_EXPORT Window fl_prepare_form_window( FL_FORM	* form,
										 int		  place,
										 int		  border,
										 const char * name );

FL_EXPORT Window fl_show_form_window( FL_FORM * form );

FL_EXPORT double fl_adjust_form_size( FL_FORM * form );

FL_EXPORT int fl_form_is_visible( FL_FORM * form );

FL_EXPORT int fl_form_is_iconified( FL_FORM * form );

FL_EXPORT FL_RAW_CALLBACK fl_register_raw_callback( FL_FORM			* form,
													unsigned long	  mask,
													FL_RAW_CALLBACK	  rcb );

#define fl_register_call_back fl_register_raw_callback

FL_EXPORT FL_OBJECT * fl_bgn_group( void );

FL_EXPORT void fl_end_group( void );

FL_EXPORT FL_OBJECT *fl_addto_group( FL_OBJECT * group );

/****** Routines that deal with FL_OBJECTS ********/

FL_EXPORT void fl_set_object_boxtype( FL_OBJECT * ob,
									  int		  boxtype );

FL_EXPORT void fl_set_object_bw( FL_OBJECT * ob,
								 int		 bw );

FL_EXPORT void fl_get_object_bw( FL_OBJECT * ob,
								 int	   * bw );

FL_EXPORT void fl_set_object_resize( FL_OBJECT	  * ob,
									 unsigned int   what );

FL_EXPORT void fl_get_object_resize( FL_OBJECT	  * ob,
									 unsigned int * what );

FL_EXPORT void fl_set_object_gravity( FL_OBJECT	   * ob,
									  unsigned int   nw,
									  unsigned int   se );

FL_EXPORT void fl_get_object_gravity( FL_OBJECT	   * ob,
									  unsigned int * nw,
									  unsigned int * se );

FL_EXPORT void fl_set_object_lsize( FL_OBJECT * ob,
									int			lsize );

FL_EXPORT void fl_set_object_lstyle( FL_OBJECT * ob,
									 int		 lstyle );

FL_EXPORT void fl_set_object_lcol( FL_OBJECT * ob,
								   FL_COLOR	   lcol );

FL_EXPORT void fl_set_object_return( FL_OBJECT * ob,
									 int		 when );

FL_EXPORT void fl_set_object_lalign( FL_OBJECT * ob,
									 int		 align );

FL_EXPORT void fl_set_object_shortcut( FL_OBJECT  * obj,
									   const char * sstr,
									   int			showit );

FL_EXPORT void fl_set_object_shortcutkey( FL_OBJECT	   * obj,
										  unsigned int   keysym );

FL_EXPORT void fl_set_object_dblbuffer( FL_OBJECT * ob,
										int			y );

FL_EXPORT void fl_set_object_color( FL_OBJECT * ob,
									FL_COLOR	col1,
									FL_COLOR	col2 );

FL_EXPORT void fl_set_object_label( FL_OBJECT  * ob,
									const char * label );

FL_EXPORT void fl_set_object_helper( FL_OBJECT  * ob,
									 const char * tip );

FL_EXPORT void fl_set_object_position( FL_OBJECT * obj,
									   FL_Coord	   x,
									   FL_Coord	   y );

FL_EXPORT void fl_get_object_size( FL_OBJECT * obj,
								   FL_Coord  * w,
								   FL_Coord  * h );

FL_EXPORT void fl_set_object_size( FL_OBJECT * obj,
								   FL_Coord	   w,
								   FL_Coord	   h );

FL_EXPORT void fl_set_object_automatic( FL_OBJECT * ob,
										int			flag );

FL_EXPORT void fl_draw_object_label( FL_OBJECT * ob );

FL_EXPORT void fl_draw_object_label_outside( FL_OBJECT * ob );

FL_EXPORT FL_OBJECT * fl_get_object_component( FL_OBJECT * composite,
											   int			objclass,
											   int			type,
											   int			numb );

FL_EXPORT void fl_for_all_objects( FL_FORM * form,
								   int		  ( * cb )( FL_OBJECT *, void * ),
								   void	   * v );


#define fl_draw_object_outside_label fl_draw_object_label_outside

#define	fl_set_object_dblclick( ob, timeout )  \
	do {                                       \
		( ob )->click_timeout = ( timeout );   \
	} while ( 0 )

FL_EXPORT void fl_set_object_geometry( FL_OBJECT * obj,
									   FL_Coord	   x,
									   FL_Coord	   y,
									   FL_Coord	   w,
									   FL_Coord	   h );

FL_EXPORT void fl_move_object( FL_OBJECT * obj,
							   FL_Coord	   dx,
							   FL_Coord	   dy );

#define fl_set_object_lcolor  fl_set_object_lcol

FL_EXPORT void fl_fit_object_label( FL_OBJECT * obj,
									FL_Coord	xmargin,
									FL_Coord	ymargin );

FL_EXPORT void fl_get_object_geometry( FL_OBJECT * ob,
									   FL_Coord  *	x,
									   FL_Coord  *	y,
									   FL_Coord  *	w,
									   FL_Coord  *	h );

FL_EXPORT void fl_get_object_position( FL_OBJECT * ob,
									   FL_Coord  * x,
									   FL_Coord  * y );

FL_EXPORT const char * fl_get_object_label( FL_OBJECT * ob );

/* this one takes into account the label */

FL_EXPORT void fl_get_object_bbox( FL_OBJECT * obj,
								   FL_Coord  * x,
								   FL_Coord  * y,
								   FL_Coord  * w,
								   FL_Coord  * h );

#define fl_compute_object_geometry	 fl_get_object_bbox

FL_EXPORT void fl_call_object_callback( FL_OBJECT * ob );

FL_EXPORT FL_HANDLEPTR fl_set_object_prehandler( FL_OBJECT	  * ob,
												 FL_HANDLEPTR   phandler );

FL_EXPORT FL_HANDLEPTR fl_set_object_posthandler( FL_OBJECT	   * ob,
												  FL_HANDLEPTR   post );

FL_EXPORT FL_CALLBACKPTR fl_set_object_callback( FL_OBJECT	    * obj,
												 FL_CALLBACKPTR	  callback,
												 long			  argument );

#define fl_set_object_align	  fl_set_object_lalign
#define fl_set_call_back	  fl_set_object_callback

FL_EXPORT void fl_redraw_object( FL_OBJECT * obj );

FL_EXPORT void fl_scale_object( FL_OBJECT * ob,
								double		xs,
								double		ys );

FL_EXPORT void fl_show_object( FL_OBJECT * ob );

FL_EXPORT void fl_hide_object( FL_OBJECT * ob );

FL_EXPORT void fl_free_object( FL_OBJECT * obj );

FL_EXPORT void fl_delete_object( FL_OBJECT * obj );

FL_EXPORT void fl_trigger_object( FL_OBJECT * obj );

FL_EXPORT void fl_activate_object( FL_OBJECT * ob );

FL_EXPORT void fl_deactivate_object( FL_OBJECT * ob );

FL_EXPORT int fl_enumerate_fonts( void ( * output )( const char * s ),
								  int  shortform );

FL_EXPORT int fl_set_font_name( int			 n,
								const char * name );

FL_EXPORT void fl_set_font( int numb,
							int size );

/* routines that facilitate free object */

FL_EXPORT int fl_get_char_height( int	style,
								  int	size,
								  int * asc,
								  int * desc );

FL_EXPORT int fl_get_char_width( int style,
								 int size );

FL_EXPORT int fl_get_string_height( int			 style,
									int			 size,
									const char * s,
									int			 len,
									int		   * asc,
									int		   * desc );

FL_EXPORT int fl_get_string_width( int			style,
								   int			size,
								   const char * s,
								   int			len );

FL_EXPORT int fl_get_string_widthTAB( int		   style,
									  int		   size,
									  const char * s,
									  int		   len );

FL_EXPORT void fl_get_string_dimension( int			 fntstyle,
										int			 fntsize,
										const char * s,
										int			 len,
										int		   * width,
										int		   * height );

#define fl_get_string_size	fl_get_string_dimension

FL_EXPORT void fl_get_align_xy( int	  align,
								int	  x,
								int	  y,
								int	  w,
								int	  h,
								int	  xsize,
								int	  ysize,
								int	  xoff,
								int	  yoff,
								int * xx,
								int * yy );

FL_EXPORT void fl_drw_text( int		     align,
							FL_Coord     x,
							FL_Coord     y,
							FL_Coord     w,
							FL_Coord     h,
							FL_COLOR     c,
							int		     style,
							int		     size,
							const char * istr );

FL_EXPORT void fl_drw_text_beside( int		    align,
								   FL_Coord     x,
								   FL_Coord     y,
								   FL_Coord     w,
								   FL_Coord     h,
								   FL_COLOR     c,
								   int		    style,
								   int		    size,
								   const char * str );

FL_EXPORT void fl_drw_text_cursor( int		    align,
								   FL_Coord     x,
								   FL_Coord     y,
								   FL_Coord     w,
								   FL_Coord     h,
								   FL_COLOR     c,
								   int		    style,
								   int		    size,
								   const char * str,
								   int		    cc,
								   int		    pos );

FL_EXPORT void fl_drw_box( int		style,
						   FL_Coord x,
						   FL_Coord y,
						   FL_Coord w,
						   FL_Coord h,
						   FL_COLOR c,
						   int		bw_in );

typedef void ( * FL_DRAWPTR )( FL_Coord,
							   FL_Coord,
							   FL_Coord,
							   FL_Coord,
							   int,
							   FL_COLOR);

FL_EXPORT int fl_add_symbol( const char * name,
							 FL_DRAWPTR	  drawit,
							 int		  scalable );

FL_EXPORT int fl_draw_symbol( const char * label,
							  FL_Coord	   x,
							  FL_Coord	   y,
							  FL_Coord	   w,
							  FL_Coord	   h,
							  FL_COLOR	   col );

enum {
	FL_SLIDER_NONE = 0,
	FL_SLIDER_BOX  = 1,
	FL_SLIDER_KNOB = 2,
	FL_SLIDER_UP   = 4,
	FL_SLIDER_DOWN = 8,
	FL_SLIDER_ALL  = 15
};

FL_EXPORT unsigned long fl_mapcolor( FL_COLOR col,
									 int	  r,
									 int	  g,
									 int	  b );

FL_EXPORT long fl_mapcolorname( FL_COLOR	 col,
								const char * name );

#define fl_mapcolor_name  fl_mapcolorname

FL_EXPORT void fl_free_colors( FL_COLOR * c,
							   int		  n );

FL_EXPORT void fl_free_pixels( unsigned long * pix,
							   int			   n );

FL_EXPORT void fl_set_color_leak( int y );

FL_EXPORT unsigned long fl_getmcolor( FL_COLOR   i,
									  int	   * r,
									  int	   * g,
									  int	   * b );

FL_EXPORT unsigned long fl_get_pixel( FL_COLOR col );

#define fl_get_flcolor	 fl_get_pixel

FL_EXPORT void fl_get_icm_color( FL_COLOR   col,
								 int	  * r,
								 int	  * g,
								 int	  * b );

FL_EXPORT void fl_set_icm_color( FL_COLOR col,
								 int	  r,
								 int	  g,
								 int	  b );

FL_EXPORT void fl_color( FL_COLOR col );

FL_EXPORT void fl_bk_color( FL_COLOR col );

FL_EXPORT void fl_textcolor( FL_COLOR col );

FL_EXPORT void fl_bk_textcolor( FL_COLOR col );

FL_EXPORT void fl_set_gamma( double r,
							 double g,
							 double b );

FL_EXPORT void fl_show_errors( int y );

/* Some macros */

#define FL_max( a, b )			  ( ( a ) > ( b ) ? ( a ) : ( b ) )
#define FL_min( a, b )			  ( ( a ) < ( b ) ? ( a ) : ( b ) )
#define FL_abs( a )				  ( ( a ) > 0 ? ( a ) : ( - ( a ) ) )
#define FL_nint( a )			  ( ( int ) ( ( a ) > 0 ?				\
											  ( ( a ) + 0.5 ) :			\
											  ( ( a ) - 0.5 ) ) )
#define FL_clamp( a, amin, amax ) ( ( a ) < ( amin ) ?					\
									( amin ) : ( ( a ) > ( amax ) ?		\
												 ( amax ) : ( a ) ) )
#define FL_crnd( a )			  ( ( FL_Coord ) ( ( a ) > 0 ?			\
												   ( ( a ) + 0.5 ) :	\
												   ( ( a ) - 0.5 ) ) )

typedef int ( * FL_FSCB )( const char *,
						   void * );

/* utilities for new objects */

FL_EXPORT FL_FORM *fl_current_form;

FL_EXPORT void fl_add_object( FL_FORM	* form,
							  FL_OBJECT * obj );

FL_EXPORT FL_FORM *fl_addto_form( FL_FORM * form );

FL_EXPORT FL_OBJECT * fl_make_object( int			 objclass,
									  int			 type,
									  FL_Coord	     x,
									  FL_Coord	     y,
									  FL_Coord	     w,
									  FL_Coord	     h,
									  const char   * label,
									  FL_HANDLEPTR   handle );

FL_EXPORT void fl_set_coordunit( int u );

FL_EXPORT void fl_set_border_width( int bw );

FL_EXPORT void fl_set_scrollbar_type( int t );

#define fl_set_thinscrollbar( t )										 \
	fl_set_scrollbar_type( t ? FL_THIN_SCROLLBAR : FL_NORMAL_SCROLLBAR )

FL_EXPORT void fl_flip_yorigin( void );

FL_EXPORT int fl_get_coordunit( void );

FL_EXPORT int fl_get_border_width( void );

/* misc. routines */

FL_EXPORT void fl_ringbell( int percent );

FL_EXPORT void fl_gettime( long * sec,
						   long * usec );

FL_EXPORT const char * fl_now( void );

FL_EXPORT const char * fl_whoami( void );

FL_EXPORT long fl_mouse_button( void );

FL_EXPORT char * fl_strdup( const char * s );

FL_EXPORT void fl_set_err_logfp( FILE * fp );

FL_EXPORT void fl_set_error_handler( FL_ERROR_FUNC user_func );

FL_EXPORT char ** fl_get_cmdline_args( int * );

/* This function was called 'fl_set_error_logfp/' in XForms 0.89. */

#define fl_set_error_logfp fl_set_err_logfp

#define fl_mousebutton fl_mouse_button

/* these give more flexibility for future changes. Also application
 * can re-assign these pointers to whatever function it wants, e.g.,
 * to a shared memory pool allocator. */

FL_EXPORT void ( * fl_free )( void * );

FL_EXPORT void * ( * fl_malloc )( size_t );

FL_EXPORT void * ( * fl_calloc )( size_t,
								  size_t );

FL_EXPORT void * ( * fl_realloc )( void *,
								   size_t );

FL_EXPORT int fl_msleep( unsigned long msec );

#define FL_MAX_MENU_CHOICE_ITEMS   128

typedef const char * ( * FL_VAL_FILTER )( FL_OBJECT *,
										  double,
										  int );

#endif /* ! defined FL_BASIC_H */
