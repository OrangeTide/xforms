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
 * \file fd_file.c
 *
 *.
 *  This file is part of XForms package
 *  Copyright (c) 1996-2002  T.C. Zhao and Mark Overmars
 *  All rights reserved.
 *.
 *
 * This file is part of the Form Designer.
 *
 * It contains the routines to save and load forms in the internal
 * format used by the form designer. This is readable ASCII.
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include/forms.h"
#include "flinternal.h"
#include "fd_main.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>

/* Writes an obj description to a file. Note externally the coordinate
 * system always starts from lower-left corner of the screen */

#if 0
#define OBJ_Y(ob)   ob->form->h - ob->y - ob->h
#else
#define OBJ_Y(ob)   obj->y
#endif


/***************************************
 ***************************************/

static void
save_object( FILE      * fl,
			 FL_OBJECT * obj )
{
    char name[ MAX_VAR_LEN ],
		 cbname[ MAX_VAR_LEN ],
		 argname[ MAX_VAR_LEN ];
	char *label;
    double sc = get_conversion_factor( );
    FL_OBJECT fake_obj;

    if ( obj->parent )
		return;

    get_object_name( obj, name, cbname, argname );

    fprintf( fl, "\n--------------------\n" );
    fprintf( fl, "class: %s\n", class_name( obj->objclass ) );
    fprintf( fl, "type: %s\n", find_type_name( obj->objclass, obj->type ) );

    fake_obj.x = obj->x;
    fake_obj.y = obj->y;
    fake_obj.w = obj->w;
    fake_obj.h = obj->h;
    fl_scale_object( &fake_obj, sc, sc );
    fprintf( fl, "box: %d %d %d %d\n", fake_obj.x, fake_obj.y,
			 fake_obj.w, fake_obj.h );

    fprintf( fl, "boxtype: %s\n", boxtype_name( obj->boxtype ) );
    fprintf( fl, "colors: %s %s\n", fli_query_colorname( obj->col1 ),
			 fli_query_colorname( obj->col2 ) );
    fprintf( fl, "alignment: %s\n", align_name( obj->align ) );
    fprintf( fl, "style: %s\n", style_name( obj->lstyle ) );
    fprintf( fl, "size: %s\n", lsize_name( obj->lsize ) );
    fprintf( fl, "lcol: %s\n", fli_query_colorname( obj->lcol ) );
	label = get_label( obj, 0 );
    fprintf( fl, "label: %s\n", label );
	fl_free( label );
    fprintf( fl, "shortcut: %s\n", get_shortcut_string( obj ) );
    fprintf( fl, "resize: %s\n", resize_name( obj->resize ) );
    fprintf( fl, "gravity: %s %s\n",
			 gravity_name( obj->nwgravity ),
			 gravity_name( obj->segravity ) );
    fprintf( fl, "name: %s\n", name );
    fprintf( fl, "callback: %s\n", cbname );
    fprintf( fl, "argument: %s\n", argname );

    save_objclass_spec_info( fl, obj );
}


/***************************************
 * My version of fgets, removing heading name:
 ***************************************/

static void
myfgets( char * line,
		 FILE * fl )
{
    char tmpstr[ 10000 ];		/* Maximal label length is limited here. */
    int i = 0,
		j;
    int ch = fgetc(fl);

    while ( ch != '\n' && ch != EOF )
    {
		tmpstr[ i++ ] = ch;
		ch = fgetc( fl );
    }

    tmpstr[ i ] = '\0';

    i = 0;
    while ( tmpstr[ i ] != ':' && tmpstr[ i + 1 ] != ' ' )
		i++;

    i += 2;
    j = 0;

    do
		line[ j++ ] = tmpstr[ i++ ];
    while ( tmpstr[ i - 1 ] != '\0' );
}


/*
 * X version changed color systemtically, need to do a translation
 * from old fd files on the fly
 *
 */

typedef struct {
    int oldval,
	    newval;
} Trantable;

