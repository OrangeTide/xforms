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
 * \file sp_button.c
 *
 *  This file is part of XForms package
 *  Copyright (c) 1996-2002  T.C. Zhao and Mark Overmars
 *  All rights reserved.
 *
 * Settting button class specific attributes.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include/forms.h"
#include "fd_main.h"
#include "fd_spec.h"
#include <ctype.h>
#include "spec/button_spec.h"

extern FD_buttonattrib *create_form_buttonattrib( void );

static FD_buttonattrib *bt_attrib;
static SuperSPEC *button_spec;
static IconInfo *info;

static void show_spec( SuperSPEC * );
static void get_data_name( FL_OBJECT *,
                           IconInfo * );
static FL_OBJECT *edited;


#define IsIconButton( cls ) \
    ( ( cls ) == FL_BITMAPBUTTON || ( cls ) == FL_PIXMAPBUTTON )


/***************************************
 ***************************************/

void *
get_button_spec_fdform( void )
{
    if ( ! bt_attrib )
    {
        bt_attrib = create_form_buttonattrib( );
        fl_addto_choice( bt_attrib->pixalign,
                         align_name( FL_ALIGN_CENTER, 0 ) );
        fl_addto_choice( bt_attrib->pixalign,
                         align_name( FL_ALIGN_TOP, 0 ) );
        fl_addto_choice( bt_attrib->pixalign,
                         align_name( FL_ALIGN_BOTTOM, 0 ) );
        fl_addto_choice( bt_attrib->pixalign,
                         align_name( FL_ALIGN_LEFT, 0 ) );
        fl_addto_choice( bt_attrib->pixalign,
                         align_name( FL_ALIGN_RIGHT, 0 ) );

        fl_set_input_return( bt_attrib->filename, FL_RETURN_END );
        fl_set_input_return( bt_attrib->focus_filename, FL_RETURN_END );
    }

    return bt_attrib;
}


/***************************************
 ***************************************/

void
button_spec_restore( FL_OBJECT * obj   FL_UNUSED_ARG,
                     long        data  FL_UNUSED_ARG )
{
    superspec_to_spec( bt_attrib->vdata );
    show_spec( get_superspec( bt_attrib->vdata ) );

    iconbutton_filename_change( bt_attrib->filename, 0 );
    focusiconbutton_filename_change( bt_attrib->focus_filename, 0 );

    redraw_the_form( 0 );
}


/***************************************
 ***************************************/

static void
show_spec( SuperSPEC * spec )
{
    info = spec->cspecv;
    
    fl_set_button( bt_attrib->react_left,   spec->mbuttons &  1 );
    fl_set_button( bt_attrib->react_middle, spec->mbuttons &  2 );
    fl_set_button( bt_attrib->react_right,  spec->mbuttons &  4 );
    fl_set_button( bt_attrib->react_up,     spec->mbuttons &  8 );
    fl_set_button( bt_attrib->react_down,   spec->mbuttons & 16);

    fl_set_button( bt_attrib->initialval, spec->int_val );
    fl_set_button( bt_attrib->use_data,   spec->use_data );
    fl_set_button( bt_attrib->fullpath,   spec->fullpath );

    fl_set_button( bt_attrib->showfocus, info->show_focus );
    fl_set_choice_text( bt_attrib->pixalign, align_name( info->align, 0 ) );

    fl_set_input( bt_attrib->filename, info->filename );
    fl_set_input( bt_attrib->focus_filename, info->focus_filename );
}


/***************************************
 ***************************************/

void
button_apply_attrib( FL_OBJECT * obj   FL_UNUSED_ARG,
                      long        data  FL_UNUSED_ARG )
{
    int d;

    obj = bt_attrib->vdata;

    d =   ( fl_get_button( bt_attrib->react_left   ) ?  1 : 0 )
        | ( fl_get_button( bt_attrib->react_middle ) ?  2 : 0 )
        | ( fl_get_button( bt_attrib->react_right  ) ?  4 : 0 )
        | ( fl_get_button( bt_attrib->react_up     ) ?  8 : 0 )
        | ( fl_get_button( bt_attrib->react_down   ) ? 16 : 0 );

    fl_set_button_mouse_buttons( obj, d );

    if ( obj->type == FL_PUSH_BUTTON || obj->type == FL_RADIO_BUTTON )
        fl_set_button( obj, fl_get_button( bt_attrib->initialval ) );

    spec_to_superspec( obj );
    redraw_the_form( 0 );
}



