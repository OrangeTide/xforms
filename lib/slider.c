/*
 *
 *  This file is part of the XForms library package.
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


/**
 * \file slider.c
 *.
 *  This file is part of the XForms library package.
 *  Copyright (c) 1996-2002  T.C. Zhao and Mark Overmars
 *  All rights reserved.
 *.
 *
 * slider.c
 *
 */

#if defined F_ID || defined DEBUG
char *fl_id_slid = "$Id: slider.c,v 1.16 2008/12/27 22:20:51 jtt Exp $";
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include/forms.h"
#include "flinternal.h"
#include "private/pslider.h"
#include <sys/types.h>
#include <stdlib.h>


#define IS_NORMAL( t )  (    t == FL_HOR_SLIDER            \
                          || t == FL_VERT_SLIDER )

#define IS_NICE( t )    (    t == FL_VERT_NICE_SLIDER      \
                          || t == FL_VERT_NICE_SLIDER2     \
                          || t == FL_HOR_NICE_SLIDER       \
                          || t == FL_HOR_NICE_SLIDER2 )


enum
{
    COMPLETE      = 0,
	FOCUS         = 1,
	SLIDER_MOTION = 2,
	SLIDER_JUMP   = 4
};


#define VAL_BOXW   FL_max( 35, 0.18 * ob->w )	/* reporting boxsize */
#define VAL_BOXH   25		                    /* vertical RBW      */

static FLI_SCROLLBAR_KNOB osb;
static FLI_SCROLLBAR_KNOB slb;


/***************************************
* due to reporting box need to compute bounds
 ***************************************/

static void
compute_bounds( FL_OBJECT * ob )
{
    FLI_SLIDER_SPEC *sp = ob->spec;

	sp->x = ob->x;
	sp->y = ob->y;
	sp->w = ob->w;
	sp->h = ob->h;

    if ( ob->objclass == FL_VALSLIDER )
    {
		if ( IS_VSLIDER( ob->type ) )
		{
			sp->y += VAL_BOXH;
			sp->h -= VAL_BOXH;
		}
		else if ( IS_HSLIDER( ob->type ) )
		{
			sp->x += VAL_BOXW;
			sp->w -= VAL_BOXW;
		}
    }
}


/***************************************
 * reduce flicker by not painting the location the slider is going to be
 ***************************************/

static void
draw_motion( FL_OBJECT * ob )
{
    FLI_SLIDER_SPEC *sp = ob->spec;
    XRectangle xrec[ 2 ];
    int abbw = FL_abs( ob->bw );
    FL_COLOR col;

    if (    ob->type != FL_VERT_BROWSER_SLIDER2
		 && ob->type != FL_VERT_THIN_SLIDER
		 && ob->type != FL_HOR_BROWSER_SLIDER2
		 && ob->type != FL_HOR_THIN_SLIDER )
    {
		fli_calc_slider_size( ob, &slb );

		if ( IS_HSLIDER( ob->type ) )
		{
			xrec[ 0 ].x = sp->x;
			xrec[ 0 ].y = sp->y;
			xrec[ 0 ].width = slb.x - sp->x + 1;
			xrec[ 0 ].height = sp->h;

			xrec[ 1 ].x = slb.x + slb.w - 1;
			xrec[ 1 ].y = sp->y;
			xrec[ 1 ].width = sp->x + sp->w - 1;
			xrec[ 1 ].height = sp->h;
		}
		else
		{
			xrec[ 0 ].x = sp->x;
			xrec[ 0 ].y = sp->y;
			xrec[ 0 ].width = sp->w;
			xrec[ 0 ].height = slb.y - sp->y;

			xrec[ 1 ].x = sp->x;
			xrec[ 1 ].y = slb.y + slb.h - 1;
			xrec[ 1 ].width = sp->w;
			xrec[ 1 ].height = sp->y + sp->h - 1;
		}

		fl_set_clippings( xrec, 2 );
		fl_drw_box( FL_FLAT_BOX, sp->x + abbw, sp->y + abbw,
					sp->w - 2 * abbw, sp->h - 2 * abbw, ob->col1, 0 );
    }
    else if (    ob->type == FL_HOR_THIN_SLIDER
			  || ob->type == FL_VERT_THIN_SLIDER
			  || ob->type == FL_HOR_BASIC_SLIDER
			  || ob->type == FL_VERT_BASIC_SLIDER )
		fl_drw_box( FL_FLAT_BOX, sp->x, sp->y, sp->w, sp->h, ob->col1, 1 );
    else if (    ob->type == FL_HOR_BROWSER_SLIDER2
			  || ob->type == FL_VERT_BROWSER_SLIDER2 )
		fl_drw_box( ob->boxtype, sp->x, sp->y,
					sp->w, sp->h, ob->col1, ob->bw > 0 ? 1 : -1 );
    else
		fl_drw_box( FL_UP_BOX, sp->x, sp->y,
					sp->w, sp->h, ob->col1, ob->bw > 0 ? 1 : -1 );

    /* for slider jumps osb is NOT initialized */

    if ( IS_SCROLLBAR( ob->type ) && ! ( sp->draw_type & SLIDER_JUMP ) )
    {
		int knob_depth = IS_FLATBOX( ob->boxtype ) ? 1 : FL_max(abbw - 1, 1 );

		fl_drw_box( FL_DOWN_BOX, osb.x + 1, osb.y + 1, osb.w - 2, osb.h - 2,
					FL_INACTIVE, knob_depth > 2 ? 2 : knob_depth );
    }

    fl_unset_clipping( );

    col = ( IS_SCROLLBAR( ob->type ) && sp->mouse == FL_SLIDER_KNOB ) ?
		  FL_MCOL : ob->col2;
    fli_drw_slider( ob, ob->col1, col, "", FL_SLIDER_KNOB );
}


