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
 * \file goodie_alert.c
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


/****************** Alert dialog ********************{**/

typedef struct
{
    FL_FORM   * form;
    FL_OBJECT * str;
    FL_OBJECT * but;
    FL_OBJECT * title;
} FD_alert;


static FD_alert *fd_alert;


/***************************************
 ***************************************/

static FD_alert *
create_alert( const char * title,
			  const char * msg )
{
    FD_alert *fdui = fl_calloc( 1, sizeof *fdui );
    int oldy = fli_inverted_y;
    int oldu = fl_get_coordunit( );
	int style,
		size;
	int w1 = 0,
		h1 = 0,
		w2 = 0,
		h2 = 0;
	int bw,
		bh;
    FL_OBJECT *ob;

    fli_inverted_y = 0;
    fl_set_coordunit( FL_COORD_PIXEL );

	fli_get_goodies_font( &style, &size );
	if ( title )
		fl_get_string_dimension( style, size, title, strlen( title ),
								 &w1, &h1 );
	if ( msg )
		fl_get_string_dimension( style, size, msg, strlen( msg ), &w2, &h2 );

	bw = FL_max( 400, FL_max( w1, w2 ) + 80 );
	bh = h1 + h2 + 77;

    fdui->form = fl_bgn_form( FL_NO_BOX, bw, bh );
    fl_set_form_title( fdui->form, "Alert" );
    fli_get_goodie_title( fdui->form, FLAlertTitle );

    ob = fl_add_box( FL_UP_BOX, 0, 0, bw, bh, "" );
    fl_set_object_bw( ob, -2 );

    fdui->title = fl_add_box( FL_FLAT_BOX, 50, 10, bw - 70, h1,
							  title ? title : "" );
    fl_set_object_lstyle( fdui->title, FL_BOLD_STYLE );
    fl_set_object_lsize( fdui->title, FL_NORMAL_SIZE );

    fl_add_box( FL_FLAT_BOX, 50, h1 + 20, bw - 70, 5, "@DnLine" );

    fdui->str = fl_add_text( FL_FLAT_BOX, 50, h1 + 30, bw - 70, h2,
							 msg ? msg : "" );
    fl_set_object_lalign( fdui->str, FL_ALIGN_CENTER );

    fdui->but = fl_add_button( FL_RETURN_BUTTON, ( bw - 90 ) / 2, bh - 37,
							   90, 27, "Dismiss" );
    fli_add_warn_icon( 8, h1 + 5, 35, 35 );
	fli_parse_goodies_label( fdui->but, FLAlertDismissLabel );

    fli_handle_goodie_font( fdui->but, fdui->str );

    fl_set_form_hotobject( fdui->form, fdui->but );

    fl_end_form( );

    fl_register_raw_callback( fdui->form, FL_ALL_EVENT,
							  fli_goodies_preemptive );
    fl_set_form_atclose( fdui->form, fl_goodies_atclose, fdui->but );
    fdui->form->fdui = fdui;

    if ( fli_cntl.buttonFontSize != FL_DEFAULT_SIZE )
		fl_fit_object_label( fdui->but, 20, 2 );

    fli_inverted_y = oldy;
    fl_set_coordunit( oldu );

    return fdui;
}


/***************************************
 ***************************************/

static void
show_it( const char * title,
		 const char * msg,
		 int          c )
{
    if ( fd_alert )
	{
		fl_hide_form( fd_alert->form );
		fl_free_form( fd_alert->form );
		fd_alert = NULL;
	}
	else
		fl_deactivate_all_forms( );


	fd_alert = create_alert( title, msg );

    fl_show_form( fd_alert->form, c ? FL_PLACE_CENTER : FL_PLACE_HOTSPOT,
				  FL_TRANSIENT, fd_alert->form->label );

    fl_update_display( 1 );

    while ( fl_do_only_forms( ) != fd_alert->but )
		/* empty */ ;

    fl_hide_form( fd_alert->form );
	fl_free_form( fd_alert->form );
	fl_safe_free( fd_alert );
    fl_activate_all_forms( );
}


/***************************************
 * Show a simple message with an alert icon and a dismiss button
 ***************************************/

void
fl_show_alert( const char * title,
			   const char * str1,
			   const char * str2,
			   int          c )
{
    char *buf;

	buf= fl_malloc(   ( str1 ? strlen( str1 ) : 0 ) + 1
					+ ( str2 ? strlen( str2 ) : 0 ) + 1 );
    sprintf( buf,"%s\n%s", str1 ? str1 : "", str2 ? str2 : "" );
	show_it( title, buf, c );
	fl_free( buf );
}


/***************************************
 ***************************************/

void
fl_show_alert2( int c,
				const char * fmt,
				... )
{
	char *buf,
		 *p;
	int len;
	int written;
	va_list ap;

	if ( ! fmt || ! * fmt )
	{
		M_warn( "fl_show_msg", "NULL or empty format string" );
		return;
	}

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

		/* Take care: e.g. in older libc versions a negative value got
		   returned if the buffer wasn't large enough while newer ones
		   follow C99 and return the length of the string that would be
		   needed (but without the trailing '\0') */

		if ( written > -1 && written < len )
			break;

		len = written < 0 ? ( 2 * len ) : ( written + 1 );
		buf = fl_realloc( buf, len );
	}

	p = strchr( buf, '\f' );
	if ( p )
		*p++ = '\0';
	else
		p = NULL;

	show_it( buf, p, c );
}


/***************************************
 ***************************************/

void
fl_hide_alert( void )
{
    if ( fd_alert && fd_alert->form->visible )
		fli_object_qenter( fd_alert->but );
	else
		M_warn( "fl_hide_alert", "No alert box is shown" );
}
