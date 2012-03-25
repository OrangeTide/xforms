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
 *
 * Author: Jens Thoms Toerring <jt@toerring.de>
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include "include/forms.h"


typedef struct {
    FL_FORM   * x;
    void      * vdata;
    char      * cdata;
    long        ldata;
    FL_OBJECT * c;
} FD_x;


/***************************************
 ***************************************/

static void
ccb( FL_OBJECT * ob,
	 long        data  FL_UNUSED_ARG )
{
	unsigned int button = fl_mouse_button( );
	int x = fl_get_label_char_at_mouse( ob );
	char digit;
	const char * label = fl_get_object_label( ob );
	char * str;

	if (    x == -1
		 || ! (    button == FL_LEFT_MOUSE
				|| button == FL_RIGHT_MOUSE
				|| button == FL_SCROLLUP_MOUSE
				|| button == FL_SCROLLDOWN_MOUSE ) )
		return;

	digit = label[ x ];

	if ( button == FL_LEFT_MOUSE || button == FL_SCROLLUP_MOUSE )
	{
		if ( digit == '9' )
			digit = '0';
		else
			++digit;
	}
	else if ( button == FL_RIGHT_MOUSE || button == FL_SCROLLDOWN_MOUSE)
	{
		if ( digit == '0' )
			digit = '9';
		else
			--digit;
	}

	str = fl_strdup( label );
	str[ x ] = digit;
	fl_set_object_label( ob, str );
	fl_free( str );
}


/***************************************
 ***************************************/

static FD_x *
create_form_x( void )
{
    FL_OBJECT *obj;
    FD_x *fdui = ( FD_x * ) fl_malloc( sizeof *fdui );

    fdui->vdata = fdui->cdata = NULL;
    fdui->ldata = 0;

    fdui->x = fl_bgn_form( FL_NO_BOX, 170, 176 );

    obj = fl_add_box( FL_FLAT_BOX, 0, 0, 170, 176, "" );

    obj = fl_add_text( FL_NORMAL_TEXT, 20, 10, 150, 50,
					   "Click on the digits\n(left or right button)" );
    fl_set_object_lsize( obj, FL_NORMAL_SIZE );

    fdui->c = obj = fl_add_button( FL_NORMAL_BUTTON, 40, 65, 90, 40, "012345" );
    fl_set_object_boxtype( obj, FL_EMBOSSED_BOX );
    fl_set_object_color( obj, FL_YELLOW, FL_YELLOW );
    fl_set_object_lsize( obj, FL_LARGE_SIZE );
    fl_set_object_callback( obj, ccb, 0 );

    obj = fl_add_button( FL_NORMAL_BUTTON, 50, 130, 70, 30, "Exit" );

    fl_end_form( );

    fdui->x->fdui = fdui;

    return fdui;
}


/***************************************
 ***************************************/

int
main( int    argc,
      char * argv[ ] )
{
    FD_x *fd_x;

    fl_initialize( &argc, argv, 0, 0, 0 );
    fd_x = create_form_x( );

    fl_show_form( fd_x->x, FL_PLACE_CENTERFREE, FL_FULLBORDER,
				  "strange button" );

    fl_do_forms( );

    if ( fl_form_is_visible( fd_x->x ) )
        fl_hide_form( fd_x->x );

    fl_free( fd_x );
    fl_finish( );

    return 0;
}
