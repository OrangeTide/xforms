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
 * \file sp_pixmap.c
 *
 *  This file is part of XForms package
 *  Copyright (c) 1996-2002  T.C. Zhao and Mark Overmars
 *  All rights reserved.
 *
 * Settting pixmap/bitmap class specific attributes. Should
 * be absorbed into button class.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include/forms.h"
#include "fd_main.h"
#include "fd_spec.h"
#include <ctype.h>
#include "sp_pixmap.h"
#include "spec/pixmap_spec.h"


static FD_pixmapattrib *px_attrib;
static FL_OBJECT * curobj;

static FL_OBJECT * create_default_pixmap( FL_OBJECT * obj );
static void get_data_name( FL_OBJECT * obj,
                           IconInfo  * inf );


/***************************************
 ***************************************/

FL_FORM *
pixmap_create_spec_form( void )
{
    if ( px_attrib )
        return px_attrib->pixmapattrib;

    px_attrib = create_form_pixmapattrib( );

    fl_addto_choice( px_attrib->pixalign,
                     align_name( FL_ALIGN_CENTER, 0 ) );
    fl_addto_choice( px_attrib->pixalign, align_name( FL_ALIGN_TOP, 0 ) );
    fl_addto_choice( px_attrib->pixalign,
                     align_name( FL_ALIGN_BOTTOM, 0 ) );
    fl_addto_choice( px_attrib->pixalign, align_name( FL_ALIGN_LEFT, 0 ) );
    fl_addto_choice( px_attrib->pixalign, align_name( FL_ALIGN_RIGHT, 0 ) );
    fl_set_input_return( px_attrib->filename, FL_RETURN_END );

    return px_attrib->pixmapattrib;
}


/***************************************
 ***************************************/

void
pixmap_adjust_spec_form( FL_OBJECT * obj )
{
    curobj = obj;

    if ( obj->objclass == FL_PIXMAP )
        fl_show_object( px_attrib->pixalign );
    else
        fl_hide_object( px_attrib->pixalign );
}


/***************************************
 ***************************************/

void
pixmap_fill_in_spec_form( FL_OBJECT * obj )
{
    IconInfo *info = get_superspec( obj )->cspecv;

    fl_set_button( px_attrib->use_data, info->use_data );
    fl_set_button( px_attrib->fullpath, info->fullpath );
    fl_set_choice_text( px_attrib->pixalign, align_name( info->align, 0 ) );
    fl_set_input( px_attrib->filename, info->filename );
}


/***************************************
 ***************************************/

void
pixmap_reread_spec_form( FL_OBJECT * obj  FL_UNUSED_ARG )
{
    pixmap_filename_change( px_attrib->filename, 0 );
    redraw_the_form( 0 );
}


/***************************************
 ***************************************/

void
pixmap_restore_spec( FL_OBJECT * obj )
{
    IconInfo *info = get_superspec( obj )->cspecv;

    if ( *info->filename )
        ( obj->objclass == FL_PIXMAP ?
          fl_set_pixmap_file : fl_set_bitmap_file )
            ( obj, info->filename );
    else
    {
        /* Show the default broken image link image */

        if ( obj->objclass == FL_PIXMAP )
            set_testing_pixmap( obj );
        else
            set_testing_bitmap( obj );
    }
}


/***************************************
 ***************************************/

void
pixmap_emit_spec_fd_code( FILE      * fp,
                          FL_OBJECT * obj )
{
    FL_OBJECT *defobj = create_default_pixmap( obj );
    IconInfo *info     = get_superspec( obj )->cspecv,
             *definfo = get_superspec( defobj )->cspecv;

    get_data_name( obj, info );

    if ( *info->filename )
    {
        fprintf( fp, "    file: %s\n", info->filename );
        fprintf( fp, "    fullpath: %d\n", info->fullpath );
    }

    if ( *info->data && *info->filename )
        fprintf( fp, "    data: %s\n", info->data );

    if ( *info->width )
        fprintf( fp, "    width: %s\n", info->width );

    if ( *info->height )
        fprintf( fp, "    height: %s\n", info->height );

    if ( info->align != definfo->align )
        fprintf( fp, "    align: %s\n",
                 align_name( fl_to_inside_lalign( info->align ), 0 ) );

    free_superspec( defobj );
    fl_free_object( defobj );
}


/***************************************
 ***************************************/

void
pixmap_emit_spec_c_code( FILE      * fp,
                         FL_OBJECT * obj )
{
    FL_OBJECT *defobj = create_default_pixmap( obj );
    IconInfo *info    = get_superspec( obj )->cspecv,
             *definfo = get_superspec( defobj )->cspecv;

    if ( *info->filename && ! info->use_data )
        fprintf( fp, "    fl_set_%s_file( obj, \"%s\" );\n",
                 obj->objclass == FL_PIXMAP ? "pixmap" : "bitmap",
                 info->filename );

    if ( *info->data && info->use_data && *info->filename )
    {
        if ( obj->objclass == FL_PIXMAP )
            fprintf( fp, "    fl_set_pixmap_data( obj, %s );\n",
                     info->data );
        else
            fprintf( fp, "    fl_set_bitmap_data( obj, %s, %s, "
                     "( unsigned char * ) %s );\n",
                     info->width, info->height, info->data );
    }

    if ( obj->objclass == FL_PIXMAP&& info->align != definfo->align )
        fprintf( fp, "    fl_set_pixmap_align( obj, %s, %d, %d );\n",
                 align_name( fl_to_inside_lalign(info->align ), 1 ), info->dx,
                 info->dy );

    free_superspec( defobj );
    fl_free_object( defobj );
}


