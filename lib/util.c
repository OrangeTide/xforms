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
 * \file util.c
 *
 *  This file is part of the XForms library package.
 *  Copyright (c) 1996-2002  T.C. Zhao and Mark Overmars
 *  All rights reserved.
 *
 * X independent utilities
 */

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

static FLI_VN_PAIR flevent[ ] =
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
    return fli_get_vn_name( flevent, ev );
}


static FLI_VN_PAIR flclass[ ] =
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
    VN( FL_SPINNER ),
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

	return fli_get_vn_name( flclass, ob->objclass );
}


/***************************************
 * Function expects a format string as printf() and arguments which
 * must correspond to the given format string and returns a string
 * of the right length into which the arguments are written. The
 * caller of the function is responsible for free-ing the string.
 * -> 1. printf()-type format string
 *    2. As many arguments as there are conversion specifiers etc.
 *       in the format string.
 * <- Pointer to character array of exactly the right length into
 *    which the string characterized by the format string has been
 *    written. On failure, i.e. if there is not enough space, the
 *    function throws an OUT_OF_MEMORY exception.
 ***************************************/

#define STRING_TRY_LENGTH 128

char *
fli_print_to_string( const char * fmt,
					... )
{
    char *c = NULL;
	char *old_c;
    size_t len = STRING_TRY_LENGTH;
    va_list ap;
    int wr;


    while ( 1 )
    {
		old_c = c;
        if ( ( c = fl_realloc( c, len ) ) == NULL )
		{
			fl_safe_free( old_c );
			M_err( "fli_print_to_string", "Running out of memory\n" );
			return NULL;
		}

        va_start( ap, fmt );
        wr = vsnprintf( c, len, fmt, ap );
        va_end( ap );

        if ( wr < 0 )         /* indicates not enough space with older glibs */
        {
            len *= 2;
            continue;
        }

        if ( ( size_t ) wr + 1 > len )   /* newer glibs return the number of */
        {                                /* chars needed, not counting the   */
            len = wr + 1;                /* trailing '\0'                    */
            continue;
        }

        break;
    }

    /* Trim the string down to the number of required characters */

    if ( ( size_t ) wr + 1 < len )
	{
		old_c = c;
        if ( ( c = fl_realloc( c, ( size_t ) wr + 1 ) ) == NULL )
			return old_c;
	}

    return c;
}


/***************************************
 * Function tries to read a line (of arbirary length) from a file
 * On failure (either due to read error or missing memory) NULL is
 * returned, otherwise a pointer to an allocated buffer that must
 * be freed by the caller.
 ***************************************/

char *
fli_read_line( FILE *fp )
{
    char *line = NULL;
	char *old_line = NULL;
    size_t len = STRING_TRY_LENGTH;
	size_t old_len = 0;

    while ( 1 )
    {
        if ( ( line = fl_realloc( line, len ) ) == NULL )
		{
			fl_safe_free( old_line );
			M_err( "fli_read_line", "Running out of memory\n" );
			return NULL;
		}

		if ( ! fgets( line + old_len, len - old_len, fp ) )
		{
			if ( ferror( fp ) )
			{
				M_err( "fli_read_line", "Failed to read from file" );
				fl_free( line );
				return NULL;
			}

			if ( old_len == 0 )
			{
				fl_free( line );
				return NULL;
			}

			M_warn( "fli_read_line", "Missing newline at end of line" );
			break;
		}

		if ( strchr( line + old_len, '\n' ) )
			break;

		old_line = line;
		old_len = len - 1;
		len *= 2;
	}

	old_line = line;
	if ( ( line = fl_realloc( line, strlen( line ) + 1 ) ) == NULL )
		return old_line;
	return line;
}
