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
 * \file fd_spec.c
 *
 *.
 *  This file is part of XForms package
 *  Copyright (c) 1996-2002  T.C. Zhao and Mark Overmars
 *  All rights reserved.
 *.
 *
 * Driver for setting object specific info
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include/forms.h"
#include "fd_main.h"
#include "fd_spec.h"
#include <string.h>

#define MAXOBJCLASS 8

typedef struct
{
    int    objclass[ MAXOBJCLASS ];
    void * ( * get_fdform )( void );	                /* spec form     */
    int    ( * set_spec )( FL_OBJECT * );	            /* interaction   */
    void   ( * restore_spec )( FL_OBJECT *, long );	    /* how to resotre */
    void   ( * save_attrib )( FILE *, FL_OBJECT * );	/* write to .fd  */
    void   ( * emit_code )( FILE *, FL_OBJECT * );	    /* write fl code */
    void   ( * emit_header )( FILE *, FL_OBJECT * );	/* write fl code */
    void   ( * emit_global )( FILE *, FL_OBJECT * );	/* write fl code */
    void * vdata;
} ObjSPEC;

static ObjSPEC objspec[ ] =
{
    {
		{ FL_SLIDER, FL_VALSLIDER },
		get_slider_spec_fdform,
		set_slider_attrib,
		slider_spec_restore,
		save_slider_attrib,
		emit_slider_code,
		NULL,
		NULL,
		NULL
    },

    {
		{ FL_COUNTER },
		get_counter_spec_fdform,
		set_counter_attrib,
		counter_spec_restore,
		save_counter_attrib,
		emit_counter_code,
		NULL,
		NULL,
		NULL
    },

    {
		{ FL_DIAL },
		get_dial_spec_fdform,
		set_dial_attrib,
		dial_spec_restore,
		save_dial_attrib,
		emit_dial_code,
		NULL,
		NULL,
		NULL
    },

    {
		{ FL_POSITIONER },
		get_pos_spec_fdform,
		set_pos_attrib,
		pos_spec_restore,
		save_pos_attrib,
		emit_pos_code,
		NULL,
		NULL,
		NULL
    },

    {
		{ FL_THUMBWHEEL },
		get_twheel_spec_fdform,
		set_twheel_attrib,
		twheel_spec_restore,
		save_twheel_attrib,
		emit_twheel_code,
		NULL,
		NULL,
		NULL
    },

    {
		{ FL_PIXMAPBUTTON, FL_BITMAPBUTTON, FL_BUTTON, FL_CHECKBUTTON,
		  FL_ROUNDBUTTON, FL_LIGHTBUTTON, FL_ROUND3DBUTTON, FL_LABELBUTTON },
		get_button_spec_fdform,
		set_button_attrib,
		button_spec_restore,
		save_button_attrib,
		emit_button_code,
		emit_button_header,
		NULL,
		NULL
    },

    {
		{ FL_PIXMAP, FL_BITMAP },
		get_pixmap_spec_fdform,
		set_pixmap_attrib,
		pixmap_spec_restore,
		save_pixmap_attrib,
		emit_pixmap_code,
		emit_pixmap_header,
		NULL,
		NULL
    },

    {
		{ FL_SCROLLBAR },
		get_scrollbar_spec_fdform,
		set_scrollbar_attrib,
		scrollbar_spec_restore,
		save_scrollbar_attrib,
		emit_scrollbar_code,
		NULL,
		NULL,
		NULL
    },

    {
		{ FL_BROWSER },
		get_browser_spec_fdform,
		set_browser_attrib,
		browser_spec_restore,
		save_browser_attrib,
		emit_browser_code,
		NULL,
		NULL,
		NULL
    },

    {
		{ FL_CHOICE },
		get_choice_spec_fdform,
		set_choice_attrib,
		choice_spec_restore,
		save_choice_attrib,
		emit_choice_code,
		emit_menu_header,
		emit_menu_global,
		NULL
    },

    {
		{ FL_MENU },
		get_menu_spec_fdform,
		set_menu_attrib,
		menu_spec_restore,
		save_menu_attrib,
		emit_menu_code,
		emit_menu_header,
		emit_menu_global,
		NULL
    },

    {
		{ FL_XYPLOT },
		get_xyplot_spec_fdform,
		set_xyplot_attrib,
		xyplot_spec_restore,
		save_xyplot_attrib,
		emit_xyplot_code,
		NULL,
		NULL,
		NULL
    },

    {
		{ FL_FREE },
		get_freeobj_spec_fdform,
		set_freeobj_attrib,
		freeobj_spec_restore,
		save_freeobj_attrib,
		emit_freeobj_code,
		NULL,
		NULL,
		NULL
    },

    {
		{ -1 }, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL	 /* sentinel */
    }
};


