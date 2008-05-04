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
 * \file util.c
 *.
 *  This file is part of the XForms library package.
 *  Copyright (c) 1996-2002  T.C. Zhao and Mark Overmars
 *  All rights reserved.
 *.
 *
 *
 * X independent utilities
 *
 */


#if defined F_ID || defined DEBUG
char *fl_id_util = "$Id: util.c,v 1.12 2008/05/04 21:08:01 jtt Exp $";
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include/forms.h"
#include "flinternal.h"
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>



/***************************************
 * Sets the form windom
 ***************************************/

void
fli_set_form_window( FL_FORM * form )
{
    if ( form && form->window != None )
		flx->win = form->window;
}


/***************************************
 * remove RCS keywords
 ***************************************/

const char *
fli_rm_rcs_kw( const char * s )
{
    static unsigned char buf[ 5 ][ 255 ];
    static int nbuf;
    unsigned char *q = buf[ ( nbuf = ( nbuf + 1 ) % 5 ) ];
    int left = 0,
		lastc = -1;


    while ( *s && ( q - buf[ nbuf ] ) < ( int ) sizeof buf[ nbuf ] - 2 )
    {
		switch ( *s )
		{
			case '$':
				if ( ( left = ! left ) )
					while ( *s && *s != ':' )
						s++;
				break;

			default:
				/* copy the char and remove extra space */
				if ( ! ( lastc == ' ' && *s == ' ' ) )
					*q++ = lastc = *s;
				break;
		}
		s++;
    }

    *q = '\0';

    return ( const char * ) buf[ nbuf ];
}


/***************************************
 * Function is kept at the moment only for backward compatibility
 ***************************************/

static int showerrors = 1;

void
fl_show_errors( int y )
{
    showerrors = y;
}


/***************************************
 * for debugging
 ***************************************/

#define VN( v )  { v, #v }

static FL_VN_PAIR flevent[ ] =
{
    VN( FL_ENTER ),
	VN( FL_LEAVE ),
	VN( FL_PUSH ),
	VN( FL_RELEASE ),
    VN( FL_STEP ),
	VN( FL_SHORTCUT ),
	VN( FL_UPDATE ),
	VN( FL_MOTION ),
    VN( FL_KEYPRESS ),
	VN( FL_DRAW ),
	VN( FL_FOCUS ),
	VN( FL_UNFOCUS ),
    VN( FL_FREEMEM ),
	VN( FL_DRAWLABEL ),
	VN( FL_DBLCLICK ),
    VN( FL_OTHER ),
	VN( FL_ATTRIB ),
    VN( -1 )
};


/***************************************
 ***************************************/

const char *
fli_event_name( int ev )
{
    return fl_get_vn_name( flevent, ev );
}


static FL_VN_PAIR flclass[ ] =
{
    VN( FL_BUTTON ),
	VN( FL_LIGHTBUTTON ),
    VN( FL_ROUNDBUTTON ),
	VN( FL_ROUND3DBUTTON ),
    VN( FL_CHECKBUTTON ),
	VN( FL_BITMAPBUTTON ),
	VN( FL_PIXMAPBUTTON ),
    VN( FL_BITMAP ),
	VN( FL_PIXMAP ),
	VN( FL_BOX ),
	VN( FL_TEXT ),
    VN( FL_MENU ),
	VN( FL_CHART ),
	VN( FL_CHOICE ),
    VN( FL_COUNTER ),
	VN( FL_SLIDER ),
	VN( FL_VALSLIDER ),
	VN( FL_INPUT ),
    VN( FL_BROWSER ),
	VN( FL_DIAL ),
	VN( FL_TIMER ),
	VN( FL_CLOCK ),
    VN( FL_POSITIONER ),
	VN( FL_FREE ),
	VN( FL_XYPLOT ),
    VN( FL_FRAME ),
	VN( FL_LABELFRAME ),
	VN( FL_CANVAS ),
    VN( FL_GLCANVAS ),
	VN( FL_TABFOLDER ),
	VN( FL_SCROLLBAR ),
    VN( FL_SCROLLBUTTON ),
	VN( FL_MENUBAR ),
	VN( FL_IMAGECANVAS ),
    VN( FL_TEXTBOX ),
    VN( -1 )
};


/***************************************
 ***************************************/

const char *
fli_object_class_name( FL_OBJECT * ob )
{
	if ( ! ob )
		return "null";
	else if ( ob == FL_EVENT )
		return "FL_EVENT";

	return fl_get_vn_name( flclass, ob->objclass );
}
