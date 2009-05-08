#include "include/forms.h"
#include <stdlib.h>
#include "ginput.h"
FD_form *create_form_form(void)
{
  FL_OBJECT *obj;
  FD_form *fdui = (FD_form *) fl_calloc(1, sizeof(*fdui));
  fdui->form = fl_bgn_form(FL_NO_BOX, 410, 120);
  obj = fl_add_box(FL_UP_BOX,0,0,410,120,"");
  obj = fl_add_frame(FL_ENGRAVED_FRAME,8,9,394,67,"");
  fdui->input = obj = fl_add_input(FL_NORMAL_INPUT,20,33,370,30,"");
    fl_set_object_lalign(obj,FL_ALIGN_LEFT_TOP);
  fdui->cancel = obj = fl_add_button(FL_NORMAL_BUTTON,30,85,80,26,"Cancel");
    fl_set_button_shortcut(obj,"^[",1);
  obj = fl_add_button(FL_NORMAL_BUTTON,300,85,80,26,"Clear");
    fl_set_object_callback(obj,clear_cb,0);
  fdui->ok = obj = fl_add_button(FL_RETURN_BUTTON,165,85,80,26,"OK");
  fl_end_form();
  fdui->form->fdui = fdui;
  return fdui;
}
