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
 * \file sp_twheel.c
 *
 *  This file is part of XForms package
 *  Copyright (c) 1996-2002  T.C. Zhao and Mark Overmars
 *  All rights reserved.
 *
 * Settting twheel class specific attributes.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include/forms.h"
#include "fd_main.h"
#include "fd_spec.h"
#include "spec/twheel_spec.h"

extern FD_twheelattrib *create_form_twheelattrib( void );

static FD_twheelattrib *twheel_attrib;
static SuperSPEC *twheel_spec;
static void show_spec( SuperSPEC * );


/***************************************
 ***************************************/

void *
get_twheel_spec_fdform( void )
{
    if ( ! twheel_attrib )
    {
		twheel_attrib = create_form_twheelattrib( );

		set_up_how_return_menu( twheel_attrib->returnsetting );
		fl_set_menu_item_mode( twheel_attrib->returnsetting, 5,
							   FL_PUP_BOX | FL_PUP_GRAY );
		fl_set_menu_item_mode( twheel_attrib->returnsetting, 6,
							   FL_PUP_BOX | FL_PUP_GRAY );
    }
    return twheel_attrib;
}


/***************************************
 ***************************************/

void
twheel_spec_restore( FL_OBJECT * ob    FL_UNUSED_ARG,
					 long        data  FL_UNUSED_ARG )
{
    FL_OBJECT *edit_obj = twheel_attrib->vdata;

    superspec_to_spec( edit_obj );
    show_spec( get_superspec( edit_obj ) );
    redraw_the_form( 0 );
}


/***************************************
 ***************************************/

static void
show_spec( SuperSPEC * spec )
{
    set_finput_value( twheel_attrib->minval, spec->min, spec->prec );
    set_finput_value( twheel_attrib->maxval, spec->max, spec->prec );
    set_finput_value( twheel_attrib->initial_val, spec->val, spec->prec );
    set_finput_value( twheel_attrib->step, spec->step, 3 );

    fl_set_counter_value( twheel_attrib->prec, spec->prec );
    /* fl_call_object_callback( twheel_attrib->prec ); */

	reset_how_return_menu( twheel_attrib->returnsetting, spec->how_return );
}


/***************************************
 ***************************************/

int
set_twheel_attrib( FL_OBJECT * ob )
{
    twheel_attrib->vdata = ob;
    twheel_spec = get_superspec( ob );

    fl_freeze_form( twheel_attrib->twheelattrib );

    fl_set_counter_step( twheel_attrib->prec, 1, 2 );
    fl_set_counter_precision( twheel_attrib->prec, 0 );
    fl_set_counter_bounds( twheel_attrib->prec, 0, 6 );
    fl_hide_object( twheel_attrib->prec );

    show_spec( twheel_spec );

    fl_unfreeze_form( twheel_attrib->twheelattrib );

    return 0;
}


/***************************************
 ***************************************/

void
emit_twheel_code( FILE      * fp,
				  FL_OBJECT * ob )
{
    FL_OBJECT *defobj;
    SuperSPEC *sp,
		      *defsp;

    if ( ob->objclass != FL_THUMBWHEEL )
		return;

    /* create a default object */

    defobj = fl_create_thumbwheel( ob->type, 0, 0, 0, 0, "" );

    defsp = get_superspec( defobj );
    sp = get_superspec( ob );

    if ( sp->prec != defsp->prec )
		fprintf( fp, "    fl_set_thumbwheel_precision( obj, %d );\n",
				 sp->prec );

    if ( sp->min != defsp->min || sp->max != defsp->max )
		fprintf( fp, "    fl_set_thumbwheel_bounds( obj, %g, %g );\n",
				 sp->min, sp->max );

    if ( sp->val != defsp->val )
		fprintf( fp, "    fl_set_thumbwheel_value( obj, %g );\n", sp->val );

    if ( sp->step != defsp->step )
		fprintf( fp, "    fl_set_thumbwheel_step( obj, %g );\n", sp->step );
}


/***************************************
 ***************************************/

void
save_twheel_attrib( FILE      * fp,
					FL_OBJECT * ob )
{
    FL_OBJECT *defobj;
    SuperSPEC *defsp,
		      *sp;

    if ( ob->objclass != FL_THUMBWHEEL )
		return;

    /* create a default object */

    defobj = fl_create_thumbwheel( ob->type, 0, 0, 0, 0, "" );

    defsp = get_superspec( defobj );
    sp = get_superspec( ob );

    if ( sp->min != defsp->min || sp->max != defsp->max )
		fprintf( fp, "bounds: %g %g\n", sp->min, sp->max );

    if ( sp->prec != defsp->prec )
		fprintf( fp, "precision: %d\n", sp->prec );

    if ( sp->val != defsp->val )
		fprintf( fp, "value: %g\n", sp->val );

    if ( sp->ldelta != defsp->ldelta || sp->rdelta != defsp->rdelta )
		fprintf( fp, "increment: %g %g\n", sp->ldelta, sp->rdelta );

    if ( sp->slsize != defsp->slsize )
		fprintf( fp, "slsize: %.2f\n", sp->slsize );

    if ( sp->step != defsp->step )
		fprintf( fp, "step: %g\n", sp->step );
}


/***************************************
 ***************************************/

void
twheel_minmax_change( FL_OBJECT * ob    FL_UNUSED_ARG,
					  long        data  FL_UNUSED_ARG )
{
    double min = get_finput_value( twheel_attrib->minval, 3 );
    double max = get_finput_value( twheel_attrib->maxval, 3 );

    fl_set_thumbwheel_bounds( twheel_attrib->vdata, min, max );
    if ( auto_apply )
		redraw_the_form( 0 );
}


/***************************************
 ***************************************/

void
twheel_step_change( FL_OBJECT * ob    FL_UNUSED_ARG,
					long        data  FL_UNUSED_ARG )
{
    double step = get_finput_value( twheel_attrib->step, 5 );

    fl_set_thumbwheel_step( twheel_attrib->vdata, step );
    if ( auto_apply )
		redraw_the_form( 0 );
}


/***************************************
 ***************************************/

void
twheel_initialvalue_change( FL_OBJECT * ob    FL_UNUSED_ARG,
							long        data  FL_UNUSED_ARG )
{
    double val = get_finput_value( twheel_attrib->initial_val, 2 );

    fl_set_thumbwheel_value( twheel_attrib->vdata, val );
    twheel_spec->val = fl_get_thumbwheel_value( twheel_attrib->vdata );
    set_finput_value( twheel_attrib->initial_val, twheel_spec->val,
					  twheel_spec->prec );
    if ( auto_apply )
		redraw_the_form( 0 );
}


/***************************************
 ***************************************/

void
twheel_returnsetting_change( FL_OBJECT * ob    FL_UNUSED_ARG,
							 long        data  FL_UNUSED_ARG )
{
	handle_how_return_changes( twheel_attrib->returnsetting,
							   twheel_attrib->vdata );
}


/***************************************
 ***************************************/

void
twheel_adjust_precision( FL_OBJECT * ob    FL_UNUSED_ARG,
						 long        data  FL_UNUSED_ARG )
{
}


#include "spec/twheel_spec.c"