/***************************************
 ***************************************/

int
set_button_attrib( FL_OBJECT * ob )
{
    bt_attrib->vdata = edited = ob;

    button_spec = get_superspec( ob );

    info = button_spec->cspecv;

    if ( ! info )
    {
        M_err( "ButtonAttrib", "internal error" );
        return -1;
    }

    fl_show_object( bt_attrib->mbuttons_label );
    fl_show_object( bt_attrib->react_left );
    fl_show_object( bt_attrib->react_middle );
    fl_show_object( bt_attrib->react_right );
    fl_show_object( bt_attrib->react_up );
    fl_show_object( bt_attrib->react_down );

    if ( ob->type == FL_PUSH_BUTTON || ob->type == FL_RADIO_BUTTON )
        fl_show_object( bt_attrib->initialval );
    else
        fl_hide_object( bt_attrib->initialval );

    if ( IsIconButton( ob->objclass ) )
    {
        fl_show_object( bt_attrib->filename );
        fl_show_object( bt_attrib->browse );
        fl_show_object( bt_attrib->use_data );
        fl_show_object( bt_attrib->fullpath );
        ( ob->objclass == FL_PIXMAPBUTTON ?
          fl_show_object : fl_hide_object )( bt_attrib->focus_filename );
        ( ob->objclass == FL_PIXMAPBUTTON ?
          fl_show_object : fl_hide_object )( bt_attrib->browse2 );
    }
    else
    {
        fl_hide_object( bt_attrib->filename );
        fl_hide_object( bt_attrib->focus_filename );
        fl_hide_object( bt_attrib->browse );
        fl_hide_object( bt_attrib->browse2 );
        fl_hide_object( bt_attrib->use_data );
        fl_hide_object( bt_attrib->fullpath );
    }

    if ( ob->objclass == FL_PIXMAPBUTTON )
    {
        fl_show_object( bt_attrib->pixalign );
        fl_show_object( bt_attrib->showfocus );
    }
    else
    {
        fl_hide_object( bt_attrib->pixalign );
        fl_hide_object( bt_attrib->showfocus );
    }

    show_spec( button_spec );

    return 0;
}


/***************************************
 ***************************************/

static FL_OBJECT *
create_a_button( FL_OBJECT * ob )
{
    FL_OBJECT *defobj = NULL;

    /* create a default object */

    if ( ob->objclass == FL_BUTTON )
        defobj = fl_create_button( ob->type, 0, 0, 0, 0, "" );
    else if ( ob->objclass == FL_BITMAPBUTTON )
        defobj = fl_create_bitmapbutton( ob->type, 0, 0, 0, 0, "" );
    else if ( ob->objclass == FL_PIXMAPBUTTON )
        defobj = fl_create_pixmapbutton( ob->type, 0, 0, 0, 0, "" );
    else if ( ob->objclass == FL_ROUNDBUTTON )
        defobj = fl_create_roundbutton( ob->type, 0, 0, 0, 0, "" );
    else if ( ob->objclass == FL_LABELBUTTON )
        defobj = fl_create_labelbutton( ob->type, 0, 0, 0, 0, "" );
    else if ( ob->objclass == FL_LIGHTBUTTON )
        defobj = fl_create_lightbutton( ob->type, 0, 0, 0, 0, "" );
    else if ( ob->objclass == FL_CHECKBUTTON )
        defobj = fl_create_checkbutton( ob->type, 0, 0, 0, 0, "" );
    else if ( ob->objclass == FL_ROUND3DBUTTON )
        defobj = fl_create_round3dbutton( ob->type, 0, 0, 0, 0, "" );
    else
        fprintf( stderr, "Unknown Button Class: %d\n", ob->objclass );

    return defobj;
}


/***************************************
 ***************************************/

static const char *
file_tail( const char *full )
{
    char *p;

    if ( ( p = strrchr( full, '/' ) ) )
        return p + 1;

    return full;
}


/***************************************
 ***************************************/