static Trantable tcolor[ ] =
{
    {  0, FL_BLACK },
    {  1, FL_RED },
    {  2, FL_GREEN },
    {  3, FL_YELLOW },
    {  4, FL_BLUE },
    {  5, FL_MAGENTA },
    {  6, FL_CYAN },
    {  7, FL_WHITE },
    {  8, FL_BOTTOM_BCOL },	/* approx */

    {  9, FL_INDIANRED },
    { 10, FL_PALEGREEN },
    { 11, FL_PALEGREEN },
    { 12, FL_SLATEBLUE },

    { 35, FL_RIGHT_BCOL },
    { 36, FL_RIGHT_BCOL },	/* approx */
    { 37, FL_RIGHT_BCOL },	/* approx  */
    { 40, FL_BOTTOM_BCOL },
    { 47, FL_COL1 },
    { 49, FL_MCOL },
    { 51, FL_TOP_BCOL },
    { 55, FL_LEFT_BCOL }
};

static Trantable tclass[ ] =
{
    {     1, FL_BOX },
    {     2, FL_TEXT },
    {     3, FL_BITMAP },
    {     4, FL_CHART },
    {    11, FL_BUTTON },
    {    12, FL_LIGHTBUTTON },
    {    13, FL_ROUNDBUTTON },
    {    21, FL_SLIDER },
    {    22, FL_DIAL },
    {    23, FL_POSITIONER },
    {    24, FL_VALSLIDER },
    {    25, FL_COUNTER },
    {    31, FL_INPUT },
    {    41, FL_MENU },
    {    42, FL_CHOICE },
    {    61, FL_CLOCK },
    {    62, FL_TIMER },
    {    71, FL_BROWSER },
    {   101, FL_FREE },
    { 10000, FL_BEGIN_GROUP },
    { 20000, FL_END_GROUP }
};

static Trantable talign[] =
{
    { 0, FL_ALIGN_TOP },
    { 1, FL_ALIGN_BOTTOM },
    { 2, FL_ALIGN_LEFT },
    { 3, FL_ALIGN_RIGHT },
    { 4, FL_ALIGN_CENTER }
};

static Trantable tbtype[ ] =
{
    { 0, FL_NO_BOX },
    { 1, FL_UP_BOX },
    { 2, FL_DOWN_BOX },
    { 3, FL_FLAT_BOX },
    { 4, FL_BORDER_BOX },
    { 5, FL_SHADOW_BOX },
    { 6, FL_FRAME_BOX },
    { 7, FL_ROUNDED_BOX },
    { 8, FL_RFLAT_BOX },
    { 9, FL_RSHADOW_BOX }
};


/***************************************
 ***************************************/

static int
do_trans( Trantable * tab,
		  int         n,
		  int         old )
{
    Trantable *p = tab, *q;

    for ( q = p + n; p < q; p++ )
		if ( p->oldval == old )
			return p->newval;
    return old;
}


#define new_class( o )  do_trans( tclass, sizeof tclass / sizeof *tclass, o )
#define new_color( o )  do_trans( tcolor, sizeof tcolor / sizeof *tcolor, o )
#define new_align( o )  do_trans( talign, sizeof talign / sizeof *talign, o )
#define new_btype( o )  do_trans( tbtype, sizeof tbtype / sizeof *tbtype, o )


/***************************************
 ***************************************/

static void
fd_skip_comment( FILE * fp  FL_UNUSED_ARG )
{
#if 0
    int c,
		done = 0;

    while ( ! done )
    {
		if ( ( c = getc( fp ) ) == '#' || c == ';' )
			while ( ( c = getc( fp ) ) != '\n' && c != EOF )
				/* empty */ ;
		else
		{
			done = 1;
			ungetc( c, fp );
		}
    }
#endif
}


/***************************************
 * Read lines consisting of keyword: value and split. Return EOF on EOF
 * or error. On success 'key' is set to an allocated buffer with the
 * keyword and 'val' points to the value (if any). It is the responsibility
 * of the caller to free the memory for 'key', but it must be taken into
 * account that this also frees the memory used for 'val'!
 ***************************************/

int
read_key_val( FILE * fp,
			  char ** key,
			  char ** val )
{
    char *p;


    fd_skip_comment( fp );

	if ( ( *key = fli_read_line( fp ) ) == NULL )
		return EOF;

    /* Nuke the new line */

    if ( ( p = strchr( *key, '\n' ) ) )
		*p = '\0';

    if ( ! ( p = strchr ( *key, ':' ) ) )
	{
		fl_free( *key );
		return EOF;
	}

    *p = '\0';

	*val = p + 1;
    if ( **val )
		*val += 1;

    return 0;
}