/***************************************
 * Draws a slider
 ***************************************/

static void
draw_slider( FL_OBJECT * ob )
{
    FLI_SLIDER_SPEC *sp = ob->spec;
    char valstr[ 64 ];
    double val;
    FL_Coord bx = ob->x,	/* value box */
		     by = ob->y,
		     bw = ob->w,
		     bh = ob->h;

    /* Draw the value box */

    if ( ob->objclass == FL_VALSLIDER )
    {
		if ( IS_VSLIDER( ob->type ) )
			bh = VAL_BOXH;
		else
			bw = VAL_BOXW;

		if ( sp->filter )
			strcpy( valstr, sp->filter( ob, sp->val, sp->prec ) );
		else
			sprintf( valstr, "%.*f", sp->prec, sp->val );

		fl_drw_box( ob->boxtype, bx, by, bw, bh, ob->col1, ob->bw );
		fl_drw_text_beside( FL_ALIGN_CENTER, bx, by, bw, bh,
							ob->lcol, ob->lstyle, ob->lsize, valstr );
    }

    if (    ( sp->draw_type == SLIDER_MOTION || sp->draw_type == SLIDER_JUMP )
		 && (    IS_SCROLLBAR( ob->type )
			  || IS_NORMAL( ob->type )
			  || IS_NICE( ob->type ) ) )
    {
		draw_motion( ob );
		return;
    }

    /* Draw the slider */

	val = sp->min == sp->max ?
		  0.5 : ( sp->val - sp->min ) / ( sp->max - sp->min );

    if ( ob->align == FL_ALIGN_CENTER )
    {
		fli_drw_slider( ob, ob->col1, ob->col2,
						IS_FILL( ob->type ) ? "" : ob->label,
						FL_SLIDER_ALL & ~sp->mouse );
		
        /* added 10/21/00 TCZ: need this to get the inside label right
		   otherwise fli_drw_slider() draw lable centered on the filled part!*/

        if ( IS_FILL( ob->type ) )
            fl_draw_object_label( ob );
    }
    else
    {
		fli_drw_slider( ob, ob->col1, ob->col2, "",
						FL_SLIDER_ALL & ~sp->mouse );
		fl_draw_object_label_outside( ob );
    }

    if ( sp->mouse != FL_SLIDER_NONE )
		fli_drw_slider( ob, ob->col1, sp->mouse ? FL_MCOL : ob->col2,
						"", sp->mouse );
}


/***************************************
 * get the value of the mouse position
 ***************************************/

