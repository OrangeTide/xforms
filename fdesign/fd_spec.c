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
 *  This file is part of XForms package
 *  Copyright (c) 1996-2002  T.C. Zhao and Mark Overmars
 *  All rights reserved.
 *
 * Driver for setting object specific info
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include/forms.h"
#include "fd_main.h"
#include "fd_spec.h"
#include <string.h>
#include <ctype.h>

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
 * Check if ob has spec info functions
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
 ***************************************/

static int 
ff_read_sp_bounds( FL_OBJECT * obj  FL_UNUSED_ARG,
				   SuperSPEC * sp )
{
	int r;

	if ( ( r = ff_read( "%f%f", &sp->min, &sp->max ) ) < 0 )
		return ff_err( "Can't read expected object bounds" );

	if ( r == 0 )
		return ff_err( "\"bounds\" key with no or invalid values" );
	else if ( r == 1 )
		return ff_err( "\"bounds\" key with only one value" );

	return 0;
}


/***************************************
 ***************************************/

static int 
ff_read_sp_precision( FL_OBJECT * obj  FL_UNUSED_ARG,
					  SuperSPEC * sp )
{
	int r;

	if ( ( r = ff_read( "%d", &sp->prec ) ) < 0 )
		return ff_err( "Can't read expected object precision" );

	if ( r == 0 )
		return ff_err( "\"precision\" key with no or invalid value" );

	return 0;
}


/***************************************
 ***************************************/

static int 
ff_read_sp_increment( FL_OBJECT * obj  FL_UNUSED_ARG,
					  SuperSPEC * sp )
{
	int r;

	if ( ( r = ff_read( "%f%f", &sp->ldelta, &sp->rdelta) ) < 0 )
		return ff_err( "Can't read expected object increment values" );

	if ( r == 0 )
		return ff_err( "\"increment\" key with no or invalid values" );
	else if ( r == 1 )
		return ff_err( "\"increment\" key with only one valid value" );

	return 0;
}


/***************************************
 ***************************************/

static int 
ff_read_sp_value( FL_OBJECT * obj,
				  SuperSPEC * sp )
{
	int r;

	if ( ISBUTTON( obj->objclass ) || ISCHOICE( obj->objclass ) )
	{
		if ( ( r = ff_read( "%d", &sp->int_val ) ) < 0 )
			return ff_err( "Can't read expected object value" );

		if ( ISBUTTON( obj->objclass ) )
			fl_set_button( obj, sp->int_val );
	}
	else if ( ( r = ff_read( "%f", &sp->val ) ) < 0 )
		return ff_err( "Can't read expected object value" );

	if ( r == 0 )
		return ff_err( "\"value\" key with no or invalid value" );

	return 0;
}


/***************************************
 ***************************************/

static int 
ff_read_sp_slsize( FL_OBJECT * ob  FL_UNUSED_ARG,
				   SuperSPEC * sp )
{
	int r;
	if ( ( r = ff_read( "%f", &sp->slsize ) ) < 0 )
		return ff_err( "Can't read expected object slider size" );

	if ( r == 0 )
		return ff_err( "\"slsize\" key with no or invalid value" );

	return 0;
}


/***************************************
 ***************************************/

static int 
ff_read_sp_step( FL_OBJECT * obj  FL_UNUSED_ARG,
				 SuperSPEC * sp )
{
	int r;

	if ( ( r = ff_read( "%f", &sp->step ) ) < 0 )
		return ff_err( "Can't read expected object step" );

	if ( r == 0 )
		return ff_err( "\"step\" key with no or invalid value" );

	return 0;
}


/***************************************
 ***************************************/

static int 
ff_read_sp_h_pref( FL_OBJECT * obj  FL_UNUSED_ARG,
				   SuperSPEC * sp )
{
	int r;
	char *p;
	int i;

	if ( ( r = ff_read( "%v", &p ) ) < 0 )
		return ff_err( "Can't read expected object h_pref" );

	if ( r == 0 )
		return ff_err( "\"h_pref\" key with no or invalid value" );

	i = get_scrollbar_pref_value( p );

	fl_safe_free( p );

	if ( i < 0 )
		return ff_err( "Invalid value for \"h_pref\" key" );

	sp->h_pref = i;

	return 0;
}