void
emit_button_header( FILE      * fp,
                    FL_OBJECT * ob )
{
    SuperSPEC *spec;

    if ( ! IsIconButton( ob->objclass ) )
        return;

    spec = get_superspec( ob );
    info = spec->cspecv;

    if ( info->use_data && *info->data && *info->filename )
    {
        char *buf = fl_malloc( strlen( info->filename ) + 20 );

        if ( info->fullpath )
            sprintf( buf, "#include \"%s\"", info->filename );
        else
            sprintf( buf, "#include \"%s\"", file_tail( info->filename ) );

        if ( ! is_duplicate_info( ob, buf ) )
            fprintf( fp, "%s\n", buf );

        fl_free( buf );

        if ( *info->focus_filename )
        {
            buf = fl_malloc( strlen( info->focus_filename ) + 20 );

            if ( info->fullpath )
                sprintf( buf, "#include \"%s\"", info->focus_filename );
            else
                sprintf( buf, "#include \"%s\"",
                         file_tail( info->focus_filename ) );

            if ( ! is_duplicate_info( ob, buf ) )
                fprintf( fp, "%s\n", buf );

            fl_free( buf );
        }
    }
}


/***************************************
 ***************************************/

void
emit_button_code( FILE      * fp,
                  FL_OBJECT * ob )
{
    SuperSPEC *btspec,
              *defspec;
    FL_OBJECT *defobj;
    IconInfo *definfo;

    if ( ! ISBUTTON( ob->objclass ) || ! ( defobj = create_a_button( ob ) ) )
        return;

    defspec = get_superspec( defobj );
    btspec = get_superspec( ob );

    if ( btspec->mbuttons != defspec->mbuttons )
        fprintf( fp, "    fl_set_button_mouse_buttons( obj, %d );\n",
                 btspec->mbuttons );

    if ( btspec->int_val != defspec->int_val )
        fprintf( fp, "    fl_set_button( obj, %d );\n", btspec->int_val );

    if ( ! ( info = btspec->cspecv ) )
        return;

    definfo = defspec->cspecv;

    if ( *info->filename && ! info->use_data )
    {
        fprintf( fp, "    fl_set_%sbutton_file( obj, \"%s\" );\n",
                 ob->objclass == FL_PIXMAPBUTTON ? "pixmap" : "bitmap",
                 info->filename );
        if ( *info->focus_filename )
            fprintf( fp, "    fl_set_%sbutton_focus_file( obj, \"%s\" );\n",
                     ob->objclass == FL_PIXMAPBUTTON ? "pixmap" : "bitmap",
                     info->focus_filename );
    }

    if ( info->align != definfo->align && ob->objclass == FL_PIXMAPBUTTON )
    {
        fprintf( fp, "    fl_set_pixmapbutton_align( obj, %s, %d, %d );\n",
                 align_name( info->align | FL_ALIGN_INSIDE, 1 ),
                 info->dx, info->dy );
    }

    if ( *info->data && info->use_data && *info->filename )
    {
        if ( ob->objclass == FL_PIXMAPBUTTON )
            fprintf( fp, "    fl_set_pixmapbutton_data( obj, %s );\n",
                     info->data );
    else
        fprintf( fp, "    fl_set_bitmapbutton_data( obj, %s, %s, "
                 "( unsigned char * ) %s );\n",
                 info->width, info->height, info->data );
        if ( *info->focus_filename )
            fprintf( fp, "    fl_set_pixmapbutton_focus_data( obj, %s );\n",
                     info->focus_data );
    }

    if (    info->show_focus != definfo->show_focus
         && ob->objclass == FL_PIXMAPBUTTON )
        fprintf( fp, "    fl_set_pixmapbutton_focus_outline( obj, %d );\n",
                 info->show_focus );

    fl_free_object( defobj );
}


/***************************************
 ***************************************/

