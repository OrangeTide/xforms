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
 * You should have received a copy of the GNU Lesser General Public
 * License along with XForms; see the file COPYING.  If not, write to
 * the Free Software Foundation, 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 *
 */


/**
 * \file dial.c
 *.
 *  This file is part of the XForms library package.
 *  Copyright (c) 1996-2002  T.C. Zhao and Mark Overmars
 *  All rights reserved.
 *.
 *
 * dial.c
 *
 *  Default 0 is at 6 oclock and clock-wise is positive.
 */

#if defined F_ID || definedDEBUG
char *fl_id_dial = "$Id: dial.c,v 1.15 2008/09/24 18:31:58 jtt Exp $";
#endif

#define SIX_OCLOCK 1

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include/forms.h"
#include "flinternal.h"
#include "private/pdial.h"

#include <math.h>
#include <sys/types.h>
#include <stdlib.h>


#ifndef M_PI
#define M_PI 3.14159265359
#endif


static double xo,
              yo;


/***************************************
 ***************************************/

static void
rotate_it( FL_POINT * xp,
		   double     x,
		   double     y,
		   double     a )
{
    double sina = sin( a );
    double cosa = cos( a );

    xp->x = FL_crnd( xo + ( x - xo ) * cosa + ( y - yo ) * sina );
	xp->y = FL_crnd( yo - ( x - xo ) * sina + ( y - yo ) * cosa );
}


/***************************************
 * Draws a dial
 ***************************************/

static void
draw_dial( FL_OBJECT * ob )
{
    FL_Coord x,
		     y,
		     w,
		     h,
		     radius;
    double dangle;
    FLI_DIAL_SPEC *sp = ob->spec;
    FL_POINT xp[ 5 ];             /* need one extra for closing of polygon! */
    int boxtype,
		iradius;

    /* since rotate_it always does the rotation in the math way, i.e. 0 at
       three o'clock and CCW, need to translate the current theta into that
       coordiante system */

    dangle = ( sp->val - sp->b ) / sp->a;

    if ( sp->direction == FL_DIAL_CW )
		dangle = sp->origin - dangle;
    else
		dangle += sp->origin;

    while ( dangle < 0.0 )
		dangle += 360.0;
    while ( dangle > 360.0 )
		dangle -= 360.0;

    dangle *= M_PI / 180.0;

    w = ob->w - 3;
    h = ob->h - 3;

    x = xo = ob->x + ob->w / 2;
    y = yo = ob->y + ob->h / 2;

    if ( FL_IS_UPBOX( ob->boxtype ) )
		boxtype = FL_OVAL3D_UPBOX;
    else if ( FL_IS_DOWNBOX( ob->boxtype ) )
		boxtype = FL_OVAL3D_DOWNBOX;
    else if ( ob->boxtype == FL_FRAME_BOX )
		boxtype = FL_OVAL3D_FRAMEBOX;
    else if ( ob->boxtype == FL_EMBOSSED_BOX )
		boxtype = FL_OVAL3D_EMBOSSEDBOX;
    else
		boxtype = FL_OVAL_BOX;

    /* the dial itself */

    radius = 0.5 * FL_min( w, h );
    iradius = radius - 1;	      /* internal radius */

    fl_drw_box( boxtype, x - radius, y - radius, 2 * radius, 2 * radius,
				ob->col1, ob->bw );

    /* the "hand" */

    if ( ob->type == FL_NORMAL_DIAL )
    {
		FL_Coord r;

		r = FL_min( 0.5 * iradius, 15 );

		rotate_it( xp,     x + iradius - 1,     y - 2, dangle );
		rotate_it( xp + 1, x + iradius - 1 - r, y - 2, dangle );
		rotate_it( xp + 2, x + iradius - 1 - r, y + 2, dangle );
		rotate_it( xp + 3, x + iradius - 1,     y + 2, dangle );

		fl_polyf( xp, 4, ob->col2 );
    }
    else if ( ob->type == FL_LINE_DIAL )
    {
		double dx = 0.1 + 0.08 * iradius,
			   dy = 0.1 + 0.08 * iradius;

		rotate_it( xp,     x,               y,      dangle );
		rotate_it( xp + 1, x + dx,          y - dy, dangle );
		rotate_it( xp + 2, x + iradius - 2, y,      dangle );
		rotate_it( xp + 3, x + dx,          y + dy, dangle );

		fl_polybound( xp, 4, ob->col2 );
    }
    else if ( ob->type == FL_FILL_DIAL )
    {
		double ti,
			   delta;

		delta = ( sp->val - sp->b ) / sp->a;
		delta = FL_abs( sp->thetai - delta );
		delta = sp->direction == FL_DIAL_CW ? -delta : delta;

		iradius -= boxtype != FL_OVAL_BOX;

		if ( sp->direction == FL_DIAL_CCW )
			ti = sp->thetai + sp->origin;
		else
			ti = sp->origin - sp->thetai;
		
		while ( ti < 0.0 )
			ti += 360.0;
		while ( ti > 360.0 )
			ti -= 360.0;

		fl_ovalarc( 1, xo - iradius, yo - iradius, 2 * iradius, 2 * iradius,
					ti * 10, delta * 10, ob->col2 );

		rotate_it( xp,     xo + iradius - 1, yo, dangle );
		rotate_it( xp + 1, xo + iradius - 1, yo, ti * M_PI / 180.0 );
		fl_simple_line( FL_crnd( xo ), FL_crnd( yo ),
						xp[ 0 ].x, xp[ 0 ].y, FL_BLACK );
		fl_simple_line( FL_crnd( xo ), FL_crnd( yo ),
						xp[ 1 ].x, xp[ 1 ].y, FL_BLACK );

		if ( boxtype == FL_OVAL_BOX )
			fl_circ( x, y, iradius, FL_BLACK );
    }
    else
		M_err( "DrawDial", "Bad type" );

    fl_drw_text_beside( ob->align, ob->x, ob->y, ob->w, ob->h,
						ob->lcol, ob->lstyle, ob->lsize, ob->label );
}