/***************************************
 ***************************************/

static int 
ff_read_sp_v_pref( FL_OBJECT * obj  FL_UNUSED_ARG,
				   SuperSPEC * sp )
{
	int r;
	char *p;
	int i;

	if ( ( r = ff_read( "%v", &p ) ) < 0 )
		return ff_err( "Can't read expected object v_pref" );

	if ( r == 0 )
		return ff_err( "\"v_pref\" key with no or invalid value" );

	i = get_scrollbar_pref_value( p );

	fl_safe_free( p );

	if ( i < 0 )
		return ff_err( "Invalid value for \"v_pref\" key" );

	sp->v_pref = i;

	return 0;
}


/***************************************
 ***************************************/

static int 
ff_read_sp_sstep( FL_OBJECT * obj  FL_UNUSED_ARG,
				  SuperSPEC * sp )
{
	int r;

	if ( ( r = ff_read( "%f", &sp->sstep ) ) < 0 )
		return ff_err( "Can't read expected object small step" );

	if ( r == 0 )
		return ff_err( "\"sstep\" key with no or invalid value" );

	return 0;
}


/***************************************
 ***************************************/

static int 
ff_read_sp_lstep( FL_OBJECT * obj  FL_UNUSED_ARG,
				  SuperSPEC * sp )
{
	int r;

	if ( ( r = ff_read( "%f", &sp->lstep ) ) < 0 )
		return ff_err( "Can't read expected object large step" );

	if ( r == 0 )
		return ff_err( "\"lstep\" key with no or invalid value" );

	return 0;
}


/***************************************
 ***************************************/

static int 
ff_read_sp_xbounds( FL_OBJECT * obj  FL_UNUSED_ARG,
					SuperSPEC * sp )
{
	int r;

	if ( ( r = ff_read( "%f%f", &sp->xmin, &sp->xmax ) ) < 0 )
		return ff_err( "Can't read expected object xbounds" );

	if ( r == 0 )
		return ff_err( "\"xbounds\" key with no or invalid values" );
	else if ( r == 1 )
		return ff_err( "\"xbounds\" key with only one valid value" );

	return 0;
}


/***************************************
 ***************************************/

static int 
ff_read_sp_ybounds( FL_OBJECT * obj  FL_UNUSED_ARG,
					SuperSPEC * sp )
{
	int r;

	if ( ( r = ff_read( "%f%f", &sp->ymin, &sp->ymax ) ) < 0 )
		return ff_err( "Can't read expected object ybounds" );

	if ( r == 0 )
		return ff_err( "\"ybounds\" key with no or invalid values" );
	else if ( r == 1 )
		return ff_err( "\"ybounds\" key with only one valid value" );

	return 0;
}


/***************************************
 ***************************************/

static int 
ff_read_sp_xvalue( FL_OBJECT * obj  FL_UNUSED_ARG,
				   SuperSPEC * sp )
{
	int r;

	if ( ( r = ff_read( "%f", &sp->xval ) ) < 0 )
		return ff_err( "Can't read expected object xvalue" );

	if ( r == 0 )
		return ff_err( "\"xvalue\" key with no or invalid value" );

	return 0;
}


/***************************************
 ***************************************/

static int 
ff_read_sp_yvalue( FL_OBJECT * obj  FL_UNUSED_ARG,
				   SuperSPEC * sp )
{
	int r;

	if ( ( r = ff_read( "%f", &sp->yval ) ) < 0 )
		return ff_err( "Can't read expected object yvalue" );

	if ( r == 0 )
		return ff_err( "\"yvalue\" key with no or invalid value" );

	return 0;
}


/***************************************
 ***************************************/

static int 
ff_read_sp_xstep( FL_OBJECT * obj  FL_UNUSED_ARG,
				  SuperSPEC * sp )
{
	int r;

	if ( ( r = ff_read( "%f", &sp->xstep ) ) < 0 )
		return ff_err( "Can't read expected object xstep" );

	if ( r == 0 )
		return ff_err( "\"xstep\" key with no or invalid value" );

	return 0;
}