static double
get_newvalue( FL_OBJECT    * ob,
			  FL_Coord       mx,
			  FL_Coord       my )
{
    FLI_SLIDER_SPEC *sp = ob->spec;
    double newval = 0.0;
	int absbw = FL_abs( ob->bw );

	if ( IS_HSLIDER( ob->type ) )
	{
		double dmx = mx + sp->offx - ob->x - sp->x;

		if ( dmx < 0.5 * sp->mw + absbw )
			newval = sp->min;
		else if ( dmx > sp->w - 0.5 * sp->mw - absbw )
			newval = sp->max;
		else
			newval = sp->min + ( sp->max - sp->min )
				     * ( dmx - 0.5 * sp->mw - absbw )
				     / ( sp->w - sp->mw - 2 * absbw );
	}
	else
	{
		double dmy = my + sp->offy - ob->y - sp->y;

		if ( dmy < 0.5 * sp->mh + absbw )
			newval = sp->min;
		else if ( dmy > sp->h - 0.5 * sp->mh - absbw )
			newval = sp->max;
		else
			newval = sp->min + ( sp->max - sp->min )
				     * ( dmy - 0.5 * sp->mh - absbw )
				     / ( sp->h - sp->mh - 2 * absbw );
	}

	return newval;
}


/***************************************
 ***************************************/

static void
scrollbar_timeout( int    val   FL_UNUSED_ARG,
				   void * data )
{
    ( ( FLI_SLIDER_SPEC * ) data )->timeout_id = -1;
}


/***************************************
 * Handle a mouse position change
 ***************************************/

static int
handle_mouse( FL_OBJECT    * ob,
			  FL_Coord       mx,
			  FL_Coord       my,
			  int            key,
	          unsigned int   state )
{
    FLI_SLIDER_SPEC *sp = ob->spec;
    double newval;

	if ( key == FL_MBUTTON4 )
		newval = sp->val - ( shiftkey_down( state ) ?
							 sp->rdelta : sp->ldelta / 2 );
	else if ( key == FL_MBUTTON5 )
		newval = sp->val + ( shiftkey_down( state ) ?
							 sp->rdelta : sp->ldelta / 2 );
    else if ( sp->mouse_pos )
    {
		if ( sp->timeout_id == -1 )
		{
			if ( key == FL_MBUTTON1 )
				newval = sp->val + sp->mouse_pos * sp->ldelta;
			else if ( key == FL_MBUTTON2 )
				newval = sp->val + sp->mouse_pos * sp->rdelta;
			else
				return 0;
		}
		else
			return 0;
    }
    else if ( key == FL_MBUTTON1 )
		newval = get_newvalue( ob, mx, my );
	else
		return 0;

	newval = fli_valuator_round_and_clamp( ob, newval );
	
    if ( sp->val == newval )
		return 0;

	/* When we're still doing jumps in a scrollbar restart the timer */

	if ( sp->mouse_pos )
		sp->timeout_id = fl_add_timeout( sp->repeat_ms, scrollbar_timeout, sp );

	sp->val = newval;
	sp->norm_val = sp->min == sp->max ?
		           0.5 : ( sp->val - sp->min ) / ( sp->max - sp->min );
	sp->draw_type = sp->mouse_pos ? SLIDER_JUMP : SLIDER_MOTION;
	fl_redraw_object( ob );

	return 1;
}


/***************************************
 * Handles an event
 ***************************************/