void
save_button_attrib( FILE      * fp,
                    FL_OBJECT * ob )
{
    FL_OBJECT *defobj;
    SuperSPEC *defspec,
              *btspec;
    IconInfo *definfo;

    if ( ! ISBUTTON( ob->objclass ) || ! ( defobj = create_a_button( ob ) ) )
        return;

    defspec = get_superspec( defobj );
    definfo = defspec->cspecv;
    btspec = get_superspec( ob );
    info = btspec->cspecv;

    if ( btspec->mbuttons != defspec->mbuttons )
        fprintf( fp, "    mbuttons: %d\n", btspec->mbuttons );

    if ( btspec->int_val != defspec->int_val )
        fprintf( fp, "    value: %d\n", btspec->int_val );

    if ( ! info || ! definfo )
    {
        M_err( "SaveButtonAttrib", "internal error" );
        return;
    }

    get_data_name( ob, info );

    if ( *info->filename )
    {
        fprintf( fp, "    file: %s\n", info->filename );
        if ( *info->focus_filename )
            fprintf( fp, "    focus_file: %s\n", info->focus_filename );
        fprintf( fp, "    fullpath: %d\n", info->fullpath );
    }

    if ( info->align != definfo->align )
        fprintf( fp, "    align: %s\n",
                 align_name( info->align | FL_ALIGN_INSIDE, 0 ) );

    if ( info->show_focus != definfo->show_focus )
        fprintf( fp, "    focus: %d\n", info->show_focus );

    if ( *info->data && *info->filename )
    {
        fprintf( fp, "    data: %s\n", info->data );
        if ( *info->focus_data )
            fprintf( fp, "    focus_data: %s\n", info->focus_data );
    }

    if ( *info->width )
        fprintf( fp, "    width: %s\n", info->width );
    if ( *info->height )
        fprintf( fp, "    height: %s\n", info->height );

    fl_free_object( defobj );
}


/***************************************
 ***************************************/

void
usedata_change( FL_OBJECT * ob,
                long        data  FL_UNUSED_ARG )
{
    info->use_data = fl_get_button( ob );
}


/***************************************
 ***************************************/

void
fullpath_cb( FL_OBJECT * ob,
             long        data  FL_UNUSED_ARG )
{
    info->fullpath = fl_get_button( ob );
}


/***************************************
 ***************************************/

void
react_to_button( FL_OBJECT * ob,
                 long        data )
{
    unsigned int mb;

    fl_get_button_mouse_buttons( edited, &mb );

    if ( fl_get_button( ob ) )
        mb |= 1 << data;
    else
        mb &= ~ ( 1 << data );

    fl_set_button_mouse_buttons( edited, mb );
}


/***************************************
 ***************************************/

void
initialval_change( FL_OBJECT * ob    FL_UNUSED_ARG,
                   long        data  FL_UNUSED_ARG )
{
    fl_set_button( edited, fl_get_button( bt_attrib->initialval ) );

    if ( auto_apply )
        redraw_the_form( 0 );
}


/***************************************
 ***************************************/

void
showfocus_change( FL_OBJECT * ob,
                  long        data  FL_UNUSED_ARG )
{
    info->show_focus = fl_get_button( ob );
    fl_set_pixmapbutton_focus_outline( edited, info->show_focus );
}


/***************************************
 ***************************************/

void
iconbutton_filename_change( FL_OBJECT * ob,
                            long        data  FL_UNUSED_ARG )
{
    if ( ! IsIconButton( edited->objclass ) )
        return;

    strcpy( info->filename, fl_get_input( ob ) );

    if ( *info->filename )
    {
        ( edited->objclass == FL_PIXMAPBUTTON ?
          fl_set_pixmapbutton_file : fl_set_bitmapbutton_file )
            ( edited, info->filename );
    }
    else
    {
        /* Show the default broken link image */

        if ( edited->objclass == FL_PIXMAPBUTTON )
            set_testing_pixmap( edited );
    }

    if ( auto_apply )
        redraw_the_form( 0 );
}


/***************************************
 ***************************************/

void
focusiconbutton_filename_change( FL_OBJECT * ob,
                                 long        data  FL_UNUSED_ARG )
{
    if ( ! IsIconButton( edited->objclass ) )
        return;

    strcpy( info->focus_filename, fl_get_input( ob ) );

    if ( *info->focus_filename )
    {
        ( edited->objclass == FL_PIXMAPBUTTON ?
          fl_set_pixmapbutton_focus_file : fl_set_bitmapbutton_file )
            ( edited, info->focus_filename );
    }
}


/***************************************
 ***************************************/

void
pixalign_change( FL_OBJECT * obj,
                 long        data  FL_UNUSED_ARG )
{
    const char *s = fl_get_choice_text( obj );

    if ( ! s )
        return;

    info->align = align_val( s );

    /* Don't allow outside align */

    fl_set_pixmap_align( edited, fl_to_inside_lalign( info->align ),
                         info->dx, info->dy );
    if ( auto_apply )
        redraw_the_form( 0 );
}