/***************************************
* Handle a mouse position change
 ***************************************/

static int
handle_mouse( FL_OBJECT * ob,
			  FL_Coord    mousex,
			  FL_Coord    mousey )
{
    FLI_DIAL_SPEC *sp = ob->spec;
    double oldv,
		   val,
		   olda;
    double mx,
		   my,
		   angle,
		   range = sp->max - sp->min;

    oldv = sp->val;
    olda = ( oldv - sp->b ) / sp->a;

    /* convert to sane FL_coordinate system, i.e., +y up */

    mx =   mousex - ( ob->x + ob->w * 0.5 );
    my = - mousey + ( ob->y + ob->h * 0.5 );

    /* skip clicks very close to center */

    if ( fabs( mx ) < 2 && fabs( my ) < 2 )
		return 0;

    /* get angle and normalize to (0,2PI) */

    angle = atan2( my, mx ) * 180.0 / M_PI;

    if ( sp->direction == FL_DIAL_CW )
		angle = sp->origin - angle;
    else
		angle -= sp->origin;

    while ( angle < 0.0 )
		angle += 360.0;
    while ( angle > 360.0 )
		angle -= 360.0;

    val = fli_clamp( sp->a * angle + sp->b, sp->min, sp->max );

    /* check if crossed boundary. Fix it if it did. Fixing is necessary
       otherwise might be unable to reach thetaf(360) */

    if ( ! sp->cross_over && fabs( oldv - val ) > 0.6 * range )
    {
		if ( fabs( olda - sp->thetai ) < fabs( olda - sp->thetaf ) )
			angle = sp->thetai;
		else
			angle = sp->thetaf;
		val = sp->a * angle + sp->b;
    }

    if ( sp->step != 0.0 )
		val = ( int ) ( val / sp->step + 0.5 ) * sp->step;

    /* allow a resolution about 0.2 degrees */

    if ( fabs( val - oldv ) > range / 1800.0 )
    {
		sp->val = val;
		fl_redraw_object( ob );
		return 1;
    }

    return 0;
}


/***************************************
 * Function for handling mouse wheel input
 ***************************************/

