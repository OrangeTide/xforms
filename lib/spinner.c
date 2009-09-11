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


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include/forms.h"
#include "flinternal.h"
#include "private/pspinner.h"

#include <string.h>
#include <stdlib.h>
#include <float.h>


static void
set_spinner_return( FL_OBJECT *,
                    int );


/***************************************
 * This function got to be called before a redraw, at least if
 * the form the spinner belongs to has been resized. It calculates
 * the new positions and sizes of the objectes the spinner widget
 * is made up from.
 ***************************************/

static void
set_geom( FL_OBJECT * obj )
{
    FLI_SPINNER_SPEC *sp = obj->spec;
    FL_Coord bwh;

    if ( obj->w >= obj->h )
    {
        bwh = obj->h / 2;
        bwh = FL_max( bwh, 1 );
        obj->h = 2 * bwh;

        sp->input->x = obj->x;
        sp->input->y = obj->y;
        sp->input->w = obj->w - bwh - 1;
        sp->input->h = obj->h;

        sp->up->x = sp->down->x = obj->x + obj->w - bwh - 1;
        sp->up->y = obj->y;
        sp->down->y = obj->y + bwh;
        sp->up->w = sp->up->h = sp->down->w = sp->down->h = bwh;

        if ( sp->orient == 1 )
        {
            sp->orient = 0;
            fl_set_object_label( sp->up,   "@8>" );
            fl_set_object_label( sp->down, "@2>" );
        }
    }
    else
    {
        bwh = obj->w / 2;
        bwh = FL_max( bwh, 1 );
        obj->w = sp->input->w = 2 * bwh;

        sp->input->x = obj->x;
        sp->input->y = obj->y;
        sp->input->w = obj->w;
        sp->input->h = obj->h - bwh - 1;

        sp->up->y = sp->down->y = obj->y + obj->h - bwh - 1;
        sp->up->x = obj->x + bwh;
        sp->down->x = obj->x;
        sp->up->w = sp->up->h = sp->down->w = sp->down->h = bwh;

        if ( sp->orient == 0 )
        {
            sp->orient = 1;
            fl_set_object_label( sp->up,   "@6>" );
            fl_set_object_label( sp->down, "@4>" );
        }
    }
}


/***************************************
 * There aren't many types of events a spinner object must directly
 * react to. If it got resized the positions and sizes of the child
 * objects it's made up from need to be recalculated, but that can
 * be deferred until it's dran anew. Drawing it works mostly auto-
 * matically except the redraw of the label. And on deletion just
 * the FLI_SPINNER_SPEC needs to be deallocated (the child objects
 * referenced there-in have already been deallocated before)
 ***************************************/

static int
handle_spinner( FL_OBJECT * obj,
                int         event,
                FL_Coord    mx   FL_UNUSED_ARG,
                FL_Coord    my   FL_UNUSED_ARG,
                int         key  FL_UNUSED_ARG,
                void *      ev   FL_UNUSED_ARG )
{
    FLI_SPINNER_SPEC *sp = obj->spec;

    switch ( event )
    {
        case FL_ATTRIB :
        case FL_RESIZED :
            sp->attrib = 1;
            break;

        case FL_DRAW :
            if ( sp->attrib )
            {
                set_geom( obj );
                sp->attrib = 0;
            }

        case FL_DRAWLABEL :
            fl_draw_object_label_outside( obj );
            break;

        case FL_FREEMEM :
            fl_safe_free( obj->spec );
            break;
    }

    return 0;
}


/***************************************
 ***************************************/