static int
handle_it( FL_OBJECT * ob,
		   int         event,
		   FL_Coord    mx,
		   FL_Coord    my,
		   int         key,
		   void      * ev )
{
    FLI_SLIDER_SPEC *sp = ob->spec;
	static FL_Coord mx_start,
		            my_start;
	int ret;
	static int was_shift = 0;

    switch ( event )
    {
		case FL_DRAW:
			compute_bounds( ob );
			ob->align &= ~ FL_ALIGN_INSIDE;
			sp->draw_type = COMPLETE;
			draw_slider( ob );
			break;

		case FL_DRAWLABEL:
			fl_draw_object_label_outside( ob );
			break;

		case FL_ENTER:
			/* When an scrollbar is entered we want to keep track of the mouse
			   movements in order to be able to highlight the knob when the
			   mouse is on top of it ('sp->mouse' keeps track of that). */

			if ( IS_SCROLLBAR( ob->type ) )
			{
				ob->want_motion = 1;
				sp->mouse_pos = fli_slider_mouse_object( ob, mx, my );
				if ( sp->mouse_pos == ON_TOP_OF_KNOB )
				{
					sp->mouse = FL_SLIDER_KNOB;
					fl_redraw_object( ob );
				}
			}
			break;

		case FL_LEAVE:
			/* When the mouse leaves a scrollbar we no longer need reports
			   about mouse movements and may have to un-highlight the knob */

			if ( IS_SCROLLBAR( ob->type ) )
			{
				ob->want_motion = 0;
				if ( sp->mouse == FL_SLIDER_KNOB )
				{
					sp->mouse = FL_SLIDER_NONE;
					fl_redraw_object( ob );
				}
			}
			break;

		case FL_MOTION:
			/* If this is a motion while in "jumping mode" for a scrollbar
			   don't do anything, just continue jumping in the same
			   direction */

			if (    IS_SCROLLBAR( ob->type )
				 && sp->mouse_pos != ON_TOP_OF_KNOB
				 && key )
				 break;

			/* If we get here even though the left mouse button isn't pressed
			   we're monitoring the mouse movements to change hightlighting of
			   the knob of a scrollbar if necessary. Test if highlighting has
			   to be switched on or off */

			if ( key != FL_MBUTTON1 )
			{
				int old_state = sp->mouse_pos;

				sp->mouse_pos = fli_slider_mouse_object( ob, mx, my );
				if ( old_state != sp->mouse_pos )
				{
					if ( sp->mouse_pos == ON_TOP_OF_KNOB )
						sp->mouse = FL_SLIDER_KNOB;
					else
						sp->mouse = FL_SLIDER_NONE;

					fl_redraw_object( ob );
				}

				break;
			}

			/* Otherwise the left mouse button is pressed and we need to
			   update the sliders position - if a shift key is pressed
			   fake a smaller mouse movement */

			if (    ! IS_SCROLLBAR( ob->type ) )
			{
				if ( shiftkey_down( ( ( XEvent * ) ev )->xmotion.state ) )
				{
					if ( ! was_shift )
					{
						mx_start = mx;
						my_start = my;
						was_shift = 1;
					}

					if ( IS_HSLIDER( ob->type ) )
						mx = mx_start + ( mx - mx_start ) * FL_SLIDER_FINE;
					else
						my = my_start + ( my - my_start ) * FL_SLIDER_FINE;
				}
				else
					was_shift = 0;
			}

			if (    handle_mouse( ob, mx, my, key,
							   ( ( XEvent * ) ev )->xmotion.state )
				 && sp->how_return == FL_RETURN_CHANGED )
			{
				sp->start_val = sp->val;
				return 1;
			}
			else if ( sp->how_return == FL_RETURN_ALWAYS )
				return 1;
			break;

		case FL_PUSH:
			if ( key != FL_MBUTTON1 && key != FL_MBUTTON2 )
				break;

			sp->start_val = sp->val;
			sp->timeout_id = -1;
			sp->offx = sp->offy = 0;

			/* For value sliders we do not want to jump the slider to one of
			   the extreme positions just because the user clicked on the
			   number field - they may just be trying if it's possible to
			   edit the number... */

			if ( ob->objclass == FL_VALSLIDER
				 && (    ( IS_HSLIDER( ob->type ) && mx < ob->x + sp->x )
					  || ( IS_VSLIDER( ob->type ) && my < ob->y + sp->y ) ) )
				break;

			/* Check were the mouse button was clicked */

			sp->mouse_pos = fli_slider_mouse_object( ob, mx, my );

			/* If the object is a scrollbar and the mouse is on its knob
			   nothing happens yet and we're just going to wait for mouse
			   movements. If it's not on the knon we need articfical timer
			   events to make the knob jump. For non-scrollbars we're going
			   to jump the slider so the mouse will be on top of the
			   "knob" and will stay there (and we will get updates about
			   mouse movements via FL_MOTION events). */

			if ( IS_SCROLLBAR( ob->type ) )
			{
				if ( sp->mouse_pos == ON_TOP_OF_KNOB )	
					break;
				ob->want_update = 1;
				fl_add_timeout( sp->repeat_ms, scrollbar_timeout, sp );
			}
			else
				sp->mouse_pos = ON_TOP_OF_KNOB;

			/* If we got here the slider position got to be changed,
			   for scrollbars by a first jump, for normal sliders by
			   moving the slider to the current mouse postion. We then
			   need to record the position for faked slwoing of the
			   mouse */

			ret = handle_mouse( ob, mx, my, key,
								( ( XEvent * ) ev )->xbutton.state );

			/* If a shift key is pressed record the mouse position */

			if ( shiftkey_down( ( ( XEvent * ) ev )->xbutton.state ) )
			{
				mx_start = mx;
				my_start = my;
				was_shift = 1;
			}

			if ( ret && sp->how_return == FL_RETURN_CHANGED )
			{
				sp->start_val = sp->val;
				return 1;
			}
			else if ( sp->how_return == FL_RETURN_ALWAYS )
				return 1;
			break;

		case FL_UPDATE:
			if (    handle_mouse( ob, mx, my, key,
							   ( ( XEvent * ) ev )->xmotion.state )
				 && sp->how_return == FL_RETURN_CHANGED )
			{
				sp->start_val = sp->val;
				return 1;
			}
			else if ( sp->how_return == FL_RETURN_ALWAYS )
				return 1;
			break;

		case FL_RELEASE:
			if ( sp->timeout_id != -1 )
			{
				fl_remove_timeout( sp->timeout_id );
				sp->timeout_id = -1;
			}

			ob->want_update = 0;

			/* For vertical scrollbars we also accept the scroll wheel
			   "buttons" */

			if ( IS_SCROLLBAR( ob->type ) && key != FL_MBUTTON3 )
			{
				int old_mouse_pos = sp->mouse_pos;

				if (    IS_VSLIDER( ob->type )
					 && ( key == FL_MBUTTON4 || key == FL_MBUTTON5 ) )
				{
					sp->start_val = sp->val;
					handle_mouse( ob, mx, my, key,
								  ( ( XEvent * ) ev )->xbutton.state );
				}

				if ( ( sp->mouse_pos = fli_slider_mouse_object( ob, mx, my ) )
					                                          != old_mouse_pos )
				{
					if ( sp->mouse_pos == ON_TOP_OF_KNOB )
						sp->mouse = FL_SLIDER_KNOB;
					else
						sp->mouse = FL_SLIDER_NONE;

					fl_redraw_object( ob );
				}

				osb = slb;
			}

			if (    sp->how_return == FL_RETURN_END
				 || sp->how_return == FL_RETURN_ALWAYS )
				return 1;

			if (    ( sp->start_val != sp->val )
				 && (    sp->how_return == FL_RETURN_CHANGED
					  || sp->how_return == FL_RETURN_END_CHANGED ) )
				return 1;
			break;

		case FL_FREEMEM:
			fl_free( ob->spec );
			break;
    }

    return 0;
}