/***************************************
 ***************************************/

static int 
ff_read_sp_ystep( FL_OBJECT * obj  FL_UNUSED_ARG,
				  SuperSPEC * sp )
{
	int r;

	if ( ( r = ff_read( "%f", &sp->ystep ) ) < 0 )
		return ff_err( "Can't read expected object ystep" );

	if ( r == 0 )
		return ff_err( "\"ystep\" key with no or invalid value" );

	return 0;
}


/***************************************
 ***************************************/

static int 
ff_read_sp_angles( FL_OBJECT * obj  FL_UNUSED_ARG,
				   SuperSPEC * sp )
{
	int r;

	if ( ( r = ff_read( "%f%f", &sp->thetai, &sp->thetaf ) ) < 0 )
		return ff_err( "Can't read expected object angles" );

	if ( r == 0 )
		return ff_err( "\"angles\" key with no or invalid values" );
	else if ( r == 1 )
		return ff_err( "\"angles\" key with only one valid value" );

	return 0;
}


/***************************************
 ***************************************/

static int 
ff_read_sp_mbuttons( FL_OBJECT * obj  FL_UNUSED_ARG,
					 SuperSPEC * sp )
{
	int r;

	if ( ( r = ff_read( "%d", &sp->mbuttons ) ) < 0 )
		return ff_err( "Can't read expected object mbuttons setting" );

	if ( r == 0 )
		return ff_err( "\"mbuttons\" key with no or invalid value" );

	return 0;
}


/***************************************
 ***************************************/

static int 
ff_read_sp_initial_val( FL_OBJECT * obj  FL_UNUSED_ARG,
						SuperSPEC * sp )
{
	int r;

	if ( ( r = ff_read( "%f", &sp->val ) ) < 0 )
		return ff_err( "Can't read expected object initial value" );

	if ( r == 0 )
		return ff_err( "\"initial_val\" key with no or invalid value" );

	return 0;
}


/***************************************
 ***************************************/

static int 
ff_read_sp_content( FL_OBJECT * obj  FL_UNUSED_ARG,
					SuperSPEC * sp )
{
	int r;
	char *p;

	if ( ( r = ff_read( "%S", &p ) ) < 0 )
		return ff_err( "Can't read expected object content" );

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

	sp->content[ sp->nlines ]  = p;
	sp->shortcut[ sp->nlines ] = NULL;
	sp->callback[ sp->nlines ] = NULL;
	sp->mode[ sp->nlines ]     = 0;
	sp->mval[ sp->nlines ]     = sp->nlines;

	return 0;
}


/***************************************
 ***************************************/

static int 
ff_read_sp_mode( FL_OBJECT * obj  FL_UNUSED_ARG,
				 SuperSPEC * sp )
{
	int r;
	char *p;
	int i;

	if ( ( r = ff_read( "%v", &p ) ) < 0 )
		return ff_err( "Can't read expected object mode" );

	if ( r == 0 )
		return ff_err( "\"mode\" key with no or invalid value" );

	i = get_pupmode_value( p );

	fl_safe_free( p );

	if ( i < 0 )
		return ff_err( "Invalid value for \"mode\" key" );

	sp->mode[ sp->nlines ] = i;

	return 0;
}


/***************************************
 ***************************************/

static int 
ff_read_sp_shortcut( FL_OBJECT * obj  FL_UNUSED_ARG,
					 SuperSPEC * sp )
{
	int r;
	char *p;

	if ( ( r = ff_read( "%s", &p) ) < 0 )
		return ff_err( "Can't read expected object shortcut" );

	sp->shortcut[ sp->nlines ] = p;

	return 0;
}


/***************************************
 ***************************************/

static int 
ff_read_sp_callback( FL_OBJECT * obj  FL_UNUSED_ARG,
					 SuperSPEC * sp )
{
	int r;
	char *p;

	if ( ( r = ff_read( "%v", &p) ) < 0 )
		return ff_err( "Can't read expected object callback" );

	sp->callback[ sp->nlines ] = p;

	return 0;
}


