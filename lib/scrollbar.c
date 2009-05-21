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
 * \file scrollbar.c
 *
 *  This file is part of the XForms library package.
 *  Copyright (c) 1996-2002  T.C. Zhao
 *  All rights reserved.
 *
 *  Scrollbar
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include/forms.h"
#include "flinternal.h"
#include "private/pscrollbar.h"


static void get_geom( FL_OBJECT * );

static void attrib_change( FL_OBJECT * );


#define IsVThin( t )  (    t == FL_VERT_THIN_SCROLLBAR    \
                        || t == FL_VERT_PLAIN_SCROLLBAR )
#define IsHThin( t )  (    t == FL_HOR_THIN_SCROLLBAR     \
						|| t == FL_HOR_PLAIN_SCROLLBAR )
#define IsThin( t )   ( IsVThin( t ) || IsHThin( t ) )


/***************************************
 ***************************************/

static int
handle( FL_OBJECT * ob,
		int         event,
		FL_Coord    mx   FL_UNUSED_ARG,
		FL_Coord    my   FL_UNUSED_ARG,
		int         key  FL_UNUSED_ARG,
		void      * ev   FL_UNUSED_ARG )
{
    switch ( event )
    {
		case FL_ATTRIB :
			attrib_change( ob );
			get_geom( ob );
			break;

		case FL_DRAW :
			attrib_change( ob );
			get_geom( ob );
			if ( IsThin( ob->type ) )
				fl_drw_box( ob->boxtype, ob->x, ob->y, ob->w, ob->h,
							ob->col1, ob->bw );
			/* fall through */

		case FL_DRAWLABEL :
			fl_draw_object_label_outside( ob );
			break;

		case FL_FREEMEM :
			/* children will take care of themselves */
			fl_free( ob->spec );
			break;
    }

    return 0;
}

#define is_horiz( t )   (    ( t ) == FL_HOR_SCROLLBAR         \
						  || ( t ) == FL_HOR_THIN_SCROLLBAR    \
						  || ( t ) == FL_HOR_NICE_SCROLLBAR    \
						  || ( t ) == FL_HOR_PLAIN_SCROLLBAR )


/***************************************
 ***************************************/

static void
attrib_change( FL_OBJECT * ob )
{
    FLI_SCROLLBAR_SPEC *sp = ob->spec;

    sp->slider->col1 = ob->col1;
    sp->slider->col2 = ob->col2;
    sp->up->col1 = sp->down->col1 = ob->col1;
    sp->up->col2 = sp->down->col2 = ob->col2;
    sp->up->boxtype = sp->down->boxtype = sp->slider->boxtype = ob->boxtype;
}


#define IS_FLATBOX( b )   (    ( b ) == FL_BORDER_BOX    \
						    || ( b ) == FL_FRAME_BOX     \
						    || ( b ) == FL_EMBOSSED_BOX  \
						    || ( b ) == FL_ROUNDED_BOX )


/***************************************
 ***************************************/

static void
get_geom( FL_OBJECT * ob )
{
    FLI_SCROLLBAR_SPEC *sp = ob->spec;
    FL_OBJECT *up     = sp->up,
		      *down   = sp->down,
		      *slider = sp->slider;
    int x = ob->x,
		y = ob->y,
		w = ob->w,
		h = ob->h;
    int absbw = FL_abs( ob->bw );
    int t = ob->type;

    if ( is_horiz( ob->type ) )
    {
		down->x = x;
		up->x = x + w - h;
		up->y = down->y = y;
		down->h = up->h = h;
		down->w = up->w = FL_min( w, h );

		slider->x = x + h;
		slider->y = y;
		slider->h = h;

		if ( ( slider->w = w - 2 * up->w ) < 0 )
		{
			slider->w = up->w / 3;
			slider->x = x + up->w / 3;
		}
    }
    else
    {
		up->x = down->x = x;
		up->y = y;
		up->w = down->w = w;
		up->h = down->h = FL_min( w, h );

		slider->x = x;
		slider->y = y + up->h;
		slider->w = w;

		if ( ( slider->h = h - 2 * up->h ) < 0 )
		{
			slider->h = h / 3;
			slider->y = y + up->h / 3;
		}

		down->y = y + h - down->h;
    }

    up->bw = ob->bw;
    down->bw = ob->bw;
    slider->bw = ob->bw;

    if ( absbw > 2 )
		absbw--;

    if ( ob->bw > 0 )
		up->bw = down->bw = absbw;
    else
		up->bw = down->bw = -absbw;

    if ( IsThin( t ) )
    {
		absbw = IS_FLATBOX( ob->boxtype ) ? 1 : absbw;

		up->boxtype = down->boxtype = FL_NO_BOX;
		up->bw = down->bw = absbw;

		/* Due to slider double buffering we  have to be completly clear of
		   the scrollbar bounding box, otherwise the slider will wipe out the
		   scrollbars bounding box */

		if ( IsVThin( t ) )
		{
			slider->x += absbw + 1;
			slider->w -= 2 * absbw + 2;
			slider->y -= absbw + ( absbw > 1 );
			slider->h += 2 * absbw + ( absbw > 1 );
		}
		else
		{
			slider->y += absbw + 1;
			slider->h -= 2 * absbw + 2;
			slider->x -= absbw + ( absbw > 1 );
			slider->w += 2 * absbw + ( absbw > 1 );
		}
    }
}