/***************************************
 * creates an object
 ***************************************/

static FL_OBJECT *
create_it( int          objclass,
		   int          type,
		   FL_Coord     x,
		   FL_Coord     y,
		   FL_Coord     w,
		   FL_Coord     h,
		   const char * label )
{
    FL_OBJECT *ob;
    FLI_SLIDER_SPEC *sp;

    ob = fl_make_object( objclass, type, x, y, w, h, label, handle_it );
    ob->boxtype = FL_SLIDER_BOXTYPE;
    ob->col1    = FL_SLIDER_COL1;
    ob->col2    = FL_SLIDER_COL2;
    ob->align   = FL_SLIDER_ALIGN;
    ob->lcol    = FL_SLIDER_LCOL;
    ob->lsize   = FL_TINY_SIZE;

    ob->spec_size  = sizeof *sp;
    sp = ob->spec  = fl_calloc( 1, sizeof *sp );
	sp->min        = 0.0;
    sp->max        = 1.0;
    sp->val        = sp->start_val = sp->norm_val = 0.5;
    sp->filter     = NULL;
    sp->slsize     = FL_SLIDER_WIDTH;
    sp->prec       = 2;
    sp->how_return = FL_RETURN_CHANGED;
    sp->repeat_ms  = 100;
    sp->timeout_id = -1;
    sp->mouse_pos  = 0;
    if ( IS_SCROLLBAR( ob->type ) )
		sp->slsize *= 1.5;

    fl_set_object_dblbuffer( ob, 1	/* IS_FILL(ob->type) ||
									   IS_NICE(ob->type) */ );
    return ob;
}


