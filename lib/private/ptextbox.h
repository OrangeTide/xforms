/*
 *  This file is part of the XForms library package.
 *
 *  XForms is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU Lesser General Public License as
 *  published by the Free Software Foundation; either version 2.1, or
 *  (at your option) any later version.
 *
 *  XForms is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XForms.  If not, see <http://www.gnu.org/licenses/>.
 */


/**
 * \file ptextbox.h
 *
 *  This file is part of the XForms library package.
 *  Copyright (c) 1995-1997  T.C. Zhao and Mark Overmars
 *  All rights reserved.
 *.
 *
 *  private header info for textbox object (part of browser
 */

#ifndef  PTEXTBOX_H
#define  PTEXTBOX_H

typedef struct {
    char         * txt;				/* text of line                  */
    unsigned int   len;				/* line length                   */
    short          selected;		/* whether line is selected      */
    short          non_selectable;	/* if line is non-selectable     */
    short          pixels;			/* length in pixels              */
} LINE;

typedef struct {
    LINE           ** text;			/* lines of text (NULL if not used) */
    FL_CALLBACKPTR    callback;     /* double and tripple click callback */
    long              callback_data;
    GC                bkGC;			/* background GC             */
    GC                selectGC;		/* selected mark GC          */
    GC                primaryGC;	/* text drawing GC           */
    GC                specialGC;	/* handle font/color change  */
    FL_COLOR          lcol;
    FL_COLOR          col1;
    FL_COLOR          col2;
    FL_Coord          x,			/* browser drawing area      */
	                  y,
	                  w,
	                  h;
    unsigned int      drawtype;
    int               topline;		/* Current topline           */
    int               oldtopline;	/* change mark               */
    int               lines;		/* Number of lines in browser */
    int               avail_lines;	/* Max. available lines      */
    int               selectline;	/* Last selected line        */
    int               desel_mark;
    int               specialkey;	/* Key that indicates a special symbol */
    int               fontstyle;	/* Style of font                 */
    int               fontsize;		/* The character size            */
    int               charheight;	/* base font height              */
    int               chardesc;		/* base font descent             */
    int               screenlines;	/* no. of visible lines          */
    int               vmode;		/* vmode GCs are valid for       */
    int               maxpixels_line;
    int               maxpixels;
    int               attrib;		/* set if there is attrib change */
    int               xoffset;		/* horizontal scroll in pixels    */
    int               lastmy;
	int               status_changed;
} FLI_TEXTBOX_SPEC;


/***** Types    *****/

enum {
    FLI_NORMAL_TEXTBOX = FL_NORMAL_BROWSER,
    FLI_SELECT_TEXTBOX = FL_SELECT_BROWSER,
    FLI_HOLD_TEXTBOX   = FL_HOLD_BROWSER,
    FLI_MULTI_TEXTBOX  = FL_MULTI_BROWSER
};


/***** Defaults *****/

#define FLI_TEXTBOX_BOXTYPE	     FL_DOWN_BOX
#define FLI_TEXTBOX_COL1	     FL_COL1
#define FLI_TEXTBOX_COL2	     FL_YELLOW
#define FLI_TEXTBOX_LCOL	     FL_LCOL
#define FLI_TEXTBOX_ALIGN	     FL_ALIGN_BOTTOM
#define FLI_TEXTBOX_FONTSIZE     FL_SMALL_FONT


/***** Routines *****/

extern FL_OBJECT * fli_create_textbox( int,
									   FL_Coord,
									   FL_Coord,
									   FL_Coord,
									   FL_Coord,
									   const char * );

extern void fli_clear_textbox( FL_OBJECT * );

extern int fli_set_textbox_topline( FL_OBJECT *,
									int );

extern int fli_set_textbox_xoffset( FL_OBJECT *,
									FL_Coord );

extern FL_Coord fli_get_textbox_xoffset( FL_OBJECT * );

extern int fli_get_textbox_longestline( FL_OBJECT * );

extern void fli_calc_textbox_size( FL_OBJECT * );

extern void fli_add_textbox_line( FL_OBJECT *,
								  const char * );

extern void fli_addto_textbox( FL_OBJECT *,
							   const char * );

extern void fli_addto_textbox_chars( FL_OBJECT *,
									 const char * );

extern const char * fli_get_textbox_line( FL_OBJECT *,
										  int );

extern int fli_get_textbox( FL_OBJECT * );

extern void fli_set_textbox_fontsize( FL_OBJECT *,
									  int );

extern void fli_set_textbox_fontstyle( FL_OBJECT *,
									   int );

extern int fli_load_textbox( FL_OBJECT *,
							 const char * );

extern void fli_select_textbox_line( FL_OBJECT *,
									 int,
									 int );

extern int fli_isselected_textbox_line( FL_OBJECT *,
										int );

extern void fli_deselect_textbox_line( FL_OBJECT *,
									   int );

extern void fli_deselect_textbox( FL_OBJECT * );

extern void fli_delete_textbox_line( FL_OBJECT *,
									 int );

extern void fli_replace_textbox_line( FL_OBJECT *,
									  int,
									  const char * );

extern void fli_insert_textbox_line( FL_OBJECT *,
									 int,
									 const char * );

extern void fli_set_textbox_dblclick_callback( FL_OBJECT *,
											   FL_CALLBACKPTR,
											   long );

extern void fli_get_textbox_dimension( FL_OBJECT *,
									   int *,
									   int *,
									   int *,
									   int * );

extern void fli_set_textbox_line_selectable( FL_OBJECT *,
											 int,
											 int );

#endif
