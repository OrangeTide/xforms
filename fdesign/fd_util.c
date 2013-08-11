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


/**
 * \file fd_util.c
 *
 * Eliminate the emission of duplicate info. This is necessary as
 * some #include define data (pixmap for example).
 *
 * We should eventually move the functionality of already_emited in
 * fd_printC.c into this function so callback is also checked. This
 * probably means we need make struct CodeInfo more efficient.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include/forms.h"
#include "fd_main.h"

#define MAXREC   80
#define MAXLEN   79

typedef struct {
    FL_FORM * form;
    char      buf[ MAXLEN + 1 ];
} CodeInfo;

static CodeInfo info[ MAXREC ];
static int ninfo;


/***************************************
 ***************************************/

void
reset_dupinfo_cache( void )
{
    ninfo = 0;
}


/***************************************
 ***************************************/

int
is_duplicate_info( FL_OBJECT  * ob  FL_UNUSED_ARG,
                   const char * s )
{
    int i;

    for ( i = 0; i < ninfo; i++ )
        if ( strcmp( s, info[ i ].buf ) == 0 )
            return 1;

    if ( ninfo == MAXREC )
    {
        fprintf( stderr, "dupinfo cache overflown\n" );
        ninfo--;
    }

    fli_sstrcpy( info[ ninfo++ ].buf, s, MAXLEN );

    return 0;
}


/***************************************
 * Returns a newly allocated string with the aboslute path
 * for the input path
 ***************************************/

char *
rel2abs( const char * rel_path )
{
    char * abs_path = NULL;
    char *res;

    if ( *rel_path == '/' )
        abs_path = fl_strdup( rel_path );
    else
    {
        long path_max = pathconf(".", _PC_PATH_MAX);
        size_t size;

        if ( path_max == -1 )
            size = 1024;

        while ( 1 )
        {
            abs_path = fl_realloc( abs_path, size + strlen( rel_path ) + 2 );
            if ( ! getcwd( abs_path, size ) )
                size += 1024;
            else
                break;
        }

        strcat( strcat( abs_path, "/" ), rel_path );
    }

    while ( ( res = strstr( abs_path, "/./" ) ) )
        memmove( res, res + 2, strlen( res ) - 1 );

    while ( ( res = strstr( abs_path, "/../" ) ) )
    {
		if ( res != abs_path )
		{
			char * dest = res - 1;

			while ( *dest != '/' )
				dest--;

			memmove( dest, res + 3, strlen( res ) - 2 );
		}
		else
			memmove( abs_path, abs_path + 3, strlen( abs_path ) - 2 );
    }

    return fl_realloc( abs_path, strlen( abs_path ) + 1 );
}


/***************************************
 * Returns if a string can be used as a valid C identifier
 ***************************************/

int
is_valid_c_name( const char * str )
{
    const char * sp;

    if ( fdopt.lax )
        return 1;

    if (    ! isascii( ( unsigned char ) *str )
         || ! ( isalpha( ( unsigned char ) *str ) || *str == '_' ) )
        return 0;

    for ( sp = str + 1; *sp; sp++ )
        if (    ! isascii( ( unsigned char ) *sp )
             || ! ( isalnum( ( unsigned char ) *sp ) || *sp == '_' ) )
            return 0;

    return 1;
}


/*
 * Local variables:
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