/***************************************
 * check if ob has spec info functions
 ***************************************/

static ObjSPEC *
find_entry( FL_OBJECT * ob )
{
    ObjSPEC *attrib = objspec;
    int *cls;

    for ( ; ob && attrib->objclass[ 0 ] >= 0; attrib++ )
    {
		cls = attrib->objclass;
		for ( ; *cls > 0; cls++ )
			if ( *cls == ob->objclass )
				return attrib;
    }

    return 0;
}


/***************************************
 * Callback function for SPEC folder. Entry point from fd_attrib.c
 ***************************************/

void
set_objclass_spec_attributes( FL_OBJECT * ob,
							  long        data  FL_UNUSED_ARG )
{
    ObjSPEC *spec;
    FD_Any *fd;

    fl_freeze_form( fd_attrib->attrib );

    if ( ( spec = find_entry( ob ) ) )
    {
		spec->vdata = fd = spec->get_fdform( );

		if ( spec->set_spec( ob ) >= 0 )
		{
			if ( ( spec->set_spec( ob ) >= 0 ) )
				fl_set_object_callback( fd_attrib->restoreobj,
										spec->restore_spec, 0 );

			fl_replace_folder_bynumber( fd_attrib->attrib_folder, 2,
										fd->form );
		}
		else
			fl_replace_folder_bynumber( fd_attrib->attrib_folder, 2,
										fd_nullattrib->nullattrib );
    }
    else
		fl_replace_folder_bynumber( fd_attrib->attrib_folder, 2,
									fd_nullattrib->nullattrib );

    fl_unfreeze_form( fd_attrib->attrib );
}


/***************************************
 ***************************************/

void
cleanup_spec( FL_OBJECT * ob )
{
    ObjSPEC *attrib = find_entry( ob );

    if ( attrib && attrib->vdata )
		attrib->vdata = NULL;
}


/***************************************
 ***************************************/

void
save_objclass_spec_info( FILE      * fp,
						 FL_OBJECT * ob )
{
    ObjSPEC *attrib = find_entry( ob );

    if ( attrib && attrib->save_attrib )
		attrib->save_attrib( fp, ob );
	else
		M_warn( "save info", " class %d unknown", ob->objclass );
}


/***************************************
 * Don't know what to do, skip it. This way we remain
 * compatible with later versions
 ***************************************/

static void
skip_spec_info( FILE      * fp,
				FL_OBJECT * ob )
{
    char *key,
		 *val;
    int c,
		warned = 0;

    while (    ungetc( c = getc( fp ), fp ) != '-'
		    && c != '='
			&& c != EOF
		    && read_key_val( fp, &key, &val ) != EOF )
	{
		if ( strlen( fli_de_space( key ) ) > 2 && ! warned )
		{
			M_err( "SpecInfo", "Skipped object class %d(%s)",
				   ob->objclass, ob->label);
			warned = 1;
		}

		fl_free( key );
    }
}


/***************************************
 * Maybe should consider a hash for all the strcmps
 ***************************************/

#define Str( x ) #x
#define XStr( x ) Str( x )

