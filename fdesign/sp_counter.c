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
 * \file sp_counter.c
 *
 *  This file is part of XForms package
 *  Copyright (c) 1996-2002  T.C. Zhao and Mark Overmars
 *  All rights reserved.
 *
 * Settting counter class specific attributes.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include/forms.h"
#include <float.h>
#include "fd_main.h"
#include "fd_spec.h"
#include "private/pcounter.h"
#include "spec/counter_spec.h"

extern FD_counterattrib *create_form_counterattrib( void );

static FD_counterattrib *cnt_attrib;
static SuperSPEC *counter_spec;
static void init_spec( SuperSPEC * );


/***************************************
 ***************************************/

void *
get_counter_spec_fdform( void )
{
    if ( ! cnt_attrib )
    {
        cnt_attrib = create_form_counterattrib( );

        setup_how_return_menu( cnt_attrib->returnsetting );
        fl_set_menu_item_mode( cnt_attrib->returnsetting, 5,
                               FL_PUP_BOX | FL_PUP_GRAY );
        fl_set_menu_item_mode( cnt_attrib->returnsetting, 6,
                               FL_PUP_BOX | FL_PUP_GRAY );
    }
    return cnt_attrib;
}


/***************************************
 ***************************************/

void
counter_spec_restore( FL_OBJECT * ob    FL_UNUSED_ARG,
                      long        data  FL_UNUSED_ARG )
{
    superspec_to_spec( cnt_attrib->vdata );
    init_spec( get_superspec( cnt_attrib->vdata ) );
    redraw_the_form( 0 );
}


/***************************************
 ***************************************/

static void
init_spec( SuperSPEC * spec )
{
    fl_set_counter_value( cnt_attrib->prec, spec->prec );

    set_finput_value( cnt_attrib->minval,     spec->min,   spec->prec );
    set_finput_value( cnt_attrib->maxval,     spec->max,   spec->prec );
    set_finput_value( cnt_attrib->initialval, spec->val,   spec->prec );
    set_finput_value( cnt_attrib->step1,      spec->sstep, spec->prec );
    set_finput_value( cnt_attrib->step2,      spec->lstep, spec->prec );

    reset_how_return_menu( cnt_attrib->returnsetting, spec->how_return );
}


/***************************************
 ***************************************/

int
set_counter_attrib( FL_OBJECT * ob )
{
    cnt_attrib->vdata = ob;
    counter_spec = get_superspec( ob );

    fl_set_counter_precision( cnt_attrib->prec, 0 );
    fl_set_counter_step( cnt_attrib->prec, 1, 2 );
    fl_set_counter_bounds( cnt_attrib->prec, 0, DBL_DIG );

    if ( ob->type == FL_SIMPLE_COUNTER )
        fl_hide_object( cnt_attrib->step2 );

    init_spec( counter_spec );

    return 0;
}


/***************************************
 ***************************************/

void
emit_counter_code( FILE      * fp,
                   FL_OBJECT * ob )
{
    FL_OBJECT *defobj;
    SuperSPEC *spec,
              *defspec;

    if ( ob->objclass != FL_COUNTER )
        return;

    /* create a default object */

    defobj = fl_create_counter( ob->type, 0, 0, 0, 0, "" );

    defspec = get_superspec( defobj );
    spec = get_superspec( ob );

    if ( spec->prec != defspec->prec )
        fprintf( fp, "    fl_set_counter_precision( obj, %d );\n",
                 spec->prec );

    if ( spec->min != defspec->min || spec->max != defspec->max )
        fprintf( fp, "    fl_set_counter_bounds( obj, %.*f, %.*f );\n",
                 spec->prec, spec->min, spec->prec, spec->max );

    if ( spec->val != defspec->val )
        fprintf( fp, "    fl_set_counter_value( obj, %.*f );\n",
                 spec->prec, spec->val );

    if ( spec->sstep != defspec->sstep || spec->lstep != defspec->lstep )
        fprintf( fp, "    fl_set_counter_step( obj, %.*f, %.*f );\n",
                 spec->prec, spec->sstep, spec->prec, spec->lstep );

    fl_free_object( defobj );
}


/***************************************
 ***************************************/

void
save_counter_attrib( FILE      * fp,
                     FL_OBJECT * ob )
{
    FL_OBJECT *defobj;
    SuperSPEC *defspec,
              *spec;

    if ( ob->objclass != FL_COUNTER )
        return;

    /* Create a default object */

    defobj = fl_create_counter( ob->type, 0, 0, 0, 0, "" );

    defspec = get_superspec( defobj );
    spec = get_superspec( ob );

    if (    spec->min != defspec->min
         || spec->max != defspec->max )
        fprintf( fp, "    bounds: %.*f %.*f\n",
                 spec->prec, spec->min, spec->prec, spec->max );

    if ( spec->prec != defspec->prec )
        fprintf( fp, "    precision: %d\n", spec->prec );

    if ( spec->val != defspec->val )
        fprintf( fp, "    value: %.*f\n", spec->prec, spec->val );

    if ( spec->sstep != defspec->sstep )
        fprintf( fp, "    sstep: %.*f\n", spec->prec, spec->sstep );

    if ( spec->lstep != defspec->lstep )
        fprintf( fp, "    lstep: %.*f\n", spec->prec, spec->lstep );

    fl_free_object( defobj );
}


/***************************************
 ***************************************/

void
cnt_precision_cb( FL_OBJECT * ob,
                  long        data  FL_UNUSED_ARG )
{
    double p = fl_get_counter_value( ob );

    fl_set_counter_precision( cnt_attrib->vdata, p );
    if ( auto_apply )
        redraw_the_form( 0 );
}


/***************************************
 ***************************************/

void
cnt_minmax_change( FL_OBJECT * ob    FL_UNUSED_ARG,
                   long        data  FL_UNUSED_ARG )
{
    double min = get_finput_value( cnt_attrib->minval );
    double max = get_finput_value( cnt_attrib->maxval );

    fl_set_counter_bounds( cnt_attrib->vdata, min, max );

    if ( auto_apply )
        redraw_the_form( 0 );
}


/***************************************
 ***************************************/

void
cnt_stepchange_cb( FL_OBJECT * ob    FL_UNUSED_ARG,
                   long        data  FL_UNUSED_ARG )
{
    float s1 = get_finput_value( cnt_attrib->step1 );
    float s2 = get_finput_value( cnt_attrib->step2 );

    fl_set_counter_step( cnt_attrib->vdata, s1, s2 );

    if ( auto_apply )
        redraw_the_form( 0 );
}


/***************************************
 ***************************************/

void
cnt_initialvalue_change( FL_OBJECT * ob    FL_UNUSED_ARG,
                         long        data  FL_UNUSED_ARG )
{
    double val = get_finput_value( cnt_attrib->initialval );

    fl_set_counter_value( cnt_attrib->vdata, val );

    if ( val != fl_get_counter_value( cnt_attrib->vdata ) )
    {
        counter_spec->val = fl_get_counter_value( cnt_attrib->vdata );
        set_finput_value( cnt_attrib->initialval, counter_spec->val,
                          counter_spec->prec );
    }
    if ( auto_apply )
        redraw_the_form( 0 );
}


/***************************************
 ***************************************/

void
cnt_returnsetting_change( FL_OBJECT * ob    FL_UNUSED_ARG,
                          long        data  FL_UNUSED_ARG )
{
    handle_how_return_changes( cnt_attrib->returnsetting,
                               cnt_attrib->vdata );
}


#include "spec/counter_spec.c"


/*
 * Local variables:
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