/***************************************
 * Adds an object
 ***************************************/

static FL_OBJECT *
add_it( int          objclass,
		int          type,
		FL_Coord     x,
		FL_Coord     y,
		FL_Coord     w,
		FL_Coord     h,
		const char * label )
{
    FL_OBJECT *ob = create_it( objclass, type, x, y, w, h, label );

    fl_add_object( fl_current_form, ob );
    return ob;
}


/***************************************
 ***************************************/

FL_OBJECT *
fl_create_slider( int          type,
				  FL_Coord     x,
				  FL_Coord     y,
				  FL_Coord     w,
				  FL_Coord     h,
				  const char * label )
{
    return create_it( FL_SLIDER, type, x, y, w, h, label );
}


/***************************************
 ***************************************/

FL_OBJECT *
fl_add_slider( int          type,
			   FL_Coord     x,
			   FL_Coord     y,
			   FL_Coord     w,
			   FL_Coord     h,
			   const char * label )
{
    return add_it( FL_SLIDER, type, x, y, w, h, label );
}


/***************************************
 ***************************************/

FL_OBJECT *
fl_create_valslider( int          type,
					 FL_Coord     x,
					 FL_Coord     y,
					 FL_Coord     w,
					 FL_Coord     h,
					 const char * label )
{
    return create_it( FL_VALSLIDER, type, x, y, w, h, label );
}


/***************************************
 ***************************************/

FL_OBJECT *
fl_add_valslider( int          type,
				  FL_Coord     x,
				  FL_Coord     y,
				  FL_Coord     w,
				  FL_Coord     h,
				  const char * label )
{
    return add_it( FL_VALSLIDER, type, x, y, w, h, label );
}


/***************************************
 ***************************************/

void
fl_set_slider_value( FL_OBJECT * ob,
					 double      val )
{
    FLI_SLIDER_SPEC *sp;
    double smin,
		   smax;

#if FL_DEBUG >= ML_ERR
    if ( ! IsValidClass( ob, FL_SLIDER ) && ! IsValidClass( ob, FL_VALSLIDER ) )
    {
		M_err( "fl_set_slider_value", "%s is not a slider",
			   ob ? ob->label : "" );
		return;
    }
#endif

    sp = ob->spec;
    smin = FL_min( sp->min, sp->max );
    smax = FL_max( sp->min, sp->max );
    val = FL_clamp( val, smin, smax );

    if ( sp->val != val )
    {
		sp->val = val;
		sp->norm_val = sp->min == sp->max ?
			           0.5 : ( sp->val - sp->min ) / ( sp->max - sp->min );
		fl_redraw_object( ob );
    }
}


/***************************************
 ***************************************/

void
fl_set_slider_bounds( FL_OBJECT * ob,
					  double      min,
					  double      max )
{
    FLI_SLIDER_SPEC *sp;

#if FL_DEBUG >= ML_ERR
    if ( ! IsValidClass( ob, FL_SLIDER ) && ! IsValidClass( ob, FL_VALSLIDER ) )
    {
		M_err( "fl_set_slider_bounds", "%s is not a slider",
			   ob ? ob->label : "" );
		return;
    }
#endif

    sp = ob->spec;
    if ( sp->min == min && sp->max == max )
		return;

	sp->min = min;
	sp->max = max;
	if ( sp->val < sp->min && sp->val < sp->max )
		sp->val = FL_min( sp->min, sp->max );
	if ( sp->val > sp->min && sp->val > sp->max )
		sp->val = FL_max( sp->min, sp->max );

	sp->norm_val = ( sp->min == sp->max ) ?
		           0.5 : ( sp->val - sp->min ) / ( sp->max - sp->min );

	fl_redraw_object( ob );
}


