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

#include "include/forms.h"
#include "fd_main.h"
#include "fd_spec.h"
#include "private/pspinner.h"
#include "spec/spinner_spec.h"

extern FD_spinnerattrib *create_form_spinnerattrib( void );

static FD_spinnerattrib *spn_attrib;
static SuperSPEC *spinner_spec;
static void init_spec( SuperSPEC * );


/***************************************
 ***************************************/

void *
get_spinner_spec_fdform( void )
{
    if ( ! spn_attrib )
    {
		spn_attrib = create_form_spinnerattrib( );

		set_up_how_return_menu( spn_attrib->returnsetting );
		fl_set_menu_item_mode( spn_attrib->returnsetting, 5,
							   FL_PUP_BOX | FL_PUP_GRAY );
		fl_set_menu_item_mode( spn_attrib->returnsetting, 6,
							   FL_PUP_BOX | FL_PUP_GRAY );
    }

    return spn_attrib;
}


/***************************************
 ***************************************/

void
spinner_spec_restore( FL_OBJECT * ob    FL_UNUSED_ARG,
					  long        data  FL_UNUSED_ARG )
{
    superspec_to_spec( spn_attrib->vdata );
    init_spec( get_superspec( spn_attrib->vdata ) );
    redraw_the_form( 0 );
}


/***************************************
 ***************************************/

static void
init_spec( SuperSPEC * spec )
{
    fl_set_counter_value( spn_attrib->prec, spec->prec );

    set_finput_value( spn_attrib->minval,     spec->dmin,  spec->prec );
    set_finput_value( spn_attrib->maxval,     spec->dmax,  spec->prec );
    set_finput_value( spn_attrib->initialval, spec->dval,  spec->prec );
    set_finput_value( spn_attrib->step,       spec->dstep, spec->prec );

	reset_how_return_menu( spn_attrib->returnsetting, spec->how_return );
}


/***************************************
 ***************************************/

int
set_spinner_attrib( FL_OBJECT * ob )
{
    spn_attrib->vdata = ob;
    spinner_spec = get_superspec( ob );

    if ( ob->type == FL_INT_SPINNER )
		fl_hide_object( spn_attrib->prec );
	else
	{
		fl_set_counter_step( spn_attrib->prec, 1, 2 );
		fl_set_counter_bounds( spn_attrib->prec, 0, 6 );
		fl_set_counter_precision( spn_attrib->prec, 0 );
		fl_show_object( spn_attrib->prec );
	}

    init_spec( spinner_spec );

    return 0;
}


/***************************************
 ***************************************/

void
emit_spinner_code( FILE      * fp,
				   FL_OBJECT * ob )
{
    FL_OBJECT *defobj;
    SuperSPEC *spec,
		      *defspec;

    if ( ob->objclass != FL_SPINNER )
		return;

    /* Create a default object */

    defobj = fl_create_spinner( ob->type, 0, 0, 0, 0, "" );

    defspec = get_superspec( defobj );
    spec = get_superspec( ob );

    if ( spec->prec != defspec->prec )
		fprintf( fp, "    fl_set_spinner_precision( obj, %d );\n",
				 spec->prec );

    if ( spec->dmin != defspec->dmin || spec->dmax != defspec->dmax )
		fprintf( fp, "    fl_set_spinner_bounds( obj, %.*f, %.*f );\n",
				 spec->prec, spec->dmin, spec->prec, spec->dmax );

    if ( spec->dval != defspec->dval )
		fprintf( fp, "    fl_set_spinner_value( obj, %.*f );\n",
				 spec->prec, spec->dval );

    if ( spec->dstep != defspec->dstep )
		fprintf( fp, "    fl_set_spinner_step( obj, %.*f );\n",
				 spec->prec, spec->dstep );

    fl_free_object( defobj );
}


/***************************************
 ***************************************/

void
save_spinner_attrib( FILE      * fp,
					 FL_OBJECT * ob )
{
    FL_OBJECT *defobj;
    SuperSPEC *defspec,
		      *spec;

    if ( ob->objclass != FL_SPINNER )
		return;

    /* Create a default object */

    defobj = fl_create_spinner( ob->type, 0, 0, 0, 0, "" );

    defspec = get_superspec( defobj );
    spec = get_superspec( ob );

    if (    spec->dmin != defspec->dmin
		 || spec->dmax != defspec->dmax )
		fprintf( fp, "    bounds: %.*f %.*f\n",
				 spec->prec, spec->dmin, spec->prec, spec->dmax );

    if ( spec->prec != defspec->prec )
		fprintf( fp, "    precision: %d\n", spec->prec );

    if ( spec->dval != defspec->dval )
		fprintf( fp, "    value: %.*f\n", spec->prec, spec->dval );

    if ( spec->dstep != defspec->dstep )
		fprintf( fp, "    step: %.*f\n", spec->prec, spec->dstep );

    fl_free_object( defobj );
}


/***************************************
 ***************************************/

void
spn_precision_cb( FL_OBJECT * ob,
				  long        data  FL_UNUSED_ARG )
{
    double p = fl_get_counter_value( ob );

    fl_set_spinner_precision( spn_attrib->vdata, p );
    if ( auto_apply )
		redraw_the_form( 0 );
}


/***************************************
 ***************************************/

void
spn_minmax_change( FL_OBJECT * ob    FL_UNUSED_ARG,
				   long        data  FL_UNUSED_ARG )
{
    double min = get_finput_value( spn_attrib->minval );
    double max = get_finput_value( spn_attrib->maxval );

    fl_set_spinner_bounds( spn_attrib->vdata, min, max );

    if ( auto_apply )
		redraw_the_form( 0 );
}


/***************************************
 ***************************************/

void
spn_stepchange_cb( FL_OBJECT * ob    FL_UNUSED_ARG,
				   long        data  FL_UNUSED_ARG )
{
    double s1 = get_finput_value( spn_attrib->step );

    fl_set_spinner_step( spn_attrib->vdata, s1 );

    if ( auto_apply )
		redraw_the_form( 0 );
}


/***************************************
 ***************************************/

void
spn_initialvalue_change( FL_OBJECT * ob    FL_UNUSED_ARG,
						 long        data  FL_UNUSED_ARG )
{
    double val = get_finput_value( spn_attrib->initialval );

    fl_set_spinner_value( spn_attrib->vdata, val );

    if ( val != fl_get_spinner_value( spn_attrib->vdata ) )
    {
		spinner_spec->dval = fl_get_spinner_value( spn_attrib->vdata );
		set_finput_value( spn_attrib->initialval, spinner_spec->dval,
						  spinner_spec->prec );
    }

    if ( auto_apply )
		redraw_the_form( 0 );
}


/***************************************
 ***************************************/

void
spn_returnsetting_change( FL_OBJECT * ob    FL_UNUSED_ARG,
						  long        data  FL_UNUSED_ARG )
{
	handle_how_return_changes( spn_attrib->returnsetting,
							   spn_attrib->vdata );
}


#include "spec/spinner_spec.c"
