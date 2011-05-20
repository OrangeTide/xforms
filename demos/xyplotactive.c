/*
 *  This file is part of XForms.
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
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with XForms; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 59 Temple Place - Suite 330, Boston,
 *  MA 02111-1307, USA.
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include/forms.h"
#include <stdlib.h>
#include "time.h"

/**** Forms and Objects ****/

typedef struct {
    FL_FORM   * axypform;
    void      * vdata;
    long        ldata;
    FL_OBJECT * xyplot;
    FL_OBJECT * status;
} FD_axypform;

extern FD_axypform *create_form_axypform( void );

FD_axypform *xypui;

/* callbacks for form axypform */


/***************************************
 ***************************************/

void xyplot_cb( FL_OBJECT * ob,
                long        data  FL_UNUSED_ARG )
{
    float x, y;
    int i;
    char buf[ 64 ];

    fl_get_xyplot( ob, &x, &y, &i );
    if ( i < 0 )
       return;

    sprintf( buf, "X=%.3f  Y=%.3f", x, y );
    fl_set_object_label( xypui->status, buf );
}


/***************************************
 ***************************************/

void
alwaysreturn_cb( FL_OBJECT * ob,
                 long        data  FL_UNUSED_ARG )
{
    fl_set_object_return( xypui->xyplot,
                          fl_get_button( ob ) ?
                          FL_RETURN_CHANGED : FL_RETURN_END_CHANGED );
}


/***************************************
 ***************************************/

void
interpolate_cb( FL_OBJECT * ob,
                long        data  FL_UNUSED_ARG )
{
    fl_set_xyplot_interpolate( xypui->xyplot, 0, fl_get_button( ob ) ? 3 : 0,
                               0.2 );
}


/***************************************
 ***************************************/

void
inspect_cb( FL_OBJECT * ob,
            long        data  FL_UNUSED_ARG )
{
    fl_set_xyplot_inspect( xypui->xyplot, fl_get_button( ob ) );
}


/***************************************
 ***************************************/

void
notic_cb( FL_OBJECT * ob,
          long        data  FL_UNUSED_ARG )
{
    int notic = fl_get_button( ob );

    if ( notic )
    {
        fl_set_xyplot_xtics( xypui->xyplot, -1, -1 );
        fl_set_xyplot_ytics( xypui->xyplot, -1, -1 );
    }
    else
    {
        fl_set_xyplot_xtics( xypui->xyplot, 0, 0 );
        fl_set_xyplot_ytics( xypui->xyplot, 0, 0 );
    }

#if 0
	{
		fl_set_xyplot_ybounds( xypui->xyplot, 2.25, 7.7 );
		fl_set_xyplot_ytics( xypui->xyplot, 5, 5 );

		FL_COORD sllx, slly, surx, sury;
		double wllx, wlly, wurx, wury;

		fl_get_xyplot_screen_area( xypui->xyplot, &sllx, &slly, &surx, &sury );
		fl_get_xyplot_world_area( xypui->xyplot, &wllx, &wlly, &wurx, &wury );

		fprintf( stderr, "%d %d %d %d\n%lf %lf %lf %lf\n",
				 sllx, slly, surx, sury, wllx, wlly, wurx, wury );
	}
#endif
}


/***************************************
 ***************************************/

