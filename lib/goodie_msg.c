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
 * \file goodie_msg.c
 *
 *.
 *  This file is part of the XForms library package.
 *  Copyright (c) 1996-2002  T.C. Zhao and Mark Overmars
 *  All rights reserved.
 *.
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include/forms.h"
#include "flinternal.h"
#include "private/flsnprintf.h"


/*************** Three line messages *******************{*****/

typedef struct
{
    FL_FORM   * form;
    FL_OBJECT * str;
    FL_OBJECT * but;
} FD_msg;


/***************************************
 ***************************************/

static FD_msg *
create_msg( void )
{
    FD_msg *fdui = fl_calloc( 1, sizeof *fdui );
    int oldy = fl_inverted_y;
    int oldu = fl_get_coordunit( );

    fl_inverted_y = 0;
    fl_set_coordunit( FL_COORD_PIXEL );

    fdui->form = fl_bgn_form( FL_UP_BOX, 460, 130 );

    fdui->str = fl_add_box( FL_FLAT_BOX, 20, 15, 420, 65, "" );

    fdui->but = fl_add_button( FL_RETURN_BUTTON, 185, 94, 90, 27, "OK" );
    fl_set_form_hotobject( fdui->form, fdui->but );

    fl_end_form( );

    fl_register_raw_callback( fdui->form, FL_ALL_EVENT, fl_goodies_preemptive );

    fl_set_form_atclose( fdui->form, fl_goodies_atclose, fdui->but );

    if ( fl_cntl.buttonFontSize != FL_DEFAULT_SIZE )
		fl_fit_object_label( fdui->but, 18, 2 );

    fl_inverted_y = oldy;
    fl_set_coordunit( oldu );

    return fdui;
}


static FD_msg *fd_msg;


/***************************************
 * Shows a simple message with an OK button
 ***************************************/

void
fl_show_messages( const char *str )
{
    static int first = 1;
    int alreadyon;

    if ( ! fd_msg )
		fd_msg = create_msg( );

    if ( first )
    {
		fl_parse_goodies_label( fd_msg->but, FLOKLabel );
		first = 0;
    }

    fl_handle_goodie_font( fd_msg->but, fd_msg->str );

    alreadyon = fd_msg->form->visible;
    fl_set_object_label( fd_msg->str, str );

    if ( ! alreadyon )
		fl_deactivate_all_forms( );

    fl_show_form( fd_msg->form, FL_PLACE_HOTSPOT, FL_TRANSIENT, "Message" );

    fl_update_display( 1 );

    while ( fl_do_only_forms( ) != fd_msg->but )
		/* empty */ ;

    fl_hide_form( fd_msg->form );
    fl_activate_all_forms( );
}


/***************************************
 ***************************************/

void
fl_show_msg( const char * fmt,
			 ... )
{
	char *buf,
		 *p;
	int len;
	int written;
	va_list ap;

	if ( ! fmt )
		return;

	/* Try to come up with an estimate of the length required for the
	   whole string */

	len = strlen( fmt ) + 1;

	for ( p = strchr( fmt, '%' ); p; p = strchr( ++p, '%' ) )
		len += 15;

	buf = fl_malloc( len );

	while ( 1 )
	{
		va_start( ap, fmt );
		written = fl_vsnprintf( buf, len, fmt, ap );
		va_end( ap );

		/* Take care: e.g. in older libc versiobs a negative value got
		   returned if the buffer wasn't large enough while newer ones
		   follow C99 and return the length of the string that would be
		   needed (but without the trailing '\0') */

		if ( written > -1 && written < len )
			break;

		len = written < 0 ? ( 2 * len ) : ( written + 1 );
		buf = fl_realloc( buf, len );
	}

	fl_show_messages( buf );

	fl_free( buf );
}


/***************************************
 ***************************************/

void
fl_show_message( const char * s1,
				 const char * s2,
				 const char * s3 )
{
	char *buf;
	size_t len;

	len =   ( s1 ? strlen( s1 ) : 0 ) + 1
		  + ( s2 ? strlen( s2 ) : 0 ) + 1
		  + ( s3 ? strlen( s3 ) : 0 ) + 1;

	buf = fl_malloc( len );

	fl_snprintf( buf, len, "%s\n%s\n%s",
				 s1 ? s1 : "", s2 ? s2 : "", s3 ? s3 : "" );

	fl_show_messages( buf );

	fl_free( buf );
}


/***************************************
 ***************************************/

void
fl_hide_message( void )
{
    if ( fd_msg && fd_msg->form->visible )
		fl_object_qenter( fd_msg->but );
}