static int
handle_mouse_wheel( FL_OBJECT * ob,
					XEvent *    xev,
					int         key )
{
    FLI_DIAL_SPEC *sp = ob->spec;
    double val,
		   step,
		   oldv = sp->val,
		   range = sp->max - sp->min;

	if ( key != FL_MBUTTON4 && key != FL_MBUTTON5 )
		return 0;

	step = sp->step > 0.0 ? 10.0 * sp->step : 0.1 * range;

	if ( shiftkey_down( ( ( XButtonEvent * ) xev )->state ) )
		step *= 0.1;
	else if ( controlkey_down( ( ( XButtonEvent * ) xev )->state ) )
		step *= 2.5;

	if ( sp->direction == FL_DIAL_CW )
		step = key == FL_MBUTTON4 ? - step : step;
	else
		step = key == FL_MBUTTON4 ? step : - step;

	val = sp->val + step;

	if ( sp->cross_over )
	{
		while ( val > sp->max )
			val -= range;
		while ( val < sp->min )
			val += range;
	}
	else
	{
		if ( val > sp->max )
			val = sp->max;
		else if ( val < sp->min )
			val = sp->min;
	}
		
	if ( val != oldv )
	{
		sp->val = val;
		fl_redraw_object( ob );
		return 1;
	}

    return 0;
}


/***************************************
 * Handles an event
 ***************************************/

static int
handle_dial( FL_OBJECT * ob,
			 int         event,
			 FL_Coord    mx,
			 FL_Coord    my,
			 int         key  FL_UNUSED_ARG,
			 void *      ev )
{
    FLI_DIAL_SPEC *sp = ob->spec;

#if FL_DEBUG >= ML_DEBUG
    M_info( "HandleDial", fli_event_name( event ) );
#endif

    switch ( event )
    {
		case FL_DRAW:
			draw_dial( ob );
			break;

		case FL_DRAWLABEL:
			fl_drw_text_beside( ob->align, ob->x, ob->y, ob->w, ob->h,
								ob->lcol, ob->lstyle, ob->lsize, ob->label );
			break;

		case FL_PUSH:
			if ( key != FL_MBUTTON1 )
				break;
			sp->changed = 0;
			/* fall through */

		case FL_MOTION:
			if ( key != FL_MBUTTON1 )
				break;

			if ( handle_mouse( ob, mx, my ) )
				sp->changed = 1;
			if ( sp->changed && sp->how_return == FL_RETURN_CHANGED )
			{
				sp->changed = 0;
				return 1;
			}
			else if ( sp->how_return == FL_RETURN_ALWAYS )
				return 1;
			break;

		case FL_RELEASE:
			if ( key == FL_MBUTTON2 || key == FL_MBUTTON3 )
				break;

			if ( handle_mouse_wheel( ob, ev, key ) )
				sp->changed = 1;

			if (    sp->how_return == FL_RETURN_ALWAYS
				 || sp->how_return == FL_RETURN_END
				 || (    sp->changed
					  && (    sp->how_return == FL_RETURN_CHANGED
						   || sp->how_return == FL_RETURN_END_CHANGED ) ) )
				return 1;
			break;

		case FL_FREEMEM:
			fl_free( ob->spec );
			break;
    }

    return 0;
}


/***************************************
 ***************************************/

static void
get_mapping( FLI_DIAL_SPEC *sp )
{
    sp->a = ( sp->max - sp->min ) / ( sp->thetaf - sp->thetai );
    sp->b = sp->max - sp->a * sp->thetaf;
}


/***************************************
 * creates an object
 ***************************************/

