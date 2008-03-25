/*
 *
 * This file is part of XForms.
 *
 * XForms is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1, or
 * (at your option) any later version.
 *
 * XForms is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with XForms; see the file COPYING.  If not, write to
 * the Free Software Foundation, 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 *
 */


/*
 * Test the accuracy of timer
 *
 *  This file is part of xforms package
 *  T.C. Zhao and M. Overmars  (1997)
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include/forms.h"
#include <stdlib.h>
#include <unistd.h>

/**** Forms and Objects ****/

typedef struct {
	FL_FORM   * form0;
	void      * vdata;
	long        ldata;
	FL_OBJECT * timer;
	FL_OBJECT * restart;
	FL_OBJECT * report;
} FD_form0;

static FD_form0 * create_form_form0( void );


static void
exit_cb( FL_OBJECT * ob    FL_UNUSED_ARG,
			  long        data  FL_UNUSED_ARG )
{
   fl_finish( );
   exit( 0 );
}


static long start_sec,
            start_usec;
static long end_sec,
            end_usec;


/* timer expired */

static void
timer_cb( FL_OBJECT * ob,
		  long        data  FL_UNUSED_ARG )
{
   char buf[ 128 ];
   float df;
   FD_form0 *fd = ob->form->fdui;
   double timerval =  fd->ldata * 0.001;

   fl_gettime( &end_sec, &end_usec );

   df = end_sec - start_sec + ( end_usec - start_usec ) * 1.0e-6;

   sprintf( buf,"Timeout: %.3f  Actual: %.3f  DeltaE: %.3f",
			timerval, df, df - timerval );

   fl_set_object_label( fd->report, buf );

}


static void
start_timer( FL_OBJECT * ob,
			 long        data  FL_UNUSED_ARG )
{
   FD_form0 *fd = ob->form->fdui;
   char buf[ 128 ];

   fd->ldata += 200;
   sprintf( buf, "Timer accuracy testing %.3f sec ...", fd->ldata * 0.001 );
   fl_set_object_label( fd->report, buf );
   fl_gettime( &start_sec, &start_usec );
   fl_set_timer( fd->timer, fd->ldata * 0.001 );
}


int
main( int    argc,
	  char * argv[ ] )
{
   FD_form0 *fd_form0;

   fl_initialize( &argc, argv, 0, 0, 0 );
   fd_form0 = create_form_form0( );

   /* fill-in form initialization code */

   fd_form0->ldata = 2800;
   start_timer( fd_form0->timer, 0 );

   /* show the first form */

   fl_show_form( fd_form0->form0, FL_PLACE_CENTER, FL_FULLBORDER,
				 "Timer object precision" );
   fl_do_forms( );
   return 0;
}


/* Form definition file generated with fdesign. */

static FD_form0 *
create_form_form0(void)
{
	FL_OBJECT *obj;
	FD_form0 *fdui = fl_calloc( 1, sizeof *fdui );

	fdui->form0 = fl_bgn_form( FL_NO_BOX, 320, 130 );

	obj = fl_add_box( FL_UP_BOX, 0, 0, 320, 130, "" );

	obj = fl_add_button( FL_NORMAL_BUTTON, 210, 80, 90, 35, "Done" );
	fl_set_object_callback( obj, exit_cb, 0 );

	fdui->restart = obj = fl_add_button( FL_TOUCH_BUTTON, 110, 80, 90, 35,
										 "Restart" );
    fl_set_object_callback( obj, start_timer, 0 );

	fdui->timer = obj = fl_add_timer( FL_HIDDEN_TIMER, 10, 40, 100, 40,
									  "Timer" );
    fl_set_object_callback( obj, timer_cb, 0 );

	fdui->report = obj = fl_add_text( FL_NORMAL_TEXT, 10, 20, 290, 50,"" );

	fl_end_form( );

	fdui->form0->fdui = fdui;

	return fdui;
}