#define TMP_BUF_LEN 128

void
load_objclass_spec_info( FILE      * fp,
						 FL_OBJECT * ob )
{
    char *key,
		 *val,
		 buf[ TMP_BUF_LEN + 1 ];
    int c;
    SuperSPEC *sp = get_superspec( ob );

    if ( find_entry( ob ) )
    {
		while (    ungetc( ( c = getc( fp ) ), fp ) != '-'
				&& c != '='
				&& c != EOF
				&& read_key_val( fp, &key, &val ) != EOF )
		{

			if ( strlen( fli_de_space( key ) ) < 2 )
				/* empty */ ;
			else if ( strcmp( key, "bounds" ) == 0 )
				sscanf( val, "%f %f", &sp->min, &sp->max );
			else if ( strcmp( key, "precision" ) == 0 )
				sscanf( val, "%d", &sp->prec );
			else if ( strcmp( key, "increment" ) == 0 )
				sscanf( val, "%f %f", &sp->ldelta, &sp->rdelta );
			else if ( strcmp( key, "value" ) == 0 )
			{
				if ( ISBUTTON( ob->objclass ) || ISCHOICE( ob->objclass ) )
				{
					sp->int_val = atoi( val );
					if ( ISBUTTON( ob->objclass ) )
						fl_set_button( ob, sp->int_val );
				}
				else
					sp->val = atof( val );
			}
			else if ( strcmp( key, "slsize" ) == 0 )
				sscanf( val, "%f", &sp->slsize );
			else if ( strcmp( key, "step" ) == 0 )
				sscanf( val, "%f", &sp->step );
			else if ( strcmp( key, "h_pref") == 0 )
				sp->h_pref = get_scrollbar_pref_value( val );
			else if ( strcmp( key, "v_pref" ) == 0 )
				sp->v_pref = get_scrollbar_pref_value( val );
			else if ( strcmp( key, "sstep" ) == 0 )
				sscanf( val, "%f", &sp->sstep );
			else if ( strcmp( key, "lstep" ) == 0 )
				sscanf( val, "%f", &sp->lstep );
			else if ( strcmp( key, "xbounds" ) == 0 )
				sscanf( val, "%f %f", &sp->xmin, &sp->xmax );
			else if ( strcmp( key, "ybounds" ) == 0 )
				sscanf( val, "%f %f", &sp->ymin, &sp->ymax );
			else if ( strcmp( key, "xvalue" ) == 0 )
				sscanf( val, "%f", &sp->xval );
			else if ( strcmp( key, "yvalue" ) == 0 )
				sscanf( val, "%f", &sp->yval );
			else if ( strcmp( key, "xstep" ) == 0 )
				sscanf( val, "%f", &sp->xstep );
			else if ( strcmp( key, "ystep" ) == 0 )
				sscanf( val, "%f", &sp->ystep );
			else if ( strcmp( key, "angles" ) == 0 )
				sscanf( val, "%f %f", &sp->thetai, &sp->thetaf );
			else if ( strcmp( key, "mbuttons" ) == 0 )
				sscanf( val, "%d", &sp->mbuttons );
			else if ( strcmp( key, "initial_val" ) == 0 )
				sscanf( val, "%f", &sp->val );
			else if ( strcmp( key, "content" ) == 0 )
			{
				++sp->nlines;
				sp->content  = fl_realloc( sp->content,
									( sp->nlines + 1 ) * sizeof *sp->content );
				sp->shortcut = fl_realloc( sp->shortcut,
								   ( sp->nlines + 1 ) * sizeof *sp->shortcut );
				sp->callback = fl_realloc( sp->callback,
								   ( sp->nlines + 1 ) * sizeof *sp->callback );
				sp->mode     = fl_realloc( sp->mode,
								   ( sp->nlines + 1 ) * sizeof *sp->mode );
				sp->mval     = fl_realloc( sp->mval,
								   ( sp->nlines + 1 ) * sizeof *sp->mval );

				sp->content[ sp->nlines ]  = fl_strdup( val );
				sp->shortcut[ sp->nlines ] = NULL;
				sp->callback[ sp->nlines ] = NULL;
				sp->mode[ sp->nlines ]     = 0;
				sp->mval[ sp->nlines ]     = sp->nlines;
			}
			else if ( strcmp( key, "mode" ) == 0 )
				sp->mode[ sp->nlines ] = get_pupmode_value( val );
			else if ( strcmp( key, "shortcut" ) == 0 )
				sp->shortcut[ sp->nlines ] = fl_strdup( val );
			else if ( strcmp( key, "callback" ) == 0 )
				sp->callback[ sp->nlines ] = fl_strdup( val );
			else if ( strcmp( key, "id" ) == 0 )
				sscanf( val, "%d", sp->mval + sp->nlines );
			else if ( strcmp( key, "file" ) == 0 )
				strcpy( sp->filename, val );
			else if ( strcmp( key, "focus_file" ) == 0 )
				strcpy( sp->focus_filename, val );
			else if ( strcmp( key, "handler" ) == 0 )
				ob->c_vdata = fl_strdup( val );
			else if ( strcmp( key, "data" ) == 0 )
				strcpy( sp->data, val );
			else if ( strcmp( key, "focus_data" ) == 0 )
				strcpy( sp->focus_data, val );
			else if ( strcmp( key, "fullpath" ) == 0 )
				sp->fullpath = atoi( val );
			else if ( strcmp( key, "width" ) == 0 )
				strcpy( sp->width, val );
			else if ( strcmp( key, "height" ) == 0 )
				strcpy( sp->height, val );
			else if ( strcmp( key, "helper" ) == 0 )
				strcpy( sp->helper, val );
			else if ( strcmp( key, "align" ) == 0 )
				sp->align = align_val( val ) & ~FL_ALIGN_INSIDE;
			else if ( strcmp( key, "struct" ) == 0 )
				sp->new_menuapi = atoi( val );
			else if ( strcmp( key, "global" ) == 0 )
				sp->global_scope = atoi( val );
			else if ( strcmp( key, "focus" ) == 0 )
				sp->show_focus = atoi( val );
			else if ( strcmp( key, "xtics" ) == 0 )
				sscanf( val, "%d %d", &sp->xmajor, &sp->xminor );
			else if ( strcmp( key, "ytics" ) == 0 )
				sscanf( val, "%d %d", &sp->ymajor, &sp->yminor );
			else if ( strcmp( key, "xscale" ) == 0 )
			{
				sscanf( val, "%" XStr( TMP_BUF_LEN ) "s %g", buf, &sp->xbase );
				sp->xscale = get_scale_value( buf );
			}
			else if ( strcmp( key, "yscale" ) == 0 )
			{
				sscanf( val, "%" XStr( TMP_BUF_LEN ) "s %g", buf, &sp->ybase );
				sp->yscale = get_scale_value( buf );
			}
			else if ( strcmp( key, "grid" ) == 0 )
			{
				char buf1[ TMP_BUF_LEN + 1 ];

				sscanf( val,
						"%" XStr( TMP_BUF_LEN ) "s %" XStr( TMP_BUF_LEN ) "s",
						buf, buf1 );
				sp->xgrid = get_grid_value( buf );
				sp->ygrid = get_grid_value( buf1 );
			}
			else if ( strcmp( key, "gridstyle" ) == 0 )
				sp->grid_linestyle = get_linestyle_value( val );
			else if ( strcmp( key, "markactive" ) == 0 )
				sp->mark_active = atoi( val );
			else if ( strcmp( key, "dir" ) == 0 )
				sp->direction = get_direction_value( val );
			else if ( strcmp( key, "return" ) == 0 )
			{
				fli_de_space_de( val );
				sp->how_return = get_how_return_value( val );
			}
			else
				M_warn( "load_objclass_spec_info",
						"Unknown key = %s val = %s\n", key, val );

			fl_free( key );
		}

		if ( sp->data[ 0 ] )
			sp->use_data = 1;

		superspec_to_spec( ob );
    }
    else
		skip_spec_info( fp, ob );
}