/***************************************
 *  Loads an object from the file and returns it.
 *
 *  object coordinates are measured from lower-left corner. fl_add_object
 *  will do the proper conversion but if obj->y is manipulated directly
 *  need to do the transformation manually
 ***************************************/

#define Str( x ) #x
#define XStr( x ) Str( x )

static FL_OBJECT *
load_object( FILE * fl )
{
    FL_OBJECT *obj;
    int objclass,
		type;
    float x,
		  y,
		  w,
		  h;
    char name[ MAX_VAR_LEN + 1 ],
		 cbname[ MAX_VAR_LEN + 1 ] = "",
		 argname[ MAX_VAR_LEN + 1 ] = "";
    char cn1[ MAX_VAR_LEN + 1 ],
		 cn2[ MAX_VAR_LEN + 1 ];
    char objcls[ MAX_VAR_LEN + 1 ];
    char *key,
	     *val;

    /* Must demand the vital info */

    if ( fscanf( fl, "\n--------------------\n" ) == EOF )
    {
		M_err( "load_object", "Error reading input file" );
		return 0;
    }

    if (    fscanf( fl, "class: %" XStr( MAX_VAR_LEN ) "s\n", objcls ) != 1
		 || fscanf( fl, "type: %" XStr( MAX_VAR_LEN ) "s\n", name ) != 1
		 || fscanf( fl, "box: %f %f %f %f\n", &x, &y, &w, &h ) != 4 )
    {
		M_err( "load_object", "Error reading input file" );
		return 0;
    }

	objcls[ MAX_VAR_LEN - 1 ] = '\0';
	name[ MAX_VAR_LEN - 1 ] = '\0';
    objclass = class_val( objcls );
    type = find_type_value( objclass, name );

    if ( fd_magic == MAGIC2 )
		objclass = new_class( objclass );

	*name = '\0';

    if ( cur_form && fd_magic != FD_V1 )
		y = cur_form->h - y - h;

    /* Create this object */

    obj = add_an_object( objclass, type, x, y, w, h );

    if ( obj == NULL )
    {
		fl_show_alert2( 1, "Unknown Object\nObject (class=%s(%d) type=%s) "
						"discarded", objcls, objclass, val );
		return NULL;
    }

    /* Now parse the attributes */

    while ( read_key_val( fl, &key, &val ) != EOF )
    {
		if ( strcmp( key, "boxtype") == 0 )
			obj->boxtype = boxtype_val( val );
		else if ( strcmp( key, "colors" ) == 0 )
		{
			cn1[ 0 ] = cn2[ 0 ] = '\0';
			sscanf( val, "%" XStr( MAX_VAR_LEN ) "s%" XStr( MAX_VAR_LEN ) "s",
					cn1, cn2 );
			cn1[ MAX_VAR_LEN - 1 ] = '\0';
			cn2[ MAX_VAR_LEN - 1 ] = '\0';
			obj->col1 = fli_query_namedcolor( cn1 );
			obj->col2 = fli_query_namedcolor( cn2 );
			if ( obj->col1 == 0x8fffffff )
				obj->col1 = FL_NoColor;
		}
		else if ( strcmp( key, "alignment" ) == 0 )
			obj->align = align_val( val );
		else if ( strcmp( key, "style") == 0 || strcmp( key, "lstyle" ) == 0 )
			obj->lstyle = style_val( val );
		else if ( strcmp( key, "size") == 0 || strcmp( key, "lsize" ) == 0 )
			obj->lsize = lsize_val( val );
		else if ( strcmp( key, "lcol" ) == 0 )
			obj->lcol = fli_query_namedcolor( val );
		else if ( strcmp( key, "resize" ) == 0 )
			obj->resize = resize_val( val );
		else if ( strcmp( key, "label" ) == 0 )
			set_label( obj, val );
		else if ( strcmp( key, "shortcut" ) == 0 )
			set_shortcut( obj, val );
		else if ( strcmp( key, "callback" ) == 0 )
		{
			strncpy( cbname, val, MAX_VAR_LEN - 1 );
			cbname[ MAX_VAR_LEN - 1 ] = '\0';
		}
		else if ( strcmp( key, "name" ) == 0 )
		{
			strncpy( name, val, MAX_VAR_LEN - 1 );
			name[ MAX_VAR_LEN - 1 ] = '\0';
		}
		else if ( strcmp( key, "gravity" ) == 0 )
		{
			cn1[ 0 ] = cn2[ 0 ] = '\0';
			sscanf( val, "%" XStr( MAX_VAR_LEN ) "s %" XStr( MAX_VAR_LEN ) "s",
					cn1, cn2 );
			cn1[ MAX_VAR_LEN - 1 ] = '\0';
			cn2[ MAX_VAR_LEN - 1 ] = '\0';
			obj->nwgravity = gravity_val( cn1 );
			obj->segravity = gravity_val( cn2 );
		}
		else if ( strcmp( key, "argument" ) == 0 )
		{
			strncpy( argname, val, MAX_VAR_LEN - 1 );
			argname[ MAX_VAR_LEN - 1 ] = '\0';
			fl_safe_free( key );
			goto done;
		}
		else
			fprintf( stderr, "Unknown keyword %s ignored\n", key );

		fl_safe_free( key );
    }

 done:

    /* Do the translation from old fdesign on the fly */

    if ( fd_magic == MAGIC2 )
    {
		obj->col1 = new_color( obj->col1 );
		obj->col2 = new_color( obj->col2 );
		obj->lcol = new_color( obj->lcol );
		obj->align = new_align( obj->align );
		obj->boxtype = new_btype( obj->boxtype );
    }

    set_object_name( obj, name, cbname, argname );

    /* Load object specific info */

    fd_skip_comment( fl );
    load_objclass_spec_info( fl, obj );

    return obj;
}