/***************************************
 ***************************************/

void
lookfor_pixmapfile_cb( FL_OBJECT * ob   FL_UNUSED_ARG,
                       long        data )
{
    const char *fn;
    const char * def = data
                       ? ( ( FL_BUTTON_STRUCT * ) edited->spec )->focus_filename
                       : ( ( FL_BUTTON_STRUCT * ) edited->spec )->filename;
    char buf[ 2048 ];
    char *cwd;

    fl_use_fselector( XPM_FSELECTOR );
    fl_set_fselector_placement( FL_PLACE_MOUSE );

    if ( edited->objclass == FL_PIXMAPBUTTON )
        fn = fl_show_fselector( "XPM file", "", "*.xpm", def );
    else
        fn = fl_show_fselector( "XBM file", "", "*.xbm", def );

    if ( ! fn )
        return;

    if ( strstr( fn, cwd = fli_getcwd( buf, sizeof buf - 2 ) ) )
        fn += strlen( cwd ) + 1;

    ob = data ? bt_attrib->focus_filename : bt_attrib->filename;

    fl_set_input( ob, fn );
    fl_call_object_callback( ob );
}


/***************************************
 * Read the specified xpm/xbm filename, and return the data name
 * and size
 ***************************************/

static void
get_xpm_stuff( char * inbuf,
               FILE * fp )
{
    char buf[ 256 ],
         *p;
    int done = 0;

    if ( ! fp )
    {
        M_warn( "GetXPMStuff", "fp==NULL" );
        return;
    }

    while ( fgets( buf, sizeof buf - 1, fp ) && ! done )
    {
        if ( ( p = strstr( buf, "static char" ) ) )
        {
            char *q = inbuf;

            *p += 11;
            while ( *p && *++p != '*' )
                /* empty */;
            while ( *p && *++p != '[' )
            {
                if ( ! isspace( ( unsigned char ) *p ) )
                    *q++ = *p;
            }
            *q = '\0';
        }
    }
}


/***************************************
 ***************************************/

static void
get_xbm_stuff( IconInfo * in,
               FILE     * fp  FL_UNUSED_ARG )
{
    char buf[ 2048 ],
         *p;

    strcpy( buf, in->filename );

    if ( ( p = strrchr(buf, '/' ) ) )
        strcpy( buf, ++p );

    if ( ( p = strrchr( buf, '.' ) ) )
        *p = '\0';

    strcat( strcpy( in->width,  buf ) , "_width" );
    strcat( strcpy( in->height, buf ), "_height" );
    strcat( strcpy( in->data,   buf ), "_bits"   );
}


/***************************************
 ***************************************/

static void
get_data_name( FL_OBJECT * ob,
               IconInfo  * inf )
{
    FILE *fp = NULL,
         *focus_fp = NULL;

    if ( ! IsIconButton( ob->objclass ) )
    {
        *inf->filename = '\0';
        *inf->focus_filename = '\0';
        *inf->width = '\0';
        *inf->height = '\0';
        return;
    }

    if ( ! inf->use_data )
    {
        *inf->data = *inf->width = *inf->height = '\0';
        return;
    }

    if ( *inf->filename && ! ( fp = fopen( inf->filename, "r" ) ) )
    {
        fprintf( stderr, "Can't open '%s'\n", inf->filename );

        /* wipe the icon file only if there isn't anything we can do */

        if ( ! inf->use_data || ! *inf->data )
            *inf->filename = '\0';
    }

    if (    *inf->focus_filename
         && ! ( focus_fp = fopen( inf->focus_filename, "r" ) ) )
    {
        fprintf( stderr, "Can't open focusfile '%s'\n", inf->focus_filename );
        if ( ! inf->use_data || ! *inf->focus_data )
            *inf->focus_filename = '\0';
    }

    if ( ob->objclass == FL_PIXMAPBUTTON )
    {
        get_xpm_stuff( inf->data, fp );
        get_xpm_stuff( inf->focus_data, focus_fp );
    }
    else
        get_xbm_stuff( inf, fp );

    if ( fp )
        fclose( fp );
    if ( focus_fp )
        fclose( focus_fp );
}

#include "spec/button_spec.c"


/*
 * Local variables:
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