static void
spinner_callback( FL_OBJECT * obj,
                  long        data )
{
    FLI_SPINNER_SPEC *sp = obj->parent->spec;
    const char *s_val = fl_get_input( sp->input );
    int max_len = 4 + sp->prec + log10( DBL_MAX );
    char buf[ max_len ];

    /* Don't react to mere changes of the input field while it keeps focus */

    if ( data == 0 && ! ( obj->returned & FL_RETURN_END ) )
        return;

    if ( obj->parent->type == FL_INT_SPINNER )
    { 
        int old_i_val = sp->i_val;

        if ( data == 0 )
        {
            char *eptr;
            long i_val = strtol( s_val, &eptr, 10 );

            if ( eptr == s_val )
                /* empty */ ;
            else if ( i_val > sp->i_max )
                sp->i_val = sp->i_max;
            else if ( i_val < sp->i_min )
                sp->i_val = sp->i_min;
            else
                sp->i_val = i_val;
        }
        else if ( data == 1 && obj->returned & FL_RETURN_CHANGED )
        {
            if ( sp->i_val <= sp->i_max - sp->i_incr )
                sp->i_val += sp->i_incr;
            else
                sp->i_val = sp->i_max;
        }
        else if ( data == -1 && obj->returned & FL_RETURN_CHANGED )
        {
            if ( sp->i_val >= sp->i_min + sp->i_incr )
                sp->i_val -= sp->i_incr;
            else
                sp->i_val = sp->i_min;
        }

        sprintf( buf, "%d", sp->i_val );
        fl_set_input( sp->input, buf );

        if ( obj->returned & FL_RETURN_END )
            obj->parent->returned |= FL_RETURN_END;

        if ( sp->i_val != old_i_val )
            obj->parent->returned |= FL_RETURN_CHANGED;

        if (    sp->i_val != sp->old_ival
             && data != 0
             && obj->returned & FL_RETURN_END )
            obj->parent->returned |= FL_RETURN_CHANGED | FL_RETURN_END;

        if ( obj->parent->returned & FL_RETURN_END )
            sp->old_ival = sp->i_val;

        if ( obj->parent->how_return & FL_RETURN_END_CHANGED
             && ! (    obj->parent->returned & FL_RETURN_CHANGED
                    && obj->parent->returned & FL_RETURN_END ) )
            obj->parent->returned = FL_RETURN_NONE;
    }
    else
    {
        int old_f_val = sp->f_val;

        if ( data == 0 )
        {
            char *eptr;
            double f_val = strtod( s_val, &eptr );

            if ( eptr == s_val )
                /* empty */ ;
            else if ( f_val > sp->f_max )
                sp->f_val = sp->f_max;
            else if ( f_val < sp->f_min )
                sp->f_val = sp->f_min;
            else
                sp->f_val = f_val;
        }
        else if ( data == 1 && obj->returned & FL_RETURN_CHANGED )
        {
            if ( sp->f_val <= sp->f_max - sp->f_incr )
                sp->f_val += sp->f_incr;
            else
                sp->f_val = sp->f_max;
        }
        else if ( data == -1 && obj->returned & FL_RETURN_CHANGED )
        {
            if ( sp->f_val >= sp->f_min + sp->f_incr )
                sp->f_val -= sp->f_incr;
            else
                sp->f_val = sp->f_min;
        }

        sprintf( buf, "%.*f", sp->prec, sp->f_val );
        fl_set_input( sp->input, buf );

        if ( obj->returned & FL_RETURN_END )
            obj->parent->returned |= FL_RETURN_END;

        if ( sp->f_val != old_f_val )
            obj->parent->returned |= FL_RETURN_CHANGED;

        if (    sp->f_val != sp->old_fval
             && data != 0
             && obj->returned & FL_RETURN_END )
            obj->parent->returned |= FL_RETURN_CHANGED | FL_RETURN_END;

        if ( obj->parent->returned & FL_RETURN_END )
            sp->old_fval = sp->f_val;

        if ( obj->parent->how_return & FL_RETURN_END_CHANGED
             && ! (    obj->parent->returned & FL_RETURN_CHANGED
                    && obj->parent->returned & FL_RETURN_END ) )
            obj->parent->returned = FL_RETURN_NONE;
    }
}


/***************************************
 ***************************************/

FL_OBJECT *
fl_create_spinner( int          type,
                   FL_Coord     x,
                   FL_Coord     y,
                   FL_Coord     w,
                   FL_Coord     h,
                   const char * label )
{
    FL_OBJECT *obj;
    FLI_SPINNER_SPEC *sp;
    FL_Coord iw, ih,
             ux, uy,
             dx, dy,
             bwh;
    int orient;

    if ( w >= h )
    {
        orient = 0;
        bwh = h / 2;
        bwh = FL_max( bwh, 1 );
        h = ih = 2 * bwh;
        iw = w - bwh - 1;
        ux = dx = x + iw - 1;
        uy = x;
        dy = uy + bwh;
    }
    else
    {
        orient = 1;
        bwh = w / 2;
        bwh = FL_max( bwh, 1 );
        w = iw = 2 * bwh;
        ih = h - bwh - 1;
        dx = x;
        ux = dx + bwh;
        uy = dy = y + ih - 1;
    }

    obj = fl_make_object( FL_SPINNER, type, x, y, w, h, label, handle_spinner );
    obj->boxtype    = FL_NO_BOX;
    obj->align      = FL_ALIGN_LEFT;
    obj->set_return = set_spinner_return;
    obj->spec       = sp = malloc( sizeof *sp );

    sp->input = fl_create_input( type == FL_INT_SPINNER ?
                                 FL_INT_INPUT : FL_FLOAT_INPUT,
                                 0, 0, 0, 0, NULL );
    sp->up = fl_create_button( FL_TOUCH_BUTTON, 0, 0, 0, 0,
                               orient == 0 ? "@8>" : "@6>" );
    sp->down = fl_create_button( FL_TOUCH_BUTTON, 0, 0, 0, 0,
                                 orient == 0 ? "@2>" : "@4>" );

    fl_set_object_callback( sp->input, spinner_callback,  0 );
    fl_set_object_callback( sp->up,    spinner_callback,  1 );
    fl_set_object_callback( sp->down,  spinner_callback, -1 );

    fl_set_button_mouse_buttons( sp->up,   1 );
    fl_set_button_mouse_buttons( sp->down, 1 );

    fl_set_object_lcolor( sp->up,   FL_BLUE );
    fl_set_object_lcolor( sp->down, FL_BLUE );

    sp->i_val  = sp->old_ival = 0;
    sp->i_min  = - 10000;
    sp->i_max  = 10000;
    sp->i_incr = 1;

    sp->f_val  = sp->old_fval = 0.0;
    sp->f_min  = - 10000.0;
    sp->f_max  = 10000.0;
    sp->f_incr = 1.0;

    sp->orient = orient;
    sp->prec   = DEFAULT_SPINNER_PRECISION;
    sp->attrib = 1;

    fl_add_child( obj, sp->input );
    fl_add_child( obj, sp->up );
    fl_add_child( obj, sp->down );

    fl_set_input( sp->input, type == FL_INT_SPINNER ? "0" : "0.0" );

    /* Set default return policy for a spinner object */

    fl_set_object_return( obj, FL_RETURN_CHANGED );

    return obj;
}


/***************************************
 ***************************************/

FL_OBJECT *
fl_add_spinner( int          type,
                FL_Coord     x,
                FL_Coord     y,
                FL_Coord     w,
                FL_Coord     h,
                const char * label )
{
    FL_OBJECT *obj = fl_create_spinner( type, x, y, w, h, label );

    fl_add_object( fl_current_form, obj );

    return obj;
}


/***************************************
 ***************************************/

double
fl_get_spinner_value( FL_OBJECT * obj )
{
    FLI_SPINNER_SPEC *sp = obj->spec;
    const char *s_val = fl_get_input( sp->input );
    char *eptr;

    if ( obj->type == FL_INT_SPINNER )
    { 
        long i_val = strtol( s_val, &eptr, 10 );

        if ( eptr == s_val || i_val > sp->i_max || i_val < sp->i_min )
            i_val = sp->i_val;

        return sp->i_val = i_val;
    }
    else
    {
        double f_val = strtod( s_val, &eptr );

        if (    ( *eptr && *eptr != 'e' && *eptr != 'E' )
             || errno == ERANGE
             || f_val > sp->f_max
             || f_val < sp->f_min )
            f_val = sp->f_val;
        
        if ( *eptr )
        {
            int max_len = 4 + sp->prec + log10( DBL_MAX );
            char buf[ max_len ];

            sprintf( buf, "%.*f", sp->prec, f_val );
            fl_set_input( sp->input, buf );
        }

        return sp->f_val = f_val;
    }
}


/***************************************
 ***************************************/

double
fl_set_spinner_value( FL_OBJECT * obj,
                      double      val )
{
    FLI_SPINNER_SPEC *sp = obj->spec;
    int max_len = 4 + sp->prec + log10( DBL_MAX );
    char buf[ max_len ];

    if ( obj->type == FL_INT_SPINNER )
    {
        sp->i_val = FL_nint( val );

        if ( val > sp->i_max )
            sp->i_val = sp->i_max;
        else if ( val < sp->i_min )
            sp->i_val = sp->i_min;

        sprintf( buf, "%d", sp->i_val );
        fl_set_input( sp->input, buf );

        return sp->old_ival = sp->i_val;
    }

    sp->f_val = val;

    if ( val > sp->f_max )
        sp->f_val = sp->f_max;
    else if ( val < sp->f_min )
        sp->f_val = sp->f_min;

    sprintf( buf, "%.*f", sp->prec, sp->f_val );
    fl_set_input( sp->input, buf );

    return sp->old_fval = sp->f_val;
}


/***************************************
 ***************************************/

void
fl_set_spinner_bounds( FL_OBJECT * obj,
                       double      min,
                       double      max )
{
    FLI_SPINNER_SPEC *sp = obj->spec;
    double tmp;

    if ( min > max )
    {
        tmp = min;
        min = max;
        max = min;
    }

    if ( obj->type == FL_INT_SPINNER )
    {
        sp->i_min = FL_nint( min );
        sp->i_max = FL_nint( max );

        if ( min < INT_MIN )
            sp->i_min = INT_MIN;
        else if ( min > INT_MAX )
            sp->i_min = INT_MAX;

        if ( max < INT_MIN )
            sp->i_max = INT_MIN;
        else if ( max > INT_MAX )
            sp->i_max = INT_MAX;

        fl_get_spinner_value( obj );
        fl_set_spinner_value( obj, sp->i_val );
    }
    else
    {
        sp->f_min = min;
        sp->f_max = max;

        if ( min < - DBL_MAX )
            sp->f_min = - DBL_MAX;
        else if ( min > DBL_MAX )
            sp->f_min = DBL_MAX;

        if ( max < - DBL_MAX )
            sp->f_max = - DBL_MAX;
        else if ( max > DBL_MAX )
            sp->f_max = DBL_MAX;

        fl_get_spinner_value( obj );
        fl_set_spinner_value( obj, sp->f_val );
    }
}


/***************************************
 ***************************************/

void
fl_get_spinner_bounds( FL_OBJECT * obj,
                       double    * min,
                       double    * max )
{
    FLI_SPINNER_SPEC *sp = obj->spec;

    if ( obj->type == FL_INT_SPINNER )
    {
        *min = sp->i_min;
        *max = sp->i_max;
    }
    else
    {
        *min = sp->f_min;
        *max = sp->f_max;
    }
}


/***************************************
 ***************************************/

void
fl_set_spinner_step( FL_OBJECT * obj,
                     double      step )
{
    FLI_SPINNER_SPEC *sp = obj->spec;

    if ( step <= 0.0 )
        return;

    if ( obj->type == FL_INT_SPINNER )
    {
        if ( FL_nint( step ) > sp->i_max - sp->i_min )
            sp->i_incr = sp->i_max - sp->i_min;
        else
            sp->i_incr = FL_nint( step );
    }
    else
    {
        if ( step > sp->f_max - sp->f_min )
            sp->f_incr = sp->f_max - sp->f_min;
        else
            sp->f_incr = step;
    }
}


/***************************************
 ***************************************/

double
fl_get_spinner_step( FL_OBJECT * obj )
{
    FLI_SPINNER_SPEC *sp = obj->spec;

    return obj->type == FL_INT_SPINNER ? sp->i_incr : sp->f_incr;
}


/***************************************
 ***************************************/

void
fl_set_spinner_precision( FL_OBJECT * obj,
                          int         prec )
{
    FLI_SPINNER_SPEC *sp = obj->spec;

    if ( obj->type == FL_INT_SPINNER || prec < 0 )
        return;

    if ( prec > DBL_DIG )
        prec = DBL_DIG;

    if ( sp->prec != prec )
    {
        sp->prec = prec;
        fl_set_spinner_value( obj, fl_get_spinner_value( obj ) );
    }
}


/***************************************
 ***************************************/

int
fl_get_spinner_precision( FL_OBJECT * obj )
{
    if ( obj->type == FL_INT_SPINNER )
        return 0;

    return ( ( FLI_SPINNER_SPEC * ) obj->spec )->prec;
}


/***************************************
 ***************************************/

FL_OBJECT *
fl_get_spinner_input( FL_OBJECT * obj )
{
    return ( ( FLI_SPINNER_SPEC * ) obj->spec )->input;
}


/***************************************
 ***************************************/

FL_OBJECT *
fl_get_spinner_up_button( FL_OBJECT * obj )
{
    return ( ( FLI_SPINNER_SPEC * ) obj->spec )->up;
}


/***************************************
 ***************************************/

FL_OBJECT *
fl_get_spinner_down_button( FL_OBJECT * obj )
{
    return ( ( FLI_SPINNER_SPEC * ) obj->spec )->down;
}


/***************************************
 * Sets under which conditions the object is to be returned to the
 * application. This function is for internal use only and to set
 * the return policy for a spinner fl_set_object_return() should
 * be used instead (which then will call this function).
 ***************************************/

static void
set_spinner_return( FL_OBJECT * obj,
                    int         when )
{
    FLI_SPINNER_SPEC *sp = obj->spec;

    obj->how_return = when;
    fl_set_object_return( sp->input, FL_RETURN_ALWAYS );
    fl_set_object_return( sp->up,   FL_RETURN_CHANGED | FL_RETURN_END );
    fl_set_object_return( sp->down, FL_RETURN_CHANGED | FL_RETURN_END );
}


/*
 * Local variables:
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