/***************************************
 * Emit (file scope) variable code
 ***************************************/

void
emit_objclass_spec_header( FILE      * fp,
						   FL_OBJECT * ob )
{
    ObjSPEC *attrib = find_entry( ob );

    if ( attrib && attrib->emit_header )
		attrib->emit_header( fp, ob );
}


/***************************************
 * Emit global variable declarations
 ***************************************/

void
emit_objclass_spec_global( FILE      * fp,
						   FL_OBJECT * ob )
{
    ObjSPEC *attrib = find_entry( ob );

    if ( attrib && attrib->emit_global )
		attrib->emit_global( fp, ob );
}


/***************************************
 ***************************************/

void
emit_objclass_spec_info( FILE      * fp,
						 FL_OBJECT * ob )
{
    ObjSPEC *attrib = find_entry( ob );

    if ( attrib && attrib->emit_code )
		attrib->emit_code( fp, ob );
	else
		M_warn( "emit code", " class %d unknown", ob->objclass );
}


/***************************************
 ***************************************/

int
has_class_spec_info( FL_OBJECT * ob )
{
    return find_entry( ob ) != NULL;
}



/* Utilities */

/***************************************
 ***************************************/

void
set_finput_value( FL_OBJECT * ob,
				  double      f,
				  int         prec)
{
    char buf[ 32 ];

    if ( prec >= 0 )
		sprintf( buf, "%.*f", prec, f );
    else
		sprintf( buf, "%g", f );

    fl_set_input( ob, buf );
}