/***************************************
 ***************************************/

static int 
ff_read_sp_id( FL_OBJECT * obj  FL_UNUSED_ARG,
			   SuperSPEC * sp )
{
	int r;

	if ( ( r = ff_read( "%d", sp->mval + sp->nlines ) ) < 0 )
		return ff_err( "Can't read expected object id" );

	if ( r == 0 )
		return ff_err( "\"id\" key with no or invalid value" );

	return 0;
}


/***************************************
 ***************************************/

static int 
ff_read_sp_file( FL_OBJECT * obj  FL_UNUSED_ARG,
				 SuperSPEC * sp )
{
	int r;
	char *p;

	if ( ( r = ff_read( "%S", &p ) ) < 0 )
		return ff_err( "Can't read expected object \"file\" attribute" );

	if ( strlen( p ) >= sizeof sp->filename )
		return ff_err( "Filename for \"file\" key too long" );

	strcpy( sp->filename, p );
 
	fl_safe_free( p );

	return 0;
}


/***************************************
 ***************************************/

static int
ff_read_sp_focus_file( FL_OBJECT * obj  FL_UNUSED_ARG,
					   SuperSPEC * sp )
{
	int r;
	char *p;

	if ( ( r = ff_read( "%S", &p ) ) < 0 )
		return ff_err( "Can't read expected object \"focus\" attribute" );

	if ( strlen( p ) >= sizeof sp->focus_filename )
		return ff_err( "Filename for \"focus_file\" key too long" );

	strcpy( sp->focus_filename, p );
 
	fl_safe_free( p );

	return 0;
}


/***************************************
 ***************************************/

static int 
ff_read_sp_handler( FL_OBJECT * obj,
					SuperSPEC * sp  FL_UNUSED_ARG )
{
	int r;
	char *p;

	if ( ( r = ff_read( "%v", &p ) ) < 0 )
		return ff_err( "Can't read expected object handler" );

	if ( r == 0 )
		return ff_err( "\"handler\" key with no or invalid value" );

	obj->c_vdata = p;

	return 0;
}


/***************************************
 ***************************************/

static int 
ff_read_sp_data( FL_OBJECT * obj  FL_UNUSED_ARG,
				 SuperSPEC * sp )
{
	int r;
	char *p;

	if ( ( r = ff_read( "%v", &p ) ) < 0 )
		return ff_err( "Can't read expected object data attribute" );

	strcpy( sp->data, p );

	fl_safe_free( p );

	return 0;
}


/***************************************
 ***************************************/

static int 
ff_read_sp_focus_data( FL_OBJECT * obj  FL_UNUSED_ARG,
					   SuperSPEC * sp )
{
	int r;
	char *p;

	if ( ( r = ff_read( "%v", &p ) ) < 0 )
		return ff_err( "Can't read expected object focus_data attribute" );

	strcpy( sp->focus_data, p );

	fl_safe_free( p );

	return 0;
}


/***************************************
 ***************************************/

static int 
ff_read_sp_fullpath( FL_OBJECT * obj  FL_UNUSED_ARG,
					 SuperSPEC * sp )
{
	int r;

	if ( ( r = ff_read( "%d", &sp->fullpath) ) < 0 )
		return ff_err( "Can't read expected object fullpath attribute" );

	if ( r == 0 )
		return ff_err( "\"fullpath\" key with no or invalid value" );

	return 0;
}


/***************************************
 ***************************************/

static int 
ff_read_sp_width( FL_OBJECT * obj  FL_UNUSED_ARG,
				  SuperSPEC * sp )
{
	int r;
	char *p;

	if ( ( r = ff_read( "%v", &p ) ) < 0 )
		return ff_err( "Can't read expected object width attribute" );

	strcpy( sp->width, p );

	fl_safe_free( p );

	return 0;
}


/***************************************
 ***************************************/