/***************************************
 ***************************************/

static void
slider_cb( FL_OBJECT * obj,
		   long        data  FL_UNUSED_ARG )
{
	FLI_SCROLLBAR_SPEC *sp = obj->parent->spec;
	double nval = fl_get_slider_value( obj );

	switch ( obj->parent->how_return )
	{
		case FL_RETURN_END_CHANGED :
			if (     obj->returned & FL_RETURN_END
			     && sp->old_val != nval )
			{
				obj->parent->returned = FL_RETURN_END_CHANGED;
				sp->old_val = nval;
			}
			break;

		case FL_RETURN_END :
			if ( obj->returned & FL_RETURN_END )
			{
				sp->old_val = nval;
				obj->parent->returned = FL_RETURN_END;
			}
			break;

		case FL_RETURN_CHANGED :
			if ( obj->returned & FL_RETURN_CHANGED )
			{
				sp->old_val = nval;
				obj->parent->returned = FL_RETURN_CHANGED;
			}
			break;

		case FL_RETURN_ALWAYS :
			sp->old_val = nval;
			obj->parent->returned = obj->returned;
			break;
	}
}


/***************************************
 ***************************************/

static void
up_cb( FL_OBJECT * obj,
	   long        data  FL_UNUSED_ARG )
{
    FLI_SCROLLBAR_SPEC *sp = obj->parent->spec;
	double nval,
           slmax,
		   slmin;

    fl_get_slider_bounds( sp->slider, &slmin, &slmax );

    if ( slmax > slmin )
		nval = fl_get_slider_value( sp->slider ) + sp->increment;
    else
		nval = fl_get_slider_value( sp->slider ) - sp->increment;

    fl_set_slider_value( sp->slider, nval );
	nval = fl_get_slider_value( sp->slider );

	if ( obj->parent->how_return == FL_RETURN_NONE )
		return;

	if ( ! obj->pushed )
		obj->parent->returned |= FL_RETURN_END;

	if ( nval != sp->old_val )
		obj->parent->returned |= FL_RETURN_CHANGED;

	if ( ! ( obj->parent->how_return & FL_RETURN_END ) )
		sp->old_val = nval;

	if ( obj->parent->how_return == FL_RETURN_END_CHANGED )
	{
		if ( obj->parent->returned != FL_RETURN_END_CHANGED )
			obj->parent->returned = FL_RETURN_NONE;
		else
			sp->old_val = nval;
	}

	obj->parent->returned &= obj->parent->how_return;
}


/***************************************
 ***************************************/

static void
down_cb( FL_OBJECT * obj,
		 long        data  FL_UNUSED_ARG )
{
    FLI_SCROLLBAR_SPEC *sp = obj->parent->spec;
    double nval,
           slmax,
		   slmin;

    fl_get_slider_bounds( sp->slider, &slmin, &slmax );

    if ( slmax > slmin )
		nval = fl_get_slider_value( sp->slider ) - sp->increment;
    else
		nval = fl_get_slider_value( sp->slider ) + sp->increment;

    fl_set_slider_value( sp->slider, nval );
	nval = fl_get_slider_value( sp->slider );

	if ( obj->parent->how_return == FL_RETURN_NONE )
		return;

	if ( ! obj->pushed )
		obj->parent->returned |= FL_RETURN_END;

	if ( nval != sp->old_val )
		obj->parent->returned |= FL_RETURN_CHANGED;

	if ( ! ( obj->parent->how_return & FL_RETURN_END ) )
		sp->old_val = nval;

	if ( obj->parent->how_return == FL_RETURN_END_CHANGED )
	{
		if ( obj->parent->returned != FL_RETURN_END_CHANGED )
			obj->parent->returned = FL_RETURN_NONE;
		else
			sp->old_val = nval;
	}

	obj->parent->returned &= obj->parent->how_return;
}