/***************************************
 * Returns value of the slider
 ***************************************/

double
fl_get_slider_value( FL_OBJECT * ob )
{
#if FL_DEBUG >= ML_ERR
    if ( ! IsValidClass( ob, FL_SLIDER ) && ! IsValidClass( ob, FL_VALSLIDER ) )
    {
		M_err( "GetSliderValue", "%s is not a slider", ob ? ob->label : "" );
		return 0;
    }
#endif
    return ( ( FLI_SLIDER_SPEC * ) ob->spec )->val;
}


/***************************************
 * Returns the slider bounds
 ***************************************/

void
fl_get_slider_bounds( FL_OBJECT * ob,
					  double *    min,
					  double *    max )
{
    *min = ( ( FLI_SLIDER_SPEC * ) ob->spec )->min;
    *max = ( ( FLI_SLIDER_SPEC * ) ob->spec )->max;
}


/***************************************
 * Sets whether to return value all the time
 ***************************************/

void
fl_set_slider_return( FL_OBJECT * ob,
					  int         value )
{
    ( ( FLI_SLIDER_SPEC * ) ob->spec )->how_return = value;
}


/***************************************
 * Sets the step size to which values are rounded
 ***************************************/

void
fl_set_slider_step( FL_OBJECT * ob,
				    double      value )
{
    ( ( FLI_SLIDER_SPEC * ) ob->spec )->step = value;
}


/***************************************
 * Set slider incrments for clicks with left and middle mouse button
 ***************************************/

void
fl_set_slider_increment( FL_OBJECT * ob,
						 double      l,
						 double      r )
{
    ( ( FLI_SLIDER_SPEC * ) ob->spec )->ldelta = l;
    ( ( FLI_SLIDER_SPEC * ) ob->spec )->rdelta = r;
}


/***************************************
 ***************************************/

void
fl_get_slider_increment( FL_OBJECT * ob,
						 double *    l,
						 double *    r )
{
    *l = ( ( FLI_SLIDER_SPEC * ) ob->spec )->ldelta;
    *r = ( ( FLI_SLIDER_SPEC * ) ob->spec )->rdelta;
}


/***************************************
 * Sets the portion of the slider box covered by the slider
 ***************************************/

void
fl_set_slider_size( FL_OBJECT * ob,
					double      size )
{
    FLI_SLIDER_SPEC *sp = ob->spec;
	double psize,
	 	   dim;
	int min_knob = IS_SCROLLBAR( ob->type ) ? MINKNOB_SB : MINKNOB_SL;

    if ( size <= 0.0 )
		size = 0.0;
    else if (size >= 1.0)
		size = 1.0;

    /* Impose min knob size limit */

	dim = IS_VSLIDER( ob->type ) ? ob->h : ob->w;
	dim -= 2 * FL_abs( ob->bw );
	psize = dim * size;
	if ( psize < min_knob && dim > 0.0 )
		size = min_knob / dim;
		
    if ( size != sp->slsize )
    {
		sp->slsize = size;
		fl_redraw_object( ob );
    }
}


/***************************************
 * Only for value sliders.
 ***************************************/

void
fl_set_slider_precision( FL_OBJECT * ob,
						 int         prec )
{
    FLI_SLIDER_SPEC *sp = ob->spec;


	if ( prec > 10 )
		prec = 10;

    if ( sp->prec != prec )
    {
		sp->prec = prec;
		fl_redraw_object( ob );
    }
}


/***************************************
 ***************************************/

void
fl_set_slider_filter( FL_OBJECT *   ob,
					  FL_VAL_FILTER filter )
{
    ( ( FLI_SLIDER_SPEC * ) ob->spec )->filter = filter;
}


/***************************************
 ***************************************/

int fl_get_slider_repeat( FL_OBJECT * ob )
{
    return ( ( FLI_SLIDER_SPEC * ) ob->spec )->repeat_ms;
}


/***************************************
 ***************************************/

void fl_set_slider_repeat( FL_OBJECT * ob,
                           int         millisec )
{
	if ( millisec > 0 )
		( ( FLI_SLIDER_SPEC * ) ob->spec )->repeat_ms = millisec;
}