static int 
ff_read_sp_height( FL_OBJECT * obj  FL_UNUSED_ARG,
				   SuperSPEC * sp )
{
	int r;
	char *p;

	if ( ( r = ff_read( "%v", &p ) ) < 0 )
		return ff_err( "Can't read expected object height attribute" );

	strcpy( sp->height, p );

	fl_safe_free( p );

	return 0;
}


/***************************************
 ***************************************/

static int 
ff_read_sp_helper( FL_OBJECT * obj  FL_UNUSED_ARG,
				   SuperSPEC * sp )
{
	int r;
	char *p;

	if ( ( r = ff_read( "%S", &p ) ) < 0 )
		return ff_err( "Can't read expected object helper attribute" );

	if ( strlen( p ) >= sizeof sp->helper )
		return ff_err( "Text for \"helper\" key too long" );

	strcpy( sp->helper, p );

	fl_safe_free( p );

	return 0;
}


/***************************************
 ***************************************/

static int 
ff_read_sp_align( FL_OBJECT * obj  FL_UNUSED_ARG,
				  SuperSPEC * sp )
{
	int r;

	if ( ( r = ff_read( "%a", &sp->align ) ) < 0 )
		return ff_err( "Can't read expected object align attribute" );

	if ( r == 0 )
		return ff_err( "\"align\" key with no or invalid value" );

	sp->align &= ~FL_ALIGN_INSIDE;

	return 0;
}


/***************************************
 ***************************************/

static int 
ff_read_sp_struct( FL_OBJECT * obj  FL_UNUSED_ARG,
				   SuperSPEC * sp )
{
	int r;

	if ( ( r = ff_read( "%d", &sp->new_menuapi ) ) < 0 )
		return ff_err( "Can't read expected object struct attribute" );

	if ( r == 0 )
		return ff_err( "\"struct\" key with no or invalid value" );

	return 0;
}


/***************************************
 ***************************************/

static int 
ff_read_sp_global( FL_OBJECT * obj  FL_UNUSED_ARG,
				   SuperSPEC * sp )
{
	int r;

	if ( ( r = ff_read( "%d", &sp->global_scope ) ) < 0 )
		return ff_err( "Can't read expected object global attribute" );

	if ( r == 0 )
		return ff_err( "\"global\" key with no or invalid value" );

	return 0;
}


/***************************************
 ***************************************/

static int 
ff_read_sp_focus( FL_OBJECT * obj  FL_UNUSED_ARG,
				  SuperSPEC * sp )
{
	int r;

	if ( ( r = ff_read( "%d", &sp->show_focus ) ) < 0 )
		return ff_err( "Can't read expected object focus attribute" );

	if ( r == 0 )
		return ff_err( "\"focus\" key with no or invalid value" );

	return 0;
}


/***************************************
 ***************************************/

static int 
ff_read_sp_xtics( FL_OBJECT * obj  FL_UNUSED_ARG,
				  SuperSPEC * sp )
{
	int r;

	if ( ( r = ff_read( "%d%d", &sp->xmajor, &sp->xminor ) ) < 0 )
		return ff_err( "Can't read expected object xtics values" );

	if ( r == 0 )
		return ff_err( "\"xtics\" key with no or invalid values" );
	else if ( r == 1 )
		return ff_err( "\"xtics\" key with only one valid value" );

	return 0;
}


/***************************************
 ***************************************/

static int 
ff_read_sp_ytics( FL_OBJECT * obj  FL_UNUSED_ARG,
				  SuperSPEC * sp )
{
	int r;

	if ( ( r = ff_read( "%d%d", &sp->ymajor, &sp->yminor ) ) < 0 )
		return ff_err( "Can't read expected object ytics values" );

	if ( r == 0 )
		return ff_err( "\"ytics\" key with no or invalid values" );
	else if ( r == 1 )
		return ff_err( "\"ytics\" key with only one valid value" );

	return 0;
}


/***************************************
 ***************************************/