/***************************************
 ***************************************/

double
get_finput_value( FL_OBJECT * ob,
				  int         prec  FL_UNUSED_ARG )
{
    double f = 0;
    const char *s = fl_get_input( ob );

    if ( s && *s )
		sscanf( s, "%lf", &f );

    return f;
}


/*
 * Object Return settings
 */

#define VN( v )  { v, #v }

static FLI_VN_PAIR howreturn[ ] =
{
    VN( FL_RETURN_NONE        ),
    VN( FL_RETURN_END_CHANGED ),
    VN( FL_RETURN_CHANGED     ),
    VN( FL_RETURN_END         ),
    VN( FL_RETURN_SELECTION   ),
    VN( FL_RETURN_DESELECTION ),
    VN( FL_RETURN_ALWAYS      ),
    VN( -1 ),
};


/* this is used in the spec popups */

static FLI_VN_PAIR howreturn_str[ ] =
{
    { FL_RETURN_NONE,        "Never"            },
    { FL_RETURN_END_CHANGED, "End & Changed"    },
    { FL_RETURN_CHANGED,     "Whenever Changed" },
    { FL_RETURN_END,         "At End"           },
    { FL_RETURN_SELECTION,   "On Selection"     },
    { FL_RETURN_DESELECTION, "On Deselection"   },
    { FL_RETURN_ALWAYS,      "Always"           },
    { -1,                    NULL }
};


/***************************************
 ***************************************/

const char *
get_how_return_str( void )
{
    static char buf[ 256 ];
    int i = 0;

    for ( i = 1, strcpy( buf, howreturn_str[ 0 ].name );
		  howreturn_str[ i ].val >= 0; i++ )
		strcat( strcat( buf, "|" ), howreturn_str[ i ].name );
    return buf;
}


/***************************************
 ***************************************/

const char *
get_how_return_str_name( int a )
{
    return fli_get_vn_name( howreturn_str, a );
}


/***************************************
 ***************************************/

int
get_how_return_str_value( const char *s )
{
    return fli_get_vn_value( howreturn_str, s );
}


/* this is for output */

/***************************************
 ***************************************/

