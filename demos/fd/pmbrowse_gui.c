/* Form definition file generated with fdesign. */

#include "forms.h"
#include <stdlib.h>
#include "pmbrowse_gui.h"

FD_ttt *create_form_ttt(void)
{
  FL_OBJECT *obj;
  FD_ttt *fdui = (FD_ttt *) fl_calloc(1, sizeof(*fdui));

  fdui->ttt = fl_bgn_form(FL_NO_BOX, 330, 320);
  obj = fl_add_box(FL_UP_BOX,0,0,330,320,"");
  fdui->bm = obj = fl_add_bitmap(FL_NORMAL_BITMAP,30,20,270,240,"");
    fl_set_object_boxtype(obj,FL_FLAT_BOX);
  fdui->pm = obj = fl_add_pixmap(FL_NORMAL_PIXMAP,30,20,270,240,"");
    fl_set_object_boxtype(obj,FL_FLAT_BOX);
  fdui->done = obj = fl_add_button(FL_NORMAL_BUTTON,220,280,90,30,"Done");
    fl_set_object_callback(obj,done,0);
  fdui->load = obj = fl_add_button(FL_NORMAL_BUTTON,20,280,90,30,"Load");
    fl_set_button_shortcut(obj,"L",1);
    fl_set_object_callback(obj,reload,0);
  fl_end_form();

  fdui->ttt->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

