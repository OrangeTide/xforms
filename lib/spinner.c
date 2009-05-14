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


/***************************************
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
		sp->input->w = obj->w - bwh;
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

			/* This shouldn't be necessary */

			fl_redraw_object( sp->up );
			fl_redraw_object( sp->down );
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
		sp->input->h = obj->h - bwh;

		sp->up->y = sp->down->y = obj->y + obj->h - bwh - 1;
		sp->up->x = obj->x + bwh;
		sp->down->x = obj->x;
		sp->up->w = sp->up->h = sp->down->w = sp->down->h = bwh;

		if ( sp->orient == 0 )
		{
			sp->orient = 1;
			fl_set_object_label( sp->up,   "@6>" );
			fl_set_object_label( sp->down, "@4>" );

			/* This shouldn't be necessary */

			fl_redraw_object( sp->up );
			fl_redraw_object( sp->down );
		}
	}
}


/***************************************
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
		case FL_DRAW :
			/* Why can't this be handled via FL_RESIZED events? */
			set_geom( obj );

		case FL_DRAWLABEL :
			fl_draw_object_label_outside( obj );
			break;

		default :
			fprintf( stderr, "Got event %d\n", event );
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
	char *eptr;
	int max_len = 4 + sp->prec + log10( DBL_MAX );
	char buf[ max_len ];

	if ( obj->parent->type == FL_INT_SPINNER )
	{ 
		long i_val = strtol( s_val, &eptr, 10 );

		if ( *eptr != '\0' || i_val > sp->i_max || i_val < sp->i_min )
			i_val = sp->i_val;

		if ( data == 1 )
		{
			if ( i_val <= sp->i_max - sp->i_incr )
				i_val += sp->i_incr;
			else
				i_val = sp->i_max;
		}
		else if ( data == -1 )
		{
			if ( i_val >= sp->i_min + sp->i_incr )
				i_val -= sp->i_incr;
			else
				i_val = sp->i_min;
		}

		sp->i_val = i_val;
		sprintf( buf, "%d", sp->i_val );
		fl_set_input( sp->input, buf );
	}
	else
	{
		double f_val = strtod( s_val, &eptr );

		if (    *eptr != '\0'
			 || errno == ERANGE
			 || f_val > sp->f_max
			 || f_val < sp->f_min )
			f_val = sp->f_val;

		if ( data == 1 )
		{
			if ( f_val <= sp->f_max - sp->f_incr )
				f_val += sp->f_incr;
			else
				f_val = sp->f_max;
		}
		else if ( data == -1 )
		{
			if ( f_val >= sp->f_min + sp->f_incr )
				f_val -= sp->f_incr;
			else
				f_val = sp->f_min;
		}

		sp->f_val = f_val;
		sprintf( buf, "%.*f", sp->prec, sp->f_val );
		fl_set_input( sp->input, buf );
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
		iw = w - bwh;
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
    obj->boxtype = FL_NO_BOX;
	obj->align = FL_ALIGN_LEFT;

	obj->spec_size = sizeof *sp;
	sp = obj->spec = malloc( obj->spec_size );

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

	sp->i_val = 0;
	sp->i_min = INT_MIN;
	sp->i_max = INT_MAX;
	sp->i_incr = 1;

	sp->f_val = 0.0;
	sp->f_min = - DBL_MAX;
	sp->f_max = DBL_MAX;
	sp->f_incr = 1.0;

	sp->orient = orient;
	sp->prec = 1;

    fli_add_child( obj, sp->input );
    fli_add_child( obj, sp->up );
    fli_add_child( obj, sp->down );

	fl_set_input( sp->input, type == FL_INT_SPINNER ? "0" : "0.0" );

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