const char *
get_how_return_name( int a )
{
    return fli_get_vn_name( howreturn, a );
}


/***************************************
 ***************************************/

int
get_how_return_value( const char *s )
{
    return fli_get_vn_value( howreturn, s );
}


/*  scrollbar preference settings */

static FLI_VN_PAIR scrbpref[ ] =
{
    VN( FL_OFF  ),
	VN( FL_ON   ),
	VN( FL_AUTO ),
	VN( -1 )
};


/* must be in the same order as the above */


/***************************************
 ***************************************/

const char *
get_scrollbar_pref_string( void )
{
    return "Off|On|Auto ";
}


/***************************************
 ***************************************/

const char *
get_scrollbar_pref_name( int a )
{
    return fli_get_vn_name( scrbpref, a );
}


/***************************************
 ***************************************/

int
get_scrollbar_pref_value( const char * s )
{
    return fli_get_vn_value(scrbpref, s);
}

/* xyplot scale */

static FLI_VN_PAIR scale_vn[ ] =
{
    VN( FL_LINEAR ),
	VN( FL_LOG ),
	VN( -1 )
};


/***************************************
 ***************************************/

const char *
get_scale_name( int a )
{
    return fli_get_vn_name( scale_vn, a );
}


/***************************************
 ***************************************/

int
get_scale_value( const char * s )
{
    return fli_get_vn_value( scale_vn, s );
}


/***************************************
 ***************************************/

const char *
get_scale_string( void )
{
    return "Linear|Log";
}


/* xyplot grid */

static FLI_VN_PAIR grid_vn[ ] =
{
    VN( FL_GRID_NONE ),
	VN( FL_GRID_MAJOR ),
	VN( FL_GRID_MINOR ),
	VN( -1 )
};


/***************************************
 ***************************************/

const char *
get_grid_name( int a )
{
    return fli_get_vn_name( grid_vn, a );
}


/***************************************
 ***************************************/

int
get_grid_value( const char * s )
{
    return fli_get_vn_value( grid_vn, s );
}


/***************************************
 ***************************************/

const char *
get_grid_string( void )
{
    return "None|Major|Minor";
}


/* line style string stuff */

static FLI_VN_PAIR linestyle[ ] =
{
    VN( FL_SOLID ),
	VN( FL_USERDASH ),
	VN( FL_USERDOUBLEDASH ),
    VN( FL_DOT ),
	VN( FL_DOTDASH ),
	VN( FL_DASH ),
	VN( FL_LONGDASH ),
	VN( -1 )
};


/***************************************
 ***************************************/

const char *
get_linestyle_string( void )
{
    return "Solid|UserDash|DoubleDash|Dot|DotDash|Dash|LongDash";
}


/***************************************
 ***************************************/

const char *
get_linestyle_name( int a )
{
    return fli_get_vn_name( linestyle, a );
}


/***************************************
 ***************************************/

int
get_linestyle_value( const char * s )
{
    return fli_get_vn_value( linestyle, s );
}


/* popup mode */

static FLI_VN_PAIR pupmode[ ] =
{
    VN( FL_PUP_NONE ),
    VN( FL_PUP_GRAY ),
    VN( FL_PUP_BOX ),
    VN( FL_PUP_CHECK ),
    VN( FL_PUP_RADIO ),
    VN( -1 )
};


/***************************************
 ***************************************/

const char *
get_pupmode_string( void )
{
    return "PUP_NONE|PUP_GRAY|PUP_BOX|PUP_CHECK|PUP_RADIO";
}


/***************************************
 ***************************************/

const char *
get_pupmode_name( int a )
{
    return fli_get_vn_name( pupmode, a );
}


/***************************************
 ***************************************/

int
get_pupmode_value( const char * s )
{
    char buf[ 32 ];

    strcpy( buf, s );
    if ( buf[ 0 ] != 'F' )
		strcat( strcpy( buf, "FL_" ), s );
    return fli_get_vn_value( pupmode, buf );
}