/***************************************
 ***************************************/

FL_OBJECT *
fl_create_scrollbar( int          type,
					 FL_Coord     x,
					 FL_Coord     y,
					 FL_Coord     w,
					 FL_Coord     h,
					 const char * l )
{
    FLI_SCROLLBAR_SPEC *sp;
    FL_OBJECT *obj;

    obj = fl_make_object( FL_SCROLLBAR, type, x, y, w, h, l, handle );

    obj->spec_size  = sizeof *sp;
    obj->spec       = sp = fl_calloc( 1, obj->spec_size );
    obj->col1       = FL_COL1;
	obj->col2       = FL_COL1;
    obj->align      = FL_ALIGN_BOTTOM;

    if ( IsThin( type ) )
		obj->boxtype = FL_DOWN_BOX;
    else if ( type == FL_HOR_NICE_SCROLLBAR || type == FL_VERT_NICE_SCROLLBAR )
		obj->boxtype = FL_FRAME_BOX;
    else
		obj->boxtype = FL_UP_BOX;

    if ( is_horiz( type ) )
    {
		fl_set_object_resize( obj, FL_RESIZE_X );

		sp->up   = fl_create_scrollbutton( FL_TOUCH_BUTTON, 1, 1, 1, 1, "6" );
		sp->down = fl_create_scrollbutton( FL_TOUCH_BUTTON, 1, 1, 1, 1, "4" );
		fl_set_object_callback( sp->up, up_cb, 0 );
		fl_set_object_resize( sp->up, FL_RESIZE_NONE );
		fl_set_object_callback( sp->down, down_cb, 0 );
		fl_set_object_resize( sp->down, FL_RESIZE_NONE );

		if ( type == FL_HOR_SCROLLBAR )
			sp->slider = fl_create_slider( FL_HOR_BROWSER_SLIDER2,
										   1, 1, 1, 1, "" );
		else if ( type == FL_HOR_THIN_SCROLLBAR )
			sp->slider = fl_create_slider( FL_HOR_THIN_SLIDER,
										   1, 1, 1, 1, "" );
		else if ( type == FL_HOR_PLAIN_SCROLLBAR )
			sp->slider = fl_create_slider( FL_HOR_BASIC_SLIDER,
										   1, 1, 1, 1, "" );
		else if ( type == FL_HOR_NICE_SCROLLBAR )
			sp->slider = fl_create_slider( FL_HOR_NICE_SLIDER2,
										   1, 1, 1, 1, "" );

		fl_set_object_resize( sp->slider, FL_RESIZE_X );
    }
    else
    {
		fl_set_object_resize( obj, FL_RESIZE_Y );

		sp->up = fl_create_scrollbutton( FL_TOUCH_BUTTON, 1, 1, 1, 1, "8" );
		sp->down = fl_create_scrollbutton( FL_TOUCH_BUTTON, 1, 1, 1, 1, "2" );
		fl_set_object_callback( sp->up, down_cb, 0 );
		fl_set_object_resize( sp->up, FL_RESIZE_NONE );
		fl_set_object_callback( sp->down, up_cb, 0 );
		fl_set_object_resize( sp->down, FL_RESIZE_NONE );

		if ( type == FL_VERT_SCROLLBAR )
			sp->slider = fl_create_slider( FL_VERT_BROWSER_SLIDER2, 1, 1,
										   1, 1, "" );
		else if ( type == FL_VERT_THIN_SCROLLBAR )
			sp->slider = fl_create_slider( FL_VERT_THIN_SLIDER, 1, 1,
										   1, 1, "" );
		else if ( type == FL_VERT_PLAIN_SCROLLBAR )
			sp->slider = fl_create_slider( FL_VERT_BASIC_SLIDER, 1, 1,
										   1, 1, "" );
		else if ( type == FL_VERT_NICE_SCROLLBAR )
			sp->slider = fl_create_slider( FL_VERT_NICE_SLIDER2, 1, 1,
										   1, 1, "" );
		else
			M_err( "CreateScrollbar", "Unknown type %d", type );

		fl_set_object_resize( sp->slider, FL_RESIZE_Y );
    }

    sp->increment = 0.02;
    fl_set_slider_increment( sp->slider, 5 * sp->increment, sp->increment );
    fl_set_object_callback( sp->slider, slider_cb, 0 );
	fl_set_slider_bounds( sp->slider, 0, 1.0 );
    get_geom( obj );

	sp->old_val = fl_get_slider_value( sp->slider );

    fl_add_child( obj, sp->slider );
    fl_add_child( obj, sp->down );
    fl_add_child( obj, sp->up );

	/* This must come after adding the slider as a child since in that
	   function all attributes are inherited from the parent */

	fl_set_scrollbar_return( obj, FL_RETURN_CHANGED );

    return obj;
}


