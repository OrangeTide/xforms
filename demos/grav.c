/*
 *  This file is part of XForms.
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
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with XForms; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 59 Temple Place - Suite 330, Boston,
 *  MA 02111-1307, USA.
 * Author: Jens Thoms Toerring <jt@toerring.de>
 *
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include "include/forms.h"

/**** Forms and Objects ****/

typedef struct {
	FL_FORM *   grav;
	FL_OBJECT * box;
	FL_OBJECT * rx;
	FL_OBJECT * ry;
} FD_gravity;

typedef struct {
	FL_OBJECT *  box;
	FL_OBJECT *  rx;
	FL_OBJECT *  ry;
	unsigned int grav;
} FD_grav_data;

typedef struct {
	FL_FORM * help;
	int       is_shown;
} FD_help;


static FD_grav_data gd[ 9 ];
static unsigned int g[ ] = { FL_NorthWest, FL_North,     FL_NorthEast,
							 FL_West,      FL_NoGravity, FL_East,
							 FL_SouthWest, FL_South,     FL_SouthEast };
static int w = 500,
		   h = 400;
static int bw = 200,
	       bh = 200;


#define ULC_POS_LEFT_FIXED( obj )                \
	(    ( obj )->nwgravity == FL_NorthWest	     \
      || ( obj )->nwgravity == FL_West           \
	  || ( obj )->nwgravity == FL_SouthWest )

#define ULC_POS_RIGHT_FIXED( obj )               \
	(    ( obj )->nwgravity == FL_NorthEast	     \
      || ( obj )->nwgravity == FL_East           \
	  || ( obj )->nwgravity == FL_SouthEast )

#define LRC_POS_LEFT_FIXED( obj )                \
	(    ( obj )->segravity == FL_NorthWest	     \
      || ( obj )->segravity == FL_West           \
	  || ( obj )->segravity == FL_SouthWest )

#define LRC_POS_RIGHT_FIXED( obj )               \
	(    ( obj )->segravity == FL_NorthEast	     \
      || ( obj )->segravity == FL_East           \
	  || ( obj )->segravity == FL_SouthEast )

#define HAS_FIXED_HORI_ULC_POS( obj )                             \
	( ULC_POS_LEFT_FIXED( obj ) || ULC_POS_RIGHT_FIXED( obj ) )

#define HAS_FIXED_HORI_LRC_POS( obj )                             \
	( LRC_POS_LEFT_FIXED( obj ) || LRC_POS_RIGHT_FIXED( obj ) )

#define HAS_FIXED_WIDTH( obj )                                          \
	( HAS_FIXED_HORI_ULC_POS( obj ) && HAS_FIXED_HORI_LRC_POS( obj ) ) 


#define ULC_POS_TOP_FIXED( obj )                 \
	(    ( obj )->nwgravity == FL_NorthWest	     \
      || ( obj )->nwgravity == FL_North          \
	  || ( obj )->nwgravity == FL_NorthEast )

#define ULC_POS_BOTTOM_FIXED( obj )              \
	(    ( obj )->nwgravity == FL_SouthWest	     \
      || ( obj )->nwgravity == FL_South          \
	  || ( obj )->nwgravity == FL_SouthEast )

#define LRC_POS_TOP_FIXED( obj )                 \
	(    ( obj )->segravity == FL_NorthWest	     \
      || ( obj )->segravity == FL_North          \
	  || ( obj )->segravity == FL_NorthEast )

#define LRC_POS_BOTTOM_FIXED( obj )              \
	(    ( obj )->segravity == FL_SouthWest	     \
      || ( obj )->segravity == FL_South          \
	  || ( obj )->segravity == FL_SouthEast )


#define HAS_FIXED_VERT_ULC_POS( obj )                             \
	( ULC_POS_TOP_FIXED( obj ) || ULC_POS_BOTTOM_FIXED( obj ) )

#define HAS_FIXED_VERT_LRC_POS( obj )                             \
	( LRC_POS_TOP_FIXED( obj ) || LRC_POS_BOTTOM_FIXED( obj ) )

#define HAS_FIXED_HEIGHT( obj )                                         \
	( HAS_FIXED_VERT_ULC_POS( obj ) && HAS_FIXED_VERT_LRC_POS( obj ) ) 




/***************************************
 ***************************************/

static void
check_resize( FD_grav_data * g )
{
	fl_set_button( g->rx, g->box->resize & FL_RESIZE_X );
	fl_set_button( g->ry, g->box->resize & FL_RESIZE_Y );

	if ( HAS_FIXED_WIDTH( g->box ) )
		fl_set_object_lcol( g->rx, FL_INACTIVE_COL );
	else
		fl_set_object_lcol( g->rx, FL_BLACK );

	if ( HAS_FIXED_HEIGHT( g->box ) )
		fl_set_object_lcol( g->ry, FL_INACTIVE_COL );
	else
		fl_set_object_lcol( g->ry, FL_BLACK );
}


/***************************************
 ***************************************/

static void
nw_callback( FL_OBJECT * obj   FL_UNUSED_ARG,
			 long              data )
{
	FD_grav_data *g = ( FD_grav_data * ) data;

	fl_set_object_gravity( g->box, g->grav, g->box->segravity );
	check_resize( g );
}


/***************************************
 ***************************************/

static void
se_callback( FL_OBJECT * obj   FL_UNUSED_ARG,
			 long              data )
{
	FD_grav_data *g = ( FD_grav_data * ) data;

	fl_set_object_gravity( g->box, g->box->nwgravity, g->grav );
	check_resize( g );
}


/***************************************
 ***************************************/

static void
rx_callback( FL_OBJECT * obj   FL_UNUSED_ARG,
			 long              data )
{
	FD_grav_data *g = ( FD_grav_data * ) data;
	unsigned int r = g->box->resize;

	if ( r & FL_RESIZE_X )
		r &= ~ FL_RESIZE_X;
	else
		r |= FL_RESIZE_X;

	fl_set_object_resize( g->box, r );
}
	

/***************************************
 ***************************************/

static void
ry_callback( FL_OBJECT * obj   FL_UNUSED_ARG,
			 long              data )
{
	FD_grav_data *g = ( FD_grav_data * ) data;
	unsigned int r = g->box->resize;

	if ( r & FL_RESIZE_Y )
		r &= ~ FL_RESIZE_Y;
	else
		r |= FL_RESIZE_Y;

	fl_set_object_resize( g->box, r );
}
	

/***************************************
 ***************************************/

static void
reset_callback( FL_OBJECT * obj   FL_UNUSED_ARG,
				long        data )
{
	FD_grav_data *g = ( FD_grav_data * ) data;
	
	fl_set_form_size( g->box->form, w, h );
	fl_set_object_geometry( g->box, ( w - bw ) / 2, ( h - bh ) / 2, bw, bh );
}


/***************************************
 ***************************************/

static void
help_callback( FL_OBJECT * obj  FL_UNUSED_ARG,
				long       data )
{
	FD_help *h = ( FD_help * ) data;

	if ( ! h->is_shown )
	{
		fl_show_form( h->help, FL_PLACE_CENTER | FL_FREE_SIZE,
					  FL_FULLBORDER, "Gravity Demo Help" );
		h->is_shown = 1;
	}
}


/***************************************
 ***************************************/

static void
close_callback( FL_OBJECT * obj   FL_UNUSED_ARG,
				long        data )
{
	FD_help *h = ( FD_help * ) data;

	fl_hide_form( h->help );
	h->is_shown = 0;
}


/***************************************
 ***************************************/

static FD_gravity *
create_form_gravity( FD_help * help )
{
	FL_OBJECT *obj;
	FD_gravity *fdui = fl_malloc( sizeof *fdui );
	const char *label[ ] = { "NW", "N", "NE", "W", "-", "E", "SW", "S", "SE" };
	int i;
	int s = 25;
	int m = 5;


	fdui->grav = fl_bgn_form( FL_NO_BOX, 500, 400 );

	obj = fl_add_box( FL_UP_BOX, 0, 0, w, h, "" );
	fl_set_object_bw( obj, -1 );

	fdui->box = obj = fl_add_box( FL_FRAME_BOX, ( w - bw ) / 2, ( h - bh ) / 2,
								  bw, bh, "" );
	fl_set_object_color( obj, FL_GREEN, FL_GREEN );

	fl_bgn_group( );
	for ( i = 0; i < 9; i++ )
	{
		obj = fl_add_button( FL_RADIO_BUTTON,
							 s * ( i % 3 ) + m, s * ( i / 3 ) + m, s, s,
							 label[ i ] );
		fl_set_object_bw( obj, -1 );
		fl_set_object_resize( obj, FL_RESIZE_NONE );
		fl_set_object_gravity( obj, FL_NorthWest, FL_NorthWest );
		fl_set_object_callback( obj, nw_callback, ( long ) ( gd + i ) );
		fl_set_object_color( obj, FL_COL1, FL_MCOL );
		fl_set_button( obj, fdui->box->nwgravity == g[ i ] );
	}
	fl_end_group( );

	fl_bgn_group( );
	for ( i = 0; i < 9; i++ )
	{
		obj = fl_add_button( FL_RADIO_BUTTON,
							 s * ( i % 3 ) + w - 3 * s - m,
							 s * ( i / 3 ) + h - 3 * s - m, s, s,
							 label[ i ] );
		fl_set_object_bw( obj, -1 );
		fl_set_object_resize( obj, FL_RESIZE_NONE );
		fl_set_object_gravity( obj, FL_SouthEast, FL_SouthEast );
		fl_set_object_callback( obj, se_callback, ( long ) ( gd + i ) );
		fl_set_object_color( obj, FL_COL1, FL_MCOL );
		fl_set_button( obj, fdui->box->segravity == g[ i ] );
	}
	fl_end_group( );

	fdui->rx = obj = fl_add_button( FL_PUSH_BUTTON, m, h - s - m, 80, s,
									"X Resize" ); 
	fl_set_object_bw( obj, -1 );
	fl_set_object_resize( obj, FL_RESIZE_NONE );
	fl_set_object_gravity( obj, FL_SouthWest, FL_SouthWest );
	fl_set_object_callback( obj, rx_callback, ( long ) gd );
	fl_set_object_color( obj, FL_COL1, FL_MCOL );
	fl_set_button( obj, fdui->box->resize & FL_RESIZE_X );

	fdui->ry = obj = fl_add_button( FL_PUSH_BUTTON, 2 * m + 80, h - s - m,
									80, s, "Y Resize" ); 
	fl_set_object_bw( obj, -1 );
	fl_set_object_resize( obj, FL_RESIZE_NONE );
	fl_set_object_gravity( obj, FL_SouthWest, FL_SouthWest );
	fl_set_object_callback( obj, ry_callback, ( long ) gd );
	fl_set_object_color( obj, FL_COL1, FL_MCOL );
	fl_set_button( obj, fdui->box->resize & FL_RESIZE_Y );

	obj = fl_add_button( FL_NORMAL_BUTTON, w - 85, 5, 80, s, "Help" );
	fl_set_object_bw( obj, -1 );
	fl_set_object_gravity( obj, FL_NorthEast, FL_NorthEast );
	fl_set_object_callback( obj, help_callback, ( long ) help );

	obj = fl_add_button( FL_NORMAL_BUTTON, 200, h - s - m,
						 80, s, "Reset" );
	fl_set_object_bw( obj, -1 );
	fl_set_object_resize( obj, FL_RESIZE_NONE );
	fl_set_object_gravity( obj, FL_South, FL_South );
	fl_set_object_callback( obj, reset_callback, ( long ) gd );

	obj = fl_add_button( FL_NORMAL_BUTTON, m + 280, h - s - m, 80, s, "Quit" );
	fl_set_object_bw( obj, -1 );
	fl_set_object_resize( obj, FL_RESIZE_NONE );
	fl_set_object_gravity( obj, FL_South, FL_South );

	fl_end_form();

	return fdui;
}


/***************************************
 ***************************************/

static FD_help *
create_form_help( void )
{
	FL_OBJECT *obj;
	FD_help *fdui = fl_malloc( sizeof *fdui );	
	size_t i;
	const char *text[ ] = {
		"Gravity and resize settings demonstration",
		"",
		"The interaction between gravity and resize settings",
		"can sometimes be difficult to understand  This pro-",
		"gram allows you to test some of the effects.",
		"",
		"With the sets of buttons in the upper left hand and",
		"lower right hand corner you can set the gravity for",
		"the corresponding corners of the green rectangle.",
		"",
		"With the buttons labeled 'X Resize' and 'Y Resize'",
		"you can set if the rectangle may be scaled in x-",
		"and/or y-direction. Please note that for several",
		"combinations of gravity settings the resizing",
		"settings are not taken into account by XForms. In",
		"these cases the corresponding buttons are grayed",
		"out (but not deactivated)."
	};

	fdui->help = fl_bgn_form( FL_NO_BOX, 345, 325 );

	fdui->is_shown = 0;

	obj = fl_add_box( FL_UP_BOX, 0, 0, 345, 325, "" );
	fl_set_object_bw( obj, -1 );

	obj = fl_add_browser( FL_NORMAL_BROWSER, 5, 5, 335, 285, "" );
	fl_set_object_bw( obj, -1 );
	fl_set_object_color( obj, FL_WHITE, FL_WHITE );
	fl_set_object_gravity( obj, FL_NorthWest, FL_SouthEast );

	for ( i = 0; i < sizeof text / sizeof *text; i++ )
		fl_add_browser_line( obj, text[ i ] );

	obj = fl_add_button( FL_NORMAL_BUTTON, 133, 295, 80, 25, "Close" );
	fl_set_object_bw( obj, -1 );
	fl_set_object_gravity( obj, FL_South, FL_South );
	fl_set_object_resize( obj, FL_RESIZE_NONE );
	fl_set_object_callback( obj, close_callback, ( long ) fdui );

	fl_end_form();

	return fdui;
}


/***************************************
 ***************************************/

int
main( int    argc,
	  char * argv[ ] )
{
	FD_gravity *grav;
	FD_help *help;
	int i;

	fl_initialize( &argc, argv, "Gravity Demo", 0, 0 );
	help = create_form_help( );
	grav = create_form_gravity( help );
	fl_set_app_mainform( grav->grav );

	for ( i = 0; i < 9; i++ )
	{
		gd[ i ].box = grav->box;
		gd[ i ].grav = g[ i ];
		gd[ i ].rx = grav->rx;
		gd[ i ].ry = grav->ry;
	}

    fl_show_form( grav->grav, FL_PLACE_CENTER | FL_FREE_SIZE,
				  FL_FULLBORDER, "Gravity Demo" );

	fl_do_forms( );

	fl_finish( );

	return 0;
}