static int 
ff_read_sp_xscale( FL_OBJECT * obj  FL_UNUSED_ARG,
				   SuperSPEC * sp )
{
	int r;
	char *p;
	int xscale;

	if ( ( r = ff_read( "%s%f", &p, &sp->xbase ) ) < 0 )
		return ff_err( "Can't read expected object xscale values" );

	if ( r == 0 )
		return ff_err( "\"yscale\" key with no or invalid values" );

	xscale = get_scale_value( p );

	fl_safe_free( p );

	if ( xscale < 0 )
		return ff_err( "Invalid value for first value for \"xscale\" key" );

	sp->xscale = xscale;

	if ( r == 1 )
		return ff_err( "\"xscale\" key with only one valid value" );

	return 0;
}


/***************************************
 ***************************************/

static int 
ff_read_sp_yscale( FL_OBJECT * obj  FL_UNUSED_ARG,
				   SuperSPEC * sp )
{
	int r;
	char *p;
	int yscale;

	if ( ( r = ff_read( "%s%f", &p, &sp->ybase ) ) < 0 )
		return ff_err( "Can't read expected object yscale values" );

	if ( r == 0 )
		return ff_err( "\"yscale\" key with no or invalid values" );

	yscale = get_scale_value( p );

	fl_safe_free( p );

	if ( yscale < 0 )
		return ff_err( "Invalid value for first value for \"yscale\" key" );

	sp->yscale = yscale;

	if ( r == 1 )
		return ff_err( "\"yscale\" key with only one valid value" );

	return 0;
}


/***************************************
 ***************************************/

static int 
ff_read_sp_grid( FL_OBJECT * obj  FL_UNUSED_ARG,
				 SuperSPEC * sp )
{
	int r;
	char *p1, *p2;
	int g;

	if ( ( r = ff_read( "%v%v", &p1, &p2) ) < 0 )
		return ff_err( "Can't read expected object grid attribute" );

	if ( r == 0 )
		return ff_err( "\"grid\" key with no or invalid values" );

	g = get_grid_value( p1 );

	fl_safe_free( p1 );

	if ( g < 0 )
		return ff_err( "Invalid first value for \"grid\" key" );

	sp->xgrid = g;

	if ( r != 2 )
		return ff_err( "\"grid\" key with only one valid value" );

	g = get_grid_value( p2 );

	fl_safe_free( p2 );

	if ( g < 0 )
		return ff_err( "Invalid second value for \"grid\" key" );

	sp->ygrid = g;

	return 0;
}


/***************************************
 ***************************************/

static int 
ff_read_sp_gridstyle( FL_OBJECT * obj  FL_UNUSED_ARG,
					  SuperSPEC * sp )
{
	int r;
	char *p;
	int g;

	if ( ( r = ff_read( "%v", &p ) ) < 0 )
		return ff_err( "Can't read expected object gridstyle attribute" );

	if ( r == 0 )
		return ff_err( "\"gridstyle\" key with no or invalid value" );

	g = get_linestyle_value( p );

	fl_safe_free( p );

	if ( g < 0 )
		return ff_err( "Invalid value for \"gridstyle\" key" );

	sp->grid_linestyle = g;

	return 0;
}


/***************************************
 ***************************************/

static int 
ff_read_sp_markactive( FL_OBJECT * obj  FL_UNUSED_ARG,
					   SuperSPEC * sp )
{
	int r;

	if ( ( r = ff_read( "%d", &sp->mark_active ) ) < 0 )
		return ff_err( "Can't read expected object markactive attribute" );

	if ( r == 0 )
		return ff_err( "\"markactive\" key with no or invalid value" );

	return 0;
}


/***************************************
 ***************************************/

static int 
ff_read_sp_dir( FL_OBJECT * obj  FL_UNUSED_ARG,
				SuperSPEC * sp )
{
	int r;
	char *p;
	int dir;

	if ( ( r = ff_read( "%v", &p ) ) < 0 )
		return ff_err( "Can't read expected object dir attribute" );

	if ( r == 0 )
		return ff_err( "\"dir\" key with no or invalid value" );

	dir = get_direction_value( p );

	fl_safe_free( p );

	if ( dir < 0 )
		return ff_err( "Invalid value for \"dir\" key" );

	sp->direction = dir;

	return 0;
}