FL_OBJECT *
fl_create_dial( int          type,
				FL_Coord     x,
				FL_Coord     y,
				FL_Coord     w,
				FL_Coord     h,
				const char * label )
{
    FL_OBJECT *ob;
    FLI_DIAL_SPEC *sp;

    ob = fl_make_object( FL_DIAL, type, x, y, w, h, label, handle_dial );
    ob->col1 = FL_DIAL_COL1;
    ob->col2 = FL_DIAL_COL2;
    ob->align = FL_DIAL_ALIGN;
    ob->lcol = FL_DIAL_LCOL;
    ob->boxtype = FL_DIAL_BOXTYPE;

    ob->spec_size = sizeof *sp;
    sp = ob->spec = fl_calloc( 1, sizeof *sp );
    sp->min = 0.0;
    sp->max = 1.0;
    sp->val = 0.5;
    sp->step = 0.0;
    sp->thetai = 0.0;
    sp->thetaf = 360.0;
    sp->origin = 270.0;
    sp->direction = FL_DIAL_CW;
    get_mapping( sp );
    sp->how_return = FL_RETURN_END_CHANGED;

    return ob;
}


/*************************************** 
 * Adds an object
***************************************/

FL_OBJECT *
fl_add_dial( int          type,
			 FL_Coord     x,
			 FL_Coord     y,
			 FL_Coord     w,
			 FL_Coord     h,
			 const char * label )
{
    FL_OBJECT *ob;

    ob = fl_create_dial( type, x, y, w, h, label );
    fl_add_object( fl_current_form, ob );
    fl_set_object_dblbuffer( ob, 1 );
    return ob;
}


/***************************************
 ***************************************/

void
fl_set_dial_value( FL_OBJECT * ob,
				   double      val )
{
    FLI_DIAL_SPEC *sp = ob->spec;

    if ( sp->val != val )
    {
		sp->val = val;
		fl_redraw_object( ob );
    }
}


/***************************************
 ***************************************/

void
fl_set_dial_bounds( FL_OBJECT * ob,
					double      min,
					double      max )
{
    FLI_DIAL_SPEC *sp = ob->spec;

    if ( sp->min != min || sp->max != max )
    {
		sp->min = min;
		sp->max = max;
		get_mapping( sp );
		sp->val = fli_clamp( sp->val, sp->min, sp->max );
		fl_redraw_object( ob );
    }
}


/***************************************
 ***************************************/

void
fl_set_dial_angles( FL_OBJECT * ob,
					double      amin,
					double      amax )
{
    FLI_DIAL_SPEC *sp = ob->spec;

    while ( amin < 0.0 )
		amin += 360.0;
	while ( amin > 360 )
		amin -= 360.0;

    while ( amax < 0.0 )
		amax += 360.0;
    while ( amax > 360.0 )
		amax -= 360.0;

    if ( sp->thetaf != amax || sp->thetai != amin )
    {
		sp->thetaf = amax;
		sp->thetai = amin;
		get_mapping( sp );
		fl_redraw_object( ob );
    }
}


/***************************************
 ***************************************/

void
fl_set_dial_cross( FL_OBJECT * ob,
				   int         flag )
{
    ( ( FLI_DIAL_SPEC * ) ob->spec )->cross_over = flag;
}


/***************************************
 ***************************************/

double
fl_get_dial_value( FL_OBJECT * ob )
{
    return ( ( FLI_DIAL_SPEC * ) ob->spec )->val;
}


/***************************************
 ***************************************/

void
fl_get_dial_bounds( FL_OBJECT * ob,
					double *    min,
					double *    max )
{
    *min = ( ( FLI_DIAL_SPEC * ) ob->spec )->min;
    *max = ( ( FLI_DIAL_SPEC * ) ob->spec )->max;
}


/***************************************
 * Sets whether to return value all the time
 ***************************************/

void
fl_set_dial_return( FL_OBJECT * ob,
				    int         value )
{
    ( ( FLI_DIAL_SPEC * ) ob->spec )->how_return = value;
}


/***************************************
 * Sets the step size to which values are rounded.
 ***************************************/

void
fl_set_dial_step( FL_OBJECT * ob,
				  double      value )
{
    ( ( FLI_DIAL_SPEC * ) ob->spec )->step = value;
}


/***************************************
 ***************************************/

void
fl_set_dial_direction( FL_OBJECT * ob,
					   int         dir )
{
    FLI_DIAL_SPEC *sp = ob->spec;

    if ( sp->direction != dir )
    {
		sp->direction = dir;
		get_mapping( sp );
		fl_redraw_object( ob );
    }
}
