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
draw_positioner( FL_OBJECT * obj )
{
    FLI_POSITIONER_SPEC *sp = obj->spec;
    FL_Coord absbw = FL_abs( obj->bw );
    FL_Coord x1 = obj->x + absbw + 1,
             y1 = obj->y + absbw + 1;
    FL_Coord w1 = obj->w - 2 * absbw - 2,
             h1 = obj->h - 2 * absbw - 2;
    FL_Coord xx,
             yy;
    int oldmode = fl_get_drawmode( );
    int newmode = obj->type == FL_OVERLAY_POSITIONER ? GXxor : GXcopy;

    if ( ! sp->partial )
    {
        if ( obj->type != FL_OVERLAY_POSITIONER )
            fl_drw_box( obj->boxtype, obj->x, obj->y, obj->w, obj->h,
                        obj->col1, obj->bw );
        fl_draw_object_label_outside( obj );
    }
    else
    {
        long col = obj->type == FL_OVERLAY_POSITIONER ? obj->col2 : obj->col1;

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

    fl_diagline( x1, yy, w1, 1, obj->col2 );
    fl_diagline( xx, y1, 1, h1, obj->col2 );

    if ( oldmode != newmode )
        fl_drawmode( oldmode );
}


/***************************************
 * Handle a mouse position change
 ***************************************/

static int
handle_mouse( FL_OBJECT * obj,
              FL_Coord    mx,
              FL_Coord    my )
{
    FLI_POSITIONER_SPEC * sp = obj->spec;
    FL_Coord absbw = FL_abs( obj->bw );
    FL_Coord x1 = obj->x + absbw + 1,
             y1 = obj->y + absbw + 1;
    FL_Coord w1 = obj->w - 2 * absbw - 2,
             h1 = obj->h - 2 * absbw - 2;
    double oldx = sp->xval,
           oldy = sp->yval;

    sp->xval = flinear( mx, x1, x1 + w1 - 1.0, sp->xmin, sp->xmax );
    sp->yval = flinear( my, y1 + h1 - 1.0, y1, sp->ymin, sp->ymax );

    if ( sp->xstep != 0.0 )
        sp->xval = ( ( int ) ( sp->xval / sp->xstep + 0.5 ) ) * sp->xstep;
    if ( sp->ystep != 0.0 )
        sp->yval = ( ( int ) ( sp->yval / sp->ystep + 0.5 ) ) * sp->ystep;

    /* Make sure the position is within bounds */

    sp->xval = fli_clamp( sp->xval, sp->xmin, sp->xmax );
    sp->yval = fli_clamp( sp->yval, sp->ymin, sp->ymax );

    if ( sp->xval != oldx || sp->yval != oldy )
    {
        sp->partial = 1;
        sp->lxval = oldx;
        sp->lyval = oldy;
        fl_redraw_object( obj );

        if ( ! ( obj->how_return & FL_RETURN_END_CHANGED ) )
            return FL_RETURN_CHANGED;
    }

    return FL_RETURN_NONE;
}


/***************************************
 * Handles an event
 ***************************************/

static int
handle_positioner( FL_OBJECT * obj,
                   int         event,
                   FL_Coord    mx,
                   FL_Coord    my,
                   int         key  FL_UNUSED_ARG,
                   void      * ev   FL_UNUSED_ARG )
{
    FLI_POSITIONER_SPEC *sp = obj->spec;
    int ret = FL_RETURN_NONE;

    switch ( event )
    {
        case FL_DRAW:
            if ( obj->type != FL_INVISIBLE_POSITIONER )
                draw_positioner( obj );
            sp->partial = 0;
            break;

        case FL_DRAWLABEL:
            fl_draw_object_label_outside( obj );
            break;

        case FL_PUSH:
            if (    key < FL_MBUTTON1
                 || key > FL_MBUTTON5
                 || ! sp->react_to[ key - 1 ] )
            {
                fli_int.pushobj = NULL;
                break;
            }

            sp->mousebut = key;
            sp->old_x = sp->xval;
            sp->old_y = sp->yval;
            /* fall through */

        case FL_MOTION:
            ret = handle_mouse( obj, mx, my );
            break;

        case FL_RELEASE:
            if ( sp->mousebut != key )
            {
                fli_int.pushobj = obj;
                break;
            }

            ret = FL_RETURN_END;
            if (    obj->how_return & FL_RETURN_END_CHANGED
                 && ( sp->xval != sp->old_x || sp->yval != sp->old_y ) )
                 ret |= FL_RETURN_CHANGED;
            break;

        case FL_FREEMEM:
            fl_free( obj->spec );
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
    FL_OBJECT * obj;
    FLI_POSITIONER_SPEC * sp;
    int i;

    obj = fl_make_object( FL_POSITIONER, type, x, y, w, h, label,
                          handle_positioner );
    obj->boxtype = FL_POSITIONER_BOXTYPE;
    obj->col1    = FL_POSITIONER_COL1;
    obj->col2    = FL_POSITIONER_COL2;
    obj->align   = FL_POSITIONER_ALIGN;
    obj->lcol    = FL_POSITIONER_LCOL;

    if (    obj->type == FL_OVERLAY_POSITIONER
         || obj->type == FL_INVISIBLE_POSITIONER )
    {
        obj->bw = 0;
        obj->boxtype = FL_NO_BOX;
    }

    obj->spec = sp = fl_calloc( 1, sizeof *sp );

    sp->xmin = 0.0;
    sp->ymin = 0.0;
    sp->xmax = 1.0;
    sp->ymax = 1.0;
    sp->xval = sp->lxval = 0.5;
    sp->yval = sp->lyval = 0.5;

    /* Per default a positioner reacts to the left mouse button only */

    sp->react_to[ 0 ] = 1;
    for ( i = 1; i < 5; i++ )
        sp->react_to[ i ] = 0;

    fl_set_object_return( obj, FL_RETURN_CHANGED );

    return obj;
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
fl_set_positioner_xvalue(FL_OBJECT * obj,
                         double      val)
{
    FLI_POSITIONER_SPEC * sp = obj->spec;

    val = fli_clamp( val, sp->xmin, sp->xmax );

    if ( sp->xval != val )
    {
        sp->lxval = sp->xval;
        sp->xval = val;
        fl_redraw_object( obj );
    }
}


/***************************************
 ***************************************/

void
fl_set_positioner_yvalue( FL_OBJECT * obj,
                          double      val )
{
    FLI_POSITIONER_SPEC * sp = obj->spec;

    val = fli_clamp( val, sp->ymin, sp->ymax );

    if ( sp->yval != val )
    {
        sp->lyval = sp->yval;
        sp->yval = val;
        fl_redraw_object( obj );
    }
}


/***************************************
 ***************************************/

void
fl_set_positioner_xbounds( FL_OBJECT * obj,
                           double      min,
                           double      max )
{
    FLI_POSITIONER_SPEC * sp = obj->spec;

    if ( sp->xmin != min || sp->xmax != max )
    {
        sp->xmin = min;
        sp->xmax = max;
        sp->xval = fli_clamp( sp->xval, sp->xmin, sp->xmax );
        fl_redraw_object( obj );
    }
}


/***************************************
 ***************************************/

void
fl_set_positioner_ybounds( FL_OBJECT * obj,
                           double      min,
                           double     max )
{
    FLI_POSITIONER_SPEC * sp = obj->spec;

    if ( sp->ymin != min || sp->ymax != max )
    {
        sp->ymin = min;
        sp->ymax = max;
        sp->yval = fli_clamp( sp->yval, sp->ymin, sp->ymax );
        fl_redraw_object( obj );
    }
}


/***************************************
 ***************************************/

double
fl_get_positioner_xvalue( FL_OBJECT * obj )
{
    return ( ( FLI_POSITIONER_SPEC * ) obj->spec )->xval;
}


/***************************************
 ***************************************/

double
fl_get_positioner_yvalue( FL_OBJECT * obj )
{
    return ( ( FLI_POSITIONER_SPEC * ) obj->spec )->yval;
}


/***************************************
 ***************************************/

void
fl_get_positioner_xbounds( FL_OBJECT * obj,
                           double    * min,
                           double    * max )
{
    *min = ( ( FLI_POSITIONER_SPEC * ) obj->spec)->xmin;
    *max = ( ( FLI_POSITIONER_SPEC * ) obj->spec)->xmax;
}

void
fl_get_positioner_ybounds( FL_OBJECT * obj,
                           double    * min,
                           double    * max)
{
    *min = ( ( FLI_POSITIONER_SPEC * ) obj->spec)->ymin;
    *max = ( ( FLI_POSITIONER_SPEC * ) obj->spec)->ymax;
}


/***************************************
 * Sets the step size to which values are rounded.
 ***************************************/

void
fl_set_positioner_xstep( FL_OBJECT * obj,
                         double      value )
{
    ( ( FLI_POSITIONER_SPEC * ) obj->spec )->xstep = value;
}


/***************************************
 * Sets the step size to which values are rounded.
 ***************************************/

void
fl_set_positioner_ystep( FL_OBJECT * obj,
                         double      value )
{
    ( ( FLI_POSITIONER_SPEC * ) obj->spec )->ystep = value;
}


/***************************************
 * Sets under which conditions the object is to be returned to the
 * application. This function should be regarded as deprecated and
 * fl_set_object_return() should be used instead.
 * Please note that this function doesn't work like the other
 * object specific functions for setting the return policy!
 ***************************************/

void
fl_set_positioner_return( FL_OBJECT    * obj,
                          unsigned int   when )
{
    if ( when )
        fl_set_object_return( obj, FL_RETURN_CHANGED );
    else
        fl_set_object_return( obj, FL_RETURN_END );
}


/***************************************
 * Function allows to set up to which mouse
 * buttons the positioner object will react.
 ***************************************/

void
fl_set_positioner_mouse_buttons( FL_OBJECT    * obj,
                                 unsigned int   mouse_buttons )
{
    FLI_POSITIONER_SPEC *sp = obj->spec;
    unsigned int i;

    for ( i = 0; i < 5; i++, mouse_buttons >>= 1 )
        sp->react_to[ i ] = mouse_buttons & 1;
}


/***************************************
 * Function returns a value via 'mouse_buttons', indicating
 * which mouse buttons the positioner object will react to.
 ***************************************/

void
fl_get_positioner_mouse_buttons( FL_OBJECT    * obj,
                                 unsigned int * mouse_buttons )
{
    FLI_POSITIONER_SPEC *sp;
    int i;
    unsigned int k;

    if ( ! obj )
    {
        M_err( "fl_get_positioner_mouse_buttons", "NULL object" );
        return;
    }

    if ( ! mouse_buttons )
        return;

    sp = obj->spec;

    *mouse_buttons = 0;
    for ( i = 0, k = 1; i < 5; i++, k <<= 1 )
        *mouse_buttons |= sp->react_to[ i ] ? k : 0;
}


/***************************************
 * Returns the number of the last used mouse button.
 * fl_mouse_button will also return the mouse number
 ***************************************/

int
fl_get_positioner_numb( FL_OBJECT * obj )
{
    return ( ( FLI_POSITIONER_SPEC * ) obj->spec )->mousebut;
}


/*
 * Local variables:
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