/***************************************
 * Duplication of code in fd_file.c since in older versions
 * the return setting was considered an "object specific"
 * attribute
 ***************************************/

static int 
ff_read_sp_return( FL_OBJECT * obj,
				   SuperSPEC * sp )
{
	int r;
	char *return_name;
	int ret;

	if ( ( r = ff_read( "%s", &return_name ) ) < 0 )
		return ff_err( "Can't read expected object return" );

	if ( r == 0 )
		return ff_err( "\"return\" key with no or invalid value" );

	ret = get_how_return_val( return_name );

	fl_safe_free( return_name );

	if ( ret == -1 )
		return ff_err( "Invalid value for \"return\" key" );

	fl_set_object_return( obj, ret );
	sp->how_return = obj->how_return;

	return 0;
}


/***************************************
 ***************************************/

typedef int ( * spec_attr_func )( FL_OBJECT *, SuperSPEC * );

typedef struct {
	const char     * name;
	spec_attr_func   func;
} spec_attr_handlers;


static spec_attr_handlers attr_array[ ] =
{
	{ "bounds",      ff_read_sp_bounds      },
	{ "precision",	 ff_read_sp_precision   },
	{ "increment",	 ff_read_sp_increment   },
	{ "value",		 ff_read_sp_value       },
	{ "slsize",		 ff_read_sp_slsize      },
	{ "step",		 ff_read_sp_step        },
	{ "h_pref",		 ff_read_sp_h_pref      },
	{ "v_pref",		 ff_read_sp_v_pref      },
	{ "sstep",		 ff_read_sp_sstep       },
	{ "lstep",		 ff_read_sp_lstep       },
	{ "xbounds",	 ff_read_sp_xbounds     },
	{ "ybounds",	 ff_read_sp_ybounds     },
	{ "xvalue",		 ff_read_sp_xvalue      },
	{ "yvalue",		 ff_read_sp_yvalue      },
	{ "xstep",		 ff_read_sp_xstep       },
	{ "ystep",		 ff_read_sp_ystep       },
	{ "angles",		 ff_read_sp_angles      },
	{ "mbuttons",	 ff_read_sp_mbuttons    },
	{ "initial_val", ff_read_sp_initial_val },
	{ "content",	 ff_read_sp_content     },
	{ "mode",		 ff_read_sp_mode        },
	{ "shortcut",	 ff_read_sp_shortcut    },
	{ "callback",	 ff_read_sp_callback    },
	{ "id",			 ff_read_sp_id          },
	{ "file",		 ff_read_sp_file        },
	{ "focus_file",	 ff_read_sp_focus_file  },
	{ "handler",	 ff_read_sp_handler     },
	{ "data",		 ff_read_sp_data        },
	{ "focus_data",	 ff_read_sp_focus_data  },
	{ "fullpath",	 ff_read_sp_fullpath    },
	{ "width",		 ff_read_sp_width       },
	{ "height",		 ff_read_sp_height      },
	{ "helper",		 ff_read_sp_helper      },
	{ "align",		 ff_read_sp_align       },
	{ "struct",		 ff_read_sp_struct      },
	{ "global",		 ff_read_sp_global      },
	{ "focus",		 ff_read_sp_focus       },
	{ "xtics",		 ff_read_sp_xtics       },
	{ "ytics",		 ff_read_sp_ytics       },
	{ "xscale",		 ff_read_sp_xscale      },
	{ "yscale",		 ff_read_sp_yscale      },
	{ "grid",		 ff_read_sp_grid        },
	{ "gridstyle",	 ff_read_sp_gridstyle   },
	{ "markactive",	 ff_read_sp_markactive  },
	{ "dir",		 ff_read_sp_dir         },
	{ "return",      ff_read_sp_return      }
};


/***************************************
 * If we don't know what to do with object specific informations skip them.
 * This way we (hopefully) remain compatible with later versions
 ***************************************/