int
main( int    argc,
      char * argv[ ] )
{
    float x[ 11 ],
          y[ 11 ];
    int i;

    fl_initialize( &argc, argv, "FormDemo", 0, 0 );
    xypui = create_form_axypform( );

    /* Fill-in form initialization code */

    fl_set_xyplot_ybounds( xypui->xyplot, 0, 10 );

    for ( i  = 0; i <= 10; i++ )
        x[ i ] = y[ i ] = i;

    fl_add_xyplot_overlay( xypui->xyplot, 1, x, y, 11, FL_YELLOW );
    fl_set_xyplot_overlay_type( xypui->xyplot, 1, FL_LINEPOINTS_XYPLOT );
    fl_set_xyplot_interpolate( xypui->xyplot, 1, 2, 0.1 );

    fl_set_xyplot_key( xypui->xyplot, 0, "Experiment" );
    fl_set_xyplot_key( xypui->xyplot, 1, "Theory" );
    fl_set_xyplot_key_position( xypui->xyplot, 0.25, 9.75,
                                    FL_ALIGN_RIGHT_BOTTOM );

    srand( time( NULL ) );

    for ( i = 0; i <= 10; i++ )
        y[ i ] +=  ( double ) rand( ) / RAND_MAX - 0.5;

    fl_set_xyplot_data( xypui->xyplot, x, y, 11, "", "", "" );
    fl_set_xyplot_linewidth( xypui->xyplot, 0, 2 );
    fl_set_xyplot_xgrid( xypui->xyplot, FL_GRID_MINOR );

    /* Show the first form */

    fl_show_form( xypui->axypform, FL_PLACE_MOUSE | FL_FREE_SIZE,
                  FL_FULLBORDER, "axypform" );

    fl_do_forms( );

    return 0;
}


/***************************************
 ***************************************/

FD_axypform *
create_form_axypform( void )
{
    FL_OBJECT *obj;
    FD_axypform *fdui = fl_calloc( 1, sizeof *fdui );

    fdui->axypform = fl_bgn_form( FL_NO_BOX, 431, 301 );

    fl_add_box( FL_UP_BOX, 0, 0, 431, 301, "" );

    fdui->xyplot = obj = fl_add_xyplot( FL_ACTIVE_XYPLOT, 20, 50, 285, 235,
                                        "" );
    fl_set_object_boxtype( obj, FL_DOWN_BOX );
    fl_set_object_color( obj, FL_BLACK, FL_GREEN );
    fl_set_object_lalign( obj, FL_ALIGN_BOTTOM | FL_ALIGN_INSIDE );
    fl_set_object_callback( obj, xyplot_cb, 0 );
    fl_set_object_gravity( obj, FL_NorthWest, FL_SouthEast );

    obj = fl_add_checkbutton( FL_PUSH_BUTTON, 315, 40, 80, 25, "AlwaysReturn" );
    fl_set_object_color( obj, FL_COL1, FL_BLUE );
    fl_set_object_callback( obj, alwaysreturn_cb, 0 );
    fl_set_object_gravity( obj, FL_NorthEast, FL_NorthEast );

    obj = fl_add_checkbutton( FL_PUSH_BUTTON, 315, 65, 80, 25, "Interpolate" );
    fl_set_object_color( obj, FL_COL1, FL_BLUE );
    fl_set_object_callback( obj, interpolate_cb, 0 );
    fl_set_object_gravity( obj, FL_NorthEast, FL_NorthEast );

    obj = fl_add_checkbutton( FL_PUSH_BUTTON, 315, 90, 85, 25, "InspectOnly" );
    fl_set_object_color( obj, FL_COL1, FL_BLUE );
    fl_set_object_callback( obj, inspect_cb, 0 );
    fl_set_object_gravity( obj, FL_NorthEast, FL_NorthEast );

    obj = fl_add_checkbutton( FL_PUSH_BUTTON, 315, 120, 85, 25, "NoTics" );
    fl_set_object_color( obj, FL_COL1, FL_BLUE );
    fl_set_object_callback( obj, notic_cb, 0 );
    fl_set_object_gravity( obj, FL_NorthEast, FL_NorthEast );

    fdui->status = obj = fl_add_box( FL_BORDER_BOX, 20, 15, 285, 25, "" );
    fl_set_object_boxtype( obj, FL_DOWN_BOX );
    fl_set_object_gravity( obj, FL_NorthWest, FL_NorthEast );
    fl_set_object_lalign( obj, FL_ALIGN_CENTER | FL_ALIGN_INSIDE );

    obj = fl_add_button( FL_NORMAL_BUTTON, 325, 250, 90, 30, "Done" );
    fl_set_object_gravity( obj, FL_SouthEast, FL_SouthEast );

    fl_end_form( );

  return fdui;
}