/***************************************
 * Saves a form definition to the file
 ***************************************/

void
write_form( FILE    * fl,
			FL_FORM * form,
			char      fname[ ] )
{
    int onumb;
    FL_OBJECT *obj;

    fprintf( fl, "\n=============== FORM ===============\n" );
    fprintf( fl, "Name: %s\n", fname );
    fprintf( fl, "Width: %d\n", convert_u( form->w ) );
    fprintf( fl, "Height: %d\n", convert_u( form->h ) );

    /* print the object number */

    for ( onumb = 0, obj = form->first->next; obj; obj = obj->next )
		onumb += obj->parent == NULL;

    fprintf( fl, "Number of Objects: %d\n", onumb );

    /* print the objects */

    obj = form->first->next;
    while ( obj != NULL )
    {
		save_object( fl, obj );
		obj = obj->next;
    }
}


/***************************************
 * Loads a form definition from the file
 ***************************************/

int
read_form( FILE * fl,
		   char * fname )
{
    double w, h;
    int onumb,
		i,
		ok;
    char buf[ 256 ],
		 *s;

    /* Skip until we get ===, the form seperator */

    while ( fgets( buf, sizeof buf - 1, fl ) && strncmp( buf, "===", 3 ) )
		/* empty */ ;

    myfgets( fname, fl );
    if (    fscanf( fl, "Width: %lf\n", &w ) != 1
			|| fscanf( fl, "Height: %lf\n", &h ) != 1 )
    {
		M_err( "LoadForm", " Can't read Width or Height" );
		return -1;
    }

    if ( w <= 0.0 || h <= 0.0 || feof( fl ) )
    {
		M_err( "LoadForm", " Invalid Width/Height: %f %f", w, h );
		return -1;
    }

    cur_form = fl_bgn_form( FL_NO_BOX, ( FL_Coord ) w, ( FL_Coord ) h );
    fl_end_form( );

    /* between width/height and number of objects, we can put anything we
       want. */

    while (    ( s = fgets( buf, sizeof buf  - 1, fl ) )
			&& strncmp( s, "Number of O", 11 ) )
    {
		/* see what this is */		
    }

    sscanf( buf, "Number of Objects: %d", &onumb );

    for ( ok = 1, i = 0; i < onumb && ok; i++ )
		ok = load_object( fl ) != 0;

    return 0;
}
