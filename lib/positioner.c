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
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with XForms.  If not, see <http://www.gnu.org/licenses/>.
 */


/**
 * \file positioner.c
 *
 *  This file is part of the XForms library package.
 *  Copyright (c) 1996-2002  T.C. Zhao and Mark Overmars
 *  All rights reserved.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include/forms.h"
#include "flinternal.h"
#include "private/ppositioner.h"

#include <sys/types.h>
#include <stdlib.h>


/* The special information for positioners.
 * ymin is the value at the bottom and ymax is the value at the top */


/***************************************
 * performs linear interpolation
 ***************************************/

static double
flinear( double val,
		 double smin,
		 double smax,
		 double gmin,
		 double gmax )
{
    if ( smin == smax )
		return gmax;

	return gmin + ( gmax - gmin ) * ( val - smin ) / ( smax - smin );
}


/***************************************
 * Draws a positioner
 ***************************************/

static void
draw_positioner( FL_OBJECT * ob )
{
    FLI_POSITIONER_SPEC *sp = ob->spec;
    FL_Coord absbw = FL_abs( ob->bw );
    FL_Coord x1 = ob->x + absbw + 1,
		     y1 = ob->y + absbw + 1;
    FL_Coord w1 = ob->w - 2 * absbw - 2,
              h1 = ob->h - 2 * absbw - 2;
    FL_Coord xx,
             yy;
    int oldmode = fl_get_drawmode( );
    int newmode = ob->type == FL_OVERLAY_POSITIONER ? GXxor : GXcopy;


    if ( ! sp->partial )
    {
		if ( ob->type != FL_OVERLAY_POSITIONER )
			fl_drw_box( ob->boxtype, ob->x, ob->y, ob->w, ob->h,
						ob->col1, ob->bw );
		fl_draw_object_label_outside( ob );

    }
    else
    {
		long col = ob->type == FL_OVERLAY_POSITIONER ? ob->col2 : ob->col1;

		xx = flinear( sp->lxval, sp->xmin, sp->xmax, x1, x1 + w1 - 1.0 );
		yy = flinear( sp->lyval, sp->ymin, sp->ymax, y1 + h1 - 1.0, y1 );

		if ( oldmode != newmode )
			fl_drawmode( newmode );

		fl_diagline( x1, yy, w1, 1, col );
		fl_diagline( xx, y1, 1, h1, col );
    }

    xx = flinear( sp->xval, sp->xmin, sp->xmax, x1, x1 + w1 - 1.0 );
    yy = flinear( sp->yval, sp->ymin, sp->ymax, y1 + h1 - 1.0, y1 );

    if ( oldmode != newmode )
		fl_drawmode( newmode );

    fl_diagline( x1, yy, w1, 1, ob->col2 );
    fl_diagline( xx, y1, 1, h1, ob->col2 );

    if ( oldmode != newmode )
		fl_drawmode( oldmode );
}


/***************************************
 * Handle a mouse position change
 ***************************************/

static int
handle_mouse( FL_OBJECT * ob,
			  FL_Coord    mx,
			  FL_Coord    my )
{
    FLI_POSITIONER_SPEC *sp = ob->spec;
    FL_Coord absbw = FL_abs(ob->bw);
    FL_Coord x1 = ob->x + absbw + 1,
		     y1 = ob->y + absbw + 1;
    FL_Coord w1 = ob->w - 2 * absbw - 2,
		     h1 = ob->h - 2 * absbw - 2;
    double oldx = sp->xval,
		   oldy = sp->yval;

    sp->xval = flinear( mx, x1, x1 + w1 - 1.0, sp->xmin, sp->xmax );
    sp->yval = flinear( my, y1 + h1 - 1.0, y1, sp->ymin, sp->ymax );

    if ( sp->xstep != 0.0 )
		sp->xval = ( ( int ) (sp->xval / sp->xstep + 0.5 ) ) * sp->xstep;
    if ( sp->ystep != 0.0 )
		sp->yval = ( ( int ) ( sp->yval / sp->ystep + 0.5 ) ) * sp->ystep;

    /* make sure the position is in bounds */

    sp->xval = fli_clamp( sp->xval, sp->xmin, sp->xmax );
    sp->yval = fli_clamp( sp->yval, sp->ymin, sp->ymax );

    if ( sp->xval != oldx || sp->yval != oldy )
    {
		sp->partial = 1;
		sp->lxval = oldx;
		sp->lyval = oldy;
		fl_redraw_object( ob );
		if ( ! ( ob->how_return & FL_RETURN_END_CHANGED ) )
			return FL_RETURN_CHANGED;
    }

    return FL_RETURN_NONE;
}


