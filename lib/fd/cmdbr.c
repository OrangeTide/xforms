#include "include/forms.h"
#include <stdlib.h>
#include "cmdbr.h"

FD_cmd * create_form_cmd( void )
{
	FL_OBJECT *obj;
	FD_cmd *fdui = fl_calloc( 1, sizeof *fdui );

	fdui->cmd = fl_bgn_form( FL_NO_BOX, 430, 190 );
	fdui->backface = obj = fl_add_box( FL_UP_BOX, 0, 0, 430, 190, "" );

	obj = fl_add_frame( FL_ENGRAVED_FRAME, 10, 152, 410, 31, "" );
    fl_set_object_gravity( obj, FL_SouthWest, FL_SouthEast );

	fdui->browser = obj = fl_add_browser( FL_NORMAL_BROWSER, 10, 8, 410, 140,
										  "" );
    fl_set_object_gravity( obj, FL_NorthWest, FL_SouthEast );

	fdui->close_br = obj = fl_add_button( FL_NORMAL_BUTTON, 299, 155, 60, 25,
										  "Close" );
    fl_set_object_gravity( obj, FL_SouthEast, FL_SouthEast );
    fl_set_object_callback( obj, fl_cmdbr_close_cb, 0 );

	fdui->clear_br = obj = fl_add_button( FL_NORMAL_BUTTON, 238, 155, 61, 25,
										  "Clear" );
    fl_set_object_gravity( obj, FL_SouthEast, FL_SouthEast );
    fl_set_object_callback( obj, fl_cmdbr_clear_cb, 0 );
	fl_end_form( );
	fdui->cmd->fdui = fdui;

	return fdui;
}
