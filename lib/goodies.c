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
 * \file goodies.c
 *.
 *  This file is part of the XForms library package.
 *  Copyright (c) 1996-2002  T.C. Zhao and Mark Overmars
 *  All rights reserved.
 *.
 *
 *  Common routines for the goodies
 *
 */

#if defined F_ID || defined DEBUG
char *fl_id_gds = "$Id: goodies.c,v 1.11 2008/05/12 12:07:41 jtt Exp $";
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include/forms.h"
#include "flinternal.h"
#include "bitmaps/q.xbm"		/* in directory bitmaps */
#include "bitmaps/warn.xbm"		/* in directory bitmaps */


/***************************************
 ***************************************/

void
fl_update_display( int block )
{
    if ( block )
		XSync( flx->display, 0 );
    else
		XFlush( flx->display );
}


/***************************************
 * Preemptive handler to work around a bug in some window managers
 * where iconification causes a window to close. For normal closing,
 * we don't see any unmap events as winclose eats it
 ***************************************/

int
fli_goodies_preemptive( FL_FORM * form,
						void *    ev )
{
    if ( ( ( XEvent * ) ev )->type == UnmapNotify )
    {
		fl_trigger_object( form->u_vdata );
		return FL_PREEMPT;
    }

    return 0;
}


/***************************************
 ***************************************/

int
fl_goodies_atclose( FL_FORM * form,
					void *    data )
{
    fl_trigger_object( data ? data : form->u_vdata );
    if ( form->sort_of_modal )
		form->sort_of_modal = 0;

    return FL_IGNORE;
}


/***************************************
 ***************************************/

void
fli_parse_goodies_label( FL_OBJECT *  ob,
						 const char * name )
{
    char s[ 256 ];

    if ( fl_get_resource( name, NULL, FL_STRING, NULL, s, 256 ) )
    {
		fl_set_object_label( ob, s );
		fl_fit_object_label( ob, 5, 2 );
    }
}


/***************************************
 ***************************************/

void
fli_get_goodie_title( FL_FORM *    form,
					  const char * res )
{
    char s[ 256 ];

    if ( fl_get_resource( res, NULL, FL_STRING, NULL, s, 256 ) )
		fl_set_form_title( form, s );
}



static int goodie_style = FL_NORMAL_STYLE,
           goodie_size = FL_DEFAULT_SIZE;


/***************************************
 ***************************************/

void
fl_set_goodies_font( int style,
					 int size )
{
    goodie_style = style;
    goodie_size = size;
}


/***************************************
 ***************************************/

void
fli_get_goodies_font( int * style,
					  int * size )
{
    *style = goodie_style;
    *size = goodie_size;
}


/***************************************
 ***************************************/

void
fli_handle_goodie_font( FL_OBJECT * ob1,
						FL_OBJECT * ob2 )
{
    if ( goodie_style < 0 )
		return;

    if ( ob1 )
    {
		fl_set_object_lstyle( ob1, goodie_style );
		fl_set_object_lsize( ob1, goodie_size );
		fl_fit_object_label( ob1, 1, 1 );
    }

    if ( ob2 )
    {
		fl_set_object_lstyle( ob2, goodie_style );
		fl_set_object_lsize( ob2, goodie_size );
    }
}


/***************************************
 ***************************************/

static void
fli_box_vert( FL_Coord x,
			  FL_Coord y,
			  FL_Coord w,
			  FL_Coord h )
{
    int xy[ 2 ];
    int halfh = 0.5 * h,
		halfw = 0.5 * w;

    xy[ 0 ] = x;
    xy[ 1 ] = y + halfh;
    fl_v2i( xy );
    xy[ 0 ] = x + halfw;
    xy[ 1 ] = y;
    fl_v2i( xy );
    xy[ 0 ] = x + 2 * halfw;
    xy[ 1 ] = y + halfh;
    fl_v2i( xy );
    xy[ 0 ] = x + halfw;
    xy[ 1 ] = y + 2 * halfh;
    fl_v2i( xy );
}


/***************************************
 ***************************************/

static int
draw_box( FL_OBJECT * ob,
		  int         ev,
		  FL_Coord    x   FL_UNUSED_ARG,
		  FL_Coord    y   FL_UNUSED_ARG,
		  int         k   FL_UNUSED_ARG,
		  void      * sp  FL_UNUSED_ARG )
{
	int p = ( FL_max( ob->w, ob->h ) / 2 ) * 2 + 1;

    if ( ev != FL_DRAW )
		return 0;

	fl_winset( FL_ObjWin( ob ) );

	if ( ! fli_dithered( fl_vmode ) )
	{
		fl_color( FL_YELLOW );
		fl_bgnpolygon( );
		fli_box_vert( ob->x, ob->y, p, p );
		fl_endpolygon( );
	}

	fl_linewidth( 2 );
	fl_color( FL_BLACK );
	fl_bgnclosedline( );
	fli_box_vert( ob->x, ob->y, ob->w, ob->h );
	fl_endclosedline( );
	fl_linewidth( 0 );

	return 0;
}


/***************************************
 ***************************************/

void
fli_add_q_icon( FL_Coord x,
			   FL_Coord y,
			   FL_Coord w,
			   FL_Coord h )
{
    FL_OBJECT *obj;

    fl_add_free( FL_SLEEPING_FREE, x, y, w, h, "", draw_box );
    obj = fl_add_bitmap( FL_NORMAL_BITMAP, x, y, w, h, "" );
    fl_set_bitmap_data( obj, q_width, q_height, q_bits );
    fl_set_object_color( obj, FL_YELLOW, FL_YELLOW );
    fl_set_object_lcol( obj, FL_BLACK );
}


/***************************************
 ***************************************/

void
fli_add_warn_icon( FL_Coord x,
				   FL_Coord y,
				   FL_Coord w,
				   FL_Coord h )
{
    FL_OBJECT *obj;

    fl_add_free( FL_SLEEPING_FREE, x, y, w, h, "", draw_box );
    obj = fl_add_bitmap( FL_NORMAL_BITMAP, x, y, w, h, "" );
    fl_set_bitmap_data( obj, warn_width, warn_height, warn_bits );
    fl_set_object_color( obj, FL_YELLOW, FL_YELLOW );
    fl_set_object_lcol( obj, FL_BLACK );
}