/*
 * User routines
 */

#define ISSCROLLBAR( o )  ( ( o ) && ( o )->objclass == FL_SCROLLBAR )


/***************************************
 ***************************************/

FL_OBJECT *
fl_add_scrollbar( int          type,
				  FL_Coord     x,
				  FL_Coord     y,
				  FL_Coord     w,
				  FL_Coord     h,
				  const char * l )
{
    FL_OBJECT *obj = fl_create_scrollbar( type, x, y, w, h, l );

    fl_add_object( fl_current_form, obj );
    return obj;
}


/***************************************
 ***************************************/

double
fl_get_scrollbar_value( FL_OBJECT * ob )
{
    if ( ! ISSCROLLBAR( ob ) )
    {
		M_err( "GetScrollBarVal", "%s not a scrollbar",
			   ob ? ob->label : "Object" );
		return -1000;
    }

    return fl_get_slider_value( ( ( FLI_SCROLLBAR_SPEC * ) ob->spec )->slider );
}


/***************************************
 ***************************************/

void
fl_set_scrollbar_value( FL_OBJECT * ob,
						double      val )
{
	FLI_SCROLLBAR_SPEC *sp = ob->spec;

    if ( ! ISSCROLLBAR( ob ) )
    {
		M_err( "fl_set_scrollbar_value", "%s not a scrollbar",
			   ob ? ob->label : "Object" );
		return;
    }

	sp->old_val = val;
    fl_set_slider_value( sp->slider, val );
}


/***************************************
 ***************************************/

void
fl_set_scrollbar_size( FL_OBJECT * ob,
					   double      val )
{
    fl_set_slider_size( ( ( FLI_SCROLLBAR_SPEC * ) ob->spec )->slider, val );
}


/***************************************
 ***************************************/

void
fl_set_scrollbar_increment( FL_OBJECT * ob,
							double      l,
							double      r )
{
    FLI_SCROLLBAR_SPEC *sp = ob->spec;

    fl_set_slider_increment( sp->slider, l, r );
    sp->increment = r;
}


/***************************************
 ***************************************/

void
fl_get_scrollbar_increment( FL_OBJECT * ob,
						    double    * a,
							double    * b )
{
    fl_get_slider_increment( ( ( FLI_SCROLLBAR_SPEC * ) ob->spec )->slider,
							 a, b );
}


/***************************************
 ***************************************/

void
fl_set_scrollbar_bounds( FL_OBJECT * ob,
						 double      b1,
						 double      b2 )
{
    if ( ! ISSCROLLBAR( ob ) )
    {
		M_err( "SetScrollBarBounds", "%s not a scrollbar",
			   ob ? ob->label : "Object" );
		return;
    }

    fl_set_slider_bounds( ( ( FLI_SCROLLBAR_SPEC * ) ob->spec )->slider,
						  b1, b2 );
}


/***************************************
 ***************************************/

void
fl_get_scrollbar_bounds( FL_OBJECT * obj,
						 double *    b1,
						 double *    b2 )
{
    fl_get_slider_bounds( ( ( FLI_SCROLLBAR_SPEC * ) obj->spec )->slider,
						  b1, b2 );
}


/***************************************
 ***************************************/

void
fl_set_scrollbar_return( FL_OBJECT * obj,
						 int         when )
{
	FLI_SCROLLBAR_SPEC *sp = obj->spec;

	obj->how_return = when;

	if (    when == FL_RETURN_NONE
		 || when == FL_RETURN_CHANGED )
		fl_set_slider_return( sp->slider, FL_RETURN_CHANGED );
	else
		fl_set_slider_return( sp->slider, FL_RETURN_ALWAYS );

	/* We may need the value of the slider at this moment in the
	   callback function... */

	sp->old_val = fl_get_slider_value( sp->slider );
}


/***************************************
 ***************************************/

void
fl_set_scrollbar_step( FL_OBJECT * obj,
					   double      step )
{
    fl_set_slider_step( ( ( FLI_SCROLLBAR_SPEC * ) obj->spec )->slider, step );
}