/***************************************
 ***************************************/

void
pixmap_emit_spec_header( FILE      * fp,
                         FL_OBJECT * obj )
{
    IconInfo *info = get_superspec( obj )->cspecv;

    if ( info->use_data && *info->data && *info->filename )
    {
        const char *fn = info->fullpath ? info->filename
                                        : file_tail( info->filename );

        if ( ! is_duplicate_info( fn ) )
            fprintf( fp, "#include \"%s\"\n", fn );
    }
}


/***************************************
 ***************************************/

void
pixmapusedata_change( FL_OBJECT * obj,
                      long        data  FL_UNUSED_ARG )
{
    ( ( IconInfo * ) get_superspec( obj )->cspecv )->use_data =
                                                          fl_get_button( obj );
}


/***************************************
 ***************************************/

void
pixmapfullpath_cb( FL_OBJECT * obj,
                   long        data  FL_UNUSED_ARG )
{
    ( ( IconInfo * ) get_superspec( obj )->cspecv )->fullpath =
                                                          fl_get_button( obj );
}


/***************************************
 ***************************************/

void
pixmap_filename_change( FL_OBJECT * obj,
                        long        data  FL_UNUSED_ARG )
{
    IconInfo *info = get_superspec( obj )->cspecv;

    strcpy( info->filename, fl_get_input( obj ) );

    if ( *info->filename )
        ( curobj->objclass == FL_PIXMAP ?
          fl_set_pixmap_file : fl_set_bitmap_file )
            ( curobj, info->filename );
    else
    {
        if ( obj->objclass == FL_PIXMAP )
            set_testing_pixmap( obj );
        else
            set_testing_bitmap( obj );
    }

    redraw_the_form( 0 );
}


/***************************************
 ***************************************/

void
pixmaplookfor_pixmapfile_cb( FL_OBJECT * obj   FL_UNUSED_ARG,
                             long        data  FL_UNUSED_ARG )
{
    const char *fn;
    char buf[ 2048 ];
    char *cwd;

    fl_use_fselector( XPM_FSELECTOR );
    fl_set_fselector_placement( FL_PLACE_MOUSE );

    if ( curobj->objclass == FL_PIXMAP )
        fn = fl_show_fselector( "XPM file", "", "*.xpm", "" );
    else
        fn = fl_show_fselector( "XBM file", "", "*.xbm", "" );

    if ( ! fn )
        return;

    if ( strstr( fn, cwd = fli_getcwd( buf, sizeof buf - 2 )) )
        fn += strlen( cwd ) + 1;

    fl_set_input( px_attrib->filename, fn );
    fl_call_object_callback( px_attrib->filename );
}


/***************************************
 ***************************************/

void
pixmapalign_change( FL_OBJECT * obj,
                    long        data  FL_UNUSED_ARG )
{
    IconInfo *info = get_superspec( obj )->cspecv;
    const char *s = fl_get_choice_text( obj );

    info->align = align_val( s );

    /* Don't allow outside align */

    fl_set_pixmap_align( curobj, fl_to_inside_lalign( info->align ),
                         info->dx, info->dy );
    redraw_the_form( 0 );
}


/***************************************
 ***************************************/

static FL_OBJECT *
create_default_pixmap( FL_OBJECT * obj )
{
    FL_OBJECT *defobj = NULL;

    /* Create a default object */

    if ( obj->objclass == FL_PIXMAP )
        defobj = fl_create_pixmap( obj->type, 0, 0, 0, 0, "" );
    else
        defobj = fl_create_bitmapbutton( obj->type, 0, 0, 0, 0, "" );

    return defobj;
}


/***************************************
 ***************************************/

static void
get_data_name( FL_OBJECT * ob,
               IconInfo  * inf )
{
    FILE *fp;

    *inf->data   = '\0';
    *inf->width  = '\0';
    *inf->height = '\0';

    if ( ! inf->use_data || ! *inf->filename )
        return;

    if ( ( fp = fopen( inf->filename, "r" ) ) )
    {
        if ( ob->objclass == FL_PIXMAP )
            get_xpm_stuff( inf->data, fp );
        else
            get_xbm_stuff( inf );

        fclose( fp );
    }

    if ( ! fp || ! *inf->data )
    {
        fprintf( stderr, "Can't open or read %s\n", inf->filename );
        *inf->filename = '\0';
    }
}


#include "spec/pixmap_spec.c"


/*
 * Local variables:
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */

