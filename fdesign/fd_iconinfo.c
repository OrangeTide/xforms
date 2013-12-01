/*
 * This file is part of XForms.
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

#include <ctype.h>

#include "include/forms.h"
#include "fd_main.h"
#include "fd_iconinfo.h"


/***************************************
 ***************************************/

IconInfo *
get_iconinfo( FL_OBJECT * obj )
{
	IconInfo *info = obj->c_vdata;

    if (    obj->objclass != FL_PIXMAPBUTTON
         && obj->objclass != FL_BITMAPBUTTON
         && obj->objclass != FL_PIXMAP
         && obj->objclass != FL_BITMAP )
		return NULL;

	if ( ! info )
	{
		info = obj->c_vdata = fl_malloc( sizeof *info );

        info->use_data        = 1;
		info->show_focus      = 1;
		info->dx              = info->dy = 3;
		info->align           = FL_ALIGN_CENTER;
		info->fullpath        = 1;
		*info->filename       = '\0';
		*info->focus_filename = '\0';
		*info->data           = '\0';
		*info->focus_data     = '\0';
		*info->width          = '\0';
		*info->height         = '\0';
	}

	return info;
}


/***************************************
 ***************************************/

void
copy_iconinfo( FL_OBJECT * target,
			   FL_OBJECT * src )
{
	IconInfo *si = get_iconinfo( src ),
		     *ti;

	fli_safe_free( target->c_vdata );

	if ( ! si )
		return;

	ti = get_iconinfo( target );
	*ti = *si;
}
		

/***************************************
 ***************************************/

void
free_iconinfo( FL_OBJECT * obj )
{
	fli_safe_free( obj->c_vdata );
}


/***************************************
 ***************************************/

void
get_xbm_stuff( IconInfo * info )
{
    const char *start,
               *end;
    size_t len;

    if ( ! ( start = strrchr( info->filename, '/' ) ) )
        start = info->filename;

    if ( ( end = strrchr( start, '.' ) ) )
        len = end - start;
    else
        len = strlen( start );

    strcpy( strncpy( info->width,  start, len ) + len, "_width"  );
    strcpy( strncpy( info->height, start, len ) + len, "_height" );
    strcpy( strncpy( info->data,   start, len ) + len, "_bits"   );
}


/***************************************
 * Read in an (already opened) xpm file and return via 'in'
 * the name of the variable for the data. Not very robust
 * code...
 ***************************************/

void
get_xpm_stuff( char * in,
               FILE * fp )
{
    char buf[ 128 ],
         *p;

    while ( fgets( buf, sizeof buf, fp ) )
        if ( ( p = strstr( buf, "static char" ) ) )
        {
            *p += 11;
            while ( *p && *p++ != '*' )
                /* empty */ ;

            if ( ! *p )
                break;

            while ( *p && isspace( ( unsigned char ) *p ) )
                p++;

            while ( *p && *p != '[' )
                *in++ = *p++;

            break;
        }

    *in = '\0';
}


/*
 * Local variables:
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