/***************************************
 * Handles an event
 ***************************************/

static int
handle_positioner( FL_OBJECT * ob,
				   int         event,
				   FL_Coord    mx,
				   FL_Coord    my,
				   int         key  FL_UNUSED_ARG,
				   void *      ev   FL_UNUSED_ARG )
{
    FLI_POSITIONER_SPEC *sp = ob->spec;
	int ret = FL_RETURN_NONE;

    switch ( event )
    {
		case FL_DRAW:
			draw_positioner( ob );
			sp->partial = 0;
			break;

		case FL_DRAWLABEL:
			fl_draw_object_label_outside( ob );
			break;

		case FL_PUSH:
			if ( key != FL_MBUTTON1 )
				break;
			sp->old_x = sp->xval;
			sp->old_y = sp->yval;
			/* fall through */

		case FL_MOTION:
			if ( key != FL_MBUTTON1 )
				break;
			ret = handle_mouse( ob, mx, my );
			break;

		case FL_RELEASE:
			if ( key != FL_MBUTTON1 )
				break;
			ret = FL_RETURN_END;
			if (    ob->how_return & FL_RETURN_END_CHANGED
				 && ( sp->xval != sp->old_x || sp->yval != sp->old_y ) )
				 ret |= FL_RETURN_CHANGED;
			break;

		case FL_FREEMEM:
			fl_free( ob->spec );
			break;
    }

    return ret;
}

/***************************************
 * Creates a posiioner object
 ***************************************/

FL_OBJECT *
fl_create_positioner( int          type,
					  FL_Coord     x,
					  FL_Coord     y,
					  FL_Coord     w,
					  FL_Coord     h,
					  const char * label )
{
    FL_OBJECT *ob;
    FLI_POSITIONER_SPEC *sp;

    ob = fl_make_object( FL_POSITIONER, type, x, y, w, h, label,
						 handle_positioner );
    ob->boxtype    = FL_POSITIONER_BOXTYPE;
    ob->col1       = FL_POSITIONER_COL1;
    ob->col2       = FL_POSITIONER_COL2;
    ob->align      = FL_POSITIONER_ALIGN;
    ob->lcol       = FL_POSITIONER_LCOL;

    if ( ob->type == FL_OVERLAY_POSITIONER )
    {
		ob->bw = 0;
		ob->boxtype = FL_NO_BOX;
    }

    ob->spec = sp = fl_calloc( 1, sizeof *sp );

    sp->xmin = 0.0;
    sp->ymin = 0.0;
    sp->xmax = 1.0;
    sp->ymax = 1.0;
    sp->xval = sp->lxval = 0.5;
    sp->yval = sp->lyval = 0.5;

	fl_set_object_return( ob, FL_RETURN_CHANGED );

    return ob;
}


/***************************************
 * Adds a positioner object
 ***************************************/

FL_OBJECT *
fl_add_positioner( int          type,
				   FL_Coord     x,
				   FL_Coord     y,
				   FL_Coord     w,
				   FL_Coord     h,
				   const char * label )
{
    FL_OBJECT *ob = fl_create_positioner( type, x, y, w, h, label );

    fl_add_object( fl_current_form, ob );
    return ob;
}