static int
skip_spec_info( char * key )
{
    char *rest,
  	     *p;
    int r;

	/* Skip everything up to either the start of the next form or object or
	   the end of the file */

	do
	{
		fl_safe_free( key );

		if ( ff_read( "%s", &rest ) < 0 )
			return ff_err( "Failed to read from file" );

		fl_safe_free( rest );

		if ( ( r = ff_read( "%k", &key ) ) < 0 )
			return ff_err( "Failed to read from file" );

	} while ( r != 0 && strcmp( key, "Name" ) && strcmp( key, "class" ) );

	/* Check if we arrived at the start of a form or class */

	if (  ! strcmp( key, "Name" ) )
	{
		fl_safe_free( key );
		return FF_AT_START_OF_FORM;
	}
	else if ( ! strcmp( key, "class" ) )
	{
		fl_safe_free( key );
		return FF_AT_START_OF_OBJECT;
	}

	/* Otherwise we now have to look for the name of the main function */

	if ( ff_read( "%v", &p ) < 1 )
		return ff_err( "Expected main function name, not found here" );
	else
	{
		fli_sstrcpy( main_name, p, MAX_VAR_LEN );
		fl_safe_free( p );
	}

	return FF_AT_END_OF_FILE;
}


/***************************************
 * Function for reading in object specific information
 ***************************************/

int
load_objclass_spec_info( FL_OBJECT * obj,
						 char *      key )
{
	int r;
	size_t i;
	char *p;
    SuperSPEC *sp = get_superspec( obj );

    if ( ! find_entry( obj ) )
        return skip_spec_info( key );

	/* Loop until no more object specific key is found. Note: when we
	   arrive here the first key already has been read in! */

	do
	{
		for ( i = 0; i < sizeof attr_array / sizeof *attr_array; i++ )
			if ( ! strcmp( key, attr_array[ i ].name ) )
			{
				fl_safe_free( key );

				if ( attr_array[ i ].func( obj, sp ) == FF_READ_FAILURE )
					return FF_READ_FAILURE;
				break;
			}

		/* An unexpected key has read, give up */

		if ( i >= sizeof attr_array / sizeof *attr_array )
		{
			char *tmp = fli_get_string( "Read invalid object specific key "
										"\"%s\"", key );

			fl_safe_free( key );
			ff_err( tmp );
			fl_safe_free( tmp );
			return FF_READ_FAILURE;
		}

		fl_safe_free( key );

		if ( ( r = ff_read( "%k", &key ) ) < 0 )
			return ff_err( "Failed to read from file" );

	} while ( r != 0 && strcmp( key, "Name" ) && strcmp( key, "class" ) );

	if ( *sp->data )
		sp->use_data = 1;

	superspec_to_spec( obj );

	/* Check if we should be at the last line of the file and then read
	   name of main function */

	if ( r == 0 )
	{
		if ( ff_read( "%v", &p ) < 1 )
			return ff_err( "Expected main function name, not found here" );
		else
		{
			fli_sstrcpy( main_name, p, MAX_VAR_LEN );
			fl_safe_free( p );
		}

		return FF_AT_END_OF_FILE;
	}

	/* Otherwise we should be at start of new object... */

	if ( ! strcmp( key, "class" ) )
	{
		fl_safe_free( key );
		return FF_AT_START_OF_OBJECT;
	}

	/* ...or start of new form */

	fl_safe_free( key );
	return FF_AT_START_OF_FORM;
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


/*  scrollbar preference settings */

#define VN( v )  { v, #v }

static FLI_VN_PAIR scrbpref[ ] =
{
    VN( FL_OFF  ),
	VN( FL_ON   ),
	VN( FL_AUTO ),
	{ -1, NULL }
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
    return fli_get_vn_value( scrbpref, s );
}

/* xyplot scale */

static FLI_VN_PAIR scale_vn[ ] =
{
    VN( FL_LINEAR ),
	VN( FL_LOG ),
	{ -1, NULL }
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
	{ -1, NULL }
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
	{ -1, NULL }
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
    { -1, NULL }
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
    if ( *buf != 'F' )
		strcat( strcpy( buf, "FL_" ), s );
    return fli_get_vn_value( pupmode, buf );
}