/***************************************
 ***************************************/

void
fl_set_positioner_xvalue(FL_OBJECT * ob, double val)
{
    FLI_POSITIONER_SPEC *sp = ob->spec;

    val = fli_clamp( val, sp->xmin, sp->xmax );

    if ( sp->xval != val )
    {
		sp->lxval = sp->xval;
		sp->xval = val;
		fl_redraw_object( ob );
    }
}


/***************************************
 ***************************************/

void
fl_set_positioner_yvalue( FL_OBJECT * ob,
						  double      val )
{
    FLI_POSITIONER_SPEC *sp = ob->spec;

    val = fli_clamp( val, sp->ymin, sp->ymax );

    if ( sp->yval != val )
    {
		sp->lyval = sp->yval;
		sp->yval = val;
		fl_redraw_object( ob );
    }
}


/***************************************
 ***************************************/

void
fl_set_positioner_xbounds( FL_OBJECT * ob,
						   double      min,
						   double      max )
{
    FLI_POSITIONER_SPEC *sp = ob->spec;

    if ( sp->xmin != min || sp->xmax != max )
    {
		sp->xmin = min;
		sp->xmax = max;
		sp->xval = fli_clamp( sp->xval, sp->xmin, sp->xmax );
		fl_redraw_object( ob );
    }
}


/***************************************
 ***************************************/

void
fl_set_positioner_ybounds( FL_OBJECT * ob,
						   double      min,
						   double     max )
{
    FLI_POSITIONER_SPEC *sp = ob->spec;

    if ( sp->ymin != min || sp->ymax != max )
    {
		sp->ymin = min;
		sp->ymax = max;
		sp->yval = fli_clamp( sp->yval, sp->ymin, sp->ymax );
		fl_redraw_object( ob );
    }
}


/***************************************
 ***************************************/

double
fl_get_positioner_xvalue( FL_OBJECT * ob )
{
    return ( ( FLI_POSITIONER_SPEC * ) ob->spec )->xval;
}


/***************************************
 ***************************************/

double
fl_get_positioner_yvalue( FL_OBJECT * ob )
{
    return ( ( FLI_POSITIONER_SPEC * ) ob->spec )->yval;
}


/***************************************
 ***************************************/

void
fl_get_positioner_xbounds( FL_OBJECT * ob,
						   double *    min,
						   double *    max )
{
    *min = ( ( FLI_POSITIONER_SPEC * ) ob->spec)->xmin;
    *max = ( ( FLI_POSITIONER_SPEC * ) ob->spec)->xmax;
}

void
fl_get_positioner_ybounds( FL_OBJECT * ob,
						   double *    min,
						   double *    max)
{
    *min = ( ( FLI_POSITIONER_SPEC * ) ob->spec)->ymin;
    *max = ( ( FLI_POSITIONER_SPEC * ) ob->spec)->ymax;
}


/***************************************
 * Sets the step size to which values are rounded.
 ***************************************/

void
fl_set_positioner_xstep( FL_OBJECT * ob,
						 double      value )
{
    ( ( FLI_POSITIONER_SPEC * ) ob->spec )->xstep = value;
}


/***************************************
 * Sets the step size to which values are rounded.
 ***************************************/

void
fl_set_positioner_ystep( FL_OBJECT * ob,
						 double      value )
{
    ( ( FLI_POSITIONER_SPEC * ) ob->spec )->ystep = value;
}


/***************************************
 * Sets under which conditions the object is to be returned to the
 * application. This function should be regarded as deprecated and
 * fl_set_object_return() should be used instead.
 * Please note that this function doesn't work like the other
 * object specific functions for setting the return policy!
 ***************************************/

void
fl_set_positioner_return( FL_OBJECT * obj,
						  int         always )
{
	if ( always )
		fl_set_object_return( obj, FL_RETURN_CHANGED );
	else
		fl_set_object_return( obj, FL_RETURN_END );
}
